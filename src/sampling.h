#ifndef SAMPLING_H
#define SAMPLING_H
#ifdef __cplusplus
 extern "C" {
#endif
#include <common/manikin_types.h>
#include <stddef.h>
manikin_status_t init_gpio_for_sensors();

manikin_status_t init_i2c_sensor1();

manikin_status_t init_i2c_sensor2();

manikin_status_t init_i2c_sensor3();

manikin_status_t check_and_sample_sensor1(uint8_t *data_buf, size_t data_buf_size);

manikin_status_t check_and_sample_sensor2(uint8_t *data_buf, size_t data_buf_size);

manikin_status_t check_and_sample_sensor3(uint8_t *data_buf, size_t data_buf_size);
#ifdef __cplusplus
}
#endif
#endif /* SAMPLING_H */