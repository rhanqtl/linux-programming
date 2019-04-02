#include <stdio.h>
#include <stdlib.h>

void bindiff(char *fle1, char *fle2, char *fleo);

int main(void) {
    bindiff("file.txt", "11.c", "wtf");

    return 0;
}

void bindiff(char *fle1, char *fle2, char *fleo) {
    FILE *fin1 = fopen(fle1, "r");
    if (fin1 == NULL) {
        perror("ERROR: cannot open fle1\n");
        exit(EXIT_FAILURE);
    }
    FILE *fin2 = fopen(fle2, "r");
    if (fin2 == NULL) {
        perror("ERROR: cannot open fle2\n");
        exit(EXIT_FAILURE);
    }
    FILE *fout = fopen(fleo, "w+");
    if (fout == NULL) {
        perror("ERROR: cannot open fleo\n");
        exit(EXIT_FAILURE);
    }
    char c1, c2;
    while ((c1 = getc(fin1)) != EOF && (c2 = getc(fin2)) != EOF) {
        if (c1 == c2) {
            putc(c1, fout);
        }
    }
}