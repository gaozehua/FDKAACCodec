#include <stdio.h>
#include <stdlib.h>
#include "aacdecoder_lib.h"
#include "cmdl_parser.h"
#include "conv_string.h"
#include "wav_file.h"
#include "machine_type.h"
#include "mpegFileRead.h"

#define FILE_NAME_MAX 256
#define NO_FILENAME "__no.filename.given__"
//#define IN_BUF_SIZE ( 8192*256 )
//#define OUT_BUF_SIZE ( (6)*(2048)*4 )
#define IN_BUF_SIZE ( 8192*2 )
#define OUT_BUF_SIZE ( (2)*(2048)*4 )
#define ANC_BUF_SIZE ( 128 )
#define N_FLUSH_FRAMES 0
#define ERR_PROC(a)   {if(a > 0){ printf("Error : %d\n",a); return 0;}}
int frame_cnt;
int main(int argc,char **argv)
{
	FILE *inf = NULL;
	HANDLE_AACDECODER aacDecoderInfo;
	AAC_DECODER_ERROR  ErrorStatus;
	UCHAR nlayer;
	UINT flags = 0;
	UINT bytesRead = IN_BUF_SIZE;
	UINT bytesValid = bytesRead;
	UCHAR inBuffer_mem[FILEREAD_MAX_LAYERS][IN_BUF_SIZE];
    RAM_ALIGN INT_PCM TimeData [OUT_BUF_SIZE];
	UCHAR * inBuffer[FILEREAD_MAX_LAYERS] = {inBuffer_mem};
	UINT ret;
	CStreamInfo *pCSI=NULL;
	HANDLE_WAV pWav;
	UINT outfile_open=0; 
	UINT fcnt = 0;

	if(argc<3)
	{
		printf("FDKAACDec.exe [input aac] [output wav]");
		return 0;
	}
	aacDecoderInfo = aacDecoder_Open(TT_MP4_ADTS, 1 );
	inf = fopen(argv[1],"rb+");

	do
	{
		bytesRead = fread(inBuffer[0],1,bytesRead,inf);
	    bytesValid = bytesRead;
		while(bytesValid)
		{
			ErrorStatus = aacDecoder_Fill(aacDecoderInfo, inBuffer, &bytesRead, &bytesValid);
            ERR_PROC(ErrorStatus);
			//printf("************** frame %d******************\n",frame_cnt++);
			ErrorStatus = aacDecoder_DecodeFrame(aacDecoderInfo, TimeData, OUT_BUF_SIZE,flags);
			ERR_PROC(ErrorStatus);
			
			
			pCSI = aacDecoder_GetStreamInfo(aacDecoderInfo);

			if((!outfile_open)&&(pCSI))
			{
				outfile_open = (((ret=WAV_OutputOpen(&pWav, argv[2], pCSI->sampleRate, pCSI->numChannels, 16)) == 0) ? 1 : 0 );
				//printf("pCSI->aacSampleRate = %d,pCSI->numChannels= %d\n",pCSI->aacSampleRate,pCSI->numChannels);
				if(outfile_open == 0)
				{
					printf("out file error \n");
					return;
				}
			}

			WAV_OutputWrite(pWav,TimeData,pCSI->frameSize*pCSI->numChannels,16,16);
			fprintf(stderr,"[%d]\r",fcnt++); fflush(stderr);
		//	printf("ErrorStatus = %d,bytesRead= %d,bytesValid = %d\n",ErrorStatus,bytesRead,bytesValid);
		}

		if(bytesRead==0)
			break;
	}while(1);

	aacDecoder_Close(aacDecoderInfo);
	WAV_OutputClose(&pWav);
	printf("\ndecoding finished\n");
}