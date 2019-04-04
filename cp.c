#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[]) {
    if (argc < 3) {
        printf("%s: \33[1;31m错误:\33[0m 需要两个参数\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    FILE *fsrc = fopen(argv[1], "r");
    if (fsrc == NULL) {
        printf("%s: \33[1;31m错误:\33[0m %s不是文件\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }
    FILE *fdst = fopen(argv[2], "w");

    char c;
    while ((c = getc(fsrc)) != EOF) {
        if (putc(c, fdst) == EOF) {
            printf("%s: \33[1;31m错误:\33[0m 未知错误\n", argv[0]);
        }
    }
    
    return 0;
}