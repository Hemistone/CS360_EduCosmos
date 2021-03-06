/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2013-2015, Kyu-Young Whang, KAIST                         */
/*    All rights reserved.                                                    */
/*                                                                            */
/*    Redistribution and use in source and binary forms, with or without      */
/*    modification, are permitted provided that the following conditions      */
/*    are met:                                                                */
/*                                                                            */
/*    1. Redistributions of source code must retain the above copyright       */
/*       notice, this list of conditions and the following disclaimer.        */
/*                                                                            */
/*    2. Redistributions in binary form must reproduce the above copyright    */
/*       notice, this list of conditions and the following disclaimer in      */
/*       the documentation and/or other materials provided with the           */
/*       distribution.                                                        */
/*                                                                            */
/*    3. Neither the name of the copyright holder nor the names of its        */
/*       contributors may be used to endorse or promote products derived      */
/*       from this software without specific prior written permission.        */
/*                                                                            */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     */
/*    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       */
/*    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS       */
/*    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE          */
/*    COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,    */
/*    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;        */
/*    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER        */
/*    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT      */
/*    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN       */
/*    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE         */
/*    POSSIBILITY OF SUCH DAMAGE.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational Purpose Object Storage System            */
/*    (Version 1.0)                                                           */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: odysseus.educosmos@gmail.com                                    */
/*                                                                            */
/******************************************************************************/
/*
 * Module: EduBtM_FetchNext.c
 *
 * Description:
 *  Find the next ObjectID satisfying the given condition. The current ObjectID
 *  is specified by the 'current'.
 *
 * Exports:
 *  Four EduBtM_FetchNext(PageID*, KeyDesc*, KeyValue*, Four, BtreeCursor*, BtreeCursor*)
 */

#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"

/*@ Internal Function Prototypes */
Four edubtm_FetchNext(KeyDesc *, KeyValue *, Four, BtreeCursor *, BtreeCursor *);

/*@================================
 * EduBtM_FetchNext()
 *================================*/
/*
 * Function: Four EduBtM_FetchNext(PageID*, KeyDesc*, KeyValue*,
 *                              Four, BtreeCursor*, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Fetch the next ObjectID satisfying the given condition.
 * By the B+ tree structure modification resulted from the splitting or merging
 * the current cursor may point to the invalid position. So we should adjust
 * the B+ tree cursor before using the cursor.
 *
 * Returns:
 *  error code
 *    eBADPARAMETER_BTM
 *    eBADCURSOR
 *    some errors caused by function calls
 */
Four EduBtM_FetchNext(
    PageID *root,         /* IN root page's PageID */
    KeyDesc *kdesc,       /* IN key descriptor */
    KeyValue *kval,       /* IN key value of stop condition */
    Four compOp,          /* IN comparison operator of stop condition */
    BtreeCursor *current, /* IN current B+ tree cursor */
    BtreeCursor *next)    /* OUT next B+ tree cursor */
{

    int i;
    Four e;               /* error number */
    Four cmp;             /* comparison result */
    Two slotNo;           /* slot no. of a leaf page */
    Two oidArrayElemNo;   /* element no. of the array of ObjectIDs */
    Two alignedKlen;      /* aligned length of key length */
    PageID overflow;      /* temporary PageID of an overflow page */
    Boolean found;        /* search result */
    ObjectID *oidArray;   /* array of ObjectIDs */
    BtreeLeaf *apage;     /* pointer to a buffer holding a leaf page */
    BtreeOverflow *opage; /* pointer to a buffer holding an overflow page */
    btm_LeafEntry *entry; /* pointer to a leaf entry */
    BtreeCursor tCursor;  /* a temporary Btree cursor */

    /*@ check parameter */
    if (root == NULL || kdesc == NULL || kval == NULL || current == NULL || next == NULL)
        ERR(eBADPARAMETER_BTM);

    /* Is the current cursor valid? */
    if (current->flag != CURSOR_ON && current->flag != CURSOR_EOS)
        ERR(eBADCURSOR);

    if (current->flag == CURSOR_EOS)
        return (eNOERROR);

    /* Error check whether using not supported functionality by EduBtM */
    for (i = 0; i < kdesc->nparts; i++)
    {
        if (kdesc->kpart[i].type != SM_INT && kdesc->kpart[i].type != SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // Using Function : edubtm_FetchNext()
    // edubtm_KeyCompare(), BfM_GetTrain(),   BfM_FreeTrain() - Maybe should use for overflow control, but not in this code

    // typedef struct
    // {
    //     One flag;           /* state of the cursor */
    //     ObjectID oid;       /* object pointed by the cursor */
    //     KeyValue key;       /* what key value? */
    //     PageID leaf;        /* which leaf page? */
    //     PageID overflow;    /* which overflow page? */
    //     Two slotNo;         /* which slot? */
    //     Two oidArrayElemNo; /* which element of the object array? */
    // } BtreeCursor;

    // typedef struct
    // {                                   /* Leaf Page */
    //     BtreeLeafHdr hdr;               /* header of btree leaf page */
    //     char data[PAGESIZE - BL_FIXED]; /* data area */
    //     Two slot[1];                    /* the first slot */
    // } BtreeLeaf;

    // No useful usage in this API function. Everything runs in the internal function
    e = edubtm_FetchNext(kdesc, kval, compOp, current, next);
    // e = btm_FetchNext(kdesc, kval, compOp, current, next);
    if (e < 0)
        ERR(e);

    return (eNOERROR);

} /* EduBtM_FetchNext() */

/*@================================
 * edubtm_FetchNext()
 *================================*/
/*
 * Function: Four edubtm_FetchNext(KeyDesc*, KeyValue*, Four,
 *                              BtreeCursor*, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Get the next item. We assume that the current cursor is valid; that is.
 *  'current' rightly points to an existing ObjectID.
 *
 * Returns:
 *  Error code
 *    eBADCOMPOP_BTM
 *    some errors caused by function calls
 */
Four edubtm_FetchNext(
    KeyDesc *kdesc,       /* IN key descriptor */
    KeyValue *kval,       /* IN key value of stop condition */
    Four compOp,          /* IN comparison operator of stop condition */
    BtreeCursor *current, /* IN current cursor */
    BtreeCursor *next)    /* OUT next cursor */
{
    Four e;               /* error number */
    Four cmp;             /* comparison result */
    Two alignedKlen;      /* aligned length of a key length */
    PageID leaf;          /* temporary PageID of a leaf page */
    PageID overflow;      /* temporary PageID of an overflow page */
    ObjectID *oidArray;   /* array of ObjectIDs */
    BtreeLeaf *apage;     /* pointer to a buffer holding a leaf page */
    BtreeOverflow *opage; /* pointer to a buffer holding an overflow page */
    btm_LeafEntry *entry; /* pointer to a leaf entry */

    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for (i = 0; i < kdesc->nparts; i++)
    {
        if (kdesc->kpart[i].type != SM_INT && kdesc->kpart[i].type != SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // Using Func : edubtm_KeyCompare(), BfM_GetTrain(), BfM_FreeTrain()
    // Do not concerned about overflow
    // Do not check about uniqueness of key since EduBtM does not admit same key when inserting

    // typedef struct
    // {
    //     One flag;           /* state of the cursor */
    //     ObjectID oid;       /* object pointed by the cursor */
    //     KeyValue key;       /* what key value? */
    //     PageID leaf;        /* which leaf page? */
    //     PageID overflow;    /* which overflow page? */
    //     Two slotNo;         /* which slot? */
    //     Two oidArrayElemNo; /* which element of the object array? */
    // } BtreeCursor;

    // typedef struct
    // {                                   /* Leaf Page */
    //     BtreeLeafHdr hdr;               /* header of btree leaf page */
    //     char data[PAGESIZE - BL_FIXED]; /* data area */
    //     Two slot[1];                    /* the first slot */
    // } BtreeLeaf;

    // typedef struct
    // {
    //     PageID pid;           /* page id of this page, should be located on the beginning */
    //     Four flags;           /* flag to store page information */
    //     Four reserved;        /* reserved space to store page information */
    //     One type;             /* Internal, Leaf, or Overflow */
    //     Two nSlots;           /* # of entries in this page */
    //     Two free;             /* starting point of the free space */
    //     ShortPageID prevPage; /* Previous page */
    //     ShortPageID nextPage; /* Next page */
    //     Two unused;           /* number of unused bytes which are not part of the contiguous freespace */
    // } BtreeLeafHdr;

    ShortPageID NextPage;
    ShortPageID PrevPage;
    Two entryOffset;

    *next = *current; // Firstly move non-using parameters (overflow, etc)

    leaf = current->leaf;
    e = BfM_GetTrain((TrainID *)&leaf, (char **)&apage, PAGE_BUF);
    if (e < 0)
        ERR(e);

    if (compOp == SM_LT || compOp == SM_LE || compOp == SM_EOF)
    { // IF compOp is indexing in increasing order
        next->slotNo += 1;
        if (next->slotNo >= apage->hdr.nSlots)
        { // Gained to the end of entries
            NextPage = apage->hdr.nextPage;
            if (NextPage == NIL) // If the page is last page
            {
                next->flag = CURSOR_EOS;
                e = BfM_FreeTrain((TrainID *)&leaf, PAGE_BUF);
                if (e < 0)
                    ERR(e);
                return (eNOERROR);
            }
            e = BfM_FreeTrain((TrainID *)&leaf, PAGE_BUF);
            if (e < 0)
                ERR(e);

            // Update apage as NextPage
            next->leaf.pageNo = NextPage;
            MAKE_PAGEID(leaf, leaf.volNo, NextPage);
            e = BfM_GetTrain((TrainID *)&leaf, (char **)&apage, PAGE_BUF);
            if (e < 0)
                ERR(e);

            // Set next Entry as the first element of page
            next->slotNo = 0;
        }
    }
    else if (compOp == SM_GT || compOp == SM_GE || compOp == SM_BOF)
    { // IF compOp is indexing in decrease order
        next->slotNo -= 1;
        if (next->slotNo < 0)
        { // Gained to the beginning of entries
            PrevPage = apage->hdr.prevPage;
            if (PrevPage == NIL) // If the page is first page
            {
                next->flag = CURSOR_EOS;
                e = BfM_FreeTrain((TrainID *)&leaf, PAGE_BUF);
                if (e < 0)
                    ERR(e);
                return (eNOERROR);
            }
            e = BfM_FreeTrain((TrainID *)&leaf, PAGE_BUF);
            if (e < 0)
                ERR(e);

            // Update apage as PrevPage
            next->leaf.pageNo = PrevPage;
            MAKE_PAGEID(leaf, leaf.volNo, PrevPage);
            e = BfM_GetTrain((TrainID *)&leaf, (char **)&apage, PAGE_BUF);
            if (e < 0)
                ERR(e);

            // Set next Entry as the last element of page
            next->slotNo = (apage->hdr.nSlots - 1);
        }
    }
    else if (compOp == SM_EQ)
    { // IF compOp is SM_EQ or SM_NE - SM_EQ should not call this FetchNext function.
        next->flag = CURSOR_EOS;
        e = BfM_FreeTrain((TrainID *)&leaf, PAGE_BUF);
        if (e < 0)
            ERR(e);
        return (eNOERROR);
    }
    // --------- End of changing slotNo / pageNo -------------

    // Several Settings for next cursor
    entryOffset = apage->slot[-(next->slotNo)];
    entry = (btm_LeafEntry *)&apage->data[entryOffset];
    alignedKlen = ((entry->klen + 3) >> 2) << 2;
    oidArray = (ObjectID *)&(entry->kval[alignedKlen]);

    next->oid = oidArray[0];
    next->key = *(KeyValue *)&entry->klen;

    if (compOp == SM_BOF || compOp == SM_EOF)
    {
        // Greatest or Least must have the value
        next->flag = CURSOR_ON;
    }
    else
    {
        // For conditions, if the compare fails, it ends.
        cmp = edubtm_KeyCompare(kdesc, kval, &next->key);
        // cmp = EQUAL 0 || GREAT 1 || LESS 2
        if (compOp == SM_LT && cmp == GREAT)
            next->flag = CURSOR_ON;
        else if (compOp == SM_LE && (cmp == GREAT || cmp == EQUAL))
            next->flag = CURSOR_ON;
        else if (compOp == SM_GT && cmp == LESS)
            next->flag = CURSOR_ON;
        else if (compOp == SM_GE && (cmp == LESS || cmp == EQUAL))
            next->flag = CURSOR_ON;
        else
            next->flag = CURSOR_EOS;
    }

    e = BfM_FreeTrain((TrainID *)&leaf, PAGE_BUF);
    if (e < 0)
        ERR(e);

    return (eNOERROR);

} /* edubtm_FetchNext() */
