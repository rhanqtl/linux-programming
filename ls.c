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
    PARSE_SYNTAX_ERROR = -1,
    PARSE_UNSUPPORTED_OPT = -2
};

struct Record {
    ino_t         re_ino;
    mode_t        re_mode;
    short         re_nlink;
    uid_t         re_uid;
    gid_t         re_gid;
    size_t        re_size;
    time_t        re_mtime;
    char          re_filename[256];
    struct Record *child_head;
    struct Record *child_tail;
};

struct Record *Record_init(struct stat *statbuf, char fn[]);
void Record_delete(struct Record *self);

void print_record(struct Record *rec);

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
        return PARSE_SYNTAX_ERROR;
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
            return PARSE_UNSUPPORTED_OPT;
        }
    }
    return PARSE_SUCCEEDED;
}

/**
 * 
 * \33[30m -- \33[37m
 * 30: black    31: red         32: green  33: yellow  34: blue
 * 35: purple   36: dark green  37: white
 */
int
execute() {
    struct stat stat_buf;
    struct Record *rec;
    if (current_dir_only) {
        lstat(".", &stat_buf);
        rec = Record_init(&stat_buf, ".");
        print_record(rec);
        Record_delete(rec);
    } else {
        struct Record *head, *tail;
        head = tail = NULL;
        DIR *pdir = opendir(".");
        struct dirent *pde;
        while ((pde = readdir(pdir)) != NULL) {
            lstat(pde->d_name, &stat_buf);
            rec = Record_init(&stat_buf, pde->d_name);
            // TODO: 输出排序
            print_record(rec);
        }
        closedir(pdir);
    }
}

struct Record *
Record_init(struct stat *statbuf, char *fn) {
    struct Record *result = (struct Record *) malloc(sizeof(struct Record));
    
    result->re_ino = statbuf->st_ino;
    result->re_mode = statbuf->st_mode;
    result->re_nlink = statbuf->st_nlink;
    result->re_uid = statbuf->st_uid;
    result->re_gid = statbuf->st_gid;
    result->re_size = statbuf->st_size;
    result->re_mtime = statbuf->st_mtime;
    strncpy(result->re_filename, fn, strlen(fn));
    result->re_filename[strlen(fn)] = '\0';
    result->child_head = result->child_tail = NULL;

    return result;
}

void 
Record_delete(struct Record *self) {
    if (self->child_head == NULL) {
        free(self);
    }
}

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

        // TODO: 对齐
        printf("%7d ", rec->re_size);

        parse_time(rec->re_mtime, buf);
        printf("%s ", buf);
    }
    if (S_ISDIR(rec->re_mode)) {
        printf("\33[1;34m");
    }
    printf("%s", rec->re_filename);
    printf("\33[0m\n");
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
    strncpy(buf, p->pw_name, strlen(p->pw_name));
    buf[strlen(p->pw_name)] = '\0';
}

void
parse_gid(gid_t gid, char buf[]) {
    struct group *p = getgrgid(gid);
    strncpy(buf, p->gr_name, strlen(p->gr_name));
    buf[strlen(p->gr_name)] = '\0';
}

void sort(struct Record *head) {

}