#include <png.h>
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;

u8 *LoadImage(char *path, int *w, int *h, int *channels){

	FILE *fp = fopen(path,"rb");

	if( fp == NULL ){
		printf("Error loading PNG %s: No such file.\n", path);
		return NULL;
	}

	u8 header[8];
	fread(header, 1, 8, fp);
	int ispng = !png_sig_cmp(header, 0, 8);

	if(!ispng){
		fclose(fp);
		printf("Not png %s\n", path);
		return NULL;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if(!png_ptr) {
		printf("Error loading %s\n",path );
		fclose(fp);
		return NULL;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){
		printf("Error loading %s\n",path );
		fclose(fp);
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return NULL;
	}

	if(setjmp(png_jmpbuf(png_ptr))){
		printf("Error loading %s\n",path );
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return NULL;
	}

	png_set_sig_bytes(png_ptr, 8);
	png_init_io(png_ptr, fp);
	png_read_info(png_ptr, info_ptr);

	int bit_depth, color_type;
	png_uint_32 twidth, theight;

	png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

	if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	if(color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

	if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	if(bit_depth < 8)
		png_set_packing(png_ptr);

	if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
		*channels = 3;
		// png_set_add_alpha(png_ptr, 255, PNG_FILLER_AFTER);
	else
		*channels = 4;

	png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

	*w = twidth;
	*h = theight;

	png_read_update_info(png_ptr, info_ptr);

	int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	png_byte *imageData = (png_byte *)malloc(sizeof(png_byte) * rowbytes * theight);
	if(!imageData){
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return NULL;
	}

	png_bytep *row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * theight);
	if(!row_pointers){
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		free(imageData);
		fclose(fp);
		return NULL;
	}

	int i;
	for(i = 0; i < (int)theight; ++i)
		row_pointers[theight - 1 - i] = imageData + i * rowbytes;

	png_read_image(png_ptr, row_pointers);
	png_read_end(png_ptr, NULL);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	free(row_pointers);

	fclose(fp);

	return (u8 *)imageData;
}

int morton(int x, int y){

/*
	https://github.com/citra-emu/citra/blob/5d635986bf789bcec9615d3750c6766dbff35d7e/src/video_core/utils.h

	Images are split into 8x8 tiles. Each tile is composed of four 4x4 subtiles each
	of which is composed of four 2x2 subtiles each of which is composed of four texels.
	Each structure is embedded into the next-bigger one in a diagonal pattern, e.g.
	texels are laid out in a 2x2 subtile like this:
	2 3
	0 1

	The full 8x8 tile has the texels arranged like this:

	42 43 46 47 58 59 62 63
	40 41 44 45 56 57 60 61
	34 35 38 39 50 51 54 55
	32 33 36 37 48 49 52 53
	10 11 14 15 26 27 30 31
	08 09 12 13 24 25 28 29
	02 03 06 07 18 19 22 23
	00 01 04 05 16 17 20 21

	This pattern is what's called Z-order curve, or Morton order.
*/

	int coarse = x & ~7;
	
	int i = (x & 7) | ((y & 7) << 8); // ---- -210
	i = (i ^ (i << 2)) & 0x1313;      // ---2 --10
	i = (i ^ (i << 1)) & 0x1515;      // ---2 -1-0
	i = (i | (i >> 7)) & 0x3F;
	
	return ((coarse * 8) + i);
}

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

	if(argc < 3) return 1;

	int w, h, channels;
	u8 *img = LoadImage(argv[1], &w, &h, &channels);

	if(!img){
		printf("Could not load image.\n");
		return 1;
	}

	FILE *fp = fopen(argv[2], "wb");

	if(!fp) {
		printf("Could not open output.\n");
		free(img);
		return 1;
	}

	int size = w * h * channels;

	// u8 *data = (u8 *)malloc(size);

	// int y, x;

	// for(y = 0; y < h; y++){

		// for(x = 0; x < w; x++){

		// 	int coarseY = y & ~7;

		// 	int offset = (morton(x, y) * channels) + (coarseY * w * channels);

		// 	u8 *pixel = img + ((((h-1-y) * w) + x) * channels);

		// 	u8 *outPixel = data + offset;

		// 	if(channels == 4){
		// 		outPixel[3] = pixel[0];
		// 		outPixel[2] = pixel[1];
		// 		outPixel[1] = pixel[2];
		// 		outPixel[0] = pixel[3];
		// 	} else {
		// 		outPixel[2] = pixel[0];
		// 		outPixel[1] = pixel[1];
		// 		outPixel[0] = pixel[2];
		// 	}
		// }
	// }

	// free(img);

	fwrite(&w, 1, sizeof(int), fp);
	fwrite(&h, 1, sizeof(int), fp);

	// fwrite(data, 1, size, fp);

	// Compress(data, size, fp);

	Compress(img, size, fp);

	fclose(fp);

	// free(data);
	free(img);

	return 0;
}