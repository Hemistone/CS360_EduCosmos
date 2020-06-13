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
 * Module: edubtm_FreePages.c
 *
 * Description :
 *  Free all pages which were related with the given page. 
 *
 * Exports:
 *  Four edubtm_FreePages(FileID*, PageID*, Pool*, DeallocListElem*)
 */

#include "EduBtM_common.h"
#include "Util.h"
#include "BfM.h"
#include "EduBtM_Internal.h"

/*@================================
 * edubtm_FreePages()
 *================================*/
/*
 * Function: Four edubtm_FreePages(FileID*, PageID*, Pool*, DeallocListElem*)
 *
 * Description :
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Free all pages which were related with the given page. If the given page
 *  is an internal page, recursively free all child pages before it is freed.
 *  In a leaf page, examine all leaf items whether it has an overflow page list
 *  before it is freed. If it has, recursively call itself by using the first
 *  overflow page. In an overflow page, it recursively calls itself if the
 *  'nextPage' exist.
 *
 * Returns:
 *  error code
 *    eBADBTREEPAGE_BTM
 *    some errors caused by function calls
 */
Four edubtm_FreePages(
    PhysicalFileID *pFid,    /* IN FileID of the Btree file */
    PageID *curPid,          /* IN The PageID to be freed */
    Pool *dlPool,            /* INOUT pool of dealloc list elements */
    DeallocListElem *dlHead) /* INOUT head of the dealloc list */
{
    /* These local variables are used in the solution code. However, you don��t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    Four e;                    /* error number */
    Two i;                     /* index */
    Two alignedKlen;           /* aligned length of the key length */
    PageID tPid;               /* a temporary PageID */
    PageID ovPid;              /* a temporary PageID of an overflow page */
    BtreePage *apage;          /* a page pointer */
    BtreeOverflow *opage;      /* page pointer to a buffer holding an overflow page */
    Two iEntryOffset;          /* starting offset of an internal entry */
    Two lEntryOffset;          /* starting offset of a leaf entry */
    btm_InternalEntry *iEntry; /* an internal entry */
    btm_LeafEntry *lEntry;     /* a leaf entry */
    DeallocListElem *dlElem;   /* an element of dealloc list */

    // Using : BfM_GetNewTrain(), BfM_FreeTrain(), BfM_SetDirty(), Util_getElementFromPool()
    // typedef union {
    //     BtreeAny any;     /* btree any page */
    //     BtreeInternal bi; /* btree internal page */
    //     BtreeLeaf bl;     /* btree leaf page */
    //     BtreeOverflow bo; /* btree overflow page */
    // } BtreePage;

    // {
    //     PageID pid;    /* page id of this page, should be located on the beginning */
    //     Four flags;    /* flag to store page information */
    //     Four reserved; /* reserved space to store page information */
    //     One type;      /* Internal, Leaf, or Overflow */
    // } BtreeAnyHdr;

    // {
    //     ShortPageID spid; /* pointer to the child page */
    //     Two klen;     /* key length */
    //     char kval[1]; /* key value */
    // } btm_InternalEntry;

    e = BfM_GetNewTrain((TrainID *)curPid, (char **)&apage, PAGE_BUF);
    if (e < 0)
        ERR(e);

    if (apage->any.hdr.type & INTERNAL) // If the page is Internal
    {
        // Free p0 - page
        MAKE_PAGEID(tPid, curPid->volNo, apage->bi.hdr.p0);
        e = edubtm_FreePages(pFid, &tPid, dlPool, dlHead);
        if (e < 0)
            ERRB1(e, curPid, PAGE_BUF);

        // Free slotted pages
        for (i = 0; i < apage->bi.hdr.nSlots; i++)
        {
            iEntryOffset = apage->bi.slot[-i];
            //printf("iEntryOffset : %d\n", iEntryOffset);
            iEntry = (btm_InternalEntry *)&(apage->bi.data[iEntryOffset]);
            MAKE_PAGEID(tPid, curPid->volNo, iEntry->spid);
            e = edubtm_FreePages(pFid, &tPid, dlPool, dlHead);
            if (e < 0)
                ERRB1(e, curPid, PAGE_BUF);
        }
    }

    /* Insert the deallocated page into the dealloc list */
    e = Util_getElementFromPool(dlPool, &dlElem);
    if (e < 0)
        ERR(e);
    dlElem->type = DL_PAGE;
    dlElem->elem.pid = *curPid; /* ID of the deallocated page */
    dlElem->next = dlHead->next;
    dlHead->next = dlElem;

    e = BfM_SetDirty((TrainID *)curPid, PAGE_BUF);
    if (e < 0)
        ERRB1(e, curPid, PAGE_BUF);

    e = BfM_FreeTrain((TrainID *)curPid, PAGE_BUF);
    if (e < 0)
        ERR(e);

    apage->any.hdr.type = FREEPAGE;

    return (eNOERROR);

} /* edubtm_FreePages() */
