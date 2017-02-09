#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int shiftamt[15] = { 0, 1, 3, 6, 10, 15, 21, 28, 36, 43, 49, 54, 58, 61, 63 }; 
int length[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1 };

int main(int argc, char **argv)
{
	uint64_t brd = strtoull(argv[1], NULL, 0);
	int diag;
	char row;
	int i;
	
	for (diag = 14; diag >= 0; diag--)
	{
		row = (brd >> shiftamt[diag]) & 0xff;
		for (i = 0; i < 8-length[diag]; i++)
		{
			printf("  ");
		}
		for (i = 0; i < length[diag]; i++)
		{
			char square;
			square = ((row >> i) & 0x1) ? '#' : '.';
			printf("%c%c  ", square, square);
		}
		printf("\n");
	}
	return 0;
}
