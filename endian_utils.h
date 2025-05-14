#ifndef ENDIAN_UTILS_H
#define ENDIAN_UTILS_H

#include <stdint.h>

uint16_t read_be16(const uint8_t *buf);
void     write_be16(uint8_t *buf, uint16_t val);
uint32_t read_be32(const uint8_t *buf);
void     write_be32(uint8_t *buf, uint32_t val);

#endif // ENDIAN_UTILS_H

