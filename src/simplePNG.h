#ifndef __SIMPLE_PNG_IMPL
#define __SIMPLE_PNG_IMPL

#define __SIMPLE_PNG_DIRECT_INCLUDE  //this line should be uncommented if this file is included directly

#ifdef __SIMPLE_PNG_DIRECT_INCLUDE
#define __SIMPLE_PNG_REQUIRE_STATIC static
#else
#include "simplePNG.h"
#define __SIMPLE_PNG_REQUIRE_STATIC
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define __SIMPLE_PNG_HEADER_SIZE 8
#define __SIMPLE_PNG_CHUNK_NAME_SIZE 4
#define __SIMPLE_PNG_IHDR_SIZE (sizeof(uint32_t)*2+sizeof(uint8_t)*5)
#define __SIMPLE_PNG_IEND_SIZE 0
#define __SIMPLE_PNG_DEFLATE_BLOCK_SIZE 0xffff
#define __SIMPLE_PNG_NUM_CHANNELS 3

/*************** Endian functions ***************/
__SIMPLE_PNG_REQUIRE_STATIC
uint8_t __simplePNG_is_little_endian()
{
	uint16_t val = 1;
	uint8_t * p = (uint8_t*)&val;
	return p[0];
}

__SIMPLE_PNG_REQUIRE_STATIC
void __simplePNG_swap(void *data, size_t size)
{
	uint8_t *data_c = (uint8_t *) data;
	uint8_t temp;
	
	for(size_t i=0; i<size/2; i++)
	{
		temp = data_c[i];
		data_c[i] = data_c[size-i-1];
		data_c[size-i-1] = temp;
	}
}

__SIMPLE_PNG_REQUIRE_STATIC
void __simplePNG_to_lendian(void *data, size_t size)
{
	if(__simplePNG_is_little_endian())
		return;
	else
		__simplePNG_swap(data, size);
}

__SIMPLE_PNG_REQUIRE_STATIC
void __simplePNG_to_bendian(void *data, size_t size)
{
	if(!__simplePNG_is_little_endian())
		return;
	else
		__simplePNG_swap(data, size);
}

/*************** CRC32 functions ***************/
__SIMPLE_PNG_REQUIRE_STATIC
uint32_t __simplePNG_single_byte_crc(uint32_t initial_crc, uint8_t data)
{
	// see PNG spec for an example
	// http://the9375.com/cc/crc/direct_crc.html is also good
	// using this function directly is slow, could precompute to a table
	
	//png crc poly 0000 0100 1100 0001 0001 1101 1011 0111, LSB is x^31
	uint32_t poly = 0xedb88320;
	uint32_t new_crc = initial_crc ^ data;
	
	for(size_t b=0; b<8; b++)
	{
		uint8_t lsb_active = new_crc & 0x01;
		if(lsb_active)
			new_crc = (new_crc >> 1) ^ poly;
		else
			new_crc = new_crc >> 1;
	}
	
	return new_crc;
}

__SIMPLE_PNG_REQUIRE_STATIC
uint32_t __simplePNG_get_crc(uint32_t initial_crc, uint8_t const * data, size_t size)
{
	uint32_t new_crc = initial_crc;
	for(size_t i=0; i<size; i++)
		new_crc = __simplePNG_single_byte_crc(new_crc, data[i]);
	return new_crc;
}

__SIMPLE_PNG_REQUIRE_STATIC
uint32_t __simplePNG_start_crc(uint8_t const * data, size_t size)
{
	uint32_t crc = 0xffffffff;
	crc = __simplePNG_get_crc(crc, data, size);
	return crc;
}

__SIMPLE_PNG_REQUIRE_STATIC
uint32_t __simplePNG_end_crc(uint32_t initial_crc, uint8_t const * data, size_t size)
{
	uint32_t crc = __simplePNG_get_crc(initial_crc, data, size);
	crc = ~crc;
	return crc;
}

/*************** zlib + deflate functions ***************/
__SIMPLE_PNG_REQUIRE_STATIC
uint32_t __simplePNG__adler32(const uint8_t* data, size_t size)
{
	// adler sum: http://www.rfc-editor.org/rfc/rfc1950.txt
	
	uint16_t a = 1;
	uint16_t b = 0;
	uint16_t m = 65521;  //large prime from spec

	for(size_t i=0; i<size; i++)
	{
		
		uint8_t v = data[i];
		a = (a + v) % m;
		b = (b + a) % m;
	}

	__simplePNG_to_bendian(&a, sizeof(a));
	__simplePNG_to_bendian(&b, sizeof(b));

	uint32_t final;
	uint16_t* p = (uint16_t*)&final;
	p[0] = b;
	p[1] = a;

	return final;
}

__SIMPLE_PNG_REQUIRE_STATIC
size_t __simplePNG__final_deflate_block_size(size_t size)
{
	size_t def_final_block_size = size % __SIMPLE_PNG_DEFLATE_BLOCK_SIZE;
	return def_final_block_size;
}

__SIMPLE_PNG_REQUIRE_STATIC
size_t __simplePNG__deflate_block_count(size_t size)
{
	size_t num_deflate_blocks = size / __SIMPLE_PNG_DEFLATE_BLOCK_SIZE;
	size_t def_final_block_size = __simplePNG__final_deflate_block_size(size);
	if(def_final_block_size > 0)
		num_deflate_blocks++;

	return num_deflate_blocks;
}

__SIMPLE_PNG_REQUIRE_STATIC
size_t __simplePNG__store_deflate_blocks(uint8_t* dst, const uint8_t* src, size_t size)
{
	size_t dst_offset = 0;
	size_t img_offset = 0;
	size_t num_deflate_blocks = __simplePNG__deflate_block_count(size);
	size_t def_final_block_size = __simplePNG__final_deflate_block_size(size);
	uint16_t* u16_ptr;

	for(size_t i=0; i<num_deflate_blocks; i++)
	{
		//deflate header: http://www.rfc-editor.org/rfc/rfc1951.txt
		//1 bit : final block
		//2 bit : type = 00 no compression
		//next byte boundary
		uint8_t def_final = 0x00;
		uint8_t is_last_block = i+1 == num_deflate_blocks;
		if(is_last_block)
			def_final = 0x01; //first bit 1 means last block
		uint8_t def_compression = 0x00; //no compression
		uint8_t def_header = def_final | def_compression;

		//deflate uncompressed block format
		//2 byte : len
		size_t block_size = __SIMPLE_PNG_DEFLATE_BLOCK_SIZE;
		if(is_last_block)
			block_size = def_final_block_size;
		uint16_t def_len = block_size;
		__simplePNG_to_lendian(&def_len, sizeof(def_len));
		//2 byte : bitwise_not(len)
		uint16_t def_nlen = ~def_len;

		dst[dst_offset] = def_header;
		dst_offset++;
		u16_ptr = (uint16_t*) (dst+dst_offset);
		u16_ptr[0] = def_len;
		u16_ptr[1] = def_nlen;
		dst_offset += sizeof(uint16_t) * 2;

		//block data
		memcpy(dst+dst_offset, src+img_offset, block_size);
		img_offset += block_size;
		dst_offset += block_size;
	}

	return dst_offset;
}

/*************** fopen function ***************/
__SIMPLE_PNG_REQUIRE_STATIC
FILE* __simplePNG__sfopen(char const * name, char const * mode)
{
	FILE *file = NULL;
	file = fopen(name, mode);
	
	if(file == NULL)
	{
		fprintf(stderr, "Unable to open file '%s' in mode '%s'.\n", name, mode);
		exit(2);
	}
	
	return file;
}

/*************** PNG functions ***************/
__SIMPLE_PNG_REQUIRE_STATIC
void __simplePNG__write_chunk(FILE * f, uint8_t const chunk_type[__SIMPLE_PNG_CHUNK_NAME_SIZE], uint8_t const * chunk_data, uint32_t length)
{	
	uint32_t endian_length = length;
	
	__simplePNG_to_bendian(&endian_length, sizeof(endian_length));
	fwrite(&endian_length, sizeof(endian_length), 1, f);
	
	fwrite(chunk_type, sizeof(uint8_t), __SIMPLE_PNG_CHUNK_NAME_SIZE, f);
	
	fwrite(chunk_data, sizeof(uint8_t), length, f);
	
	uint32_t crc_val = __simplePNG_start_crc( chunk_type, __SIMPLE_PNG_CHUNK_NAME_SIZE);
	crc_val = __simplePNG_end_crc(crc_val, chunk_data, length);
	__simplePNG_to_bendian(&crc_val, sizeof(crc_val));
	fwrite(&crc_val, sizeof(crc_val), 1, f);
}

__SIMPLE_PNG_REQUIRE_STATIC
void __simplePNG_write_IHDR(FILE * f, uint32_t width, uint32_t height, uint8_t const * rgb_image)
{
	uint8_t chunk_type[__SIMPLE_PNG_CHUNK_NAME_SIZE];
	uint32_t* u32_ptr;
	
	chunk_type[0] = 'I';
	chunk_type[1] = 'H';
	chunk_type[2] = 'D';
	chunk_type[3] = 'R';
	
	uint8_t ihdr_chunk[__SIMPLE_PNG_IHDR_SIZE];
	
	uint32_t w = width;
	uint32_t h = height;
	__simplePNG_to_bendian(&w, 4);
	__simplePNG_to_bendian(&h, 4);
	uint8_t depth = 8;
	uint8_t color = 2; //truecolor (no alpha)
	uint8_t compression = 0;
	uint8_t filter = 0;
	uint8_t interlace = 0;
	
	u32_ptr = (uint32_t*)(ihdr_chunk);
	u32_ptr[0] = w;
	u32_ptr[1] = h;
	ihdr_chunk[8] = depth;
	ihdr_chunk[9] = color;
	ihdr_chunk[10] = compression;
	ihdr_chunk[11] = filter;
	ihdr_chunk[12] = interlace;
	
	__simplePNG__write_chunk(f, chunk_type, ihdr_chunk, __SIMPLE_PNG_IHDR_SIZE);
}

__SIMPLE_PNG_REQUIRE_STATIC
void __simplePNG_write_IDAT(FILE * f, uint32_t width, uint32_t height, uint8_t const * rgb_image)
{
	uint8_t chunk_type[__SIMPLE_PNG_CHUNK_NAME_SIZE];
	uint32_t* u32_ptr;
	
	size_t img_size = width*height*__SIMPLE_PNG_NUM_CHANNELS;
	size_t img_size_with_filter = img_size + height; //one filter byte per scanline
	size_t zlib_meta_size = sizeof(uint8_t)*2 + sizeof(uint32_t); //2 bytes: cmf and flg, 1 adler32 sum
	size_t num_deflate_blocks = __simplePNG__deflate_block_count(img_size_with_filter);
	size_t deflate_meta_per_block = (sizeof(uint8_t)+sizeof(uint16_t)*2); //1 header byte, 2 16bit lengths
	size_t deflate_size = img_size_with_filter + deflate_meta_per_block*num_deflate_blocks;
	size_t idat_size = zlib_meta_size + deflate_size;
	
	//insert the scanline filter code
	uint8_t* img_with_filter = (uint8_t*)malloc(img_size_with_filter);
	for(size_t i=0; i<height; i++)
	{
		size_t scanline_offset = i*width*__SIMPLE_PNG_NUM_CHANNELS+i;
		img_with_filter[scanline_offset] = 0x00; //no filter
		memcpy(img_with_filter + scanline_offset + 1, rgb_image+(i*width*__SIMPLE_PNG_NUM_CHANNELS), width*__SIMPLE_PNG_NUM_CHANNELS);
	}
	
	uint8_t* idat_chunk = (uint8_t*)malloc(idat_size);
	
	chunk_type[0] = 'I';
	chunk_type[1] = 'D';
	chunk_type[2] = 'A';
	chunk_type[3] = 'T';
	
	//zlib header: http://www.rfc-editor.org/rfc/rfc1950.txt
	//window & compression: 0x08
	//flags: compression none 00, no dict 0, multiple of 31 11101 (1d)
	uint8_t zlib_compression = 0x08; //8 = deflate
	uint8_t zlib_window = 0x00; //0 = smallest window
	uint8_t zlib_cmf = zlib_window << 4 | zlib_compression;
	uint8_t zlib_dict = 0x00; //no dict
	uint8_t zlib_flevel = 0x00; //least amount of compression
	uint8_t zlib_fcheck = 0x1d; //to make cmf & flg bytes multiple of 31
	uint8_t zlib_flg = zlib_dict | zlib_flevel | zlib_fcheck;
	
	idat_chunk[0] = zlib_cmf;
	idat_chunk[1] = zlib_flg;
	size_t idat_offset = 2;
	
	size_t deflate_bytes = __simplePNG__store_deflate_blocks(idat_chunk+idat_offset, img_with_filter, img_size_with_filter);
	
	//zlib adler32
	uint32_t zlib_adler = __simplePNG__adler32(img_with_filter, img_size_with_filter);
	u32_ptr = (uint32_t*) (idat_chunk + idat_offset + deflate_size);
	u32_ptr[0] = zlib_adler;
	
	__simplePNG__write_chunk(f, chunk_type, idat_chunk, idat_size);
	free(img_with_filter);
	free(idat_chunk);
}

__SIMPLE_PNG_REQUIRE_STATIC
void __simplePNG_write_IEND(FILE * f)
{
	uint8_t chunk_type[__SIMPLE_PNG_CHUNK_NAME_SIZE];
	//uint8_t iend_chunk[__SIMPLE_PNG_IEND_SIZE];
	
	chunk_type[0] = 'I';
	chunk_type[1] = 'E';
	chunk_type[2] = 'N';
	chunk_type[3] = 'D';
	
	__simplePNG__write_chunk(f, chunk_type, NULL, __SIMPLE_PNG_IEND_SIZE);
}

__SIMPLE_PNG_REQUIRE_STATIC
int simplePNG_write(char const * filename, uint32_t width, uint32_t height, uint8_t const * rgb_image)
{
	//png spec: http://www.libpng.org/pub/png/spec/1.2/
	FILE* f = __simplePNG__sfopen(filename, "wb");

	//header
	uint8_t png_header[__SIMPLE_PNG_HEADER_SIZE] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
	fwrite(png_header, 1, __SIMPLE_PNG_HEADER_SIZE, f);

	//ihdr chunk
	__simplePNG_write_IHDR(f, width, height, rgb_image);

	//idat
	__simplePNG_write_IDAT(f, width, height, rgb_image);
	
	//iend
	__simplePNG_write_IEND(f);

	fclose(f);

	return 0;
}

#endif
