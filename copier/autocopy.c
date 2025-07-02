#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define BUF_SIZE 4096

void reset_machine() {
    __asm__ volatile ("\tjmp\t0");
}

void delete_self(char *argv0) {
    unlink(argv0);
}

int copy_file(const char *src, const char *dst) {
    FILE *src_fd = fopen(src, "r");
    if (!src_fd) return -1;

    FILE *dst_fd = fopen(dst, "w");
    if (!dst_fd) {
        fclose(src_fd);
        return -1;
    }

    char buffer[BUF_SIZE];
    long read, written;
    do {
        read = fread(buffer, 1, BUF_SIZE, src_fd);
        if (read > 0) {
            written = fwrite(buffer, 1, read, dst_fd);
            if (written != read) {
                fclose(src_fd);
                fclose(dst_fd);
                return -3;
            }
        }
    } while (read > 0);

    fclose(src_fd);
    fclose(dst_fd);
    return 0;
}

int main(int argc, char **argv) {
    int rc = copy_file("A:\\HDDRIVER.SYS", "C:\\HDDRIVER.SYS");
    if (rc != 0) {
        printf("Copy failed.\r\n");
        return (rc << 8); // show error code in low byte
    }

    if (argc > 0) delete_self(argv[0]);

    reset_machine();
    return 0;
}

