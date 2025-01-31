#ifndef _LZS_COMPRESS
#define _LZS_COMPRESS

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "bitstream.h"
#include <stdint.h>
#include <stdbool.h>

#define WINDOW_SIZE 2048
#define WINDOW_MASK WINDOW_SIZE-1
#define PREFIX_SIZE 2
#define HASHCHAIN_SIZE WINDOW_SIZE*2
#define HASHCHAIN_MASK HASHCHAIN_SIZE-1
#define MIN_MATCH_LEN 2
//this is absurdly high, but time for compression tradeoff is incredible
#define MAX_HIT_COUNT WINDOW_SIZE 

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef int_fast32_t pos_t;

#define DEBUG_COMPRESS
#undef DEBUG_COMPRESS

#ifdef DEBUG_COMPRESS
# define DEBUG_PRINT(x) PySys_WriteStdout x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

struct _hashchain
{
    pos_t head_table[HASHCHAIN_SIZE];
    pos_t chain[HASHCHAIN_SIZE];
};
typedef struct _hashchain hashchain;
hashchain* new_hashchain();

byte* lzs_compress(byte *data, pos_t data_len, bool optimal, size_t *out_size);

static uint_fast32_t lzs_hash_prefix(byte *buffer, pos_t pos);
static uint_fast16_t lzs_hash16(uint_fast16_t x);
static uint_fast32_t lzs_hash32(uint_fast32_t x);
static void lzs_hash_insert(hashchain *hc, byte *buffer, pos_t pos);
static int lzs_simple_find_match(byte *buffer, byte *buffer_end, pos_t pos, pos_t *match_pos);
static int lzs_hash_find_match(hashchain *hc, byte *buffer, byte *buffer_end, pos_t pos, pos_t *match_pos);
static inline uint_fast16_t lzs_match_len(byte *aPtr, byte *bPtr, const byte *end);

#endif