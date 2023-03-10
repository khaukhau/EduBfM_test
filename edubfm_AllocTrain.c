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
 * Module: edubfm_AllocTrain.c
 *
 * Description : 
 *  Allocate a new buffer from the buffer pool.
 *
 * Exports:
 *  Four edubfm_AllocTrain(Four)
 */


#include <errno.h>
#include "EduBfM_common.h"
#include "EduBfM_Internal.h"


extern CfgParams_T sm_cfgParams;

/*@================================
 * edubfm_AllocTrain()
 *================================*/
/*
 * Function: Four edubfm_AllocTrain(Four)
 *
 * Description : 
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Allocate a new buffer from the buffer pool.
 *  The used buffer pool is specified by the parameter 'type'.
 *  This routine uses the second chance buffer replacement algorithm
 *  to select a victim.  That is, if the reference bit of current checking
 *  entry (indicated by BI_NEXTVICTIM(type), macro for
 *  bufInfo[type].nextVictim) is set, then simply clear
 *  the bit for the second chance and proceed to the next entry, otherwise
 *  the current buffer indicated by BI_NEXTVICTIM(type) is selected to be
 *  returned.
 *  Before return the buffer, if the dirty bit of the victim is set, it 
 *  must be force out to the disk.
 *
 * Returns;
 *  1) An index of a new buffer from the buffer pool
 *  2) Error codes: Negative value means error code.
 *     eNOUNFIXEDBUF_BFM - There is no unfixed buffer.
 *     some errors caused by fuction calls
 */
Four edubfm_AllocTrain(
    Four 	type)			/* IN type of buffer (PAGE or TRAIN) */
{
    Four 	e;			/* for error */
    Four 	victim;			/* return value */
    Four 	i;
    

	
	/* Error check whether using not supported functionality by EduBfM */
	if(sm_cfgParams.useBulkFlush) ERR(eNOTSUPPORTED_EDUBFM);

    /* A. Allocate a buffer element in bufferPool to store page/train*/
    if (BI_NBUFS(type) == 0){ ERR(eNOUNFIXEDBUF_BFM);} // Check whether there is still existed of number of buffer element

    // Buffer replacement policy (clock algorithm) to select a victim as the buffer element. Therefore, allocate on it
    victim = BI_NEXTVICTIM(type);
    i = 0;
    while (TRUE) {
        if (BI_FIXED(type, victim) == 0) { // Check if the page or train is unfixed by any transaction
            if (BI_BITS(type, victim) & REFER) { // Check the REFER bit
                BI_BITS(type, victim) &= ~REFER; // Clear the REFER bit
            } else {
                break;
            }
        }
        i++;
        victim = (victim + 1) % BI_NBUFS(type); 
    }
    
    if (i == (BI_NBUFS(type) * 2)){ // Check whether if all pages are fixed
        ERR(eNOUNFIXEDBUF_BFM);
        }
    
    /* B. Flush the contents of the buffer element into the disk if page/train is modified
    & Delete the array index of the buffer element from the hashTable*/
    if (BI_KEY(type, victim).pageNo != NIL) {
        if (BI_BITS(type, victim) & DIRTY) { // Check whether page/train is modififed, flush out/ write to the disk 
            e = edubfm_FlushTrain(&BI_KEY(type, victim), type);
            if (e < 0) { ERR(e);}
        }
        
        e = edubfm_Delete(&BI_KEY(type, victim), type); // If the page/train is removed from the bufferPool, delete from the hashTable 
        if (e < 0) { ERR(e);}
    }
    
    /* B. Initialize the data structure related to the buffer element selected */
    BI_BITS(type, victim) = 0;
    BI_FIXED(type, victim) = 0;
    BI_NEXTHASHENTRY(type, victim) = NIL;
    SET_NILBFMHASHKEY(BI_KEY(type, victim));
    BI_NEXTVICTIM(type) = (victim + 1) % BI_NBUFS(type);

    return( victim );
    
}  /* edubfm_AllocTrain */