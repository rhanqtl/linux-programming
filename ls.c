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

#define DEBUG

#define FILENAMEMAX 4095

const char *CURRENT_DIR = ".";
const char *PARENT_DIR = "..";

typedef int bool;
#define TRUE 1
#define FALSE 0

int parse_opt(const char *opt);

int execute();
void parse_type(mode_t mode, char buf[]);
void parse_mode(mode_t mode, char buf[]);
void parse_time(time_t time, char buf[]);
void parse_uid(uid_t uid, char buf[]);
void parse_gid(gid_t gid, char buf[]);

enum ParseResult {
    PARSE_SUCCEEDED = 0,
    PARSE_FAILED = -1
};

struct Record {
    ino_t         re_ino;
    mode_t        re_mode;
    short         re_nlink;
    uid_t         re_uid;
    gid_t         re_gid;
    size_t        re_size;
    time_t        re_mtime;
    char          re_filename[FILENAMEMAX + 1];
    struct Record *re_prev;
    struct Record *re_next;
    struct Record *re_child_head;
    struct Record *re_child_tail;
};

struct Record *Record_init(struct stat *statbuf, char fn[]);
void Record_delete(struct Record *self);

void append_child(struct Record *child, struct Record *parent);

void traverse(struct Record **proot);

void print_result(struct Record *root);
void print_record(struct Record *rec);

void sort(struct Record **phead, struct Record **ptail);

int last_index_of(const char *str, char c);
bool equals(const char *s1, const char *s2);

bool verbose = FALSE;
bool current_dir_only = FALSE;
bool recursive = FALSE;
bool all = FALSE;
bool show_inode = FALSE;

int
main(int argc, const char *argv[]) {
    int result;
    for (int i = 1; i < argc; i++) {
        if ((result = parse_opt(argv[i])) < 0) {
            exit(EXIT_FAILURE);
        }
    }
    execute();

    return 0;
}

int
parse_opt(const char *opt) {
    static const char *SYNTAX_ERROR_FMT = "ls: 语法错误\n";
    static const char *UNSUPPORTED_OPT_FMT = "ls: 不适用的选项 -- %c\n";
    char *ptr = opt;
    char ch;
    if ((ch = *ptr++) != '-') {
        perror(SYNTAX_ERROR_FMT);
        return PARSE_FAILED;
    }
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
int
execute() {
    struct stat stat_buf;
    if (current_dir_only) {
        lstat(".", &stat_buf);
        struct Record *rec = Record_init(&stat_buf, ".");
        print_record(rec);
        // TODO: 析构
    } else {
        struct Record *root = Record_init(NULL, ".");
        char buf[FILENAMEMAX + 1];
        DIR *pdir = opendir(".");
        struct dirent *pde;
        while ((pde = readdir(pdir)) != NULL) {
            lstat(pde->d_name, &stat_buf);
            strcpy(buf, "./");
            strncpy(buf + 2, pde->d_name, strlen(pde->d_name) + 1);
            struct Record *rec = Record_init(&stat_buf, buf);
            append_child(rec, root);
        }
        closedir(pdir);
        if (recursive) {
            traverse(&root);
        }
        print_result(root);
        // TODO: 析构
    }
}

void 
traverse(struct Record **proot) {
    struct Record *parent = (*proot)->re_child_head;
    while (parent != NULL) {
        int offset = last_index_of(parent->re_filename, '/') + 1;
        if (S_ISDIR(parent->re_mode) 
                && !equals(parent->re_filename + offset, ".") 
                && !equals(parent->re_filename + offset, "..")) {
            // TODO: 什么情况下会为 NULL？
            DIR *pdir = opendir(parent->re_filename);
            if (pdir != NULL) {
                struct dirent *pde;
                struct Record *child;
                while ((pde = readdir(pdir)) != NULL) {
                    char buf[FILENAMEMAX + 1] = {'\0'};
                    struct stat statbuf;
                    lstat(pde->d_name, &statbuf);
                    strncpy(buf, parent->re_filename, strlen(parent->re_filename) + 1);
                    strncat(buf, "/", 2);
                    strncat(buf, pde->d_name, strlen(pde->d_name) + 1);
                    child = Record_init(&statbuf, buf);
                    append_child(child, parent);
                }
                closedir(pdir);
            }
            traverse(&parent);
        }
        parent = parent->re_next;
    }
}

struct Record *
Record_init(struct stat *statbuf, char *fn) {
    struct Record *result = (struct Record *) malloc(sizeof(struct Record));  
    if (statbuf != NULL) {
        result->re_ino = statbuf->st_ino;
        result->re_mode = statbuf->st_mode;
        result->re_nlink = statbuf->st_nlink;
        result->re_uid = statbuf->st_uid;
        result->re_gid = statbuf->st_gid;
        result->re_size = statbuf->st_size;
        result->re_mtime = statbuf->st_mtime;
    }
    strncpy(result->re_filename, fn, strlen(fn) + 1);
    result->re_prev = result->re_next = NULL;
    result->re_child_head = result->re_child_tail = NULL;
    return result;
}

/**
 * TODO
 */
void 
Record_delete(struct Record *self) {
    if (self->re_child_head == NULL) {
        free(self);
    }
}

void
append_child(struct Record *child, struct Record *parent) {
    if (parent->re_child_head == NULL) {
        parent->re_child_head = parent->re_child_tail = child;
    } else {
        parent->re_child_tail->re_next = child;
        child->re_prev = parent->re_child_tail;
        parent->re_child_tail = child;
    }
}

/**
 * TODO: recursivly
 */
void
print_result(struct Record *root) {
    if (recursive) {
        printf("%s:\n", root->re_filename);
    }
    sort(&root->re_child_head, &root->re_child_tail);
    struct Record *p = root->re_child_head;
    while (p != NULL) {
        if (all || p->re_filename[last_index_of(p->re_filename, '/') + 1] != '.') {
            print_record(p);
        }
        p = p->re_next;
    }
    printf(verbose ? "\n" : "\n\n");

    struct Record *q = root->re_child_head;
    while (q != NULL) {
        int offset = last_index_of(q->re_filename, '/') + 1;
        if (S_ISDIR(q->re_mode) 
                && !equals(q->re_filename + offset, ".") 
                && !equals(q->re_filename + offset, "..")) {
            print_result(q);
        }
        q = q->re_next;
    }
}

/**
 * TODO: 对齐
 */
void
print_record(struct Record *rec) {
    char buf[1024];
    if (show_inode) {
        printf("%d ", rec->re_ino);
    }
    if (verbose) {
        parse_mode(rec->re_mode, buf);
        printf("%s ", buf);

        printf("%d ", rec->re_nlink);

        parse_uid(rec->re_uid, buf);
        printf("%s ", buf);

        parse_gid(rec->re_gid, buf);
        printf("%s ", buf);

        printf("%7d ", rec->re_size);

        parse_time(rec->re_mtime, buf);
        printf("%s ", buf);
    }
    if (S_ISDIR(rec->re_mode)) {
        printf("\33[1;34m");
    }
    printf("%s", 
        rec->re_filename + last_index_of(rec->re_filename, '/') + 1);
    printf("\33[0m");
    printf((current_dir_only || verbose) ? "\n" : "  ");
}

void
parse_mode(mode_t mode, char buf[]) {
    memset(buf, '-', 10);

    // 设置文件类型
    if (S_ISDIR(mode)) {
        buf[0] = 'd';
    } else if (S_ISBLK(mode)) {
        buf[0] = 'b';
    } else if (S_ISCHR(mode)) {
        buf[0] = 'c';
    } else if (S_ISFIFO(mode)) {
        buf[0] = 'p';
    } else if (S_ISLNK(mode)) {
        buf[0] = 'l';
    } else if (S_ISSOCK(mode)) {
        buf[0] = 's';
    }

    // 设置权限
    if (mode & S_IRUSR) {
        buf[1] = 'r';
    };
    if (mode & S_IWUSR) {
        buf[2] = 'w';
    }
    if (mode & S_IXUSR) {
        buf[3] = 'x';
    }
    if (mode & S_ISUID) {
        buf[3] = 's';
    }
    if (mode & S_IRGRP) {
        buf[4] = 'r';
    };
    if (mode & S_IWGRP) {
        buf[5] = 'w';
    }
    if (mode & S_IXGRP) {
        buf[6] = 'x';
    }
    if (mode & S_ISGID) {
        buf[6] = 's';
    }
    if (mode & S_IROTH) {
        buf[7] = 'r';
    };
    if (mode & S_IWOTH) {
        buf[8] = 'w';
    }
    if (mode & S_IXOTH) {
        buf[9] = 'x';
    }
    if (mode & S_ISVTX) {
        buf[9] = 't';
    }
    buf[10] = '\0';
}

void
parse_time(time_t time, char buf[]) {
    struct tm *p = localtime(&time);
    sprintf(buf, "%2d月  %2d %02d:%02d", p->tm_mon + 1,
        p->tm_mday, p->tm_hour, p->tm_min);
}

void
parse_uid(uid_t uid, char buf[]) {
    struct passwd *p = getpwuid(uid);
    strncpy(buf, p->pw_name, strlen(p->pw_name) + 1);
}

void
parse_gid(gid_t gid, char buf[]) {
    struct group *p = getgrgid(gid);
    strncpy(buf, p->gr_name, strlen(p->gr_name) + 1);
}

// TODO: 修改 ptail
void 
sort(struct Record **phead, struct Record **ptail) {
    struct Record *p = (*phead)->re_next;
    while (p != NULL) {
        struct Record *r = p->re_next;
        struct Record *q = p->re_prev;
        while (q != NULL && strcmp(p->re_filename, q->re_filename) < 0) {
            q = q->re_prev;
        }
        if (q != NULL) {
            if (p->re_prev != q) {
                p->re_prev->re_next = p->re_next;
                if (p->re_next != NULL) {
                    p->re_next->re_prev = p->re_prev;
                }
                q->re_next->re_prev = p;
                p->re_next = q->re_next;
                p->re_prev = q;
                q->re_next = p;
            }
        } else {
            p->re_prev->re_next = p->re_next;
            if (p->re_next != NULL) {
                p->re_next->re_prev = p->re_prev;
            }
            p->re_next = *phead;
            p->re_next->re_prev = p;
            *phead = p;
        }
        p = r;
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