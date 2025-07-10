#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "endian_utils.h"


#define UINT32_MAX 0xFFFFFFFF
#define INT16_MIN (-32768)
#define INT16_MAX 32767

#define INST_PATTERN 0x48412281  // swap d1; move.l d1,(a1)
#define STUB_PATTERN 0x6C847566  // "läuf" <- inject code here
#define MAX_PATCH_SITES 2
#define HEADER_SIZE 28

static const uint16_t patch_stub_code[] = {
    0x3281,             // move.w d1,(a1)
    0x4841,             // swap d1
    0x3341, 0x0002,     // move.w d1,2(a1)
    0x4E75              // rts
};

struct __attribute__((packed, scalar_storage_order("big-endian"))) opcode16_offset16 {
    uint16_t opcode;
    int16_t offset;
};

static inline bool apply_bsr_patch(uint8_t *data, uint32_t site_offset, uint32_t target_runtime_offset, const char *label) {
    if (site_offset & 1) {
        printf("%s: Patch site not word-aligned (0x%X)\n", label, (int)site_offset);
        return false;
    }

    uint32_t site_runtime_offset = site_offset - HEADER_SIZE;
    int32_t rel = (int32_t)target_runtime_offset - (int32_t)(site_runtime_offset + 2);

    if (rel < INT16_MIN || rel > INT16_MAX) {
        printf("%s: Relative offset %d out of range for bsr.w\n", label, (int)rel);
        return false;
    }

    struct opcode16_offset16 instr = {
        .opcode = 0x6100,
        .offset = (int16_t)rel
    };

    *(struct opcode16_offset16 *)&data[site_offset] = instr;

    printf("%s: Patched bsr.w at file offset 0x%X (rel %d)\n", label, (int)site_offset, (int)rel);
    return true;
}

static void write_patch_stub(uint8_t *data, uint32_t offset) {
    memcpy(&data[offset], patch_stub_code, sizeof(patch_stub_code));
    printf("Replacement code written at file offset: 0x%lX\r\n", offset);
}

static uint32_t find_aligned_pattern(const uint8_t *data, size_t len, uint32_t pattern, int alignment) {
    for (size_t i = 0; i <= len - 4; i += (alignment ? alignment : 1)) {
        if (*(const uint32_t *)&data[i] == pattern)
            return (uint32_t)i;
    }
    return UINT32_MAX;
}

uint32_t find_string(const uint8_t *data, size_t size) {
    const uint8_t prefix = (STUB_PATTERN >> 24) & 0xFF;  // optimization: first byte of pattern

    for (uint32_t i = 0; i <= size - 4; ++i) {
        if (data[i] != prefix)
            continue;

        uint32_t val = (data[i] << 24) | (data[i + 1] << 16) | (data[i + 2] << 8) | data[i + 3];
        if (val == STUB_PATTERN) {
            uint32_t aligned = (i & 1) ? i + 1 : i;

            if (aligned + 8 > size) {
                printf("site too close to EOF, no room for code\r\n");
                return UINT32_MAX;
            }

            printf("Found string pattern at offset 0x%X (aligned: 0x%X)\r\n", (unsigned int)i, (unsigned int)aligned);
            return aligned;
        }
    }

    printf("Target string pattern not found\r\n");
    return UINT32_MAX;
}


int main(int argc, char **argv)
{
    printf("\r\nHDDriver STE DMA fix patcher v0.1\r\n\r\n");
    printf("    by Carpet Ritz Crumbs\n\r\n\r\n");
    printf("NB! ALPHA LEVEL SOFTWARE \r\nBACK YOUR FILES UP BEFORE USE!\r\n\r\n");
    printf("   Educational use only.\r\n\r\n");

    if (argc != 3) {
        printf("\r\nUsage: %s original.sys patched.sys\r\n", argv[0]);
        gemdos_cconin();

        return 1;
    }

    FILE *fin = fopen(argv[1], "rb");
    if (!fin) {
        printf("Can't open input file: %s\r\n", argv[1]);
        return 1;
    }

    fseek(fin, 0, SEEK_END);
    long insize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    uint8_t *data = malloc(insize);
    if (!data) {
        printf("malloc failed\r\n");
        fclose(fin);
        return 1;
    }

    fread(data, 1, insize, fin);
    fclose(fin);

    if (read_be16(data) != 0x601A) {
        printf("Not a GEMDOS executable file\r\n");
        free(data);
        return 1;
    }

    uint32_t patch1_offset = find_aligned_pattern(data, insize, INST_PATTERN, 2);
    if (patch1_offset == UINT32_MAX) {
        printf("First occurrence of problematic code not found\r\n");
        free(data);
        return 1;
    }

    uint32_t patch2_offset = find_aligned_pattern(data + patch1_offset + 4,
                                              insize - patch1_offset - 4,
                                              INST_PATTERN, 2);
    if (patch2_offset == UINT32_MAX) {
        printf("Second occurrence of problematic code not found\r\n");
        free(data);
        return 1;
    }

    patch2_offset += patch1_offset + 4;

    uint32_t stub_offset = find_string(data, insize);       // file offset
    uint32_t stub_runtime_offset = stub_offset - HEADER_SIZE;

    apply_bsr_patch(data, patch1_offset, stub_runtime_offset, "#1 move.l -> double move.w");
    apply_bsr_patch(data, patch2_offset, stub_runtime_offset, "#2 move.l -> double move.w");
    write_patch_stub(data, stub_offset);

    FILE *fout = fopen(argv[2], "wb");
    if (!fout) {
        printf("Can't open output file: %s\r\n", argv[2]);
        free(data);
        return 1;
    }

    fwrite(data, 1, insize, fout);
    fclose(fout);
    free(data);

    printf("Patched driver written to: %s\r\n", argv[2]);
    return 0;
}

