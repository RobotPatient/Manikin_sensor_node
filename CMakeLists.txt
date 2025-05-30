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
# -fno-rtti: Disable generation of information about every class with virtual functions for use by the C++ runtime type identification features (dynamic_cast and typeid). 
#            (Saves precious RAM and flash Space!)
# -fno-use-cxa-atexit: Register destructors for objects with static storage duration with the _cxa_atexit function rather than the atexit function. 
#                      (Not needed as it is for shared libraries, everything is static anyway. It saves precious RAM disabling this! )
add_compile_options(-g3
    -Og
    -fno-exceptions
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
    -fmacro-prefix-map=${CMAKE_CURRENT_LIST_DIR}/=/
    -DLFS_YES_TRACE)

# We need to download the full cube hal for the F4 USB middleware
stm32_fetch_cube(F4)

# Get the HAL and CMSIS components for our STM32 series mcu
find_package(CMSIS COMPONENTS STM32F405RG REQUIRED)
find_package(HAL COMPONENTS STM32F4 REQUIRED)

# Global config dir for the HAL and CMSIS
include_directories(config)
FetchContent_Declare(
    EMBEDDED_CLI
    GIT_REPOSITORY https://github.com/funbiscuit/embedded-cli
    GIT_TAG v0.1.4
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/embedded-cli
)
FetchContent_MakeAvailable(EMBEDDED_CLI)
add_library(cli src/cli/cli.c)
target_include_directories(cli PUBLIC src/cli)

target_link_libraries(embedded_cli_lib CMSIS::STM32::F405RG)
# Add seperate USB_device target containing all usb middleware
add_library(USB_device ${CMAKE_CURRENT_LIST_DIR}/src/usb/usbd_conf_template.c
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
target_link_libraries(USB_device CMSIS::STM32::F405RG HAL::STM32::F4::LL_USB HAL::STM32::F4::PCDEx)

add_library(USB_device_conf ${CMAKE_CURRENT_LIST_DIR}/src/usb/usb_device.c
    ${CMAKE_CURRENT_LIST_DIR}/src/usb/usbd_cdc_if.c
    ${CMAKE_CURRENT_LIST_DIR}/src/usb/usbd_conf.c
    ${CMAKE_CURRENT_LIST_DIR}/src/usb/usbd_desc.c
    src/cli/cli_setup.c)
target_include_directories(USB_device_conf PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/usb ${CMAKE_CURRENT_LIST_DIR}/src/hooks ${CMAKE_CURRENT_LIST_DIR}/src/cli)
target_link_libraries(USB_device_conf USB_device embedded_cli_lib CMSIS::STM32::F405RG)
target_link_libraries(cli embedded_cli_lib USB_device_conf CMSIS::STM32::F405RG)
# Pull in the sensor & flash drivers for this project
set(PLATFORM_DEFINED STM32F4)
FetchContent_Declare(
    Manikin_Software_Libraries_V3
    GIT_REPOSITORY https://github.com/RobotPatient/Manikin_Software_Libraries_V3.git
    GIT_TAG dev
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/Manikin_Software_Libraries_V3
)
FetchContent_MakeAvailable(Manikin_Software_Libraries_V3)

# Pull in SEGGER RTT for debug statements
FetchContent_Declare(
    SEGGER_RTT
    GIT_REPOSITORY https://github.com/SEGGERMicro/RTT.git
    GIT_TAG master
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/RTT
)
FetchContent_MakeAvailable(SEGGER_RTT)

# Add littlefs for a small filesystem to put on the flash memory
FetchContent_Declare(
    LITTLEFS
    GIT_REPOSITORY https://github.com/littlefs-project/littlefs
    GIT_TAG v2.10.2
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/littlefs
)
FetchContent_MakeAvailable(LITTLEFS)

add_library(littlefs lib/littlefs/lfs_util.c
                     lib/littlefs/lfs.c
                    )
target_include_directories(littlefs PUBLIC lib/littlefs)
target_link_libraries(littlefs CMSIS::STM32::F405RG)






set(MANIKIN_PHY_CAN_PLATFORM "STM32F4")
add_subdirectory(src/manikin_software_hooks)
target_link_libraries(manikin_software_libraries manikin_software_hooks)

# Create and link the target executable
add_executable(${PROJECT_NAME} src/main.cpp 
                               src/hooks/syscalls.c 
                               src/hooks/hal_msp.c 
                               src/hooks/interrupt_handler.c 
                               src/hooks/error_handling.c 
                               src/sampling.c 
                               src/flash/flash.c 
                               src/can/can_wrapper.c
                               lib/RTT/RTT/SEGGER_RTT.c 
                               lib/RTT/RTT/SEGGER_RTT_printf.c 
                               lib/RTT/Syscalls/SEGGER_RTT_Syscalls_GCC.c)

target_link_libraries(${PROJECT_NAME} CMSIS::STM32::F405RG 
                                      HAL::STM32::F4::CORTEX 
                                      HAL::STM32::F4::DMA 
                                      HAL::STM32::F4::PCD 
                                      HAL::STM32::F4::GPIO 
                                      HAL::STM32::F4::RCC 
                                      HAL::STM32::F4::LL_USB 
                                      HAL::STM32::F4::CAN  
                                      STM32::NoSys 
                                      USB_device_conf
                                      manikin_software_libraries
                                      littlefs
                                      cli
                                      )

target_include_directories(${PROJECT_NAME} PUBLIC src/hooks src/can src/flash lib/RTT/Config lib/RTT/RTT)
stm32_print_size_of_target(${PROJECT_NAME})
stm32_generate_hex_file(${PROJECT_NAME})
