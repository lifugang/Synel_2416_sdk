/*
 * version.h
 *
 *  Created on: 2013-8-27
 *      Author: aduo
 */

#ifndef VERSION_H_
#define VERSION_H_

//#define _x86
//#define _2410
#define _2416
//#define sdk_version "sdk6.1.2"
//#define sdk_version "sdk6.1.3"		// 20180607 lfg 增加新的接口

//#define sdk_version "sdk6.1.4"		// 20190215 lfg 增加新的接口 天线开启或关闭

//#define sdk_version "sdk6.1.5"		// 20190327 A20 gpio 操作

//#define sdk_version "sdk6.1.6"		// 20190618 type B type

//#define sdk_version "sdk6.1.7"		// 20190618 spi通信数据增加校验，修正天线开关操作后，读取应答的超时时间

#define sdk_version "sdk6.1.8"		// 20200611 修正ICLASS（0x9D）, 数据解析bug，导致与之前的版本不一致

#endif /* VERSION_H_ */
