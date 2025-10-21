#include "common.h"
#include "chsum.h"

u32 chsum32 (u8 *data, u32 len, u32 start_value)
{
	int msb;

	while (len--)
	{
		msb = (start_value >> 31) & 1;
		start_value = (start_value<<1) + msb;
		start_value += *data;
		data++;
	}
	return (start_value);
}

