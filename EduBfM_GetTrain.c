/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational-Purpose Object Storage System            */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Database and Multimedia Laboratory                                      */
/*                                                                            */
/*    Computer Science Department and                                         */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: kywhang@cs.kaist.ac.kr                                          */
/*    phone: +82-42-350-7722                                                  */
/*    fax: +82-42-350-8380                                                    */
/*                                                                            */
/*    Copyright (c) 1995-2013 by Kyu-Young Whang                              */
/*                                                                            */
/*    All rights reserved. No part of this software may be reproduced,        */
/*    stored in a retrieval system, or transmitted, in any form or by any     */
/*    means, electronic, mechanical, photocopying, recording, or otherwise,   */
/*    without prior written permission of the copyright owner.                */
/*                                                                            */
/******************************************************************************/
/*
 * Module: EduBfM_GetTrain.c
 *
 * Description : 
 *  Return a buffer which has the disk content indicated by `trainId'.
 *
 * Exports:
 *  Four EduBfM_GetTrain(TrainID *, char **, Four)
 */


#include "EduBfM_common.h"
#include "EduBfM_Internal.h"



/*@================================
 * EduBfM_GetTrain()
 *================================*/
/*
 * Function: EduBfM_GetTrain(TrainID*, char**, Four)
 *
 * Description : 
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Return a buffer which has the disk content indicated by `trainId'.
 *  Before the allocation of a buffer, look up the train in the buffer
 *  pool using hashing mechanism.   If the train already  exist in the pool
 *  then simply return it and set the reference bit of the correponding
 *  buffer table entry.   Otherwise, i.e. the train does not exist in the
 *  pool, allocate a buffer (a buffer selected as victim may be forced out
 *  by the buffer replacement algorithm), read a disk train into the 
 *  selected buffer train, and return it.
 *
 * Returns:
 *  error code
 *    eBADBUFFER_BFM - Invalid Buffer
 *    eBADBUFFERTYPE_BFM - Invalid Buffer type
 *    some errors caused by function calls
 *
 * Side effects:
 *  1) parameter retBuf
 *     pointer to buffer holding the disk train indicated by `trainId'
 */
Four EduBfM_GetTrain(
    TrainID             *trainId,               /* IN train to be used */
    char                **retBuf,               /* OUT pointer to the returned buffer */
    Four                type )                  /* IN buffer type */
{
    Four                e;                      /* for error */
    Four                index;                  /* index of the buffer pool */


    /*@ Check the validity of given parameters */
    /* Some restrictions may be added         */
    if(retBuf == NULL) ERR(eBADBUFFER_BFM);

    /* Is the buffer type valid? */
    if(IS_BAD_BUFFERTYPE(type)) ERR(eBADBUFFERTYPE_BFM);

    /* Fix the page/train in the bufferPool and return the pointer to the buffer element containing page/train*/
    index = edubfm_LookUp(trainId, type); // Search for the array index of the buffer element from the hashTable
    /* If the page/train does not exist in the bufferPool */
    if (index == NOTFOUND_IN_HTABLE){
        index = edubfm_AllocTrain(type); // Allocate a buffer element to store page/train from the bufferPool
        if (index < 0) {ERR(index);}

        e = edubfm_ReadTrain(trainId, BI_BUFFER(type,index), type); // Store the page/train in the allocated buffer element reading it from disk
        if (e < 0) { ERR(e);}

        // Update the element of bufTable
        //BI_KEY(type, index).pageNo = trainId->pageNo;
        //BI_KEY(type, index).volNo = trainId->volNo;
        BI_KEY(type, index) = *((BfMHashKey *)trainId); // Update buffer table
        BI_FIXED(type, index) = 1;
        BI_BITS(type, index) |= REFER;
        
        e = edubfm_Insert(trainId, index, type); // Insert the array index into hashTable
        if (e < 0) {ERR(e);}
    }
    else {
        /* If the fixed page/train exists in the bufferPool */
        BI_FIXED(type, index) += 1; //Update fixed count
        BI_BITS(type, index) |= REFER; // Set the Refer bit to 1
    }
        *retBuf = BI_BUFFER(type, index); // Set the return value 
    

    return(eNOERROR);   /* No error */

}  /* EduBfM_GetTrain() */
