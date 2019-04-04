#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>

int digits(int x);

int num_each_line = 16;

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(argv[1], "r");
    char c[16];
    int addr = 0;
    int i = 0;
    while ((c[i] = getc(fp)) != EOF) {
        if ((i+1) % num_each_line == 0) {
            printf("%08x: ", addr);
            for (int j = 0; j < num_each_line; j++) {
                printf("%02x", (c[j] & 0xff));
                if (j % 2 != 0) {
                    printf(" ");
                }
            }
            printf(" ");
            for (int j = 0; j < num_each_line; j++) {
                printf("%c", isprint(c[j]) ? c[j] : '.');
            }
            printf("\n");
            i = 0;
            addr += num_each_line;
        } else {
            i++;
        }
    }
    if (i > 0) {
        printf("%08x: ", addr);
        for (int j = 0; j < i; j++) {
            printf("%02x", (c[j] & 0xff));
            if (j % 2 != 0) {
                printf(" ");
            }
        }
        int n_spaces = (num_each_line - i) * 2 + (num_each_line - i) / 2 + 1;
        for (int j = 0; j < n_spaces; j++) {
            printf(" ");
        }
        printf(" ");
        for (int j = 0; j < i; j++) {
            printf("%c", isprint(c[j]) ? c[j] : '.');
        }
        printf("\n");
    }

    return 0;
}

int digits(int x) {
    int result = 1;
    while (x >= 10) {
        x /= 10;
        result++;
    }
    return result;
}