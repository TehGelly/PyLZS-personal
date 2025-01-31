#ifndef _LZS_BITSTREAM
#define _LZS_BITSTREAM

#include <stdint.h>
#include <Python.h>
typedef unsigned char byte;

struct _bitstream
{
	byte *buffer;
	//byte *buffer_end;
	uint_fast32_t bytepos;
	uint_fast8_t bitpos;
};
typedef struct _bitstream bitstream;

uint_fast16_t read_bits(bitstream *bs, uint_fast8_t req_amt);
void write_bits(bitstream *bs, uint_fast16_t value, uint_fast8_t req_amt);
bitstream* new_bitstream();

#endif