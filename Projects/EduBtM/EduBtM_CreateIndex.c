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
 * Module :	EduBtM_CreateIndex.c
 *
 * Description : 
 *  Create the new B+ tree Index. 
 *
 * Exports:
 *  Four EduBtM_CreateIndex(ObjectID*, PageID*)
 */

#include "EduBtM_common.h"
#include "EduBtM_Internal.h"
#include "OM_Internal.h"
#include "BfM.h"

/*@================================
 * EduBtM_CreateIndex()
 *================================*/
/* 
 * Function: Four  EduBtM_CreateIndex(ObjectID*, PageID*)
 *
 * Description : 
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Create the new B+ tree Index. 
 *  We allocate the root page and initialize it.
 *
 * Returns :
 *  error code
 *    some errors caused by function calls
 *
 * Side effects:
 *  The parameter rootPid is filled with the new root page's PageID. 
 */
Four EduBtM_CreateIndex(
    ObjectID *catObjForFile, /* IN catalog object of B+ tree file */
    PageID *rootPid)         /* OUT root page of the newly created B+tree */
{
    /* These local variables are used in the solution code. However, you don��t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    Four e; /* error number */
    Boolean isTmp;
    SlottedPage *catPage;            /* buffer page containing the catalog object */
    sm_CatOverlayForBtree *catEntry; /* pointer to Btree file catalog information */
    PhysicalFileID pFid;             /* physical file ID */

    // Using Function : edubtm_InitLeaf(), btm_AllocPage(), BfM_GetTrain(), BfM_FreeTrain()

    // typedef struct
    // {
    //     SlottedPageHdr header;          /* header of the slotted page */
    //     char data[PAGESIZE - SP_FIXED]; /* data area */
    //     SlottedPageSlot slot[1];        /* slot arrays, indexes backwards */
    // } SlottedPage;

    // typedef struct
    // {
    //     FileID fid;            /* B+-tree file's file identifier */
    //     Two eff;               /* B+-tree file's extent fill factor */
    //     ShortPageID firstPage; /* B+-tree file's first page No */ - ShortPageID = pageNo
    // } sm_CatOverlayForBtree;

    e = BfM_GetTrain((TrainID *)catObjForFile, (char **)&catPage, PAGE_BUF);
    if (e < 0)
        ERR(e);

    GET_PTR_TO_CATENTRY_FOR_BTREE(catObjForFile, catPage, catEntry);
    MAKE_PHYSICALFILEID(pFid, catEntry->fid.volNo, catEntry->firstPage);

    /* Allocate a new page to be used as a B+ tree index page */
    e = btm_AllocPage(catObjForFile, (PageID *)&pFid, rootPid);
    if (e < 0)
        ERR(e);

    // Four edubtm_InitLeaf(
    //     PageID * leaf, /* IN the PageID to be initialized */
    //     Boolean root,  /* IN Is it root ? */
    //     Boolean isTmp) /* IN Is it temporary ? */

    // e = edubtm_InitLeaf(rootPid, TRUE, isTmp);
    e = btm_InitLeaf(rootPid, TRUE, isTmp);
    if (e < 0)
        ERRB1(e, catObjForFile, PAGE_BUF);

    e = BfM_FreeTrain((TrainID *)catObjForFile, PAGE_BUF);
    if (e < 0)
        ERR(e);

    return (eNOERROR);

} /* EduBtM_CreateIndex() */
