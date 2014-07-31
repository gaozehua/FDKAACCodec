#include <stdio.h>
#include <stdlib.h>
#include "aacenc_lib.h"
#include "mpegFileWrite.h"
#include "cmdl_parser.h"
#include "wav_file.h"
#include "genericStds.h"
#include "conv_string.h"

int main( int argc, char **argv )
{  
	INT ret;
	FILE *outf = NULL;
	AACENC_ERROR ErrorStatus;
	HANDLE_AACENCODER hAacEncoder = NULL;
	AACENC_InfoStruct encInfo;
	//HANDLE_FILEWRITE hMpegFile;
	//AACENC_PARAM parameter;
	static INT_PCM inputBuffer[8*2048];
	static UCHAR ancillaryBuffer[50];
	static AACENC_MetaData metaDataSetup;
	static UCHAR outputBuffer[8192];

	static void* inBuffer[] = { inputBuffer, ancillaryBuffer, &metaDataSetup };
	static INT inBufferIds[] = { IN_AUDIO_DATA, IN_ANCILLRY_DATA,
	IN_METADATA_SETUP };
	static INT inBufferSize[] = { sizeof(inputBuffer), sizeof(ancillaryBuffer),
	sizeof(metaDataSetup) };
	static INT inBufferElSize[] = { sizeof(INT_PCM), sizeof(UCHAR),
	sizeof(AACENC_MetaData) };
	static void* outBuffer[] = { outputBuffer };
	static INT outBufferIds[] = { OUT_BITSTREAM_DATA };
	static INT outBufferSize[] = { sizeof(outputBuffer) };
	static INT outBufferElSize[] = { sizeof(UCHAR) };

	AACENC_BufDesc inBufDesc;
	AACENC_BufDesc outBufDesc;
	AACENC_InArgs inargs;
	AACENC_OutArgs outargs;
	HANDLE_WAV pWav;

	FILE *inputFile = NULL;
    char percents[200];
    float percent, old_percent = -1.0;
    float bread = 0, fileread;
    int header_type = 0;
    int bitrate = 0;
    float length = 0;

	inBufDesc.numBufs = sizeof(inBuffer)/sizeof(void*);
	inBufDesc.bufs = (void**)&inBuffer;
	inBufDesc.bufferIdentifiers = inBufferIds;

	inBufDesc.bufSizes = inBufferSize;
	inBufDesc.bufElSizes = inBufferElSize;

	outBufDesc.numBufs = sizeof(outBuffer)/sizeof(void*);
	outBufDesc.bufs = (void**)&outBuffer;
	outBufDesc.bufferIdentifiers = outBufferIds;
	outBufDesc.bufSizes = outBufferSize;
	outBufDesc.bufElSizes = outBufferElSize;

	inargs.numAncBytes = 0;
	inargs.numInSamples = 0;

	if(argc<3)
	{
		printf("FDKAACEnc.exe [input wav] [output aac]");
		return 0;
	}


	if ( (ErrorStatus = aacEncOpen(&hAacEncoder,0,0)) != AACENC_OK ) 
	{
		return 0;
	}

	if ( (ret = WAV_InputOpen(&pWav,argv[1])) != 0 ) 
	{
		return 0;
	}

	outf = fopen(argv[2],"wb+");
	if(outf == NULL)
	{
		printf("Output file error\n");
		return 1;
	}

	fileread = pWav->header.dataSize;

	//ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_AOT, 2);
	//ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_AOT, 29);
	ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_AOT, 5);
	ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_BITRATEMODE, 0);
	ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_BITRATE, 24000);
	ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_SAMPLERATE, pWav->header.sampleRate);
	ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_CHANNELMODE, pWav->header.numChannels);

	ErrorStatus = aacEncEncode(hAacEncoder, NULL, NULL, NULL, NULL);
	ErrorStatus = aacEncInfo(hAacEncoder, &encInfo);
	do
	{
		inargs.numInSamples += WAV_InputRead ( pWav,&inputBuffer[inargs.numInSamples],FDKmin(encInfo.inputChannels*encInfo.frameLength,sizeof(inputBuffer) /sizeof(INT_PCM)-inargs.numInSamples),SAMPLE_BITS);
		bread+=(inargs.numInSamples*2);
		if(inargs.numInSamples == 0)
			goto ENC_END;

		ErrorStatus = aacEncEncode(hAacEncoder,
						&inBufDesc,
						&outBufDesc,
						&inargs,
						&outargs);

		if (outargs.numInSamples>0) {
		FDKmemmove( inputBuffer,&inputBuffer[outargs.numInSamples],sizeof(INT_PCM)*(inargs.numInSamples-outargs.numInSamples) );
		inargs.numInSamples -= outargs.numInSamples;
		}

        percent = min((float)(bread*10000)/fileread, 10000);
        if (percent > old_percent)
        {
            old_percent = percent;
            sprintf(percents, "%0.2f%% Encoding %s.", (percent/100.0), argv[1]);
            fprintf(stderr, "%s\r", percents);
        }

		if (outargs.numOutBytes>0) {
			fwrite(outputBuffer,1,outargs.numOutBytes,outf);
		//	printf("outargs.numOutBytes = %d\n",outargs.numOutBytes);
		} /* (outBytes>0) */
	} while (ErrorStatus==AACENC_OK);
ENC_END:
	fclose(outf);
	WAV_InputClose(&pWav);
	printf("\nencoding finished\n");

  return 0;
}