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
 * Module : eduom_CreateObject.c
 * 
 * Description :
 *  eduom_CreateObject() creates a new object near the specified object.
 *
 * Exports:
 *  Four eduom_CreateObject(ObjectID*, ObjectID*, ObjectHdr*, Four, char*, ObjectID*)
 */

#include <string.h>

#include "EduOM_common.h"
// Intellisense Padding
#include "RDsM.h" /* for the raw disk manager call */
// Intellisense Padding
#include "BfM.h" /* for the buffer manager call */
// Intellisense Padding
#include "EduOM_Internal.h"

/*@================================
 * eduom_CreateObject()
 *================================*/
/*
 * Function: Four eduom_CreateObject(ObjectID*, ObjectID*, ObjectHdr*, Four, char*, ObjectID*)
 * 
 * Description :
 * (Following description is for original ODYSSEUS/COSMOS OM.
 *  For ODYSSEUS/EduCOSMOS EduOM, refer to the EduOM project manual.)
 *
 *  eduom_CreateObject() creates a new object near the specified object; the near
 *  page is the page holding the near object.
 *  If there is no room in the near page and the near object 'nearObj' is not
 *  NULL, a new page is allocated for object creation (In this case, the newly
 *  allocated page is inserted after the near page in the list of pages
 *  consiting in the file).
 *  If there is no room in the near page and the near object 'nearObj' is NULL,
 *  it trys to create a new object in the page in the available space list. If
 *  fail, then the new object will be put into the newly allocated page(In this
 *  case, the newly allocated page is appended at the tail of the list of pages
 *  cosisting in the file).
 *
 * Returns:
 *  error Code
 *    eBADCATALOGOBJECT_OM
 *    eBADOBJECTID_OM
 *    some errors caused by fuction calls
 */
Four eduom_CreateObject(
    ObjectID *catObjForFile, /* IN file in which object is to be placed */
    ObjectID *nearObj,       /* IN create the new object near this object */
    ObjectHdr *objHdr,       /* IN from which tag & properties are set */
    Four length,             /* IN amount of data */
    char *data,              /* IN the initial data for the object */
    ObjectID *oid)           /* OUT the object's ObjectID */
{
    /* These local variables are used in the solution code. However, you don��t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    Four e;                  /* error number */
    Four neededSpace;        /* space needed to put new object [+ header] */
    SlottedPage *apage;      /* pointer to the slotted page buffer */
    Four alignedLen;         /* aligned length of initial data */
    Boolean needToAllocPage; /* Is there a need to alloc a new page? */
    PageID pid;              /* PageID in which new object to be inserted */
    PageID nearPid;
    Four firstExt;                  /* first Extent No of the file */
    Object *obj;                    /* point to the newly created object */
    Two i;                          /* index variable */
    sm_CatOverlayForData *catEntry; /* pointer to data file catalog information */
    SlottedPage *catPage;           /* pointer to buffer containing the catalog */
    FileID fid;                     /* ID of file where the new object is placed */
    Two eff;                        /* extent fill factor of file */
    Boolean isTmp;
    PhysicalFileID pFid;

    Two offset;  // Variable of offset

    /*@ parameter checking */

    if (catObjForFile == NULL) ERR(eBADCATALOGOBJECT_OM);

    if (objHdr == NULL) ERR(eBADOBJECTID_OM);

    /* Error check whether using not supported functionality by EduOM */
    if (ALIGNED_LENGTH(length) > LRGOBJ_THRESHOLD) ERR(eNOTSUPPORTED_EDUOM);

    // Load Basic Page Data
    e = BfM_GetTrain((TrainID *)catObjForFile, (char **)&catPage, PAGE_BUF);
    if (e < 0) ERR(e);
    offset = catPage->slot[-(catObjForFile->slotNo)].offset;
    obj = (Object *)&(catPage->data[offset]);
    catEntry = (sm_CatOverlayForData *)obj->data;
    MAKE_PHYSICALFILEID(pFid, catEntry->fid.volNo, catEntry->firstPage);
    /*Get the first extent number of the file */
    e = RDsM_PageIdToExtNo((PageID *)&pFid, &firstExt);
    if (e < 0) ERR(e);

    // Calculate Required Space Size
    alignedLen = ALIGNED_LENGTH(length);
    neededSpace = sizeof(ObjectHdr) + alignedLen + sizeof(SlottedPageSlot);

    //printf("STARTING NILL\n");

    // Select the page to insert object
    if (nearObj != NULL) {
        //printf("nearObj is not NILL\n");
        MAKE_PAGEID(nearPid, nearObj->volNo, nearObj->pageNo);
        e = BfM_GetTrain((TrainID *)&nearPid, (char **)&apage, PAGE_BUF);
        if (e < 0) ERR(e);
        if (neededSpace <= SP_FREE(apage)) {
            //printf("There are some space near nearobj in the page\n");
            // If there are some space near "nearobj" in the page
            pid = nearPid;  // take this near object as page to insert
            e = om_RemoveFromAvailSpaceList(catObjForFile, &pid, apage);
            if (e < 0) ERRB1(e, &pid, PAGE_BUF);
            if (neededSpace > SP_CFREE(apage)) {
                e = EduOM_CompactPage(apage, nearObj->slotNo);
                if (e < 0) ERR(e);
            }
        } else {
            //printf("No available space in the page storing nearObj\n");
            // If there is no available space in the page storing nearObj
            e = BfM_FreeTrain((TrainID *)&nearPid, PAGE_BUF);
            if (e < 0) ERR(e);

            /*Allocate a new page */
            e = RDsM_AllocTrains(catEntry->fid.volNo, firstExt, &nearPid, catEntry->eff, 1, PAGESIZE2, &pid);
            if (e < 0) ERR(e);

            // Initialize header
            e = BfM_GetNewTrain((TrainID *)&pid, (char **)&apage, PAGE_BUF);
            if (e < 0) ERR(e);
            apage->header.fid = catEntry->fid;
            SET_PAGE_TYPE(apage, SLOTTED_PAGE_TYPE);
            apage->header.free = 0;
            apage->header.unused = 0;

            // Inserting page to File
            om_FileMapAddPage(catObjForFile, (PageID *)nearObj, &pid);
            if (e < 0) ERRB1(e, &pid, PAGE_BUF);
        }
    } else {
        ////printf("nearObj is NILL\n");
        if (neededSpace <= SP_50SIZE) {
            //printf("Able to get page in availSpaceList\n");
            // If there is page  to put in - type 1
            ShortPageID availPageNo;
            if (neededSpace <= SP_10SIZE) {
                availPageNo = catEntry->availSpaceList10;
            } else if (neededSpace <= SP_20SIZE) {
                availPageNo = catEntry->availSpaceList20;
            } else if (neededSpace <= SP_30SIZE) {
                availPageNo = catEntry->availSpaceList30;
            } else if (neededSpace <= SP_40SIZE) {
                availPageNo = catEntry->availSpaceList40;
            } else {
                availPageNo = catEntry->availSpaceList50;
            }
            //printf("availPageNo from availSpaceList %d\n", availPageNo);
            if (availPageNo == NIL) {
                MAKE_PAGEID(pid, pFid.volNo, catEntry->lastPage);
                e = BfM_GetTrain((TrainID *)&pid, (char **)&apage, PAGE_BUF);
                if (e < 0) ERR(e);
                // If there is space in the last page - type 2
                if (neededSpace <= SP_FREE(apage)) {
                    //printf("Found space in the last page\n");
                    e = om_RemoveFromAvailSpaceList(catObjForFile, &pid, apage);
                    if (e < 0) ERRB1(e, &pid, PAGE_BUF);
                    if (neededSpace > SP_CFREE(apage)) {
                        e = EduOM_CompactPage(apage, nearObj->slotNo);
                        if (e < 0) ERR(e);
                    }
                } else {
                    //printf("Allocate new page\n");
                    // The last type of 3 types:others - type 3
                    e = BfM_FreeTrain((TrainID *)&pid, PAGE_BUF);  // GiveUp pid
                    if (e < 0) ERR(e);

                    MAKE_PAGEID(nearPid, pFid.volNo, catEntry->lastPage);  // set nearPid as the last page of file, just like compact page

                    // Allocate new page
                    e = RDsM_AllocTrains(catEntry->fid.volNo, firstExt, &nearPid, catEntry->eff, 1, PAGESIZE2, &pid);
                    if (e < 0) ERR(e);

                    // Initialize page header
                    e = BfM_GetNewTrain((TrainID *)&pid, (char **)&apage, PAGE_BUF);
                    if (e < 0) ERR(e);

                    apage->header.fid = catEntry->fid;
                    SET_PAGE_TYPE(apage, SLOTTED_PAGE_TYPE);
                    apage->header.free = 0;
                    apage->header.unused = 0;

                    e = om_FileMapAddPage(catObjForFile, &nearPid, &pid);
                    if (e < 0) ERRB1(e, &pid, PAGE_BUF);
                }
            } else {
                MAKE_PAGEID(pid, pFid.volNo, availPageNo);
                e = BfM_GetNewTrain((TrainID *)&pid, (char **)&apage, PAGE_BUF);
                if (e < 0) ERR(e);
                e = om_RemoveFromAvailSpaceList(catObjForFile, &pid, apage);
                if (e < 0) ERRB1(e, &pid, PAGE_BUF);
                if (neededSpace > SP_CFREE(apage)) {
                    e = EduOM_CompactPage(apage, nearObj->slotNo);
                    if (e < 0) ERR(e);
                }
            }
        } else {
            MAKE_PAGEID(pid, pFid.volNo, catEntry->lastPage);
            e = BfM_GetTrain((TrainID *)&pid, (char **)&apage, PAGE_BUF);
            if (e < 0) ERR(e);
            // If there is space in the last page - type 2
            if (neededSpace <= SP_FREE(apage)) {
                //printf("Found space in the last page\n");
                e = om_RemoveFromAvailSpaceList(catObjForFile, &pid, apage);
                if (e < 0) ERRB1(e, &pid, PAGE_BUF);
                if (neededSpace > SP_CFREE(apage)) {
                    e = EduOM_CompactPage(apage, nearObj->slotNo);
                    if (e < 0) ERR(e);
                }
            } else {
                //printf("Allocate new page\n");
                // The last type of 3 types:others - type 3
                e = BfM_FreeTrain((TrainID *)&pid, PAGE_BUF);  // GiveUp pid
                if (e < 0) ERR(e);

                MAKE_PAGEID(nearPid, pFid.volNo, catEntry->lastPage);  // set nearPid as the last page of file, just like compact page

                // Allocate new page
                e = RDsM_AllocTrains(catEntry->fid.volNo, firstExt, &nearPid, catEntry->eff, 1, PAGESIZE2, &pid);
                if (e < 0) ERR(e);

                // Initialize page header
                e = BfM_GetNewTrain((TrainID *)&pid, (char **)&apage, PAGE_BUF);
                if (e < 0) ERR(e);

                apage->header.fid = catEntry->fid;
                SET_PAGE_TYPE(apage, SLOTTED_PAGE_TYPE);
                apage->header.free = 0;
                apage->header.unused = 0;

                e = om_FileMapAddPage(catObjForFile, &nearPid, &pid);
                if (e < 0) ERRB1(e, &pid, PAGE_BUF);
            }
        }
    }

    //printf("Founded page %d\n", pid.pageNo);

    // Insert object in the page
    for (i = 0; i < apage->header.nSlots; i++) {
        if (apage->slot[-i].offset == EMPTYSLOT)
            break;
    }

    // Update object hdr
    obj = (Object *)&(apage->data[apage->header.free]);
    obj->header.properties = objHdr->properties;
    obj->header.tag = objHdr->tag;
    obj->header.length = length;
    memcpy(obj->data, data, length);  // copy data to the allocated object data array
    e = om_GetUnique(&pid, &(apage->slot[-i].unique));
    if (e < 0) ERRB1(e, &pid, PAGE_BUF);
    apage->slot[-i].offset = apage->header.free;

    //Update object hdr
    if (i == apage->header.nSlots) {
        apage->header.nSlots += 1;
    }
    apage->header.free += sizeof(ObjectHdr) + alignedLen;  // Increase offset as size of object

    MAKE_OBJECTID(*oid, pFid.volNo, pid.pageNo, i, apage->slot[-i].unique);

    e = om_PutInAvailSpaceList(catObjForFile, &pid, apage);
    if (e < 0) ERRB1(e, &pid, PAGE_BUF);

    e = BfM_SetDirty((TrainID *)&pid, PAGE_BUF);
    if (e < 0) ERR(e);
    e = BfM_FreeTrain((TrainID *)&pid, PAGE_BUF);
    if (e < 0) ERR(e);
    e = BfM_FreeTrain((TrainID *)catObjForFile, PAGE_BUF);
    if (e < 0) ERR(e);

    return (eNOERROR);

    // NOT FINISHED YET....

} /* eduom_CreateObject() */
