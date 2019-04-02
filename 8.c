#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>

int main(int argc, const char *argv[]) {
    if (argc <= 1) {
        perror("\33[1;31mERROR: \33[0mNO INPUT FILE\n");
        exit(EXIT_FAILURE);
    }
    DIR *p_dir = opendir(argv[1]);
    if (p_dir == NULL) {
        perror("\33[1;31mERROR: \33[0mCANNOT OPEN DIR\n");
        exit(EXIT_FAILURE);
    }
    char buf[1024];
    struct stat stat_buf;
    struct dirent *p_de;
    while ((p_de = readdir(p_dir)) != NULL) {
        strncpy(buf, argv[1], strlen(argv[1]) + 1);
        strncat(buf, "/", 1 + 1);
        memset(&stat_buf, 0x00, sizeof(struct stat));
        strncat(buf, p_de->d_name, strlen(p_de->d_name) + 1);
        lstat(buf, &stat_buf);
        printf("%s", p_de->d_name);
        if (S_ISREG(stat_buf.st_mode)) {
            printf("(ordinary)");
        } else if (S_ISDIR(stat_buf.st_mode)) {
            printf("(directory)");
        }
        printf(" ");
    }
    printf("\n");
    closedir(p_dir);

    return 0;
}