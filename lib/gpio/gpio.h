#ifndef __GPIO_H
#define __GPIO_H
#include "../_precomp.h"
#include "../version.h"

#define get_DAT 0x4800
#define put_DAT 0x4801
#define con_DAT 0x4802
#define get_GPFDAT 0x4800
#define put_GPFDAT 0x4801


/****************************
pin define:

7	128	red-led
6	64	green-led
5	32	lcd-backlight
4	16	input-point
3	8	relay
2	4	camera
1	2	NC
0	1	NC
*****************************/
 typedef struct _Gpio_INFO
 {
	int ledno;
	char opt_type;
	int duration;
	int enable;
	time_t s_time,r_time;
 }GPIOINFO;

 int OpenGpioBoard(IN char *path);
 int GpioOpt(IN int opt);
 int Gpio_Sub(const char *buf);
 int LedFlashing(void);
 int CloseGpioBoard(void);
 int GetIostate(void);

 int red_on(void);
 int red_off(void);
 int green_on(void);
 int green_off(void);
 int lcd_on(void);     
 int lcd_off(void);     
 int relay_on(void);   
 int relay_off(void);   
 int video_on(void);
 int video_off(void);

/*
root@A20:/weds# cat arm/lib/gpio.sh 
mkdir /dev/gpio

#绿灯0  /dev/gpio/out0
#pin0
ln -s /sys/class/gpio_sw/PI7/data /dev/gpio/out0
#红灯1 /dev/gpio/out1
#pin1
ln -s /sys/class/gpio_sw/PI6/data /dev/gpio/out1
#按钮输入1
#pin3 /dev/gpio/in0
ln -s /sys/class/gpio_sw/PB13/data /dev/gpio/in0

#索引  gpio名字  解释
#1      PI7     绿灯
#2      PI6     红灯
#3      PB6     继电器1
#4      PB13    按钮输入1
#5      PB7     门磁检测1
#6      PH7     背光
#7      PB8     usb电源控制
#8      PH12    usb摄像头
#9      PH9     4g_pow
#10     PH11    4g_on_of
#11     PB3     侧面usb电源控制
#12     PB2     下方usb电源控制
#13     PH6     sd卡电源控制
#14     PH8     指纹是否按下
#15     PI14    指纹灯
#19     PH5     usbwifi电源
*/

#endif 
