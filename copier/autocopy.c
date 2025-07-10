#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <tos/xbios.h>
#include <tos/system-variable.h>

#define BUF_SIZE 4096
#define DEFAULT_SELF "G:\\AUTO\\AUTOCOPY.TOS"

struct args = {
    .nvbls = 50;
};

void reset_machine()
{
	__asm__ volatile(" lea 0x4.w,%a0\n move.l (%a0),%a0\n jmp (%a0)");
}

void wait_no_vbls(struct args vbls) {
    volatile int *frclock = (int*)0x466;
    int done_at = vbls + *frclock;
    int current;

    do { current = *frclock; }
        while ( current < done_at);
}

int delete_self(const char *self) {
    printf("\r\nAttempting to delete self: %s\r\n", self);
    return unlink(self);
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
    argc = argc; argv = argv;

    int rc = copy_file("G:\\HDDRIVER.SYS", "C:\\HDDRIVER.SYS");
    if (rc != 0) {
        printf("Copy failed. Aborting.\r\n");
        return (rc << 8);
    }

    char* self = (char*)argv[0];
    if (!strlen(self)) self = DEFAULT_SELF;
    int error = delete_self(self);
    if (error) goto handle_error;

    printf("Self deleted - rebooting in.\r\n\n");
    int wt = 3;
    struct args args;
    do {
        printf("%d...\r", wt);
        xbios_supexecarg(wait_no_vbls, &args);
    } while(--wt);

    xbios_supexecarg(reset_machine, NULL);
    return 0;

handle_error:
    printf("Failed to delete, halting to avoid reset loop.");
    while(true);
}

