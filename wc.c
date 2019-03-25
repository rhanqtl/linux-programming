#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#define MAX(a, b) ((a) >= (b) ? (a) : (b))

typedef int bool;
#define true 1
#define false 0

struct Node {
    const char *filename;
    int num_newlines;
    int num_words;
    int num_bytes;
    struct Node *next;
};

struct Node *Node_init(const char *fn, int newlines, int words, int bytes);
void Node_delete(struct Node *self);

void append(struct Node *x);
void destroy();

struct Node *count(const char *filename);

void print();
int digits(int x);

int total_num_newlines = 0;
int total_num_words = 0;
int total_num_bytes = 0;

struct Node *head, *tail;

int main(int argc, const char *argv[]) {
    if (argc <= 1) {
        perror("wc: 错误: 没有输入文件\n");
        exit(-1);
    }
    for (int i = 1; i < argc; i++) {
        struct Node *ret = count(argv[i]);
        append(ret);
    }
    print();
    destroy();

    return 0;
}

struct Node * count(const char *filename) {
    int newlines = 0, words = 0, bytes = 0;
    FILE *fp = fopen(filename, "r");
    bool in_word = false;
    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            newlines++;
        }
        // 简单的自动机
        if (!isspace(ch)) {
            if (!in_word) {
                words++;
                in_word = true;
            }
        } else {
            in_word = false;
        }
        bytes++;
    }

    total_num_newlines += newlines;
    total_num_words += words;
    total_num_bytes += bytes;

    fclose(fp);

    return Node_init(filename, newlines, words, bytes);
}

struct Node *Node_init(const char *fn, int newlines, int words, int bytes) {
    struct Node *result = (struct Node *) malloc(sizeof(struct Node));
    if (result == NULL) {
        perror("\33[1;31m错误:\33[0m 内存不足\n");
        exit(-1);
    }
    // 因为 fn 来自 argv，所以可以直接保存指针
    result->filename = fn;
    result->num_newlines = newlines;
    result->num_words = words;
    result->num_bytes = bytes;
    result->next = NULL;
    return result;
}

void Node_delete(struct Node *self) {
    free(self);
}

void append(struct Node *x) {
    if (head == NULL) {
        head = tail = x;
    } else {
        tail->next = x;
        tail = x;
    }
}

void destroy() {
    struct Node *p = head, *q;
    while (p != NULL) {
        q = p;
        p = p->next;
        Node_delete(q);
    }
}

void print() {
    int width = digits(MAX(total_num_newlines, 
        MAX(total_num_words, total_num_bytes))) + 1;
    struct Node *p = head;
    while (p != NULL) {
        printf("%*d%*d%*d %s\n", width, p->num_newlines,
            width, p->num_words, width, p->num_bytes, p->filename);
        p = p->next;
    }
    printf("%*d%*d%*d 总用量\n", width, total_num_newlines,
        width, total_num_words, width, total_num_bytes);
}

int digits(int x) {
    int result = 1;
    while (x >= 10) {
        result++;
        x /= 10;
    }
    return result;
}