//
// Created by rhanqtl on 19-3-28.
//

#ifndef LINUX_PROGRAMMING_LIST_H
#define LINUX_PROGRAMMING_LIST_H

#include "record.h"

struct ListNode {
    struct Record *m_item;
    struct ListNode *m_prev;
    struct ListNode *m_next;
};

struct ListNode *ListNode_new();
void ListNode_delete(struct ListNode *self);

struct List {
    struct ListNode *m_head;
    struct ListNode *m_tail;
};

struct List *List_new();
void List_delete(struct List *self);

#endif //LINUX_PROGRAMMING_LIST_H
