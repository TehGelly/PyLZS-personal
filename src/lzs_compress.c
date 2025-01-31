#include "lzs_compress.h"

//this whole file borrows from https://glinscott.github.io/lz/index.html#toc4.2.2
//and also in part craig mcqueen's (cmcqueen's) github implementation
//it seems like LZS is a "cascaded window" implementation of lempel-ziv
//but i'm not gonna smoke myself out on suffix array sets or whatever the heck
//so we're going with the "trivial" hash-links/hash-chain implementation

byte* lzs_compress(byte *data, pos_t data_len, bool optimal, size_t *out_size)
{
    DEBUG_PRINT(("Compressing data of length %d\n", data_len));
    bitstream *bs = new_bitstream();
    if(bs == NULL)
        return NULL;
    DEBUG_PRINT(("Bitstream initialized...\n"));

    byte *new_buffer = (byte *)calloc(data_len*9/8+2,sizeof(byte));
    if(new_buffer == NULL)
    {
        free(bs);
        return PyErr_NoMemory();
    }
    bs->buffer = new_buffer;
    DEBUG_PRINT(("Buffer callocated...\n"));

    hashchain *hc = new_hashchain();
    if(hc == NULL)
    {
        free(bs->buffer);
        free(bs);
        return NULL;
    }
    DEBUG_PRINT(("Hashchain callocated...\n"));

    //initialize with the first byte, which will always be a literal
    write_bits(bs, data[0], 9);

    for (pos_t i = 1; i < data_len; i++)
    {
        DEBUG_PRINT(("%d.\n", i));
        pos_t match_pos;
        int match_len = 0;
        if(!optimal)
            match_len = lzs_hash_find_match(hc, data, data+data_len-1, i, &match_pos);
        else
            match_len = lzs_simple_find_match(data, data+data_len-1, i, &match_pos);
        
        if(match_len >= MIN_MATCH_LEN && i < data_len)
        {
            
            //encode the matching position and length
            pos_t offset = i - match_pos;
            DEBUG_PRINT(("Offset=%d, from i=%d and match_pos=%d\n",offset,i,match_pos));
            if(offset < 128)
                write_bits(bs,0x180 | offset,9);
            else if(offset < 2048)
                write_bits(bs,0x1000 | offset,13);
            else
            {
                char err_str[100];
		        snprintf(err_str,sizeof(err_str),"Illegal calculated offset of %X. (talk to the dev/maint about this one)", offset);
		        PyErr_SetString(PyExc_IndexError,err_str);
                return NULL;
            }
            DEBUG_PRINT(("Match found! Offset %d, position %d, length %d.\n", offset, match_pos, match_len));
            //write the match_len
            if(match_len < 5)
                write_bits(bs,match_len-2,2);
            else if(match_len < 8)
                write_bits(bs,0xC | match_len-5,4);
            else
            {
                write_bits(bs,0xf,4);
                int tml = match_len-8;
                while(tml > 14)
                {
                    write_bits(bs,0xf,4);
                    tml -= 15;
                }
                write_bits(bs,tml,4);
            }

            //reup the hashchain
            while(--match_len > 0)
                lzs_hash_insert(hc, data, ++i);
        }
        else
        {
            //add a literal byte
            DEBUG_PRINT(("No match found, writing literal %d\n",data[i]));
            write_bits(bs, data[i], 9);
        }
    }
    //write end token
    write_bits(bs,0x180,9);
    *out_size = bs->bytepos;
    //DEBUG_PRINT(("Done! Bytes at address %X, with out_size of %d\n",bs->buffer,bs->bytepos));
    return bs->buffer;
}

hashchain* new_hashchain()
{
	hashchain *hc = NULL;
	
	hc = (hashchain*)calloc(1, sizeof(hashchain));
	if(hc == NULL)
		return PyErr_NoMemory();
	
    //initialize the tables with all 1s for performance
    memset(hc->head_table,0xff,HASHCHAIN_SIZE*sizeof(pos_t));
    memset(hc->chain,0xff,HASHCHAIN_SIZE*sizeof(pos_t));
	return hc;
}

//from hash-prospector
static uint_fast16_t lzs_hash16(uint_fast16_t x)
{
    x ^= x >> 8; x *= 0x88b5U;
    x ^= x >> 7; x *= 0xdb2dU;
    x ^= x >> 9;
    return x;
}

//from hash-prospector
static uint_fast32_t lzs_hash32(uint_fast32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

static uint_fast32_t lzs_hash_prefix(byte *buffer, pos_t pos)
{
    switch(PREFIX_SIZE)
    {
        case 1:
            return lzs_hash16(buffer[pos]);
        case 2:
            return lzs_hash16(buffer[pos] | (buffer[pos + 1] << 8));
        case 3:
            return lzs_hash32(buffer[pos] | (buffer[pos + 1] << 8 | (buffer[pos + 1] << 16 )));
        default:
            return 0;
    };
}

void lzs_hash_insert(hashchain *hc, byte *buffer, pos_t pos)
{
    //minimum match is PREFIX_SIZE
    int key = lzs_hash_prefix(buffer, pos);
    key &= HASHCHAIN_MASK;
    // ht_[key] is the first element in the linked list - add it to the chain.
    hc->chain[pos & HASHCHAIN_MASK] = hc->head_table[key];
    // insert pos as the first element in the list.
    hc->head_table[key] = pos;
    return;
}

static int lzs_simple_find_match(byte *buffer, byte *buffer_end, pos_t pos, pos_t *match_pos)
{
    int best_len = 0;

    pos_t min_pos = MAX(0, pos - WINDOW_SIZE);
    for(pos_t i = pos - 1; i > min_pos; i--)
    {
        int match_len = lzs_match_len(buffer + pos, buffer + i, buffer_end);

        if(match_len > best_len)
        {
            best_len = match_len;
            *match_pos = i;
        }
    }

    return best_len;
}

static int lzs_hash_find_match(hashchain *hc, byte *buffer, byte *buffer_end, pos_t pos, pos_t *match_pos)
{
    int best_len = 0;

    int key = lzs_hash_prefix(buffer, pos);
    key &= HASHCHAIN_MASK;
    pos_t next = hc->head_table[key];
    //DEBUG_PRINT(("Next: %d, Key: %d\n",next, key));

    //look back through the window
    pos_t min_pos = MAX(0, pos - WINDOW_SIZE);
    int hits = 0;
    while(next > min_pos && ++hits < MAX_HIT_COUNT)
    {
        int match_len = lzs_match_len(buffer + pos, buffer + next, buffer_end);
        //if(match_len != 0)
            //DEBUG_PRINT(("Candidate %d, at position %d, has length %d.\n", hits, next, match_len));
        if(match_len > best_len)
        {
            best_len = match_len;
            *match_pos = next;
        }

        next = hc->chain[next & HASHCHAIN_MASK];
    }

    hc->chain[pos & HASHCHAIN_MASK] = hc->head_table[key];
    hc->head_table[key] = pos;

    return best_len; //no post-chain optimization

    //POST-CHAIN OPTIMIZATION:
    //run back through lengths to get better matches
    //HOWEVER: this only saves maybe 500 bytes out of the 7k items i've been testing
    //(avg 0.06 byte/item)
    //and also it crashes so there's some weird bug so
    //TODO: fix bug and determine if tradeoffs are worth it
    pos_t candidate_pos = *match_pos;
    //degenerate run-length encoding of repeated byte
    while((candidate_pos > 0) && (2047 > (pos-candidate_pos)) && (2047 > best_len) &&
            lzs_match_len(buffer + pos, buffer + candidate_pos - 1, buffer_end) > best_len)
    {
        best_len++;
        candidate_pos--;
    }

    *match_pos = candidate_pos;

    return best_len;
}

static inline uint_fast16_t lzs_match_len(byte *aPtr, byte *bPtr, const byte* end)
{
    uint_fast16_t len = 0;

    //the python one-liner-writer in me got to me
    while(aPtr != (end+1) && *aPtr++ == *bPtr++)
        len++;

    return len;
}