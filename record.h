//
// Created by rhanqtl on 19-3-28.
//

#ifndef LINUX_PROGRAMMING_RECORD_H
#define LINUX_PROGRAMMING_RECORD_H

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

#define FILE_NAME_MAX 4095

struct Record {
    ino_t m_ino;
    mode_t m_mode;
    short m_nlink;
    uid_t m_uid;
    gid_t m_gid;
    size_t m_size;
    time_t m_mtime;
    char m_filename[FILE_NAME_MAX + 1];
    struct Record *m_prev;
    struct Record *m_next;
    struct Record *m_child_head;
    struct Record *m_child_tail;
};

struct Record *Record_new(struct stat *stat_buf, char *fn);

void Record_delete(struct Record *self);

void append_child(struct Record *child, struct Record *parent);

void parse_mode(mode_t mode, char buf[]);

void parse_time(time_t time, char buf[]);

void parse_uid(uid_t uid, char buf[]);

void parse_gid(gid_t gid, char buf[]);

#endif //LINUX_PROGRAMMING_RECORD_H
