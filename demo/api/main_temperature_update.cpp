#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../libfunc.h"


#define _2416


void get_stdin_str(char *oStr)
{
	while(1)
	{
		fgets(oStr, 64, stdin);
		if(strlen(oStr) > 6)
		{
			break;
		}

		usleep(50000);
	}
}

int main(int argc, char *argv[])
{
	int port = 3; // if usb module， please set port to 30+port
	int cur_temp = -1;
	int ret = -1;

	port = 30 + 0;// mean /dev/ttyUSB0
//	port = 30 + 1;// mean /dev/ttyUSB1

	int cnt = 0;

	char _bin_name[64] = "", bin_name[64] = "";


	while(true)
	{
		cnt++;
		cur_temp = MLX90621_RecvData_Event( port );
		if(cur_temp == -1) printf("uart init failed\n");
		else if(cur_temp == -3) {
		}
		else{
			printf("current temperature(%d) is = %d.%d\n", cnt, cur_temp/100, cur_temp%100);			
		}
		usleep(50000);

		if(cnt > 2)
			break;
	}
	
	ret = MLX90621_Get_Version(port);
	if(ret > 0)
	{
		printf("\ncurrent firmware version = [%d]\n", ret);
	}

	sleep(1);
	printf("\nnow ready to update module\n-------------------------please input file name\n");

	getcwd(bin_name, sizeof(bin_name));
	get_stdin_str(_bin_name);
	_bin_name[strlen(_bin_name) - 1] = 0; // cut '\n'
	strcat(bin_name, "/");
	strcat(bin_name, _bin_name);
	
	printf("bin name = %s\n", bin_name);
	sleep(1);

	ret = MLX90621_Firmware_Update( port, bin_name );
	if(ret == 0)
	{
		sleep(1);
		ret = MLX90621_Get_Version(port);
		if(ret > 0)
		{
			printf("\nnow new firmware version = [%d]\n", ret);
		}		
	}
	else
	{
		printf("update failed\n");
	}

	return 0;
}



