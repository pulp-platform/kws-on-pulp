#include "mfcc_utils.h"
#include "pmsis.h"

#define WAV_HEADER_SIZE	44

#include "bsp/ram.h"
#include "bsp/fs.h"
#include "bsp/ram/hyperram.h"
#define INTER_BUFF_SIZE     (1000*2)
static uint8_t *_tmp_buffer;



static void progress_bar(char * OutString, int n, int tot)
{
	int tot_chars = 30;
	printf("%s",OutString);
	printf(" [");
	int chars = (n*tot_chars)/tot;

	for(int i=0;i<tot_chars;i++){
		if(i<=chars)
			printf("#");
		else printf(" ");
	}
	printf("]");
	printf("\n");

}

static int ReadWAVHeader(char *FileName, header_struct *HeaderInfo, struct pi_device fs)
{
	// switch_fs_t fs;
	// __FS_INIT(fs);
	// switch_file_t File = __OPEN_READ(fs, FileName);

	// if (!File) {
	// 	printf("Unable to open file %s\n", FileName);
	// 	return 1;
	// }

	// unsigned int Err = 0;
	// unsigned char *Header = (unsigned char *) __ALLOC_L2(WAV_HEADER_SIZE);
	// Err |= (Header == 0);
	// if ((__READ(File, Header, WAV_HEADER_SIZE) != WAV_HEADER_SIZE) || Err) return 1;

	// HeaderInfo->FileSize      = Header[4]  | (Header[5]<<8)  | (Header[6]<<16)  |	(Header[7]<<24);
	// HeaderInfo->format_type   = Header[20] | (Header[21]<<8);
	// HeaderInfo->NumChannels   = Header[22] | (Header[23]<<8);
	// HeaderInfo->SampleRate    = Header[24] | (Header[25]<<8) | (Header[26]<<16) |	(Header[27]<<24);
	// HeaderInfo->byterate      = Header[32] | (Header[33]<<8);
	// HeaderInfo->BitsPerSample = Header[34] | (Header[35]<<8);
	// HeaderInfo->DataSize      = Header[40] | (Header[41]<<8) | (Header[42]<<16) |	(Header[43]<<24);
	// __FREE_L2(Header, WAV_HEADER_SIZE);
	// __CLOSE(File);
	// __FS_DEINIT(fs);
	// return Err;




	pi_fs_file_t *file;

	file = pi_fs_open(&fs, FileName, 0);

	if (file == NULL)
    {
        printf("file open failed\n");
        return -1;
    }

	unsigned int Err = 0;
	unsigned char *Header = (unsigned char *) pi_l2_malloc(WAV_HEADER_SIZE);
	Err |= (Header == 0);
	if ((pi_fs_read(file, Header, WAV_HEADER_SIZE) != WAV_HEADER_SIZE) || Err) return 1;

	HeaderInfo->FileSize      = Header[4]  | (Header[5]<<8)  | (Header[6]<<16)  |	(Header[7]<<24);
	HeaderInfo->format_type   = Header[20] | (Header[21]<<8);
	HeaderInfo->NumChannels   = Header[22] | (Header[23]<<8);
	HeaderInfo->SampleRate    = Header[24] | (Header[25]<<8) | (Header[26]<<16) |	(Header[27]<<24);
	HeaderInfo->byterate      = Header[32] | (Header[33]<<8);
	HeaderInfo->BitsPerSample = Header[34] | (Header[35]<<8);
	HeaderInfo->DataSize      = Header[40] | (Header[41]<<8) | (Header[42]<<16) |	(Header[43]<<24);
	// __FREE_L2(Header, WAV_HEADER_SIZE);
	// __CLOSE(File);
	// __FS_DEINIT(fs);
	return Err;


}

static int ReadWavShort(pi_fs_file_t * File, short int* OutBuf, unsigned int NumSamples, unsigned int Channels)
{
	int ChunkSize = 1024;
	unsigned char *data_buf = (unsigned char *) pi_l2_malloc(ChunkSize*Channels*sizeof(short int));
	int BytesPerSample = Channels * sizeof(short int);
	if (data_buf==NULL) {
		printf("Error allocating\n");
		return 1;
	}
	int i, ch;
	int RemainBytes = NumSamples*BytesPerSample;
	int read_size;
	while (RemainBytes>0){
		#ifndef SILENT
			progress_bar("Reading Wav ", NumSamples*BytesPerSample-RemainBytes, NumSamples*BytesPerSample);
		#endif
		if (RemainBytes > ChunkSize*BytesPerSample) read_size = ChunkSize*BytesPerSample;
		else 										read_size = RemainBytes;
		// int len = __READ(File, data_buf, read_size);
		int len = pi_fs_read(File, data_buf, read_size);
		if (!len) return 1;
		RemainBytes -= len;
		int offset = 0;
		for (i=0; i<read_size/2; i++){
			int data_in_channel;
			for (ch=0; ch<Channels; ch++){
				data_in_channel = data_buf[offset*2] | (data_buf[offset*2+1] << 8);
				OutBuf[offset*Channels + ch] = (short int) data_in_channel;
				offset += 1; //Bytes in each channel
			}
		}
		OutBuf += len/BytesPerSample;
	}
	return 0;
}

static int ReadWavChar(pi_fs_file_t * File, char* OutBuf, unsigned int NumSamples, unsigned int Channels)
{
	int ChunkSize = 1024;
	unsigned char *data_buf = (unsigned char *) pi_l2_malloc(ChunkSize*Channels*sizeof(char));
	int BytesPerSample = Channels * sizeof(char);
	if (data_buf==NULL) {
		printf("Error allocating\n");
		return 1;
	}
	int i, ch;
	int RemainBytes = NumSamples*BytesPerSample;
	int read_size;
	while (RemainBytes>0){
		#ifndef SILENT
			progress_bar("Reading Wav ", NumSamples*BytesPerSample-RemainBytes, NumSamples*BytesPerSample);
		#endif
		if (RemainBytes > ChunkSize*BytesPerSample) read_size = ChunkSize*BytesPerSample;
		else 										read_size = RemainBytes;
		// int len = __READ(File, data_buf, read_size);
		int len = pi_fs_read(File, data_buf, read_size);
		if (!len) return 1;
		RemainBytes -= len;
		int offset = 0;
		for (i=0; i<read_size; i++){
			int data_in_channel;
			for (ch=0; ch<Channels; ch++){
				data_in_channel = data_buf[offset];
				OutBuf[offset*Channels + ch] = (short int) data_in_channel;
				offset += 1; //Bytes in each channel
			}
		}
		OutBuf += len/BytesPerSample;
	}
	return 0;
}

int ReadWavFromFile(char *FileName, void* OutBuf, unsigned int BufSize, header_struct *HeaderInfo, struct pi_device fs) 
{
	// if (ReadWAVHeader(FileName, HeaderInfo, fs)) return 1;
	// switch_file_t File = (switch_file_t) 0;
	// switch_fs_t fs;
	// __FS_INIT(fs);
	// File = __OPEN_READ(fs, FileName);
	// if (File == 0) {
	// 	printf("Failed to open file, %s\n", FileName); goto Fail;
	// }

	if (ReadWAVHeader(FileName, HeaderInfo, fs)) return 1;

	pi_fs_file_t *file;

	file = pi_fs_open(&fs, FileName, 0);

	if (file == NULL)
    {
        printf("file open failed\n");
        return -1;
    }



	int NumSamples = HeaderInfo->DataSize * 8 / (HeaderInfo->NumChannels * HeaderInfo->BitsPerSample);
	int SizeOfEachSample = (HeaderInfo->NumChannels * HeaderInfo->BitsPerSample) / 8;

	// __SEEK(file, WAV_HEADER_SIZE);
	pi_fs_seek(file, WAV_HEADER_SIZE);

	int SamplesShort;
	if (HeaderInfo->BitsPerSample == 16) SamplesShort = 1;
	else if (HeaderInfo->BitsPerSample == 8) SamplesShort = 0;
	else {printf("BytesPerSample %d not supported\n", HeaderInfo->BitsPerSample); return 1;}

	if (BufSize < HeaderInfo->DataSize){
		printf("Buffer Size too small: %d required, %d given", HeaderInfo->DataSize, BufSize);
		return 1;
	}

	int res;
	if (HeaderInfo->BitsPerSample == 16)
		res = ReadWavShort(file, (short int *) OutBuf, NumSamples, HeaderInfo->NumChannels);
	else if (HeaderInfo->BitsPerSample == 8)
		res = ReadWavChar(file, (char *) OutBuf, NumSamples, HeaderInfo->NumChannels);
	else goto Fail;

	if (res) {
		printf("Input ended unexpectedly or bad format, %s\n", FileName); goto Fail;
	}
	// __CLOSE(File);
	// __FS_DEINIT(fs);
	printf("\n\nFile: %s, FileSize: %d, NumChannels: %d, SampleRate: %d, BitsPerSample: %d, DataSize: %d, NumSamples: %d\n", \
		    FileName, HeaderInfo->DataSize, HeaderInfo->NumChannels, HeaderInfo->SampleRate, HeaderInfo->BitsPerSample, HeaderInfo->DataSize, NumSamples);

	return 0;
Fail:
	// __CLOSE(File);
	// __FS_DEINIT(fs);
	printf("Failed to load file %s from flash\n", FileName);
	return 1;

}