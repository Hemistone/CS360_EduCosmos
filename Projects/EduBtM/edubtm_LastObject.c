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
 * Module: edubtm_LastObject.c
 *
 * Description : 
 *  Find the last ObjectID of the given Btree.
 *
 * Exports:
 *  Four edubtm_LastObject(PageID*, KeyDesc*, KeyValue*, Four, BtreeCursor*) 
 */

#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"

/*@================================
 * edubtm_LastObject()
 *================================*/
/*
 * Function:  Four edubtm_LastObject(PageID*, KeyDesc*, KeyValue*, Four, BtreeCursor*) 
 *
 * Description : 
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Find the last ObjectID of the given Btree. The 'cursor' will indicate
 *  the last ObjectID in the Btree, and it will be used as successive access
 *  by using the Btree.
 *
 * Returns:
 *  error code
 *    eBADPAGE_BTM
 *    some errors caused by function calls
 *
 * Side effects:
 *  cursor : the last ObjectID and its position in the Btree
 */
Four edubtm_LastObject(
    PageID *root,        /* IN the root of Btree */
    KeyDesc *kdesc,      /* IN key descriptor */
    KeyValue *stopKval,  /* IN key value of stop condition */
    Four stopCompOp,     /* IN comparison operator of stop condition */
    BtreeCursor *cursor) /* OUT the last BtreeCursor to be returned */
{
    /* These local variables are used in the solution code. However, you don��t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    int i;
    Four e;                    /* error number */
    Four cmp;                  /* result of comparison */
    BtreePage *apage;          /* pointer to the buffer holding current page */
    BtreeOverflow *opage;      /* pointer to the buffer holding overflow page */
    PageID curPid;             /* PageID of the current page */
    PageID child;              /* PageID of the child page */
    PageID ovPid;              /* PageID of the current overflow page */
    PageID nextOvPid;          /* PageID of the next overflow page */
    Two lEntryOffset;          /* starting offset of a leaf entry */
    Two iEntryOffset;          /* starting offset of an internal entry */
    btm_LeafEntry *lEntry;     /* a leaf entry */
    btm_InternalEntry *iEntry; /* an internal entry */
    Four alignedKlen;          /* aligned length of the key length */

    if (root == NULL)
        ERR(eBADPAGE_BTM);

    /* Error check whether using not supported functionality by EduBtM */
    for (i = 0; i < kdesc->nparts; i++)
    {
        if (kdesc->kpart[i].type != SM_INT && kdesc->kpart[i].type != SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // Using Func : edubtm_KeyCompare(), BfM_GetTrain(), BfM_FreeTrain()
    // edubtm_KeyCompare() is not used in here (maybe should use if concerning ovPid)
    // Did not concerned about overflow. (not using ovPid or nextovPid)

    curPid = *root;

    e = BfM_GetTrain((TrainID *)&curPid, (char **)&apage, PAGE_BUF);
    if (e < 0)
        ERR(e);

    while (!(apage->any.hdr.type & LEAF))
    {
        if (!(apage->any.hdr.type & INTERNAL)) // if the page is freepage, overflow
            ERRB1(eBADPAGE_BTM, &curPid, PAGE_BUF);

        e = BfM_FreeTrain((TrainID *)&curPid, PAGE_BUF);
        if (e < 0)
            ERR(e);

        iEntryOffset = apage->bl.slot[-(apage->bi.hdr.nSlots - 1)];
        iEntry = (btm_InternalEntry *)&apage->bi.data[iEntryOffset]; // Find the last one
        MAKE_PAGEID(child, root->volNo, iEntry->spid);
        e = BfM_GetTrain((TrainID *)&child, (char **)&apage, PAGE_BUF);
        if (e < 0)
            ERR(e);

        curPid = child;
    }

    lEntryOffset = apage->bl.slot[-(apage->bl.hdr.nSlots - 1)];
    lEntry = (btm_LeafEntry *)&apage->bl.data[lEntryOffset];
    alignedKlen = ((lEntry->klen + 3) >> 2) << 2;

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

    cursor->flag = CURSOR_ON;
    cursor->oid = *(ObjectID *)&lEntry->kval[alignedKlen];
    cursor->key = *(KeyValue *)&lEntry->klen;
    cursor->leaf = curPid;
    cursor->slotNo = apage->bl.hdr.nSlots - 1;

    e = BfM_FreeTrain(&curPid, PAGE_BUF);
    if (e < 0)
        ERR(e);

    return (eNOERROR);

} /* edubtm_LastObject() */
