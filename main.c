#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "endian_utils.h"
#include "compat_file.h"

#define HEADER_SIZE 28

const uint32_t patch1_site_offset    = 0x31B0;
const uint32_t patch2_site_offset    = 0x31C8;
const uint32_t patch_stub_offset     = 0x09FE;
const uint32_t patch_stub_runtime_offset = patch_stub_offset - HEADER_SIZE;

const uint8_t patch_stub_code[] = {
    0x32, 0x81,             // move.w d1,(a1)
    0x48, 0x41,             // swap d1
    0x33, 0x41, 0x00, 0x02, // move.w d1,2(a1)
    0x4E, 0x75              // rts
};

void apply_bsr_patch(uint8_t *data, uint32_t site_offset, uint32_t dest_runtime_offset, const char *label)
{
    uint32_t site_runtime_offset = site_offset - HEADER_SIZE;
    int16_t rel = (int16_t)(dest_runtime_offset - site_runtime_offset - 2);

    uint8_t patch[] = { 0x61, 0x00, (rel >> 8) & 0xFF, rel & 0xFF };
    memcpy(&data[site_offset], patch, sizeof(patch));

    printf("%s: bsr.w at file offset $%lX to stub at file offset $%lX (rel %d / 0x%04X)\r\n",
           label, site_offset, patch_stub_offset, rel, (uint16_t)rel);
}

void write_patch_stub(uint8_t *data)
{
    memcpy(&data[patch_stub_offset], patch_stub_code, sizeof(patch_stub_code));
    printf("Replacement code written at file offset: 0x%lX\r\n", patch_stub_offset);
}

int main(int argc, char **argv)
{
    printf("HDDriver STE DMA fix patcher by Carpet Ritz Crumbs\r\n");

    if (argc != 3) {
        printf("Usage: %s original.sys patched.sys\r\n", argv[0]);
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

    if (patch_stub_offset + sizeof(patch_stub_code) > (uint32_t)insize) {
        printf("Patch stub out of bounds\r\n");
        free(data);
        return 1;
    }

    apply_bsr_patch(data, patch1_site_offset, patch_stub_runtime_offset, "#1 move.l -> double move.w");
    apply_bsr_patch(data, patch2_site_offset, patch_stub_runtime_offset, "#2 move.l -> double move.w");

    write_patch_stub(data);

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

