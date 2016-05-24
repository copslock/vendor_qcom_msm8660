/*==============================================================================
*       WFDMMSinkMediaSource.cpp
*
*  DESCRIPTION:
*       Connects RTP decoder and parser and provides Audio and Video samples to
*       the media framework.
*
*
*  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
03/25/2013         SK            InitialDraft
================================================================================
*/

/*==============================================================================
**               Includes and Public Data Declarations
**==============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/
#include "WFDMMThreads.h"
#include "WFDMMSinkCommon.h"
#include "WFDMMSourceSignalQueue.h"
#include "MMMalloc.h"
#include "MMMemory.h"
#include "MMTimer.h"
#include "WFDMMLogs.h"
#include "wfd_netutils.h"
#include "WFDMMSinkMediaSource.h"
#include "filesourcetypes.h"
#include "wfd_netutils.h"
#include <stdio.h>
#include <linux/msm_ion.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>

#define MEDIASRC_AUDIO_BUF_SIZE      32768
#define MEDIASRC_NUM_AUDIO_BUFFERS   16
#define MEDIASRC_FLUSH_IDR_THRESHOLD (2 * 33000)

#define CRITICAL_SECT_ENTER if(mhCritSect)                                    \
                                  MM_CriticalSection_Enter(mhCritSect);       \
/*      END CRITICAL_SECT_ENTER    */

#define CRITICAL_SECT_LEAVE if(mhCritSect)                                    \
                                  MM_CriticalSection_Leave(mhCritSect);       \
/*      END CRITICAL_SECT_LEAVE    */

#define NOTIFYERROR  mpFnHandler((void*)mClientData,                           \
                                 mnModuleId,WFDMMSINK_ERROR,                   \
                                 OMX_ErrorUndefined,                           \
                                 0 );                                          \
/*NOTIFYERROR*/

#define NOTIFYBYTESLOST  mpFnHandler((void*)mClientData,                       \
                                 mnModuleId,WFDMMSINK_PACKETLOSS,              \
                                 OMX_ErrorUndefined,                           \
                                 0 );                                          \
/*NOTIFYERROR*/


#define AAC_SUBSTREAM_HEADER_SIZE 4


/*------------------------------------------------------------------------------
 State definitions
--------------------------------------------------------------------------------
*/
#define DEINIT  0
#define INIT    1
#define OPENED  2
#define CLOSING 4
#define CLOSED  3

#define ISSTATEDEINIT  (state(0, true) == DEINIT )
#define ISSTATEINIT    (state(0, true) == INIT   )
#define ISSTATEOPENED  (state(0, true) == OPENED )
#define ISSTATECLOSED  (state(0, true) == CLOSED )
#define ISSTATECLOSING (state(0, true) == CLOSING)

#define SETSTATECLOSING (state(CLOSING, false))
#define SETSTATEINIT    (state(INIT   , false))
#define SETSTATEDEINIT  (state(DEINIT , false))
#define SETSTATEOPENED  (state(OPENED,  false))

/*==============================================================================

         FUNCTION:         WFDMMSinkMediaSource

         DESCRIPTION:
*//**       @brief         WFDMMSinkMediaSource contructor
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
WFDMMSinkMediaSource::WFDMMSinkMediaSource(int moduleId,
                                           WFDMMSinkHandlerFnType pFnHandler,
                                           WFDMMSinkFBDType       pFnFBD,
                                           avInfoCbType           pFnAVInfo,
                                           int clientData)
{
    InitData();
    /*--------------------------------------------------------------------------

    ----------------------------------------------------------------------------
    */
    mpFnHandler = pFnHandler;
    mpFnFBD     = pFnFBD;
    mnModuleId  = moduleId;
    mClientData = clientData;
    mpFnAVInfoCb = pFnAVInfo;

    mhCritSect = NULL;
    int nRet = MM_CriticalSection_Create(&mhCritSect);
    if(nRet != 0)
    {
        mhCritSect = NULL;
        WFDMMLOGE("Error in Critical Section Create");
    }

    SETSTATEDEINIT;

}

/*==============================================================================

         FUNCTION:         WFDMMSinkMediaSource

         DESCRIPTION:
*//**       @brief         WFDMMSinkMediaSource contructor
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
WFDMMSinkMediaSource::~WFDMMSinkMediaSource()
{
    WFDMMLOGH("WFDMMSinkMediaSource destructor");

    deinitialize();

    if(mhCritSect)
    {
        MM_CriticalSection_Release(mhCritSect);
    }

    WFDMMLOGH("WFDMMSinkMediaSource destructor Ends...");

}


/*==============================================================================

         FUNCTION:          InitData

         DESCRIPTION:
*//**       @brief          Initializes class members
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSinkMediaSource::InitData()
{
    mVideoTrackId = 0;
    mAudioTrackId = 0;
    mpVideoFmtBlk = NULL;
    mnVideoFmtBlkSize = 0;
    mpAudioFmtBlk = NULL;
    mnAudioFmtBlkSize = 0;
    mnAudioMaxBufferSize = 0;
    mnVideoMaxBufferSize = 0;
    mnVideoTimescale = 0;
    mnActualBaseTime = 0;
    mpAudioQ = NULL;
    mpVideoQ = NULL;
    mpVideoThread = NULL;
    mpAudioThread = NULL;
    mpRTPStreamPort = NULL;
    memset(&mCfg, 0, sizeof(mCfg));
    mhCritSect = NULL;
    mpFileSource = NULL;
    meState = DEINIT;
    mnTracks = 0;
    mIonFd = -1;
    mAudioBufFd = -1;
    mAudioBufSize = 0;
    mVideoBufSize = 0;
    mAudioIonHandle = NULL;
    mVideoBufFd = -1;
    mVideoIonHandle = NULL;
    mAudioBufPtr = NULL;
    mVideoBufPtr = NULL;
    mVideoHeapPtr = NULL;
    mpFnHandler = NULL;
    mpFnFBD = NULL;
    mpFnAVInfoCb = NULL;
    mnCurrVideoTime = 0;
    mbMediaTimeSet = false;
    mnVideoFrameDropMode = FRAME_DROP_DROP_NONE;
    mnFlushTimeStamp = 0;
    mbFlushInProgress = false;
    mbPaused = false;
    meIDRTimerStatus = TIMER_RELEASED;
    mbIDRPendingRequest = false;
    mbCancelPendingIDRRequest = false;
    mbSinkIDRRequest = true;
   return;

}

/*==============================================================================

         FUNCTION:         Configure

         DESCRIPTION:
*//**       @brief         Configures the media source
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pCfg        - Configuration Parameters

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkMediaSource::Configure(mediaSourceConfigType *pCfg)
{
    CHECK_ARGS_RET_OMX_ERR_4(pCfg, pCfg->rtpPort, mpFnHandler, mpFnFBD);

    memcpy(&mCfg, pCfg, sizeof(mediaSourceConfigType));

    if(!pCfg->nLocalIP)
    {
        char sIP[24] = {0};
        getLocalIpAddress(sIP, 24);
        if(strlen(sIP) == 0)
        {
            WFDMMLOGE("Failed to get Local IP");
            return OMX_ErrorUndefined;
        }
    }
    SETSTATEINIT;
    if(createResources() != OMX_ErrorNone)
    {
        WFDMMLOGE("Failed to create MediaSrc resources");
        NOTIFYERROR;
        return OMX_ErrorInsufficientResources;
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         createResources

         DESCRIPTION:
*//**       @brief         creates resources for WFDMMSinkMediaSource
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkMediaSource::createResources()
{
    if(!configureDataSource() ||
         !configureParser()       ||
           !configureHDCPResources() ||
             !createThreadsAndQueues() ||
                !allocateAudioBuffers())
    {
        WFDMMLOGE("Media Source Failed to create resources");
        return OMX_ErrorInsufficientResources;
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          allocateAudioBuffers

         DESCRIPTION:
*//**       @brief
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            true or false
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkMediaSource::allocateAudioBuffers()
{
    /*--------------------------------------------------------------------------
     If audio type is LPCM MediaSource allocates Buffers
    ----------------------------------------------------------------------------
    */
#ifdef USE_OMX_AAC_CODEC
    if(mCfg.eAudioFmt == WFD_AUDIO_LPCM)
#endif
    {
        /*----------------------------------------------------------------------
         Allocate buffers to carry PCM data to renderer here
        ------------------------------------------------------------------------
        */
        OMX_BUFFERHEADERTYPE *pBufferHdr;
        for(int i = 0; i < MEDIASRC_NUM_AUDIO_BUFFERS; i++)
        {
            pBufferHdr = (OMX_BUFFERHEADERTYPE*)
                       MM_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
            if(!pBufferHdr)
            {
                WFDMMLOGE("Failed to allocate Audio BufferHdr");
                return false;
            }

            memset(pBufferHdr, 0, sizeof(OMX_BUFFERHEADERTYPE));

            pBufferHdr->pBuffer = (OMX_U8*)MM_Malloc(MEDIASRC_AUDIO_BUF_SIZE);

            if(!pBufferHdr->pBuffer)
            {
                WFDMMLOGE("Failed to allocate Audio Buffers");
                return false;
            }

            pBufferHdr->nAllocLen = MEDIASRC_AUDIO_BUF_SIZE;

            mpAudioQ->Push(&pBufferHdr, sizeof(&pBufferHdr));

        }
    }
    return true;
}


/*==============================================================================

         FUNCTION:          deallocateAudioBuffers

         DESCRIPTION:
*//**       @brief          deallocates Audio Buffers
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkMediaSource::deallocateAudioBuffers()
{
    /*--------------------------------------------------------------------------
     Only for LPCM MediaSource is the allocater of Buffers.
    ----------------------------------------------------------------------------
    */
#ifdef USE_OMX_AAC_CODEC
    if(mCfg.eAudioFmt == WFD_AUDIO_LPCM)
#endif
    {
        OMX_BUFFERHEADERTYPE *pBufferHdr;
        for(int i = 0; i < MEDIASRC_NUM_AUDIO_BUFFERS; i++)
        {
            pBufferHdr = NULL;

            mpAudioQ->Pop(&pBufferHdr, sizeof(&pBufferHdr), 0);

            if(pBufferHdr)
            {
                if(pBufferHdr->pBuffer)
                {
                    MM_Free(pBufferHdr->pBuffer);
                }
                MM_Free(pBufferHdr);
            }
        }
    }

    return true;
}

/*==============================================================================

         FUNCTION:          deallocateIonBufs

         DESCRIPTION:
*//**       @brief          Deallocates ION buffers
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkMediaSource::deallocateIonBufs()
{
    if(mIonFd > 0)
    {
        if(mAudioBufPtr)
        {
            munmap(mAudioBufPtr, mAudioBufSize);
        }

        if(mVideoBufPtr)
        {
            munmap(mVideoBufPtr, mVideoBufSize);
        }

        struct ion_fd_data sFdInfo;
        struct ion_handle_data sIonHandle;
        int32_t ret = 0;

        memset(&sFdInfo, 0, sizeof(sFdInfo));
        memset(&sIonHandle, 0, sizeof(sIonHandle));

        if(mAudioBufFd > 0)
        {
            sFdInfo.fd = mAudioBufFd;

            ret = ioctl(mIonFd, ION_IOC_IMPORT, &sFdInfo);

            if(sFdInfo.handle) {
                sIonHandle.handle = sFdInfo.handle;
                ret = ioctl(mIonFd, ION_IOC_FREE, &sIonHandle);
                if(ret) {
                    ALOGE("WFDSource Failed to free ION buf");
                }
            }

            close(mAudioBufFd);
        }

        if(mVideoBufFd > 0)
        {
            sFdInfo.fd = mVideoBufFd;
            sFdInfo.handle = NULL;

            ret = ioctl(mIonFd, ION_IOC_IMPORT, &sFdInfo);

            if(sFdInfo.handle) {
                sIonHandle.handle = sFdInfo.handle;
                ret = ioctl(mIonFd, ION_IOC_FREE, &sIonHandle);
                if(ret) {
                    ALOGE("WFDSource Failed to free ION buf");
                }
            }
            close(mVideoBufFd);
        }
        close(mIonFd);
    }

    if(mVideoHeapPtr)
    {
        MM_Free(mVideoHeapPtr);
        mVideoHeapPtr = NULL;
    }

    return true;
}


/*==============================================================================

         FUNCTION:         createThreadsAndQueues

         DESCRIPTION:
*//**       @brief         creates threads and queues needed for the module
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           bool
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
bool WFDMMSinkMediaSource::createThreadsAndQueues()
{
    /*--------------------------------------------------------------------------
      Create threads if session has audio and video
    ----------------------------------------------------------------------------
    */
    if(mCfg.bHasVideo)
    {
        mpVideoThread = new WFDMMThreads(1);

        if(!mpVideoThread)
        {
            WFDMMLOGE("Failed to create Video Thread");
            return false;
        }

        mpVideoThread->Start(VideoThreadEntry, -2,
                             32768, (int)this, "WFDSinkVideoSrc");

    }


    if(mCfg.bHasAudio)
    {
        mpAudioThread = new WFDMMThreads(1);

        if(!mpAudioThread)
        {
            WFDMMLOGE("Failed to create Audio Thread");
            return false;
        }

        mpAudioThread->Start(AudioThreadEntry, -2,
                             32768, (int)this, "WFDSinkAudioSrc");

    }

    mpAudioQ = MM_New_Args(SignalQueue, (100, sizeof(int*)));
    mpVideoQ = MM_New_Args(SignalQueue, (100, sizeof(int*)));

    if(!mpAudioQ || !mpVideoQ)
    {
        return false;
    }

    return true;
}



/*==============================================================================

         FUNCTION:         deinitialize

         DESCRIPTION:
*//**       @brief         deallocated all resources
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkMediaSource::deinitialize()
{

    SETSTATECLOSING;

    WFDMMLOGH("Wait for Audio buffers to be back");

    while(
#ifdef USE_OMX_AAC_CODEC
          mCfg.eAudioFmt == WFD_AUDIO_LPCM &&
#endif
          mpAudioQ && mpAudioQ->GetSize() != MEDIASRC_NUM_AUDIO_BUFFERS)
    {
        MM_Timer_Sleep(2);
    }

    WFDMMLOGH("All Audio buffers are back");

    if(mpAudioQ)
    {
        deallocateAudioBuffers();
    }

    /*--------------------------------------------------------------------------
      Close Threads and signal queues
    ----------------------------------------------------------------------------
    */
    if(mpAudioThread)
    {
        MM_Delete(mpAudioThread);
        mpAudioThread = NULL;
    }

    if(mpVideoThread)
    {
        MM_Delete(mpVideoThread);
        mpVideoThread = NULL;
    }
    /*--------------------------------------------------------------------------
      Close filesource first
    ----------------------------------------------------------------------------
    */
    if(mpFileSource != NULL)
    {
        WFDMMLOGH("WFDMMSinkMediaSrc... Filesource close file");
        mpFileSource->CloseFile();
        MM_Delete(mpFileSource);
        mpFileSource = NULL;
    }

    if(mpAudioQ)
    {
        MM_Delete(mpAudioQ);
        mpAudioQ = NULL;
    }

    if(mpVideoQ)
    {
        MM_Delete(mpVideoQ);
        mpVideoQ = NULL;
    }
    deallocateIonBufs();

    return OMX_ErrorNone;

}

/*==============================================================================

         FUNCTION:         VideoThreadEntry

         DESCRIPTION:
*//**       @brief         Static entry function called from WFDMMThread
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
void WFDMMSinkMediaSource::VideoThreadEntry(int pThis,
                                            unsigned int nSignal)
{
    CHECK_ARGS_RET_VOID_1(pThis);

    WFDMMSinkMediaSource *pMe = (WFDMMSinkMediaSource*)pThis;

    pMe->VideoThread(nSignal);
}

/*==============================================================================

         FUNCTION:         VideoThread

         DESCRIPTION:
*//**       @brief         Video Thread for fetching samples from parser
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
void WFDMMSinkMediaSource::VideoThread(unsigned int nSignal)
{
    while(ISSTATEOPENED && !mbPaused)
    {
        if(mCfg.bSecure != true)
        {
            if(fetchVideoSample(mVideoTrackId) != FILE_SOURCE_DATA_OK)
            {
                MM_Timer_Sleep(2);
            }
        }
        else
        {
            if(fetchVideoSampleSecure(mVideoTrackId) != FILE_SOURCE_DATA_OK)
            {
                MM_Timer_Sleep(2);
            }
        }
    }
}

/*==============================================================================

         FUNCTION:         AudioThreadEntry

         DESCRIPTION:
*//**       @brief         Static entry function called from WFDMMThread
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
void WFDMMSinkMediaSource::AudioThreadEntry(int pThis,
                                            unsigned int nSignal)
{
    CHECK_ARGS_RET_VOID_1(pThis);

    WFDMMSinkMediaSource *pMe = (WFDMMSinkMediaSource*)pThis;

    pMe->AudioThread(nSignal);
}

/*==============================================================================

         FUNCTION:         AudioThread

         DESCRIPTION:
*//**       @brief         Audio Thread for fetching samples from parser
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
void WFDMMSinkMediaSource::AudioThread(unsigned int nSignal)
{
    while(ISSTATEOPENED && !mbPaused)
    {
        if(!mCfg.bSecure)
        {
            if(fetchAudioSample(mAudioTrackId)!= FILE_SOURCE_DATA_OK)
            {
                MM_Timer_Sleep(2);
            }
        }
        else
        {
            if(fetchAudioSampleSecure(mAudioTrackId)!= FILE_SOURCE_DATA_OK)
            {
                MM_Timer_Sleep(2);
            }
        }
    }
}

/*==============================================================================

         FUNCTION:         fetchVideoSample

         DESCRIPTION:
*//**       @brief         checks if a video buffer is available and fills data
                           and sends to decoder
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         nTrackId - helps in multiple audio case

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
int WFDMMSinkMediaSource::fetchVideoSample(uint32 nTrackId)
{
    OMX_BUFFERHEADERTYPE *pBuffer = NULL;

    OMX_ERRORTYPE result;

    /*--------------------------------------------------------------------------
     * This pending IDR request is only set when the IDRDelayTimer expires.
     * If we receive the notification to fire this pending IDR request,
     * do so immediately from VideoThread context iff it has not been flagged
     * for cancellation (due to arrival of an IDR-Frame in the meanwhile)
     ---------------------------------------------------------------------------
     */
    if(mbIDRPendingRequest)
    {
        mbIDRPendingRequest = false;

        if(!mbCancelPendingIDRRequest)
        {
            NOTIFYBYTESLOST;
        }
    }

    /*--------------------------------------------------------------------------
      Check if there are any buffers returned back by the decoder
    ----------------------------------------------------------------------------
    */
    result = mpVideoQ->Pop(&pBuffer,
                          sizeof(pBuffer),
                          3);

    if(result != OMX_ErrorNone)
    {
        WFDMMLOGM("No Buffer available for Video. Continue");
        return FILE_SOURCE_DATA_INSUFFICIENT;
    }

    /*--------------------------------------------------------------------------
     Now we have an empty Buffer. Pull data from parser if available
    ----------------------------------------------------------------------------
    */
    FileSourceSampleInfo  sampleInfo;
    FileSourceMediaStatus mediaStatus = FILE_SOURCE_DATA_OK;
    uint32 nSize = pBuffer->nAllocLen;
    mediaStatus = mpFileSource->GetNextMediaSample
                                                 (nTrackId,
                                                  pBuffer->pBuffer,
                                                  &nSize,
                                                  sampleInfo);

#ifdef BLOCK_FILLER_NALU
    if(mediaStatus == FILE_SOURCE_DATA_OK && pBuffer->pBuffer[4] == 0xc)
    {
        uint32 nSize = pBuffer->nAllocLen;
        mediaStatus = mpFileSource->GetNextMediaSample
                                                     (nTrackId,
                                                      pBuffer->pBuffer,
                                                      &nSize,
                                                      sampleInfo);
    }
#endif

    switch(mediaStatus)
    {
        case FILE_SOURCE_DATA_OK:

            if(mbSinkIDRRequest)
            {
                /*--------------------------------------------------------------
                 * Called the first time a sample is received. i.e after initial
                 * play and then after every resumption from pause.
                 ---------------------------------------------------------------
                */

                mbSinkIDRRequest = false;
                /*--------------------------------------------------------------
                 * Handles the case where source itself sends IDR after every
                 * resumption of session. Also takes care of the IDR frame after
                 * the initial play of session.
                 ---------------------------------------------------------------
                 */
                if(sampleInfo.sync)
                {
                    mnVideoFrameDropMode = FRAME_DROP_DROP_NONE;
                }
                else
                {
                    mnVideoFrameDropMode = FRAME_DROP_DROP_TILL_IDR;
                    NOTIFYBYTESLOST;
                }
            }

            /*------------------------------------------------------------------
             * If an IDRDelayTimer is running on Sink, check if we have
             * received a non-corrupted IDR-Frame. If so, then there is no
             * need for a pending IDR request once the timer expires.
             -------------------------------------------------------------------
             */
            if(meIDRTimerStatus == TIMER_CREATED)
            {
                if(sampleInfo.sync && !sampleInfo.nBytesLost)
                {
                    mbCancelPendingIDRRequest = true;
                }
            }

            /*------------------------------------------------------------------
             * If paser is reporting byte-loss in packets, notify this info
             * to Sink to take appropriate action. Also keep the pending IDR
             * requested updated, since we have byte loss after an IDR frame
             * reception.
             -------------------------------------------------------------------
            */
            if(sampleInfo.nBytesLost)
            {
                NOTIFYBYTESLOST;
                mbCancelPendingIDRRequest = false;
            }

            /*------------------------------------------------------------------
              Check if we are recovering from frame loss or there are packet
              loss
            --------------------------------------------------------------------
            */
            pBuffer->nOffset = 0;
            pBuffer->nFlags  = 0;

            if(mnVideoFrameDropMode == FRAME_DROP_DROP_TILL_IDR)
            {
                if(sampleInfo.sync)
                {
                    WFDMMLOGE("Received IDR Frame. Stop dropping frames");
                    mnVideoFrameDropMode = FRAME_DROP_DROP_NONE;
                }
                else
                {
                    WFDMMLOGE("Dropping frame till IDR");
                    mpVideoQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
                    break;
                }
            }
            else if(sampleInfo.nBytesLost)
            {
                WFDMMLOGH("Parser reports Packet Loss");
                if(mnVideoFrameDropMode == FRAME_DROP_DROP_CURRENT_FRAME)
                {
                     WFDMMLOGE("Bytes Lost: Drop current frame");
                     mpVideoQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
                     break;
                }
                else
                {
                     WFDMMLOGE("Bytes Lost: Indicate corrupt frame");
                     pBuffer->nFlags |= OMX_BUFFERFLAG_DATACORRUPT;
                }
            }

            if(mbFlushInProgress)
            {
                if((uint64)sampleInfo.startTime * 1000 <= mnFlushTimeStamp)
                {
                    if(mnFlushTimeStamp - (uint64)sampleInfo.startTime * 1000
                        < MEDIASRC_FLUSH_IDR_THRESHOLD &&
                                   sampleInfo.sync)
                    {
                        mbFlushInProgress = false;
                    }
                    else
                    {
                        mpVideoQ->Push(&pBuffer,sizeof(OMX_BUFFERHEADERTYPE**));
                        break;
                    }
                }
                else if(sampleInfo.sync)
                {
                    mbFlushInProgress = false;
                }
            }


            /*------------------------------------------------------------------
              Set the media base time once
            --------------------------------------------------------------------
            */
            pBuffer->nTimeStamp = (OMX_TICKS)(sampleInfo.startTime * 1000);

            if(!mbMediaTimeSet)
            {
                SetMediaBaseTime((uint64)pBuffer->nTimeStamp /*us*/);
                mbMediaTimeSet = true;
            }
            if(mnCurrVideoTime && pBuffer->nTimeStamp == mnCurrVideoTime)
            {
                pBuffer->nTimeStamp += 1;
            }
            mnCurrVideoTime = pBuffer->nTimeStamp;
            WFDMMLOGD2("H264 Sample %x          %x",
                       pBuffer->pBuffer[0], pBuffer->pBuffer[1]);
            WFDMMLOGD2("H264 Sample %x          %x",
                       pBuffer->pBuffer[2], pBuffer->pBuffer[3]);
            WFDMMLOGD2("H264 Sample %x          %x",
                       pBuffer->pBuffer[4], pBuffer->pBuffer[5]);
            WFDMMLOGD2("H264 Sample %x          %x",
                       pBuffer->pBuffer[6], pBuffer->pBuffer[7]);
            pBuffer->nOffset = 0;
            pBuffer->nFlags = 0;

            pBuffer->nFilledLen = nSize;

            if(sampleInfo.sync)
            {
                pBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
            }

            pBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;



            if(mpFnFBD)
            {
                mpFnFBD(mClientData, mnModuleId, SINK_VIDEO_TRACK_ID, pBuffer);
            }
            break;
        case FILE_SOURCE_DATA_UNDERRUN:
        case FILE_SOURCE_DATA_INSUFFICIENT:
            WFDMMLOGH("FILE_SOURCE_DATA_UNDERRUN");
            mpVideoQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
            return FILE_SOURCE_DATA_UNDERRUN;
        case FILE_SOURCE_DATA_END:
        case FILE_SOURCE_DATA_ERROR:
        default:
            mpVideoQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
            return FILE_SOURCE_DATA_ERROR;
    }
    return FILE_SOURCE_DATA_OK;
}


/*==============================================================================

         FUNCTION:         fetchVideoSampleSecure

         DESCRIPTION:
*//**       @brief         checks if a video buffer is available and fills data
                           and sends to decoder
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         nTrackId - helps in multiple audio case

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
int WFDMMSinkMediaSource::fetchVideoSampleSecure(uint32 nTrackId)
{
    OMX_BUFFERHEADERTYPE *pBuffer = NULL;

    OMX_ERRORTYPE result;

    /*--------------------------------------------------------------------------
     * This pending IDR request is only set when the IDRDelayTimer expires.
     * If we receive the notification to fire this pending IDR request,
     * do so immediately from VideoThread context iff it has not been flagged
     * for cancellation (due to arrival of an IDR-Frame in the meanwhile)
     ---------------------------------------------------------------------------
     */
    if(mbIDRPendingRequest)
    {
        mbIDRPendingRequest = false;

        if(!mbCancelPendingIDRRequest)
        {
            NOTIFYBYTESLOST;
        }
    }

    if(mCfg.pFnDecrypt == NULL)
    {
        WFDMMLOGE("No function to decrypt");
        return -1;
    }

    if(!mVideoBufPtr)
    {
        WFDMMLOGE("No Local Buffer for decrypt");
        return -1;
    }

    /*--------------------------------------------------------------------------
      Check if there are any buffers returned back by the decoder
    ----------------------------------------------------------------------------
    */
    result = mpVideoQ->Pop(&pBuffer,
                          sizeof(pBuffer),
                          3);

    if(result != OMX_ErrorNone)
    {
        WFDMMLOGM("No Buffer available for Video. Continue");
        return FILE_SOURCE_DATA_INSUFFICIENT;
    }

    /*--------------------------------------------------------------------------
     Now we have an empty Buffer. Pull data from parser if available
    ----------------------------------------------------------------------------
    */
    FileSourceSampleInfo  sampleInfo;
    FileSourceMediaStatus mediaStatus = FILE_SOURCE_DATA_OK;
    uint32 nSize = mVideoBufSize;
    uint8 *pDataPtr = mVideoHeapPtr ? mVideoHeapPtr : mVideoBufPtr;
    mediaStatus = mpFileSource->GetNextMediaSample
                                                 (nTrackId,
                                                  pDataPtr,
                                                  &nSize,
                                                  sampleInfo);


#ifdef BLOCK_FILLER_NALU

    /**Warning for use. When data is secure this may lead to actual
     * frames being dropped */
    if(mediaStatus == FILE_SOURCE_DATA_OK && nSize == 8)
    {
        uint32 nSize = pBuffer->nAllocLen;
        mediaStatus = mpFileSource->GetNextMediaSample
                                                     (nTrackId,
                                                      pDataPtr,
                                                      &nSize,
                                                      sampleInfo);
    }
#endif

    switch(mediaStatus)
    {
        case FILE_SOURCE_DATA_OK:

            if(mbSinkIDRRequest)
            {
                /*--------------------------------------------------------------
                 * Called the first time a sample is received. i.e after initial
                 * play and then after every resumption from pause.
                 ---------------------------------------------------------------
                */

                mbSinkIDRRequest = false;
                /*--------------------------------------------------------------
                 * Handles the case where source itself sends IDR after every
                 * resumption of session. Also takes care of the IDR frame after
                 * the initial play of session.
                 ---------------------------------------------------------------
                 */
                if(sampleInfo.sync)
                {
                    mnVideoFrameDropMode = FRAME_DROP_DROP_NONE;
                }
                else
                {
                    mnVideoFrameDropMode = FRAME_DROP_DROP_TILL_IDR;
                    NOTIFYBYTESLOST;
                }
            }

            /*------------------------------------------------------------------
             * If an IDRDelayTimer is running on Sink, check if we have
             * received a non-corrupted IDR-Frame. If so, then there is no
             * need for a pending IDR request once the timer expires.
             -------------------------------------------------------------------
             */
            if(meIDRTimerStatus == TIMER_CREATED)
            {
                if(sampleInfo.sync && !sampleInfo.nBytesLost)
                {
                    mbCancelPendingIDRRequest = true;
                }
            }

            /*------------------------------------------------------------------
             * If paser is reporting byte-loss in packets, notify this info
             * to Sink to take appropriate action. Also keep the pending IDR
             * requested updated, since we have byte loss after an IDR frame
             * reception.
             -------------------------------------------------------------------
            */
            if(sampleInfo.nBytesLost)
            {
                NOTIFYBYTESLOST;
                mbCancelPendingIDRRequest = false;
            }

            /*------------------------------------------------------------------
              Check if we are recovering from frame loss or there are packet
              loss
            --------------------------------------------------------------------
            */
            pBuffer->nOffset = 0;
            pBuffer->nFlags  = 0;

            if(mnVideoFrameDropMode == FRAME_DROP_DROP_TILL_IDR)
            {
                if(sampleInfo.sync)
                {
                    WFDMMLOGE("Received IDR Frame. Stop dropping frames");
                    mnVideoFrameDropMode = FRAME_DROP_DROP_NONE;
                }
                else
                {
                    WFDMMLOGE("Dropping frame till IDR");
                    mpVideoQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
                    break;
                }
            }
            else if(sampleInfo.nBytesLost)
            {
                WFDMMLOGH("Parser reports Packet Loss");
                if(mnVideoFrameDropMode == FRAME_DROP_DROP_CURRENT_FRAME)
                {
                     WFDMMLOGE("Bytes Lost: Drop current frame");
                     mpVideoQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
                     break;
                }
                else
                {
                     WFDMMLOGE("Bytes Lost: Indicate corrupt frame");
                     pBuffer->nFlags |= OMX_BUFFERFLAG_DATACORRUPT;
                }
            }

            if(mbFlushInProgress)
            {
                if((uint64)sampleInfo.startTime * 1000 <= mnFlushTimeStamp)
                {
                    if(mnFlushTimeStamp - (uint64)sampleInfo.startTime * 1000
                        < MEDIASRC_FLUSH_IDR_THRESHOLD &&
                                   sampleInfo.sync)
                    {
                        mbFlushInProgress = false;
                    }
                    else
                    {
                        mpVideoQ->Push(&pBuffer,sizeof(OMX_BUFFERHEADERTYPE**));
                        break;
                    }
                }
                else if(sampleInfo.sync)
                {
                    mbFlushInProgress = false;
                }
            }
            /*------------------------------------------------------------------
              Set the media base time once
            --------------------------------------------------------------------
            */
            pBuffer->nTimeStamp = (OMX_TICKS)(sampleInfo.startTime * 1000);

            if(!mbMediaTimeSet)
            {
                SetMediaBaseTime((uint64)pBuffer->nTimeStamp /*us*/);
                mbMediaTimeSet = true;
            }

            if(mVideoHeapPtr)
            {
                memcpy(mVideoBufPtr, pDataPtr, nSize);
            }

            /*------------------------------------------------------------------
              Caching is enabled for QSECOM buffer. writeback and invalidate
            --------------------------------------------------------------------
            */
            struct ion_flush_data sFlushData;
            sFlushData.handle = (ion_user_handle_t)mVideoIonHandle;
            sFlushData.fd = 0;
            sFlushData.offset = 0;
            sFlushData.vaddr = mVideoBufPtr;
            sFlushData.length = nSize;

            if(ioctl (mIonFd, ION_IOC_CLEAN_INV_CACHES, &sFlushData) < 0)
            {
                WFDMMLOGE("Cache flush failed");
            }

            WFDMMLOGD2("Decrypt %d %lu", (int)mVideoBufFd,
                                                    (uint32)pBuffer->pBuffer);
            if(mCfg.pFnDecrypt(mClientData,SINK_VIDEO_TRACK_ID, mVideoBufFd,
                               (int)pBuffer->pBuffer, nSize,
                        (char*)sampleInfo.sSubInfo.sCP.ucInitVector,
                        sampleInfo.sSubInfo.sCP.ucInitVectorSize)!= true)
            {
                WFDMMLOGE("Failed to decrypt Payload");
                mpFnHandler((void*)mClientData, mnModuleId, WFDMMSINK_DECRYPT_FAILURE,
                            OMX_ErrorHardware, 0);
                nSize = 0;
            }

            pBuffer->nOffset = 0;
            pBuffer->nFlags = 0;

            pBuffer->nFilledLen = nSize;

            if(sampleInfo.sync)
            {
                pBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
            }

            pBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

            if(mpFnFBD)
            {
                mpFnFBD(mClientData, mnModuleId, SINK_VIDEO_TRACK_ID, pBuffer);
            }
            break;
        case FILE_SOURCE_DATA_UNDERRUN:
        case FILE_SOURCE_DATA_INSUFFICIENT:
            WFDMMLOGH("FILE_SOURCE_DATA_UNDERRUN");
            mpVideoQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
            return FILE_SOURCE_DATA_UNDERRUN;
        case FILE_SOURCE_DATA_END:
        case FILE_SOURCE_DATA_ERROR:
        default:
            mpVideoQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
            return FILE_SOURCE_DATA_ERROR;
    }
    return FILE_SOURCE_DATA_OK;
}



/*==============================================================================

         FUNCTION:         fetchAudioSample

         DESCRIPTION:
*//**       @brief         checks if a audio buffer is available and fills data
                           and sends to decoder
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         nTrackId - helps in multiple audio case

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
int WFDMMSinkMediaSource::fetchAudioSample(uint32 nTrackId)
{
    OMX_BUFFERHEADERTYPE *pBuffer = NULL;

    OMX_ERRORTYPE result;

    WFDMMLOGH("FetchAudioSampe");
    /*--------------------------------------------------------------------------
      Check if there are any buffers returned back by the decoder
    ----------------------------------------------------------------------------
    */
    result = mpAudioQ->Pop(&pBuffer,
                          sizeof(pBuffer),
                          3);

    if(result != OMX_ErrorNone || !pBuffer)
    {
        WFDMMLOGM("No Buffer available for Audio. Continue");
        return FILE_SOURCE_DATA_INSUFFICIENT;
    }

    /*--------------------------------------------------------------------------
     Now we have an empty Buffer. Pull data from parser if available
    ----------------------------------------------------------------------------
    */
    FileSourceSampleInfo  sampleInfo;
    FileSourceMediaStatus mediaStatus = FILE_SOURCE_DATA_OK;
    pBuffer->nFilledLen = pBuffer->nAllocLen;
    mediaStatus = mpFileSource->GetNextMediaSample
                                                 (nTrackId,
                                                  pBuffer->pBuffer,
                                                 (uint32*)&pBuffer->nFilledLen,
                                                  sampleInfo);



    switch(mediaStatus)
    {
        case FILE_SOURCE_DATA_OK:
            /*------------------------------------------------------------------
              Set the media base time once
            --------------------------------------------------------------------
            */
            pBuffer->nTimeStamp = ((OMX_TICKS)(sampleInfo.startTime * 1000));
            pBuffer->nOffset = 0;
            pBuffer->nFlags  = 0;
            if(!mbMediaTimeSet)
            {
                SetMediaBaseTime((uint64)pBuffer->nTimeStamp/*us*/);
                mbMediaTimeSet = true;
            }
#if 0
            if(mCfg.eAudioFmt == WFD_AUDIO_LPCM)
            {
                uint16 *pSrc = (uint16*)(pBuffer->pBuffer + 4);
                uint16 *pDst = (uint16*)(pBuffer->pBuffer);
                uint16 nVal;

                for(uint32 i = 0; i < (pBuffer->nFilledLen - 4)/2 ; i++)
                {
                    nVal = *pSrc++;

                    *pDst++ = (nVal >> 8) | (nVal << 8);

                }
                pBuffer->nFilledLen -= 4;
            }
#endif
            if(mbFlushInProgress)
            {
                if((uint64)sampleInfo.startTime * 1000 <= mnFlushTimeStamp)
                {
                     mpAudioQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
                     break;
                }
                else
                {
                     mbFlushInProgress = false;
                }
            }
            if(mpFnFBD)
            {
                mpFnFBD(mClientData, mnModuleId, SINK_AUDIO_TRACK_ID, pBuffer);
            }
            break;
        case FILE_SOURCE_DATA_UNDERRUN:
        case FILE_SOURCE_DATA_INSUFFICIENT:
            WFDMMLOGH("FILE_SOURCE_DATA_UNDERRUN");
            mpAudioQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
            return FILE_SOURCE_DATA_UNDERRUN;
        case FILE_SOURCE_DATA_END:
        case FILE_SOURCE_DATA_ERROR:
        default:
            mpAudioQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
            return FILE_SOURCE_DATA_ERROR;
    }
    return FILE_SOURCE_DATA_OK;

}

/*==============================================================================

         FUNCTION:         fetchAudioSampleSecure

         DESCRIPTION:
*//**       @brief         checks if a audio buffer is available and fills data
                           decrypts it and sends to decoder
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         nTrackId - helps in multiple audio case

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
int WFDMMSinkMediaSource::fetchAudioSampleSecure(uint32 nTrackId)
{
    OMX_BUFFERHEADERTYPE *pBuffer = NULL;

    OMX_ERRORTYPE result;

    WFDMMLOGH("FetchAudioSampe");
    /*--------------------------------------------------------------------------
      Check if there are any buffers returned back by the decoder
    ----------------------------------------------------------------------------
    */
    result = mpAudioQ->Pop(&pBuffer,
                          sizeof(pBuffer),
                          3);

    if(result != OMX_ErrorNone || !pBuffer)
    {
        WFDMMLOGM("No Buffer available for Audio. Continue");
        return FILE_SOURCE_DATA_ERROR;
    }

    if(!mAudioBufPtr)
    {
        WFDMMLOGE("No ION working Buf for audio");
        return FILE_SOURCE_DATA_ERROR;
    }
    /*--------------------------------------------------------------------------
     Now we have an empty Buffer. Pull data from parser if available
    ----------------------------------------------------------------------------
    */
    FileSourceSampleInfo  sampleInfo;
    FileSourceMediaStatus mediaStatus = FILE_SOURCE_DATA_OK;
    pBuffer->nFilledLen = pBuffer->nAllocLen;
    mediaStatus = mpFileSource->GetNextMediaSample(nTrackId,
                                                   mAudioBufPtr,
                                         (uint32*)&pBuffer->nFilledLen,
                                                   sampleInfo);



    switch(mediaStatus)
    {
        case FILE_SOURCE_DATA_OK:
            /*------------------------------------------------------------------
              Set the media base time once
            --------------------------------------------------------------------
            */
            pBuffer->nTimeStamp = ((OMX_TICKS)(sampleInfo.startTime * 1000));
            pBuffer->nOffset = 0;
            pBuffer->nFlags  = 0;
            if(!mbMediaTimeSet)
            {
                SetMediaBaseTime((uint64)pBuffer->nTimeStamp/*us*/);
                mbMediaTimeSet = true;
            }


            if(sampleInfo.sSubInfo.sCP.ucInitVectorSize)
            {
                /*--------------------------------------------------------------
                  Caching is enabled for QSECOM buffer. writeback and invalidate
                ----------------------------------------------------------------
                */
                struct ion_flush_data sFlushData;
                sFlushData.handle = (ion_user_handle_t)mAudioIonHandle;
                sFlushData.fd = 0;
                sFlushData.offset = 0;
                sFlushData.vaddr = mAudioBufPtr;
                sFlushData.length = pBuffer->nFilledLen;

                if(ioctl (mIonFd, ION_IOC_CLEAN_INV_CACHES, &sFlushData) < 0)
                {
                    WFDMMLOGE("Cache flush failed");
                }

                WFDMMLOGD2("Decrypt Audio %d %lu", (int)mAudioBufFd,
                                                    (uint32)mAudioBufFd);

                if(mCfg.pFnDecrypt(mClientData, SINK_AUDIO_TRACK_ID,
                            mAudioBufFd,
                            mAudioBufFd,
                            pBuffer->nFilledLen,
                           (char*)sampleInfo.sSubInfo.sCP.ucInitVector,
                            sampleInfo.sSubInfo.sCP.ucInitVectorSize)!= true)
                {
                    WFDMMLOGE("Failed to decrypt Payload");
                    mpFnHandler((void*)mClientData, mnModuleId,
                                WFDMMSINK_DECRYPT_FAILURE,
                                OMX_ErrorHardware, 0);
                    mpAudioQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
                    return -1;
                }
                if(mCfg.eAudioFmt == WFD_AUDIO_LPCM)
                {
                    uint16 *pSrc =
                           (uint16*)(mAudioBufPtr + AAC_SUBSTREAM_HEADER_SIZE);

                    uint16 *pDst = (uint16*)(pBuffer->pBuffer);
                    uint16 nVal;

                    /*----------------------------------------------------------
                     Remove substream ID and SWAP the bytes to make it
                     Signed LPCM 16bit LE
                    ------------------------------------------------------------
                    */
                    for(uint32 i = 0;
                        i < (pBuffer->nFilledLen - AAC_SUBSTREAM_HEADER_SIZE)/2;
                        i++)
                    {
                        nVal    = *pSrc++;
                        *pDst++ = (nVal >> 8) | (nVal << 8);
                    }
                    pBuffer->nFilledLen -= AAC_SUBSTREAM_HEADER_SIZE;
                }
                else
                {
                    memcpy(pBuffer->pBuffer, mAudioBufPtr, pBuffer->nFilledLen);
                }
            }
            else
            {
                if(mCfg.eAudioFmt == WFD_AUDIO_LPCM)
                {
                    memcpy(pBuffer->pBuffer,
                           mAudioBufPtr + AAC_SUBSTREAM_HEADER_SIZE,
                           pBuffer->nFilledLen - AAC_SUBSTREAM_HEADER_SIZE);

                    pBuffer->nFilledLen -= AAC_SUBSTREAM_HEADER_SIZE;
                }
                else
                {
                    memcpy(pBuffer->pBuffer, mAudioBufPtr, pBuffer->nFilledLen);
                }
            }
            if(mpFnFBD)
            {
                mpFnFBD(mClientData, mnModuleId, SINK_AUDIO_TRACK_ID, pBuffer);
            }
            break;
        case FILE_SOURCE_DATA_UNDERRUN:
        case FILE_SOURCE_DATA_INSUFFICIENT:
            WFDMMLOGH("FILE_SOURCE_DATA_UNDERRUN");
            mpAudioQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
            return FILE_SOURCE_DATA_UNDERRUN;
        case FILE_SOURCE_DATA_END:
        case FILE_SOURCE_DATA_ERROR:
        default:
            mpAudioQ->Push(&pBuffer, sizeof(OMX_BUFFERHEADERTYPE**));
            return FILE_SOURCE_DATA_ERROR;
    }
    return FILE_SOURCE_DATA_OK;

}

/*==============================================================================

         FUNCTION:          setFreeBuffer

         DESCRIPTION:
*//**       @brief          gives the empty buffer back to media source
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkMediaSource::setFreeBuffer
(
    int trackId,
    OMX_BUFFERHEADERTYPE *pBuffer
)
{
    if(pBuffer)
    {
        if(trackId == SINK_AUDIO_TRACK_ID)
        {
            mpAudioQ->Push(&pBuffer, sizeof(&pBuffer));
        }
        else if(trackId == SINK_VIDEO_TRACK_ID)
        {
            mpVideoQ->Push(&pBuffer, sizeof(&pBuffer));
        }
    }

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         configureDataSource

         DESCRIPTION:
*//**       @brief         configures the RTP modue
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           bool - true for success
                                - false for failure
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
bool WFDMMSinkMediaSource::configureDataSource()
{
    mpRTPStreamPort = mCfg.pRTPStreamPort;
    mpRTPStreamPort->Start();
    return true;

}

/*==============================================================================

         FUNCTION:         configureHDCPResources

         DESCRIPTION:
*//**       @brief         configures the HDCP related resources
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           bool - true for success
                                - false for failure
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
bool WFDMMSinkMediaSource::configureHDCPResources()
{
    AllocateIonBufs();
    return true;
}

/*==============================================================================

         FUNCTION:         configureParser

         DESCRIPTION:
*//**       @brief         configures the Parser
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           bool - true for success
                                - false for failure
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
bool WFDMMSinkMediaSource::configureParser()
{
    if(!mpRTPStreamPort)
    {
        WFDMMLOGE("RTPStreamPort Not ready");
        return false;
    }

    mpFileSource = MM_New_Args (FileSource, (fileSourceCallback, this));

    if(!mpFileSource)
    {
        WFDMMLOGE("Failed to create Parser Instance");
        return false;
    }

    WFDMMLOGH("Calling MMParser OPENFILE");

    if(mpFileSource->OpenFile((video::iStreamPort*)mpRTPStreamPort,
                              FILE_SOURCE_WFD_MP2TS,
                              false) != FILE_SOURCE_SUCCESS)
    {
        WFDMMLOGE("Failed to FileSource->OpenFile");
        return false;
    }

    WFDMMLOGH("Initialized Parser");
    return true;
}


/*==============================================================================

         FUNCTION:         fileSourceCallback

         DESCRIPTION:
*//**       @brief         static function that receives filesource callback
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         status - Filesource status
                           pThis  - pointer to the current object

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
void WFDMMSinkMediaSource::fileSourceCallback
(
    FileSourceCallBackStatus status,
    void *pThis
)
{
    WFDMMSinkMediaSource *pMe = (WFDMMSinkMediaSource*)pThis;
    if(pMe)
    {
        pMe->fileSourceCallbackHandler(status);
    }
    else
    {
        /*----------------------------------------------------------------------
          Invalid this pointer.
        ------------------------------------------------------------------------
        */
    }
}

/*==============================================================================

         FUNCTION:         fileSourceCallbackHandler

         DESCRIPTION:
*//**       @brief         Handles filesource callback
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         status: Filesource status

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
void WFDMMSinkMediaSource::fileSourceCallbackHandler
(
    FileSourceCallBackStatus status
)
{
    WFDMMLOGH1("Parser callback: status = %d", status);
    switch(status)
    {
    case FILE_SOURCE_OPEN_COMPLETE:
        {
            /*------------------------------------------------------------------
             File source has succesfully opened the file. Find the tracks,
             create any dynamic resources pending and trigger data fetch.
            --------------------------------------------------------------------
            */
            FileSourceTrackIdInfoType trackList[12];
            int32 numTracks = mpFileSource->GetWholeTracksIDList(trackList);

            WFDMMLOGH1("Number of detected tracks %ld", numTracks);

            for(int i = 0; i < numTracks; i++)
            {
                FileSourceMjMediaType majorType;
                FileSourceMnMediaType minorType;
                const char* pMimeType =NULL;
                FileSourceStatus status = mpFileSource->GetMimeType
                                                   (trackList[i].id, majorType,
                                                    minorType);
         /*       if(status == FILE_SOURCE_SUCCESS)
                {
                    pMimeType = MediaType2MIME(minorType);
                    if (NULL!= pMimeType)
                    {
                        WFDMMLOGH1("Parser reports Mime Type %s", pMimeType);
                    }
                    else
                    {
                        WFDMMLOGE1("Parser reports unsupported minorType %d",
                                  minorType);
                        continue;
                    }
                }
                else
                {
                    WFDMMLOGE1("Failed in GetMimeType %d", status);
                    continue;
                }*/


                MediaTrackInfo mediaTrackInfo;
                status = mpFileSource->GetMediaTrackInfo(trackList[i].id,
                                                         &mediaTrackInfo);

                if(status != FILE_SOURCE_SUCCESS)
                {
                    WFDMMLOGE("Failed to get MediaTrackInfo");
                    continue;
                }

                if(FILE_SOURCE_MJ_TYPE_VIDEO == trackList[i].majorType)
                {

                    mVideoTrackId = trackList[i].id;
                    mnVideoTimescale = mediaTrackInfo.videoTrackInfo.timeScale;

                    WFDMMLOGH2("Found Video Track height = %lu width = %lu",
                                mediaTrackInfo.videoTrackInfo.frameHeight,
                                mediaTrackInfo.videoTrackInfo.frameWidth);

                    //Get the size of the format block.
                    status = mpFileSource->GetFormatBlock(mVideoTrackId,
                                                          NULL,
                                                          &mnVideoFmtBlkSize);

                    WFDMMLOGH1("VideoFormat Size = %lu", mnVideoFmtBlkSize);

                    if((status==FILE_SOURCE_SUCCESS) && mnVideoFmtBlkSize!=0)
                    {
                        mpVideoFmtBlk = (uint8 *)MM_Malloc(mnVideoFmtBlkSize);

                        if(!mpVideoFmtBlk)
                        {
                            WFDMMLOGE("Failed to allocate FormatBlock");
                            continue;
                        }

                        status = mpFileSource->GetFormatBlock
                                                        (mVideoTrackId,
                                                         mpVideoFmtBlk,
                                                        &mnVideoFmtBlkSize);

                        if(status != FILE_SOURCE_SUCCESS)
                        {
                            WFDMMLOGE("GetFormatBlock failed");
                        }
                    }

                    ++mnTracks;
                    if(mpVideoThread)
                    {
                        mpVideoThread->SetSignal(0);
                    }
                }
                else if(FILE_SOURCE_MJ_TYPE_AUDIO == trackList[i].majorType)
                {
                    WFDMMLOGH1("Found Audio Track %lu", trackList[i].id);
                    mAudioTrackId = trackList[i].id;

                    mnAudioMaxBufferSize =
                       mpFileSource->GetTrackMaxFrameBufferSize(
                                                      trackList[i].id);

                    WFDMMLOGH4("Channels-%d SampleRate-%d BitRate-%d MaxBufSiz-%d",
                           (int)mediaTrackInfo.audioTrackInfo.numChannels,
                           (int)mediaTrackInfo.audioTrackInfo.samplingRate,
                           (int)mediaTrackInfo.audioTrackInfo.bitRate,
                           (int)mnAudioMaxBufferSize);


                    if(minorType == FILE_SOURCE_MN_TYPE_AAC )
                    {
                        status = mpFileSource->GetFormatBlock(mAudioTrackId,
                                                             NULL,
                                                           &mnAudioFmtBlkSize);
                        if((status == FILE_SOURCE_SUCCESS)
                                        && mnAudioFmtBlkSize!=0)
                        {
                            mpAudioFmtBlk =
                                       (uint8 *)MM_Malloc(mnAudioFmtBlkSize);
                            if(!mpAudioFmtBlk)
                            {
                                WFDMMLOGE("Failed to allocate FormatBlock");
                                continue;
                            }
                            status = mpFileSource->GetFormatBlock
                                                          (mAudioTrackId,
                                                           mpAudioFmtBlk,
                                                          &mnAudioFmtBlkSize);

                            if(status != FILE_SOURCE_SUCCESS)
                            {
                                WFDMMLOGE("GetFormatBlock failed");
                            }
                        }
                    }
                    else if(minorType == FILE_SOURCE_MN_TYPE_PCM &&
                      !mCfg.bSecure)
                    {
                        FileSourceConfigItem sItem;
                        mpFileSource->SetConfiguration(
                                       mAudioTrackId,
                                       &sItem,
                                       FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER);
                    }
                    else if(minorType == FILE_SOURCE_MN_TYPE_AC3)
                    {

                    }

                    ++mnTracks;

                    if(mpAudioThread)
                    {
                        mpAudioThread->SetSignal(0);
                    }
                }
            }
            FileSourceConfigItem sItem;
            mpFileSource->GetConfiguration(
                           mAudioTrackId,
                           &sItem,
                           FILE_SOURCE_MEDIA_BASETIME);
            mnActualBaseTime = sItem.nresult * 1000;
            WFDMMLOGH1("MediaSource Actual Base Time from parser %llu",
                       mnActualBaseTime);
        }
        break;
    case FILE_SOURCE_OPEN_FAIL:
            /*------------------------------------------------------------------
             Parser failed to find valid tracks in the stream. Cannot recover
             Give runtime Error.
            --------------------------------------------------------------------
            */
        WFDMMLOGE("MediaSource: Filesource failed to open file");
        NOTIFYERROR;
        break;
    case FILE_SOURCE_SEEK_COMPLETE:
            /*------------------------------------------------------------------
             Not of much intereset
            --------------------------------------------------------------------
            */
        break;
    case FILE_SOURCE_SEEK_UNDERRUN:
        break;
    case FILE_SOURCE_SEEK_FAIL:
        /*----------------------------------------------------------------------
          Who seeks in Wifi display
        ------------------------------------------------------------------------
        */
    case FILE_SOURCE_ERROR_ABORT:
    case FILE_SOURCE_SEEK_UNDERRUN_IN_FRAGMENT:
    default:
        /*----------------------------------------------------------------------
         Dont know what to do here :(. Give runtime error
        ------------------------------------------------------------------------
        */
        break;
    }
}

int WFDMMSinkMediaSource::state(int state, bool get_true_set_false)
{
    CRITICAL_SECT_ENTER;

    if(get_true_set_false == true)
    {
        /*----------------------------------------------------------------------
         Means get
        ------------------------------------------------------------------------
        */
    }
    else
    {
        meState = state;
    }

    CRITICAL_SECT_LEAVE;

    return meState;
}

/*==============================================================================

         FUNCTION:          Start

         DESCRIPTION:
*//**       @brief          Starts the media Source thread
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkMediaSource::Start()
{
    if(ISSTATEINIT)
    {
        meState = OPENED;

        if(mnTracks > 0)
        {
            if(mpVideoThread)
            {
                mpVideoThread->SetSignal(0);
            }
            if(mpAudioThread)
            {
                mpAudioThread->SetSignal(0);
            }
        }
    }
    else
    {
        WFDMMLOGE1("Start Called in Invalid state %ld", meState);
        return OMX_ErrorInvalidState;
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          Stop

         DESCRIPTION:
*//**       @brief          Stops the media source thread
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkMediaSource::Stop()
{
    if(ISSTATEOPENED)
    {
        SETSTATECLOSING;

        WFDMMLOGH("Wait for Audio buffers to be back");
#ifdef USE_OMX_AAC_CODEC
        if(mCfg.eAudioFmt == WFD_AUDIO_LPCM)
#endif
        {
            while(mpAudioQ && mpAudioQ->GetSize() != MEDIASRC_NUM_AUDIO_BUFFERS)
            {
                WFDMMLOGD1("Waiting for Audio Buffers. %ld are back",
                           mpAudioQ->GetSize());
                MM_Timer_Sleep(2);
            }
        }
        WFDMMLOGH("Done Waiting for Audio buffers to be back");
    }
    else
    {
        WFDMMLOGE1("STOP Called in Invalid state %ld", meState);
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          AllocateIonBufs

         DESCRIPTION:
*//**       @brief          Allocates Ion Buffers for HDCP session
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkMediaSource::AllocateIonBufs()
{
    static struct ion_allocation_data sAllocInfo;
    static struct ion_fd_data sFdInfo;

    WFDMMLOGD("WFDMMSinkMediaSource: Allocate ION Buffers");

    mIonFd = open("/dev/ion",  O_RDONLY);

    if(mIonFd < 1)
    {
        WFDMMLOGE("WFDMMSinkMediaSource Failed to open ION device");
    }
    else
    {
        int32_t ret;

        memset(&sFdInfo, 0, sizeof(sFdInfo));
        memset(&sAllocInfo, 0, sizeof(sAllocInfo));

        /*----------------------------------------------------------------------
          ALlocate AUdio ION buffers
        ------------------------------------------------------------------------
        */
        sAllocInfo.len = 0;
        sAllocInfo.align = 4096;
        sAllocInfo.heap_mask = ION_HEAP(ION_QSECOM_HEAP_ID);
        sAllocInfo.handle = NULL;
        sAllocInfo.flags |= ION_FLAG_CACHED;
        mAudioBufSize = 8192;
        mAudioBufSize = ((mAudioBufSize + (4096 -1)) >> 12) << 12;

        sAllocInfo.len = mAudioBufSize;
        ret = ioctl(mIonFd, ION_IOC_ALLOC, &sAllocInfo);

        if (ret != 0  || sAllocInfo.handle == NULL)
        {
            WFDMMLOGE2("Failed to Alloc ION buf %d %d", ret, errno);
            return false;
        }



        sFdInfo.handle = sAllocInfo.handle;

        ret = ioctl(mIonFd, ION_IOC_SHARE, &sFdInfo);

        if (ret || sFdInfo.fd < 0)
        {
            WFDMMLOGE("WFDSource failed to alloc ION buffers");
            return false;
        }

        mAudioBufFd = sFdInfo.fd;
        mAudioIonHandle = sFdInfo.handle = sAllocInfo.handle;
        mAudioBufPtr = (uint8_t *)
                mmap(NULL, mAudioBufSize,
                     PROT_READ | PROT_WRITE, MAP_SHARED,
                     mAudioBufFd, 0);

        if(mAudioBufPtr == NULL)
        {
            WFDMMLOGE("WFDSource Failed to mmap ION buf");
            return false;
        }
        struct ion_flush_data sFlushData;
        sFlushData.handle = (ion_user_handle_t)mAudioIonHandle;
        sFlushData.fd = 0;
        sFlushData.offset = 0;
        sFlushData.vaddr = mAudioBufPtr;
        sFlushData.length = mAudioBufSize;

        if(ioctl (mIonFd, ION_IOC_CLEAN_INV_CACHES, &sFlushData) < 0)
            WFDMMLOGE("Cache flush failed");



        /*----------------------------------------------------------------------
         Allocate Video Ion buffers
        ------------------------------------------------------------------------
        */
        sAllocInfo.align = 4096;
        sAllocInfo.heap_mask = ION_HEAP(ION_QSECOM_HEAP_ID);
        sAllocInfo.handle = NULL;
        sAllocInfo.flags |= ION_FLAG_CACHED;

        mVideoBufSize = 2048 * 1024 * 1;
       // mVideoBufSize = ((mVideoBufSize + 4096 - 1) >> 12) << 12;

        sAllocInfo.len = mVideoBufSize;

        sAllocInfo.handle = NULL;
        ret = ioctl(mIonFd, ION_IOC_ALLOC, &sAllocInfo);

        if (ret != 0  || sAllocInfo.handle == NULL)
        {
            WFDMMLOGE3("Failed to Alloc ION buf %d %d %d", (int)ret, (int)errno,
                       (int)mIonFd);
            return false;
        }

        mVideoIonHandle = sFdInfo.handle = sAllocInfo.handle;


        ret = ioctl(mIonFd, ION_IOC_SHARE, &sFdInfo);

        if (ret || sFdInfo.fd < 0)
        {
            WFDMMLOGE("WFDSource failed to alloc ION buffers");
            return false;
        }
        WFDMMLOGH2("ION Handle Input fd = %d; handle = %x",
                   sFdInfo.fd, (unsigned int)sAllocInfo.handle);
        mVideoBufFd = sFdInfo.fd;
        mVideoBufPtr = (uint8_t *)
                mmap(NULL, sAllocInfo.len,
                     PROT_READ | PROT_WRITE, MAP_SHARED, mVideoBufFd, 0);

        mVideoHeapPtr = (uint8*)MM_Malloc(sAllocInfo.len);

        if(!mVideoHeapPtr)
        {
            WFDMMLOGD("Failed to allocate temporary buffer");
        }

        if(mVideoBufPtr == NULL || (int32_t)mVideoBufPtr == -1) {
            WFDMMLOGE("WFDSource Failed to mmap ION buf");
            return false;
        }



        sFlushData.handle = (ion_user_handle_t)mVideoIonHandle;
        sFlushData.fd = 0;
        sFlushData.offset = 0;
        sFlushData.vaddr = mVideoBufPtr;
        sFlushData.length = mVideoBufSize;

        if(ioctl (mIonFd, ION_IOC_CLEAN_INV_CACHES, &sFlushData) < 0)
            WFDMMLOGE("Cache flush failed");
    }

    /*--------------------------------------------------------------------------
      Done allocation of buffers
    ----------------------------------------------------------------------------
    */

    return true;
}

/*==============================================================================

         FUNCTION:          SetMediaBaseTime

         DESCRIPTION:       Sets base time for AV Sync
*//**       @brief
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSinkMediaSource::SetMediaBaseTime(uint64 nStartTime)
{
    avInfoType sAVInfo;
    int64 nTime;
    uint64 nTimeTemp;

    if(mpFnAVInfoCb)
    {

        struct timespec sTime;
        clock_gettime(CLOCK_MONOTONIC, &sTime);

        nTimeTemp = ((OMX_U64)sTime.tv_sec * 1000000/*us*/)
                                  + ((OMX_U64)sTime.tv_nsec / 1000);

        if(mpRTPStreamPort)
        {
            WFDMMLOGH("Reading BaseTIme from RTPSource");
            mpRTPStreamPort->GetRTPBaseTimeUs(&nTime);
            /*------------------------------------------------------------------
             If the first sample out already has a non zero timestamp, basetime
             has to be adjusted.
            --------------------------------------------------------------------
            */
            sAVInfo.nBaseTime = (uint64)nTime;

        }
        if(sAVInfo.nBaseTime == -1)
        {
            sAVInfo.nBaseTime = nTimeTemp;
        }
        sAVInfo.nBaseTimeStream = mnActualBaseTime;

        WFDMMLOGH2("MediaSrc BaseTime = %llu, Curr Time = %llu",
                   sAVInfo.nBaseTime, nTimeTemp);
        mpFnAVInfoCb(mClientData, &sAVInfo);
    }
}

/*==============================================================================

         FUNCTION:          receiveIDRTimerNotification

         DESCRIPTION:
*//**       @brief          Receives IDR Timer related info from Sink
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          int: pStatus

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSinkMediaSource::receiveIDRTimerNotification(
                                                    WFDMMIDRTimerStatus pStatus)
{
    CRITICAL_SECT_ENTER;
    meIDRTimerStatus = pStatus;
    if(meIDRTimerStatus == TIMER_EXPIRED)
    {
        mbIDRPendingRequest = true;
    }
    CRITICAL_SECT_LEAVE;
}

/*==============================================================================

         FUNCTION:          setFlushTimeStamp

         DESCRIPTION:
*//**       @brief          sets a timestamp upto which data has to be flushed
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSinkMediaSource::setFlushTimeStamp(uint64 nTS)
{
    mnFlushTimeStamp = nTS;
    mbFlushInProgress = true;
}

/*==============================================================================

         FUNCTION:          streamPause

         DESCRIPTION:
*//**       @brief          Pauses the rendering and holds on to the data
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSinkMediaSource::streamPause()
{
    if(ISSTATEOPENED)
    {
        mbPaused = true;
    }
    return;
}

/*==============================================================================

         FUNCTION:          streamPlay

         DESCRIPTION:
*//**       @brief          UnPauses the rendering
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSinkMediaSource::streamPlay(bool flush)
{
    if(ISSTATEOPENED)
    {
        if(mbPaused && flush)
        {
            /*------------------------------------------------------------------
             Flush buffers
            --------------------------------------------------------------------
            */
            FileSourceSampleInfo  sampleInfo;
            FileSourceMediaStatus mediaStatus = FILE_SOURCE_DATA_OK;
            uint32 nSize = mVideoBufSize ? mVideoBufSize:1024*1024;
            uint8 *pTempBuf = (uint8*)MM_Malloc(nSize);
            uint32 nMaxCnt = 2000;
            if(pTempBuf)
            {
                while(mediaStatus == FILE_SOURCE_DATA_OK && nMaxCnt--)
                {
                    mediaStatus = mpFileSource->GetNextMediaSample
                                                         (mVideoTrackId,
                                                          pTempBuf,
                                                          &nSize,
                                                          sampleInfo);
                }
                nMaxCnt = 2000;
                while(mediaStatus == FILE_SOURCE_DATA_OK && nMaxCnt--)
                {
                    mediaStatus = mpFileSource->GetNextMediaSample
                                                         (mAudioTrackId,
                                                          pTempBuf,
                                                          &nSize,
                                                          sampleInfo);
                }
                MM_Free(pTempBuf);
            }
        }
        mbPaused = false;
        if(mpAudioThread)
        {
            mpAudioThread->SetSignal(0);
        }
        if(mpVideoThread)
        {
            mpVideoThread->SetSignal(0);
        }
    }
    return;
}
