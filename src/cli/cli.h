#ifndef CLI_H
#define CLI_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "stdint.h"
#include "stddef.h"

/**
 * @brief This function get's called when a new sample has been read from the sensors
 *        It queues the sample for transmit over USB
 */
void manikin_cli_on_new_sample(const char *sample_name, const uint8_t *buffer, size_t len);

void manikin_cli_init();



#ifdef __cplusplus
}
#endif
#endif /* CLI_H */