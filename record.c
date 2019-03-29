//
// Created by rhanqtl on 19-3-28.
//

#include <string.h>

#include "record.h"

struct Record *
Record_new(struct stat *stat_buf, char *fn) {
    struct Record *result = (struct Record *) malloc(sizeof(struct Record));
    if (stat_buf != NULL) {
        result->m_ino = stat_buf->st_ino;
        result->m_mode = stat_buf->st_mode;
        result->m_nlink = stat_buf->st_nlink;
        result->m_uid = stat_buf->st_uid;
        result->m_gid = stat_buf->st_gid;
        result->m_size = stat_buf->st_size;
        result->m_mtime = stat_buf->st_mtime;
    }
    strncpy(result->m_filename, fn, strlen(fn) + 1);
    result->m_prev = result->m_next = NULL;
    result->m_child_head = result->m_child_tail = NULL;
    return result;
}

void
Record_delete(struct Record *self) {
    if (self->m_child_head == NULL) {
        free(self);
    }
}

void
append_child(struct Record *child, struct Record *parent) {
    if (parent->m_child_head == NULL) {
        parent->m_child_head = parent->m_child_tail = child;
    } else {
        parent->m_child_tail->m_next = child;
        child->m_prev = parent->m_child_tail;
        parent->m_child_tail = child;
    }
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
    }
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
    }
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
    }
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