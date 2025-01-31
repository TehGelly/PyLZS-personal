#include "lzs_decompress.h"

byte* lzs_decompress(byte *data, size_t data_len, size_t expected_size, size_t *out_size)
{
    bitstream *bs = new_bitstream();
	if(bs == NULL)
		return NULL;
	bs->buffer = data;
	DEBUG_PRINT(("Bitstream initialized...\n"));
	
	byte *out_bytes = (byte *)calloc(expected_size, sizeof(byte));
	if(out_bytes == NULL)
		return PyErr_NoMemory();
	uint_fast32_t out_pos = 0;
	DEBUG_PRINT(("Output callocated...\n"));
	
	while(true) //loops to parse each new token
	{
		//error handling
		if(bs->bytepos > data_len)
		{
			char err_str[100];
			snprintf(err_str,100,"Decompression failed to find end token. (%d > %d)", bs->bytepos, data_len);
			PyErr_SetString(PyExc_EOFError,err_str);
			free(bs);
			return NULL;
		}
		//then token parsing
		//modelled from scummvm's decompressor.cpp
		if(read_bits(bs,1))
		{
			//compression handling
			uint_fast16_t offset = 0;
			
			DEBUG_PRINT(("Handling decompression (or terminal)...\n"));
			if(read_bits(bs,1))
			{
				//seven bit offset
				offset = read_bits(bs,7);
				if(!offset) //offset of 0 len 7 indicates EOF
					break;
			}
			else
			{
				//eleven bit offset
				offset = read_bits(bs,11);
			}
			
			//get length of compressed bytestr
			uint_fast32_t clen = _get_comp_len(bs);
			DEBUG_PRINT(("Offset: %d, Length: %d\n", offset, clen));
			
			//and finish with copy to output
			if(_copy_comp(out_bytes, &out_pos, offset, clen) == -1)
			{
				free(bs);
				return NULL;
			}
		}
		else
		{
			//write literal to output
			byte literal = (byte)read_bits(bs,8);
			DEBUG_PRINT(("Writing literal %d to output...\n", literal));
			out_bytes[out_pos++] = literal;
		}
	}
	free(bs);

    *out_size = out_pos;
    return out_bytes;
}

static uint_fast32_t _get_comp_len(bitstream *bs)
{
	uint_fast32_t clen;
	uint_fast16_t nibble;
	switch(read_bits(bs,2))
	{
		case 0:
			return 2;
		case 1:
			return 3;
		case 2:
			return 4;
		default:
			switch(read_bits(bs,2))
			{
				case 0:
					return 5;
				case 1:
					return 6;
				case 2:
					return 7;
				default:
					clen = 8;
					do 
					{
						nibble = read_bits(bs,4);
						clen += nibble;
					} while (nibble == 0xf);
					return clen;
			}
	}
}

static int _copy_comp(byte *data, uint_fast32_t *data_pos, uint_fast16_t offset, uint_fast32_t clen)
{
	//there's a couple ways to do this and i'm still not sure what the right way is.
	//it probably gets compiled all the same, right?
	uint_fast32_t temp_pos = *data_pos;
	if(temp_pos < offset)
	{
		char err_str[100];
		snprintf(err_str,sizeof(err_str),"Illegal offset to reference %d when buffer is length %d.", offset, temp_pos);
		PyErr_SetString(PyExc_IndexError,err_str);
		return -1;
	}
	while(clen--)
	{
		data[temp_pos] = data[temp_pos - offset];
		temp_pos++;
	}
	*data_pos = temp_pos;
	return 0;
}