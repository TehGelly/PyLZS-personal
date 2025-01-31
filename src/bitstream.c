#include "bitstream.h"

//bitstream code inspired by awakecoding
//https://stackoverflow.com/questions/22202879/efficient-c-bitstream-implementation

//TODO
//now you may be wondering, "why aren't you using a pre-written implementation?"
//for the love of god, show me one.
//it's apparently a rite of passage to write your own terrible bitstream
//if you know a good own, please make a pull request

bitstream* new_bitstream()
{
	bitstream *bs = NULL;
	
	bs = (bitstream*)calloc(1, sizeof(bitstream));
	if(bs == NULL)
        //tbf i _guess_ this merits writing your own bitstream implementation
		return PyErr_NoMemory();
	
	return bs;
}

//reads req_amt bits from bs (MSB order)
//TODO: so because this doesn't make any bounds checks,
//this is technically insecure 
//(you could make it be an oracle, to some extent, for the data past the end of the buffer)
//fix this before trying to add to pypl
uint_fast16_t read_bits(bitstream *bs, uint_fast8_t req_amt)
{
	//amount will always be under 12
	//we only cross, at most, two byte boundaries
	uint_fast8_t cur_amt = 0;
	uint_fast16_t result = 0;
	while(cur_amt < req_amt)
	{
		//first, let's see if we get off the hook with an easy read from byte
		//(catch condition)
		uint_fast8_t remainder = req_amt - cur_amt;
		int byte_space = 8 - bs->bitpos;
		if(remainder < byte_space)
		{
			//then we can simply grab those bits, adjust bitpos, and leave
			result <<= remainder;
			uint_fast16_t rem_mask = (1<<byte_space) - 1;
			result |= (bs->buffer[bs->bytepos] & rem_mask) >> (byte_space - remainder);
			cur_amt = req_amt;
			bs->bitpos += remainder;
		}
		else
		{
			//well, now we have to cross a byte boundary
			//we'll give up halfway by grabbing what we can, then going to the next byte
			result <<= byte_space;
			uint_fast16_t rem_mask = (1<<byte_space) - 1;
			result |= (bs->buffer[bs->bytepos++] & rem_mask);
			cur_amt += byte_space;
			bs->bitpos = 0;
			//if(bs->buffer + bs->bytepos == bs->buffer_end)
		}
	}
	return result;
}

//writes req_amt bits from value into bs (MSB order)
void write_bits(bitstream *bs, uint_fast16_t value, uint_fast8_t req_amt)
{
	uint_fast8_t cur_amt = 0;
	while(cur_amt < req_amt)
	{
		//first, let's see if we get off the hook with an easy write to byte
		//(catch condition)
		uint_fast8_t remainder = req_amt - cur_amt;
		int byte_space = 8 - bs->bitpos;
		if(remainder < byte_space)
		{
			//mask out all but the last remainder bits of the value
			//and append to the buffer (buffer assumed to be zeroes)
			uint_fast16_t value_mask = (1 << (remainder)) - 1;
			bs->buffer[bs->bytepos] |= ((value & value_mask) << (byte_space - remainder));
			cur_amt = req_amt;
			bs->bitpos += remainder;
		}
		else
		{
			//well, just fill up this byte and try again later
			//this mask grabs the remaining length of byte from the MSB end 
			//(shift up by the remainder minus that length)
			uint_fast16_t value_mask = ((1 << byte_space) - 1) << (remainder - byte_space);
			bs->buffer[bs->bytepos++] |= ((value & value_mask) >> (remainder - byte_space));
			cur_amt += byte_space;
			bs->bitpos = 0;
		}
	}
	return;
}