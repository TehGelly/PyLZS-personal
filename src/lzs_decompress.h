#ifndef _LZS_DECOMPRESS
#define _LZS_DECOMPRESS

#include "bitstream.h"
#include <stdbool.h>

#define DEBUG_DECOMPRESS
#undef DEBUG_DECOMPRESS

#ifdef DEBUG_DECOMPRESS
# define DEBUG_PRINT(x) PySys_WriteStdout x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

byte* lzs_decompress(byte *data, size_t data_len, size_t expected_size, size_t *out_size);
static uint_fast32_t _get_comp_len(bitstream *bs);
static int _copy_comp(byte *data, uint_fast32_t *data_pos, uint_fast16_t offset, uint_fast32_t clen);

#endif