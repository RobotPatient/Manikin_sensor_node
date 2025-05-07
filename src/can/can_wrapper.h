#ifndef CAN_WRAPPER_H
#define CAN_WRAPPER_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "stdint.h"
#include "stddef.h"
    void   init_can();
    size_t can_phy_transmit(uint8_t handle, uint8_t *data, size_t len);
#ifdef __cplusplus
}
#endif
#endif // CAN_WRAPPER_H