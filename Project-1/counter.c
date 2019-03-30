#include <stdio.h>
#include <unistd.h>
 
int main()
{
	for(int i = 0; i < 3; i++)
	{
		printf("%d\n", i + 1);
		sleep(1);
	}
	return 0;
}
