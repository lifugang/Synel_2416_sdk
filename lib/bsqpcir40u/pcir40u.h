#ifndef PCIR40U_H_
#define PCIR40U_H_


int MLX90621_RecvData_Event( int port );
int DT8861_RecvData_Event( int port );
int MLX90621_Get_Version(int port);
int MLX90621_Firmware_Update( int port, char *bin_name );




#endif

