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
void parse_access_mode(mode_t mode, char buf[]);
void parse_time(time_t time, char buf[]);
void parse_uid(uid_t uid, char buf[]);
void parse_gid(gid_t gid, char buf[]);

enum parse_result {
    PARSE_SUCCEEDED = 0,
    PARSE_SYNTAX_ERROR = -1,
    PARSE_UNSUPPORTED_OPT = -2
};

bool verbose = FALSE;
bool current_dir_only = FALSE;
bool recursive = FALSE;
bool all = FALSE;
bool show_inode = FALSE;

int main(int argc, const char *argv[]) {
    int result;
    for (int i = 1; i < argc; i++) {
        if ((result = parse_opt(argv[i])) < 0) {
            exit(EXIT_FAILURE);
        }
    }
    execute();

    return 0;
}

int parse_opt(const char *opt) {
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
 * \33[30m -- \33[37m 设置前景色
 * 
 * 字颜色:30 - 39
 * 30:黑
31:红
32:绿
33:黄
34:蓝色
35:紫色
36:深绿
37:白色
 */
int execute() {
    struct result_rec {
        ino_t   re_ino;
        mode_t  re_mode;
        short   re_nlink;
        uid_t   re_uid;
        gid_t   re_gid;
        off_t   re_size;
        time_t  re_mtime;
        char    re_filename[256];
    };
    struct result_rec *head = NULL, *tail = NULL;

    char parse_buf[1024];
    struct stat stat_buf;
    int fd;
    if (current_dir_only) {
        if((fd = open(".", O_RDONLY)) < 0) {
            perror("无法打开文件: .\n");
            exit(-1);
        }
        fstat(fd, &stat_buf);
        if (show_inode) {
            printf("%d ", stat_buf.st_ino);
        }
        if (verbose) {
            memset(parse_buf, 0x00, sizeof(parse_buf));
            parse_access_mode(stat_buf.st_mode, parse_buf);
            printf("%s ", parse_buf);

            printf("%d ", stat_buf.st_nlink);

            memset(parse_buf, 0x00, sizeof(parse_buf));
            parse_uid(stat_buf.st_uid, parse_buf);
            printf("%s ", parse_buf);

            memset(parse_buf, 0x00, sizeof(parse_buf));
            parse_gid(stat_buf.st_gid, parse_buf);
            printf("%s ", parse_buf);

            printf("%d ", stat_buf.st_size);

            memset(parse_buf, 0x00, sizeof(parse_buf));
            parse_time(stat_buf.st_mtime, parse_buf);
            printf("%s ", parse_buf);
        }
        printf("\33[1;34m.\33[0m\n");
    } else {
        // 只要有 -i 选项都优先显示 i-node 号
        // -R: 先序遍历
        // -l: long listing format
        // -a: 显示所有文件，包括隐藏文件
        DIR *pdir = opendir(".");
        struct dirent *pe;
        while ((pe = readdir(pdir)) != NULL) {
            struct result_rec *prec = (struct result_rec *) malloc(sizeof(struct result_rec));
            prec->re_ino = pe->d_ino;
            printf("%d %s\n", pe->d_type, pe->d_name);
            
        }
        closedir(pdir);
    }
}

/**
     * S_IFSOCK 0140000  socket
     * S_IFLNK  0120000  symbolic link
     * S_IFREG  0100000  ordinary
     * S_IFBLK  0060000  block dev
     * S_IFDIR  0040000  dir
     * S_IFCHR  0020000  character dev
     * S_IFIFO  0010000  pipe
     */
void parse_type(mode_t mode, char buf[]) {
    
}

/**
 * TODO: sticky bits
 */
void parse_access_mode(mode_t mode, char buf[]) {
    memset(buf, '-', 10);
    if (mode & S_IRUSR) {
        buf[1] = 'r';
    }
    if (mode & S_IWUSR) {
        buf[2] = 'w';
    }
    if (mode & S_IXUSR) {
        buf[3] = 'x';
    }
    if (mode & S_IRGRP) {
        buf[4] = 'r';
    }
    if (mode & S_IWGRP) {
        buf[5] = 'w';
    }
    if (mode & S_IXGRP) {
        buf[6] = 'x';
    }
    if (mode & S_IROTH) {
        buf[7] = 'r';
    }
    if (mode & S_IWOTH) {
        buf[8] = 'w';
    }
    if (mode & S_IXOTH) {
        buf[9] = 'x';
    }
    buf[10] = '\0';
}

void parse_time(time_t time, char buf[]) {
    struct tm *p = localtime(&time);
    sprintf(buf, "%2d月  %2d %02d:%02d", p->tm_mon + 1,
        p->tm_mday, p->tm_hour, p->tm_min);
}

void parse_uid(uid_t uid, char buf[]) {
    struct passwd *p = getpwuid(uid);
    strncpy(buf, p->pw_name, strlen(p->pw_name));
}

void parse_gid(gid_t gid, char buf[]) {
    struct group *p = getgrgid(gid);
    strncpy(buf, p->gr_name, strlen(p->gr_name));
}