/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * bltList.h --
 *
 * Copyright 2015 George A. Howlett. All rights reserved.  
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are
 *   met:
 *
 *   1) Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2) Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the
 *      distribution.
 *   3) Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *   4) Products derived from this software may not be called "BLT" nor may
 *      "BLT" appear in their names without specific prior written
 *      permission from the author.
 *
 *   THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY EXPRESS OR IMPLIED
 *   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *   DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef _BLT_LIST_H
#define _BLT_LIST_H

/*
 * Acceptable key types for hash tables:
 */
#ifndef BLT_STRING_KEYS
#define BLT_STRING_KEYS         0
#endif
#ifndef BLT_ONE_WORD_KEYS
#define BLT_ONE_WORD_KEYS       ((size_t)-1)
#endif

typedef struct _Blt_List *Blt_List;
typedef struct _Blt_ListNode *Blt_ListNode;

typedef union {                 /* Key has one of these forms: */
    const void *oneWordValue;   /* One-word value for key. */
    unsigned int words[1];      /* Multiple integer words for key.  The
                                 * actual size will be as large as
                                 * necessary for this table's keys. */
    char string[4];             /* String for key.  The actual size will be
                                 * as large as needed to hold the key. */
} Blt_ListKey;

/*
 * A Blt_ListNode is the container structure for the Blt_List.
 */
struct _Blt_ListNode {
    Blt_ListNode prev;          /* Link to the previous node */
    Blt_ListNode next;          /* Link to the next node */
    Blt_List list;              /* List to eventually insert node */
    ClientData clientData;      /* Pointer to the data object */
    Blt_ListKey key;            /* MUST BE LAST FIELD IN RECORD!! */
};

typedef int (Blt_ListCompareProc)(Blt_ListNode *node1Ptr, 
        Blt_ListNode *node2Ptr);

/*
 * A Blt_List is a doubly chained list structure.
 */
struct _Blt_List {
    Blt_ListNode head;                  /* Pointer to first element in
                                         * list */
    Blt_ListNode tail;                  /* Pointer to last element in
                                         * list */
    size_t numNodes;                    /* # of nodes currently in the
                                         * list. */
    size_t type;                        /* Type of keys in list. */
};

BLT_EXTERN void Blt_List_Init(Blt_List list, size_t type);
BLT_EXTERN void Blt_List_Reset(Blt_List list);
BLT_EXTERN Blt_List Blt_List_Create(size_t type);
BLT_EXTERN void Blt_List_Destroy(Blt_List list);
BLT_EXTERN Blt_ListNode Blt_List_CreateNode(Blt_List list, const char *key);
BLT_EXTERN void Blt_List_DeleteNode(Blt_ListNode node);

BLT_EXTERN Blt_ListNode Blt_List_Append(Blt_List list, const char *key, 
        ClientData clientData);
BLT_EXTERN Blt_ListNode Blt_List_Prepend(Blt_List list, const char *key, 
        ClientData clientData);
BLT_EXTERN void Blt_List_LinkAfter(Blt_List list, Blt_ListNode node, 
        Blt_ListNode afterNode);
BLT_EXTERN void Blt_List_LinkBefore(Blt_List list, Blt_ListNode node, 
        Blt_ListNode beforeNode);
BLT_EXTERN void Blt_List_UnlinkNode(Blt_ListNode node);
BLT_EXTERN Blt_ListNode Blt_List_GetNode(Blt_List list, const char *key);
BLT_EXTERN void Blt_List_DeleteNodeByKey(Blt_List list, const char *key);
BLT_EXTERN Blt_ListNode Blt_List_GetNthNode(Blt_List list, long position, 
        int direction);
BLT_EXTERN void Blt_List_Sort(Blt_List list, Blt_ListCompareProc *proc);

#define Blt_List_GetLength(list) \
        (((list) == NULL) ? 0 : ((struct _Blt_List *)list)->numNodes)
#define Blt_List_FirstNode(list) \
        (((list) == NULL) ? NULL : ((struct _Blt_List *)list)->head)
#define Blt_List_LastNode(list) \
        (((list) == NULL) ? NULL : ((struct _Blt_List *)list)->tail)
#define Blt_List_PrevNode(node) ((node)->prev)
#define Blt_List_NextNode(node)         ((node)->next)
#define Blt_List_GetKey(node)   \
        (((node)->list->type == BLT_STRING_KEYS) \
                 ? (node)->key.string : (node)->key.oneWordValue)
#define Blt_List_GetValue(node)         ((node)->clientData)
#define Blt_List_SetValue(node, value) \
        ((node)->clientData = (ClientData)(value))
#define Blt_List_AppendNode(list, node) \
        (Blt_List_LinkBefore((list), (node), (Blt_ListNode)NULL))
#define Blt_List_PrependNode(list, node) \
        (Blt_List_LinkAfter((list), (node), (Blt_ListNode)NULL))

#endif /* _BLT_LIST_H */
