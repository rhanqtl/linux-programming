#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>

#include "record.h"
#include "list.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef int bool;
#define TRUE 1
#define FALSE 0

enum ParseResult {
    PARSE_SUCCEEDED = 0,
    PARSE_FAILED = -1
};

enum ParseResult parse_opt(const char *opt);

void execute();

void traverse(struct Record **p_root);

void print_result(struct Record *root);

void print_record(struct Record *rec, int inode_width,
                  int nlink_width, int user_width,
                  int group_width, int size_width);

void sort(struct Record **p_head, struct Record **p_tail);

int last_index_of(const char *str, char c);

bool equals(const char *s1, const char *s2);

int filename_compare(const char *s1, const char *s2);

char to_upper(char c);

int digits(int x);

bool verbose = FALSE;
bool current_dir_only = FALSE;
bool recursive = FALSE;
bool all = FALSE;
bool show_inode = FALSE;

struct stat stat_buf;

int
main(int argc, const char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (parse_opt(argv[i]) < 0) {
            exit(EXIT_FAILURE);
        }
    }
    execute();

    return 0;
}

enum ParseResult
parse_opt(const char *opt) {
    static const char *SYNTAX_ERROR_FMT = "ls: 语法错误\n";
    static const char *UNSUPPORTED_OPT_FMT = "ls: 不适用的选项 -- %c\n";
    const char *ptr = opt;
    if (*ptr++ != '-') {
        perror(SYNTAX_ERROR_FMT);
        return PARSE_FAILED;
    }
    char ch;
    while ((ch = *ptr++) != '\0') {
        switch (ch) {
            case 'l':
                verbose = TRUE;
                break;
            case 'd':
                current_dir_only = TRUE;
                break;
            case 'R':
                recursive = TRUE;
                break;
            case 'a':
                all = TRUE;
                break;
            case 'i':
                show_inode = TRUE;
                break;
            default:
                fprintf(stderr, UNSUPPORTED_OPT_FMT, ch);
                return PARSE_FAILED;
        }
    }
    return PARSE_SUCCEEDED;
}

/**
 * \33[30m -- \33[37m
 * 30: black    31: red         32: green  33: yellow  34: blue
 * 35: purple   36: dark green  37: white
 */
void
execute() {
    static const char *STAT_ERROR = "ls: \33[1;31m错误: \33[0m无法获取文件信息\n";
    if (current_dir_only) {
        memset(&stat_buf, 0x00, sizeof(struct stat));
        if (lstat(".", &stat_buf) < 0) {
            perror(STAT_ERROR);
        }
        struct Record *rec = Record_new(&stat_buf, ".");
        print_record(rec, 0, 0, 0, 0, 0);
        Record_delete(rec);
    } else {
        struct Record *root = Record_new(NULL, ".");
        char buf[FILE_NAME_MAX + 1];
        DIR *p_dir = opendir(".");
        struct dirent *pde;
        while ((pde = readdir(p_dir)) != NULL) {
            memset(&stat_buf, 0x00, sizeof(struct stat));
            strncpy(buf, "./", 2 + 1);
            strncat(buf, pde->d_name, strlen(pde->d_name) + 1);
            lstat(buf, &stat_buf);
            struct Record *rec = Record_new(&stat_buf, buf);
            append_child(rec, root);
        }
        closedir(p_dir);
        sort(&root->m_child_head, &root->m_child_tail);
        if (recursive) {
            traverse(&root);
        }
        print_result(root);
        Record_delete(root);
    }
}

void
traverse(struct Record **p_root) {
    struct Record *parent = (*p_root)->m_child_head;
    while (parent != NULL) {
        int offset = last_index_of(parent->m_filename, '/') + 1;
        if (S_ISDIR(parent->m_mode)
            && !equals(parent->m_filename + offset, ".")
            && !equals(parent->m_filename + offset, "..")) {
            // TODO: 什么情况下会为 NULL？
            DIR *p_dir = opendir(parent->m_filename);
            if (p_dir != NULL) {
                char buf[FILE_NAME_MAX + 1];
                struct dirent *pde;
                while ((pde = readdir(p_dir)) != NULL) {
                    memset(&stat_buf, 0x00, sizeof(struct stat));
                    strncpy(buf, parent->m_filename, strlen(parent->m_filename) + 1);
                    strncat(buf, "/", 1 + 1);
                    strncat(buf, pde->d_name, strlen(pde->d_name) + 1);
                    lstat(buf, &stat_buf);
                    struct Record *child = Record_new(&stat_buf, buf);
                    append_child(child, parent);
                }
                closedir(p_dir);
                sort(&parent->m_child_head, &parent->m_child_tail);
            }
            traverse(&parent);
        }
        parent = parent->m_next;
    }
}

void
print_result(struct Record *root) {
    if (recursive) {
        printf("%s:\n", root->m_filename);
    }

    int inode_width = 0;
    int nlink_width = 0;
    int user_width = 0;
    int group_width = 0;
    int size_width = 0;
    char buf[1024];
    struct Record *p = root->m_child_head;
    while (p != NULL) {
        inode_width = MAX(inode_width, digits(p->m_ino));
        nlink_width = MAX(nlink_width, digits(p->m_nlink));
        parse_uid(p->m_uid, buf);
        user_width = MAX(user_width, strlen(buf));
        parse_gid(p->m_gid, buf);
        group_width = MAX(group_width, strlen(buf));
        size_width = MAX(size_width, digits(p->m_size));
        p = p->m_next;
    }

    struct Record *q = root->m_child_head;
    while (q != NULL) {
        if (all || q->m_filename[last_index_of(q->m_filename, '/') + 1] != '.') {
            print_record(q, inode_width, nlink_width, user_width, group_width, size_width);
        }
        q = q->m_next;
    }
    printf(verbose ? "\n" : "\n\n");

    struct Record *r = root->m_child_head;
    while (r != NULL) {
        int offset = last_index_of(r->m_filename, '/') + 1;
        if (S_ISDIR(r->m_mode)
            && !equals(r->m_filename + offset, ".")
            && !equals(r->m_filename + offset, "..")) {
            print_result(r);
        }
        r = r->m_next;
    }
}

void
print_record(struct Record *rec, int inode_width, int nlink_width, int user_width, int group_width, int size_width) {
    char buf[1024];
    if (show_inode) {
        printf("%*ld ", inode_width, rec->m_ino);
    }
    if (verbose) {
        parse_mode(rec->m_mode, buf);
        printf("%s ", buf);

        printf("%*d ", nlink_width, rec->m_nlink);

        parse_uid(rec->m_uid, buf);
        printf("%*s ", user_width, buf);

        parse_gid(rec->m_gid, buf);
        printf("%*s ", group_width, buf);

        printf("%*ld ", size_width, rec->m_size);

        parse_time(rec->m_mtime, buf);
        printf("%s ", buf);
    }
    printf("%s",
           rec->m_filename + last_index_of(rec->m_filename, '/') + 1);
    printf((current_dir_only || verbose) ? "\n" : "  ");
}

void
sort(struct Record **p_head, struct Record **p_tail) {
    struct Record *p = (*p_head)->m_next;
    while (p != NULL) {
        struct Record *r = p->m_next;
        struct Record *q = p->m_prev;
        while (q != NULL && filename_compare(p->m_filename, q->m_filename) < 0) {
            q = q->m_prev;
        }
        if (q != NULL) {
            if (p->m_prev != q) {
                p->m_prev->m_next = p->m_next;
                if (p->m_next != NULL) {
                    p->m_next->m_prev = p->m_prev;
                }
                q->m_next->m_prev = p;
                p->m_next = q->m_next;
                p->m_prev = q;
                q->m_next = p;
            }
        } else {
            p->m_prev->m_next = p->m_next;
            if (p->m_next != NULL) {
                p->m_next->m_prev = p->m_prev;
            }
            p->m_next = *p_head;
            p->m_prev = NULL;
            p->m_next->m_prev = p;
            *p_head = p;
        }
        p = r;
    }
    if (*p_head != NULL) {
        struct Record *t = *p_head;
        while (t->m_next != NULL) {
            t = t->m_next;
        }
        *p_tail = t;
    }
}

int last_index_of(const char *str, char c) {
    int len = strlen(str);
    for (int i = len - 1; i >= 0; i--) {
        if (str[i] == c) {
            return i;
        }
    }
    return -1;
}

bool equals(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

int filename_compare(const char *s1, const char *s2) {
    char c1, c2;
    while (s1 != NULL && s2 != NULL) {
        c1 = to_upper(*s1);
        c2 = to_upper(*s2);
        if (c1 == c2) {
            s1++;
            s2++;
        } else {
            return c1 - c2;
        }
    }
    if (s1 == NULL && s2 == NULL) {
        return 0;
    }
    if (s1 == NULL) {
        return -1;
    }
    return 1;
}

char to_upper(char c) {
    if ('a' <= c && c <= 'z') {
        return c - 'a' + 'A';
    }
    return c;
}

int digits(int x) {
    int result = 1;
    while (x >= 10) {
        result++;
        x /= 10;
    }
    return result;
}