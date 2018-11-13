#include <stdio.h>
#include <string.h>
#include <sys/time.h>	//	gettimeofday

#include "asv_type.h"
#include <nx_video_api.h>


#define CFG_STRM_SIZE   (1024)
#define ES_MEM_SIZE 	(4*1024*1024)
#define TEST_CASE_NUM	5


typedef struct
{
	VID_TYPE_E				eCodec;
	int8_t                 	chInputName[256];
	int8_t					chRefName[256];
} VIDEO_TEST_LIST;

typedef struct
{
	FILE					*fpStream;
	uint8_t					*pbyStream;
	uint8_t					*pbyStreamOrg;
	uint8_t					*pbyStreamEnd;
	uint32_t				uSize;
	uint32_t				uTotalSize;
} STREAM;


VIDEO_TEST_LIST stVidTestList[TEST_CASE_NUM] =
{
	{ NX_AVC_DEC, "/mnt/mmc0/video/stockholm_ter_1080i_29-97fps_50Mbps_hp.264", "/mnt/mmc0/video/enc.264"},
	{ NX_VC1_DEC, "/mnt/mmc0/video/Driving_d1_30f_300_4Mbps.vc1",               "/mnt/mmc0/video/enc.jpg"},
	{ NX_MP2_DEC, "/mnt/mmc0/video/DVBT_Italy_352x576_LCI_Dfree_1.00M.m2v",     "/mnt/mmc0/video/mp2_dec.yuv"},
	{ NX_MP4_DEC, "/mnt/mmc0/video/stockholm_ter_1080i_29-97fps_40000kbps.m4v", "/mnt/mmc0/video/enc.m4v"},
	{ NX_RV_DEC,  "/mnt/mmc0/video/fm_15f_64kbps_150_QCIF_IPBP.rmes",           "/mnt/mmc0/video/rv9_dec.yuv"},
};

static ASV_RESULT 			eCurStatus;
static NX_VID_ENC_HANDLE	hEnc = NULL;
static NX_VID_DEC_HANDLE 	hDec = NULL;


static uint64_t NX_GetTickCount( void )
{
	uint64_t ret;
	struct timeval	tv;
	struct timezone	zv;
	gettimeofday( &tv, &zv );
	ret = ((uint64_t)tv.tv_sec)*1000000 + tv.tv_usec;
	return ret;
}

static uint32_t ReadOneFrame ( STREAM *pstStrm, VID_TYPE_E eCodec, int32_t iFrmNum )
{
	uint8_t *pbyStrm;

	if ( iFrmNum != 0 )
	{
		pbyStrm = pstStrm->pbyStream + pstStrm->uSize;

		if ( eCodec != NX_RV_DEC )
		{
			uint32_t uPreFourByte = (uint32_t)-1;
			int32_t  iIsFrmBoundary = 0, iAccessUnitSize = 0;

			pstStrm->pbyStream = pbyStrm;

			do
			{
				int32_t  iOneByteEOF;

				if ( pbyStrm >= pstStrm->pbyStreamEnd )
				{
					if ( pstStrm->pbyStreamEnd == pstStrm->pbyStreamOrg + pstStrm->uTotalSize )
					{
						memcpy( pstStrm->pbyStreamOrg, pstStrm->pbyStream, iAccessUnitSize );
						pstStrm->pbyStreamEnd = pstStrm->pbyStreamOrg + iAccessUnitSize + fread( pstStrm->pbyStreamOrg + iAccessUnitSize, 1, ES_MEM_SIZE - iAccessUnitSize, pstStrm->fpStream );
						pbyStrm = pstStrm->pbyStreamOrg + iAccessUnitSize;
						pstStrm->pbyStream = pstStrm->pbyStreamOrg;
					}
					else	break;
				}

				iOneByteEOF        = *pbyStrm++;
				iAccessUnitSize   += 1;

				if ( uPreFourByte == 0x00000001 || uPreFourByte<<8 == 0x00000100 )
				{
					int iNaluType = (iOneByteEOF & 0xFF);

					if (iIsFrmBoundary == 0) {
						if ( eCodec == NX_AVC_DEC ) {
							iNaluType = (iNaluType & 0x1F);
							if ( iNaluType == 1 ||  iNaluType == 5 )  iIsFrmBoundary = 1;
						}
						else if ( eCodec == NX_MP4_DEC ) {
							if ( iNaluType == 0xB6 )	iIsFrmBoundary = 1;
						}
						else if ( eCodec == NX_MP2_DEC ) {
							if ( iNaluType == 0x00 )	iIsFrmBoundary = 1;
						}
						else if ( eCodec == NX_VC1_DEC  ) {
							if ( iNaluType == 0x0D )	iIsFrmBoundary = 1;
						}
					}
					else {
						if ( eCodec == NX_AVC_DEC ) {
							iNaluType = (iNaluType & 0x1F);
							if ( iNaluType == 1 ||  iNaluType == 5 || iNaluType == 7 || iNaluType == 8 || iNaluType == 9 )	break;
						}
						else if ( eCodec == NX_MP4_DEC ) {
							if ( iNaluType == 0xB6 || (iNaluType >= 0x00 && iNaluType <= 0xB0) || iNaluType == 0xB3 || iNaluType == 0xB5 || iNaluType == 0xB8 )	break;
						}
						else if ( eCodec == NX_MP2_DEC ) {
							if ( iNaluType == 0x00 || iNaluType == 0xB3 || iNaluType == 0xB8 )	break;
						}
						else if ( eCodec == NX_VC1_DEC ) {
							if ( iNaluType == 0x0D || iNaluType == 0x0F )	break;
						}
					}
				}

				uPreFourByte     = (uPreFourByte << 8) + iOneByteEOF;
			} while ( 1 );

			pstStrm->uSize = ( uPreFourByte == 0x00000001 ) ? ( iAccessUnitSize - 5 ) : ( ( uPreFourByte<<8 == 0x00000100 ) ? (iAccessUnitSize - 4) : (iAccessUnitSize) );
		}
		else
		{
	  		uint32_t uSizeLen = pbyStrm[0] | pbyStrm[1]<<8 | pbyStrm[2]<<16 | pbyStrm[3]<<24;

			if ( pbyStrm + uSizeLen >= pstStrm->pbyStreamEnd )
			{
				uint32_t uTmp  = (uint32_t)pstStrm->pbyStreamEnd - (uint32_t)pbyStrm;
				memcpy( pstStrm->pbyStreamOrg, pbyStrm, uTmp );
				pstStrm->pbyStreamEnd = pstStrm->pbyStreamOrg + uTmp + fread( pstStrm->pbyStreamOrg + uTmp, 1, ES_MEM_SIZE - uTmp, pstStrm->fpStream );
				pbyStrm = pstStrm->pbyStreamOrg;

				if ( pbyStrm + uSizeLen > pstStrm->pbyStreamEnd )	return 0;
			}

			pstStrm->pbyStream = pbyStrm + 4;
			pstStrm->uSize = uSizeLen;
		}
	}
	else
	{
		if ( eCodec != NX_VC1_DEC )
		{
			pstStrm->pbyStream += pstStrm->uSize;
			pstStrm->uSize = 0;
		}
		else
		{
			uint32_t uPreFourByte = (uint32_t)-1;
			pbyStrm = pstStrm->pbyStream;

			do
			{
				int32_t  iOneByteEOF = *pbyStrm++;
				if ( uPreFourByte == 0x0000010D )
				{
					pstStrm->pbyStream = pbyStrm - 5;
					pstStrm->uSize -= ((uint32_t)pstStrm->pbyStream - (uint32_t)pstStrm->pbyStreamOrg);
					break;
				}
				uPreFourByte = (uPreFourByte << 8) + iOneByteEOF;
			} while ( 1 );
		}
	}

	return (pstStrm->uSize);
}

ASV_RESULT ASV_Video_Run(void)
{
	int32_t				iTestCnt = 0, iFrmCnt;
	uint32_t 			uSize, uTmp;
	STREAM   			stStrm;
	VID_ERROR_E 		eRet;

	NX_VID_ENC_IN		stEncIn;
	NX_VID_ENC_OUT		stEncOut;

	NX_VID_DEC_IN		stDecIn;
	NX_VID_DEC_OUT		stDecOut;

	printf("ASV_Video_Run() \n");

	eCurStatus = ASV_RES_TESTING;

	stStrm.pbyStream = (uint8_t *)malloc(ES_MEM_SIZE);
	stStrm.pbyStreamOrg = stStrm.pbyStream;
	stStrm.uTotalSize = ES_MEM_SIZE;

	do {
		stStrm.fpStream = fopen(stVidTestList[iTestCnt].chInputName, "rb");
		if ( stStrm.fpStream == NULL )
		{
			printf("file open error \n");
			break;
		}

		// Initilize
		{
			// Decoder Init
			NX_VID_SEQ_IN	stSeqIn = {0,};
			NX_VID_SEQ_OUT	stSeqOut;

			hDec = NX_VidDecOpen(stVidTestList[iTestCnt].eCodec, 0, 0, NULL);
			if( hDec == NULL )
			{
				printf("NX_VidDecOpen() failed!!!\n");
				break;
			}

			stStrm.uSize = 0;
			stStrm.pbyStream = stStrm.pbyStreamOrg;
			stStrm.pbyStreamEnd = stStrm.pbyStreamOrg + fread(stStrm.pbyStreamOrg, 1, ES_MEM_SIZE, stStrm.fpStream);

			uSize = ReadOneFrame( &stStrm, stVidTestList[iTestCnt].eCodec, -1 );

			stSeqIn.seqInfo = stStrm.pbyStream;
			stSeqIn.seqSize = uSize;

			if ( NX_VidDecInit(hDec, &stSeqIn, &stSeqOut) != VID_ERR_NONE )
			{
				printf("NX_VidDecInit() failed!!!\n");
				break;
			}

			// Encoder Init
			if ( stVidTestList[iTestCnt].eCodec == NX_AVC_DEC || stVidTestList[iTestCnt].eCodec == NX_MP4_DEC )
			{
				NX_VID_ENC_INIT_PARAM stEncPara = {0,};

				VID_TYPE_E eEncCodec = ( stVidTestList[iTestCnt].eCodec == NX_AVC_DEC ) ? ( NX_AVC_ENC ) : ( NX_MP4_ENC );

				hEnc = NX_VidEncOpen( eEncCodec, NULL );
				if( hEnc == NULL )
				{
					printf("NX_VidEncOpen() failed!!!\n");
					break;
				}

				stEncPara.width = stSeqOut.width;
				stEncPara.height = stSeqOut.height;
				stEncPara.gopSize = 0xFFFF;
				stEncPara.fpsNum = 30;
				stEncPara.fpsDen = 1;
				stEncPara.enableRC = 1;
				stEncPara.bitrate = 30 * 1024 * 1024;

				if ( NX_VidEncInit( hEnc, &stEncPara ) != VID_ERR_NONE )
				{
					printf("NX_VidEncInit() failed \n");
					break;
				}
			}
			else
			{
				hEnc = NULL;
			}
		}

		// Process (Decode/Encoder A Frame)
		memset(&stDecIn, 0, sizeof(stDecIn));
		iFrmCnt = 0;

		do {
			stDecIn.strmSize = ReadOneFrame( &stStrm, stVidTestList[iTestCnt].eCodec, iFrmCnt );
			stDecIn.strmBuf = stStrm.pbyStream;
			stDecIn.eos = ( (stDecIn.strmSize != 0) || (iFrmCnt == 0) ) ? (0) : (1);

			if (NX_VidDecDecodeFrame( hDec, &stDecIn, &stDecOut ) != VID_ERR_NONE)
			{
				printf("NX_VidDecDecodeFrame() failed \n");
				break;
			}

			//printf("[%3d]Size = %8d(%5x), Addr = %8x (%2x %2x %2x %2x %2x), Idx = %3d %3d, Reliable = %3d \n",
			//	iFrmCnt, stDecIn.strmSize, stDecIn.strmSize, stDecIn.strmBuf, stDecIn.strmBuf[0], stDecIn.strmBuf[1], stDecIn.strmBuf[2], stDecIn.strmBuf[3], stDecIn.strmBuf[4], stDecOut.outImgIdx, stDecOut.outDecIdx, stDecOut.outFrmReliable_0_100);

			if ( stVidTestList[iTestCnt].eCodec == NX_RV_DEC && iFrmCnt > 10 )
				stStrm.pbyStreamEnd = stStrm.pbyStream;

			if ( stDecOut.outImgIdx >= 0 )
			{
				if ( hEnc != NULL )
				{
					memset(&stEncIn, 0, sizeof(stEncIn));
					stEncIn.pImage = &stDecOut.outImg;
					if (NX_VidEncEncodeFrame( hEnc, &stEncIn, &stEncOut ) != VID_ERR_NONE)
					{
						printf("NX_VidEncEncodeFrame() failed \n");
						break;
					}
				}

				//fwrite( stDecOut.outImg.luVirAddr, 1, stDecOut.width * stDecOut.height, fpOut );
				//fwrite( stDecOut.outImg.cbVirAddr, 1, stDecOut.width * stDecOut.height / 4, fpOut );
				//fwrite( stDecOut.outImg.crVirAddr, 1, stDecOut.width * stDecOut.height / 4, fpOut );

				NX_VidDecClrDspFlag( hDec, &stDecOut.outImg, stDecOut.outImgIdx );
			}
			else if ( stDecOut.outDecIdx < 0 && stDecIn.eos == 1 )
				break;

			iFrmCnt++;
		} while(1);

		if ( stVidTestList[iTestCnt].eCodec == NX_VC1_DEC )
		{
			NX_VID_ENC_INIT_PARAM stEncPara = {0,};

			hEnc = NX_VidEncOpen( NX_JPEG_ENC, NULL );
			if( hEnc == NULL )
			{
				printf("NX_VidEncOpen() failed!!!\n");
				break;
			}

			stEncPara.width = stDecOut.width;
			stEncPara.height = stDecOut.height;
			stEncPara.jpgQuality = 90;

			if ( NX_VidEncInit( hEnc, &stEncPara ) != VID_ERR_NONE )
			{
				printf("NX_VidEncInit() failed \n");
				break;
			}
			//NX_VidEncJpegGetHeader( hEnc, seqBuffer, &size );

			stEncIn.pImage = &stDecOut.outImg;
			if ( NX_VidEncJpegRunFrame( hEnc, stEncIn.pImage, &stEncOut ) != VID_ERR_NONE )
			{
				printf("NX_VidEncJpegRunFrame() failed \n");
				break;
			}
		}

		{
			void    *pvSrc = (void *)(( hEnc ) ? ( stEncOut.outBuf ) : ( stDecOut.outImg.luVirAddr ));

			int32_t iFileSize = ES_MEM_SIZE;
			FILE    *fpRef = fopen(stVidTestList[iTestCnt].chRefName, "rb");
			if ( fpRef == NULL )
			{
				printf("refernece file open error \n");
				break;
			}
			iFileSize = fread(stStrm.pbyStreamOrg, 1, iFileSize, fpRef);
			fclose(fpRef);

			if ( (hEnc != NULL) || (stDecOut.outImg.luStride == stDecOut.width) )
			{
				if (memcmp( pvSrc, (void *)stStrm.pbyStreamOrg, iFileSize) != 0)
				{
					printf("Codec Result is different (Size = %d, %d) \n", iFileSize, stEncOut.bufSize );
					break;
				}
			}
			else
			{
				int32_t i, j;
				uint8_t *pbyDst = (uint8_t *)stStrm.pbyStreamOrg;
				uint8_t *pbySrc = (uint8_t *)pvSrc;

				//printf("Stride = %d, Width = %d \n", stDecOut.outImg.luStride, stDecOut.width);

				for (i=0 ; i<stDecOut.height ; i++) {
					for (j=0 ; j<stDecOut.width ; j++) {
						if ( *pbySrc++ != *pbyDst++ )
						{
							//printf("DIFF (%d, %d) \n", i, j);
							break;
						}
					}
					pbySrc += stDecOut.outImg.luStride - stDecOut.width;
				}

				if ( (i < stDecOut.height) || (j < stDecOut.width) )
				{
					printf("Codec Result is different\n");
					break;
				}
			}
			printf("Codec Result is Same!! \n");
		}

		if ( hEnc ) NX_VidEncClose( hEnc );
		NX_VidDecClose( hDec );

		hEnc = NULL;
		hDec = NULL;

		fclose(stStrm.fpStream);
		stStrm.fpStream = NULL;

		iTestCnt++;
	}while(iTestCnt < TEST_CASE_NUM);

	if ( hEnc ) NX_VidEncClose( hEnc );
	if ( hDec ) NX_VidDecClose( hDec );
	if ( stStrm.fpStream ) fclose(stStrm.fpStream);
	free( stStrm.pbyStreamOrg );

	if ( iTestCnt < TEST_CASE_NUM )
	{
		eCurStatus = ASV_RES_ERR;
		return ASV_RES_ERR;
	}

	printf("success\n");

	eCurStatus = ASV_RES_OK;
	return ASV_RES_OK;
}

ASV_RESULT ASV_Video_Stop(void)
{
	printf("ASV_Video_Stop() \n");

	if ( ASV_RES_TESTING )
	{
		eCurStatus = ASV_RES_ERR;

		if ( hEnc ) NX_VidEncClose( hEnc );
		if ( hDec ) NX_VidDecClose( hDec );
	}

	return ASV_RES_OK;
}

ASV_RESULT ASV_Video_Status(void)
{
	printf("ASV_Video_Status() \n");
	return eCurStatus;
}

ASV_TEST_MODULE *GetVpuTestModule(void)
{
	static ASV_TEST_MODULE mASV = {  "video ASV TEST", ASV_Video_Run, ASV_Video_Stop, ASV_Video_Status };
	printf("GetVpuTestModule() \n");
	return &mASV;
}

#if 1

int32_t main( int32_t argc, char *argv[] )
{
	ASV_TEST_MODULE *test = GetVpuTestModule();
	ASV_RESULT res;
	uint32_t count = 10;
	uint64_t startTime, endTime;

	startTime = NX_GetTickCount();
	res = test->run();
	if( res < 0 )
	{
		printf("run fail \n");
		return -1;
	}
	endTime = NX_GetTickCount();
	printf("Time = %6lld \n", endTime - startTime );

	res = test->status();
	printf("curr status = %d \n", res);
	if( res < 0 )
	{
		return -1;
	}
	res = test->stop();

	return res;
}

#endif