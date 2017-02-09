#include <stdio.h>
#include <stdint.h>

/**
 * 0001 0000 as occupiedmask with 4 as position generates 1110 0000
 * 1101 0111 as occupiedmask with 4 as position generates 0110 0000
 */
uint64_t leftray(int mask /* 0-255 */, int pos /* 0-7 */)
{
	int remainingbits = 7 - pos;
	uint64_t result = 0;
	int curindex = pos+1;
	mask >>= pos+1; /* considering only the bits left of pos */
	/* note the degenerate case rook on h file, pos 7 - result stays 0 */
	while (remainingbits > 0)
	{
		result |= (((uint64_t)1) << (curindex*8));
		
		/* comes after setting the bit! */
		if (mask & 0x1)
		{
			break;
		}
		else
		{
			mask >>= 1;
			remainingbits--;
			curindex++;
		}
	}
	return result;
}
/**
 * 0001 0000 as occupiedmask with 4 as position generates 0000 1111
 * 1101 0111 as occupiedmask with 4 as position generates 0000 1100
 */
uint64_t rightray(int mask /* 0-255 */, int pos /* 0-7 */)
{
	int remainingbits = pos;
	uint64_t result = 0;
	int curindex = pos-1;
	
	while (remainingbits > 0)
	{
		result |= (((uint64_t)1) << (8*curindex));
		if (mask & (1<<curindex))
		{
			break;
		}
		else
		{
			remainingbits--;
			curindex--;
		}
	}
	return result;
}
int main()
{
	int occmask;
	int pos;
	uint64_t result;

	for (occmask = 0; occmask < 256; occmask++)
	{
		printf("\t{ ");
		
		for (pos = 0; pos < 8; pos++)
		{
			/* if (pos == 4) printf("\n\t   "); */
			printf("U64(");
			
			result = (leftray(occmask, pos) | rightray(occmask, pos));
			
			printf("0x%.16lx)", result);
			if (pos < 7) printf(",");
			printf(" ");
		}

		printf("}");
		if (occmask < 255) printf(",");
		printf(" /* 0x%.2x */\n", occmask);
	}
	return 0;
}
