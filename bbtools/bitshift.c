#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
	uint64_t a = strtoull(argv[1], NULL, 0);
	int b = atoi(argv[2]);
	
	if (b < 0)
	{
		b = -b;
		printf("0x%.16lx\n", a<<b);
	}
	else
	{
		printf("0x%.16lx\n", a>>b);
	}
	return 0;
}
