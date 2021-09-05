// thanks to puff.c from zlib.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "deflate.h"

#define DEFLATE_MAXBITS 15
#define DEFLATE_MAXLCODES 286
#define DEFLATE_MAXDCODES 30            
#define DEFLATE_MAXCODES (DEFLATE_MAXLCODES+DEFLATE_MAXDCODES) 

typedef struct {
	FILE *fp;
	u8 *in;
	u32 inPos; 
	u8 *out;
	u8 currByte;
	u8 onBit;
	u32 outLen;
	u32 outCount;
    u16 lenCnt[DEFLATE_MAXBITS+1];
    u16 lenSym[DEFLATE_MAXLCODES];
    u16 distCnt[DEFLATE_MAXBITS+1];
    u16 distSym[DEFLATE_MAXDCODES];
} DeflateContext;

static int BitStream_GetBits(DeflateContext *context, int bits){

    int val = context->currByte;

    while(context->onBit < bits) {

    	if(context->in)
	        val |= (int)context->in[context->inPos++] << context->onBit;
	    else
	        val |= (int)(fgetc(context->fp)) << context->onBit;

        context->onBit += 8;
    }

    context->currByte = (int)(val >> bits);
    context->onBit -= bits;

    return (int)(val & ((1L << bits) - 1));
}

static int Decode(DeflateContext *context, u16 *counts, u16 *symLens){

	int code = 0;
	int first = 0;
	int count = 0;
	int index = 0;

	int k;
	for(k = 1; k <= DEFLATE_MAXBITS; k++){

		code |= BitStream_GetBits(context, 1);
		
		count = counts[k];

		if(code - count < first)
			return symLens[index + (code - first)];

		index += count;
		first += count;
		first <<= 1;
		code <<= 1;
	}	

	return -1;
}

static void Huffman_Construct(u16 *count, u16 *symLens, u16 *lengths, int n){

	memset(count, 0, (DEFLATE_MAXBITS+1) * sizeof(u16));

	int k;
	for(k = 0; k < n; k++)
		++count[lengths[k]];

	u16 offsets[DEFLATE_MAXBITS+1];

    offsets[1] = 0;
    for (k = 1; k < DEFLATE_MAXBITS; k++)
        offsets[k + 1] = offsets[k] + count[k];

    for (k = 0; k < n; k++){
        if (lengths[k] != 0)
            symLens[offsets[lengths[k]]++] = k;
    }
}

static int Codes(DeflateContext *context){

    static const short lens[29] = { 
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const short lext[29] = { 
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const short dists[30] = { 
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    static const short dext[30] = { 
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

    int symbol;         

    do {

        symbol = Decode(context, context->lenCnt, context->lenSym);

        if (symbol < 256) {

            if(context->outCount < context->outLen){
	            context->out[context->outCount] = symbol;
	            context->outCount++;
            }
        
        } else if (symbol > 256) {

            symbol -= 257;

            int copyLen = lens[symbol] + BitStream_GetBits(context, lext[symbol]);

            symbol = Decode(context, context->distCnt, context->distSym);
            
            int copyDist = dists[symbol] + BitStream_GetBits(context, dext[symbol]);

            while (copyLen--) {
                
                if(context->outCount < context->outLen){
	                context->out[context->outCount] = context->out[context->outCount - copyDist];
	                context->outCount++;
                }
            }
        }
    } while (symbol != 256);

    return 0;
}

static int Read(DeflateContext *context){

	static const u8 order[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
	
    u16 lengths[DEFLATE_MAXCODES];
	
	int bFinal;

	do {

		bFinal = BitStream_GetBits(context, 1);
		
		int bType = BitStream_GetBits(context, 2);

		if(bType != 2) // not a dynamic block.
			break;
	
		int hLen = BitStream_GetBits(context, 5) + 257;
		int hDist = BitStream_GetBits(context, 5) + 1;
		int hCode = BitStream_GetBits(context, 4) + 4;

		if(hLen > DEFLATE_MAXLCODES || hDist > DEFLATE_MAXDCODES)
			break;

		memset(&lengths, 0, 19 * sizeof(u16));

		int k;
		for(k = 0; k < hCode; k++)
			lengths[order[k]] = BitStream_GetBits(context, 3);

		Huffman_Construct(context->lenCnt, context->lenSym, lengths, 19);

		k = 0;
		while(k < hLen + hDist){

			int symbol = Decode(context, context->lenCnt, context->lenSym);

			if(symbol < 16){
				lengths[k++] = symbol;
				continue;
			}

			u16 len = 0;

			if(symbol == 16){
				
				len = lengths[k - 1];
				symbol = 3 + BitStream_GetBits(context, 2);
			
			} else if(symbol == 17){

				symbol = 3 + BitStream_GetBits(context, 3);
			
			} else {

				symbol = 11 + BitStream_GetBits(context, 7);
			}

			while(symbol--)
				lengths[k++] = len;
		}

		Huffman_Construct(context->lenCnt, context->lenSym, lengths, hLen);
		Huffman_Construct(context->distCnt, context->distSym, lengths + hLen, hDist);
		
		Codes(context);

	} while(!bFinal);

	return 0;
}

int Deflate_Read(FILE *fp, void *buffer, int len){

	DeflateContext context;

	memset(&context, 0, sizeof(DeflateContext));

	context.fp = fp;
	context.out = (u8 *)buffer;
	context.outLen = len;

	return Read(&context);
}