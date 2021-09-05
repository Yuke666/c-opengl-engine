#include <stdio.h>
#include <zlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define COMPRESS 0
#define OUTPUT_WAV_DEBUG 0
#define SAMPLERATE 11025

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef struct {
	u8 chunkID[4];
	u32 chunkSize;
} DataChunk;

typedef struct {
	u8 waveID[4];
	u8 chunkID[4];
	u32 chunkSize;
	u16 formatTag;
	u16 nChannels;
	u32 samplerate;
	u32 avgBytesPerSec;
	u16 blockAlign;
	u16 bitsPerSample;
	DataChunk data;
} FmtChunk;

typedef struct {
	u8 chunkID[4];
	u32 chunkSize;
	FmtChunk fmt;
} WavHeader;


void Compress(u8 *data, int dataSize, FILE *fp){

    unsigned have;
    z_stream strm;

    u8 *out = (u8 *)malloc(dataSize);

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY);

    strm.avail_in = dataSize;
    strm.next_in = data;

    strm.avail_out = dataSize;
    strm.next_out = out;
    deflate(&strm, Z_FINISH);

    have = dataSize - strm.avail_out;
	
	fwrite(out, 1, have, fp);

    deflateEnd(&strm);

    free(out);
}

int main(int argc, char **argv){

	if(argc < 3){
		printf("Usage: wav2snd input.wav output.snd\n");
		return 1;
	}

	FILE *fp = fopen(argv[1], "rb");

	if(!fp){
		printf("%s not found.\n", argv[1]);
		return 1;
	}

	WavHeader header;

	fread(&header, 1, sizeof(WavHeader), fp);

	if(header.fmt.data.chunkSize % 2 != 0)
		fgetc(fp);

	u8 *samples = (u8 *)malloc(header.fmt.data.chunkSize);

	fread(samples, 1, header.fmt.data.chunkSize, fp);

	fclose(fp);
	
	fp = fopen(argv[2], "wb");

	int dataSize = header.fmt.data.chunkSize;
	int dataRate = header.fmt.samplerate;

	int size = (dataSize / dataRate) * SAMPLERATE;

#if !COMPRESS && OUTPUT_WAV_DEBUG

	header.fmt.data.chunkSize = size;
	header.fmt.avgBytesPerSec = (header.fmt.avgBytesPerSec / dataRate) * SAMPLERATE;
	header.fmt.samplerate = SAMPLERATE;

	fwrite(&header, 1, sizeof(WavHeader), fp);

#else

	fwrite(&size, 1, sizeof(int), fp);

#endif

	int bytesPerSample = header.fmt.bitsPerSample / 8;

	int k;
	for(k = 0; k < dataSize; k+=bytesPerSample)
		if((k/bytesPerSample) % (dataRate / SAMPLERATE) == 0)
			memcpy(&samples[k / (dataRate / SAMPLERATE)], &samples[k], bytesPerSample);


#if COMPRESS
	Compress(samples, size, fp);
#else
	fwrite(samples, 1, size, fp);
#endif

	fclose(fp);

	return 0;
}