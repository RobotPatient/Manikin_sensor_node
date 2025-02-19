cmake_minimum_required(VERSION 3.14)
include(FetchContent)
# stm32-cmake is a repository offering cmake support, well how it should be.
# With every HAL and LL component nicely seperated into cmake targets
FetchContent_Declare(
    stm32-cmake
    GIT_REPOSITORY https://github.com/ObKo/stm32-cmake.git
    GIT_TAG v2.1.0
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/board_sdk/stm32-cmake
)

# Download
FetchContent_MakeAvailable(stm32-cmake)
# Set Toolchain-file so that CMake knows what toolchain to use, with which linker & compilation options
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/board_sdk/stm32-cmake/cmake/stm32_gcc.cmake)

project(STM32F4_Sensor_Node C CXX ASM)

# Project Settings:
#
#
# C++ Standard 17, as it offers some nice template programming features :)
# C Standard 11
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# Compile definitions:
# -g3: Include all debug information
# -Og: Decrease code size, but keep the debugability
# -fno-exceptions: Disables C++ and C exceptions (enabling them requires malloc and heap functionality!)
# $<..>: Enable only when compiling C++ sources
# -fno-rtti: Disable generation of information about every class with virtual functions for use by the C++ runtime type identification features (dynamic_cast and typeid). (Saves precious RAM and flash Space!)
# -fno-use-cxa-atexit: Register destructors for objects with static storage duration with the _cxa_atexit function rather than the atexit function. (Not needed as it is for shared libraries, everything is static anyway. It saves precious RAM disabling this! )
add_compile_options(-g3
                    -Og
                    -fno-exceptions
                    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
                    $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
                    -fmacro-prefix-map=${CMAKE_CURRENT_LIST_DIR}/=/)

# We need to download the full cube hal for the F4 USB middleware
stm32_fetch_cube(F4)

# Get the HAL and CMSIS components for our STM32 series mcu
find_package(CMSIS COMPONENTS STM32F405RG REQUIRED)
find_package(HAL COMPONENTS STM32F4 REQUIRED)

# Global config dir for the HAL and CMSIS
include_directories(config)

# Set the FreeRTOS port to ARM cortex-m4 with fpu
set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "")
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)

# Create a target with the freertos-config
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config INTERFACE config) # The config file directory
target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0)
target_link_libraries(freertos_config INTERFACE CMSIS::STM32::F405RG)

# Download the FreeRTOS kernel
FetchContent_Declare(
    freertos-kernel
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
    GIT_TAG V11.1.0
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/board_sdk/freertos
)
FetchContent_MakeAvailable(freertos-kernel)

# Add seperate USB_device target containing all usb middleware
add_library(USB_device           ${CMAKE_CURRENT_LIST_DIR}/src/usb/usbd_conf_template.c
                                 ${CMAKE_BINARY_DIR}/_deps/stm32cubef4-src/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
                                 ${CMAKE_BINARY_DIR}/_deps/stm32cubef4-src/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
                                 ${CMAKE_BINARY_DIR}/_deps/stm32cubef4-src/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_desc_template.c
                                 ${CMAKE_BINARY_DIR}/_deps/stm32cubef4-src/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
                                 ${CMAKE_BINARY_DIR}/_deps/stm32cubef4-src/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc_if_template.c
                                 ${CMAKE_BINARY_DIR}/_deps/stm32cubef4-src/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c)
target_include_directories(USB_device PUBLIC ${CMAKE_BINARY_DIR}/_deps/stm32cubef4-src/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
                                             ${CMAKE_BINARY_DIR}/_deps/stm32cubef4-src/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
                                             ${CMAKE_BINARY_DIR}/_deps/stm32cubef4-src/Middlewares/ST/STM32_USB_Device_Library/Class/HID/Inc
                                             ${CMAKE_CURRENT_LIST_DIR}/src/usb)
target_link_libraries(USB_device CMSIS::STM32::F405RG HAL::STM32::F4::LL_USB HAL::STM32::F4::PCDEx freertos_kernel)

# The usb middleware requires configuration and usb-descriptior, make it a target too
add_library(USB_device_conf ${CMAKE_CURRENT_LIST_DIR}/src/usb/usb_device.c
                                      ${CMAKE_CURRENT_LIST_DIR}/src/usb/usbd_cdc_if.c
                                      ${CMAKE_CURRENT_LIST_DIR}/src/usb/usbd_conf.c
                                      ${CMAKE_CURRENT_LIST_DIR}/src/usb/usbd_desc.c)
target_include_directories(USB_device_conf PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/usb ${CMAKE_CURRENT_LIST_DIR}/src/hooks)
target_link_libraries(USB_device_conf USB_device)
set(PLATFORM_DEFINED STM32F4)

# Old manikin_sensor_libraries was made for use with old freertos_kernel
# Which had target name FreeRTOS, new one is freertos_kernel
add_library(FreeRTOS ALIAS freertos_kernel)

# Work smart, for short term
# Include adapted fork of universal_hal, so that original sensor libs can be used temporarily
FetchContent_Declare(
    Universal_hal
    GIT_REPOSITORY https://github.com/Hoog-V/Universal_hal.git
    GIT_TAG dev-stm32f4-halwrapper 
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/Universal_hal
)
FetchContent_MakeAvailable(Universal_hal)

FetchContent_Declare(
    Manikin_Software_Libraries_V2
    GIT_REPOSITORY https://github.com/RobotPatient/Manikin_Software_Libraries_V2.git
    GIT_TAG main
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/Manikin_Software_Libraries_V2
)
FetchContent_MakeAvailable(Manikin_Software_Libraries_V2)

# Create and link the target executable
add_executable(${PROJECT_NAME} src/main.cpp src/hooks/override_hal_delay.c src/hooks/freertos_hooks.c src/hooks/syscalls.c src/hooks/hal_msp.c src/hooks/interrupt_handler.c src/hooks/error_handling.c)
target_link_libraries(${PROJECT_NAME} CMSIS::STM32::F405RG HAL::STM32::F4::CORTEX  HAL::STM32::F4::DMA HAL::STM32::F4::PCD HAL::STM32::F4::GPIO HAL::STM32::F4::RCC HAL::STM32::F4::LL_USB STM32::NoSys USB_device USB_device_conf freertos_kernel sensor_compression)
target_include_directories(${PROJECT_NAME} PUBLIC src/hooks)
stm32_print_size_of_target(${PROJECT_NAME})