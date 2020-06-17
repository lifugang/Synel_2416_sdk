#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../libfunc.h"


#define _2416

int main(int argc, char *argv[])
{
	int port = 3;
	int cur_temp = -1;
	
	while(true)
	{
		cur_temp = MLX90621_RecvData_Event( port );
		if(cur_temp == -1) printf("uart init failed\n");
		else if(cur_temp == -3) printf("get temperature failed\n");
		else{
			printf("current temperature is = %d.%d\n", cur_temp/100, cur_temp%100);
		}
		usleep(50000);
	}
	return 0;
}



