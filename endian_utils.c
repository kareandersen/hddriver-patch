#include "endian_utils.h"

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define HOST_BIG_ENDIAN 1
#else
    #define HOST_BIG_ENDIAN 0
#endif

uint16_t read_be16(const uint8_t *buf) {
    if (HOST_BIG_ENDIAN)
        return *(const uint16_t *)buf;
    else
        return (uint16_t)(buf[0] << 8 | buf[1]);
}

void write_be16(uint8_t *buf, uint16_t val) {
    if (HOST_BIG_ENDIAN) {
        *(uint16_t *)buf = val;
    } else {
        buf[0] = val >> 8;
        buf[1] = val & 0xFF;
    }
}

uint32_t read_be32(const uint8_t *buf) {
    if (HOST_BIG_ENDIAN)
        return *(const uint32_t *)buf;
    else
        return (uint32_t)(buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]);
}

void write_be32(uint8_t *buf, uint32_t val) {
    if (HOST_BIG_ENDIAN) {
        *(uint32_t *)buf = val;
    } else {
        buf[0] = val >> 24;
        buf[1] = (val >> 16) & 0xFF;
        buf[2] = (val >> 8) & 0xFF;
        buf[3] = val & 0xFF;
    }
}

