
源代码结构：
/freakz :用于实现zigbee协议。包括：驱动、MAC层、NWK层
	/mac	
	/mwk
	/apl
	
/kernel :用于实现FreeRTOS操作系统：内核、CPU、编译器、平台移植
	/os
	/portable

/common :主要用于管理：驱动程序、文件系统、操作FLASH、命令行
	/driver
	/filesystem
	/flash
	/full
	/minimal
/docs  :用于存放分析、设计、说明文档





-c --model-large --stack-auto --int-long-reent --float-reent -DSDCC_CYGNAL   --nooverlay --no-peep --debug --use-stdout -V -I"c:\SiLabs\MCU_2\Inc" -I"E:\WORKSPACE\Zh_S_Zh\Common\include" -I"E:\WORKSPACE\Zh_S_Zh\kernel\include" -I"C:\SiLabs\MCU_2\Inc"




--model-large --stack-auto --int-long-reent --float-reent -DSDCC_CYGNAL --xram-loc 512 --xram-size 0x1000 --code-loc 0x0 --code-size 0xfbff --no-peep --nooverlay --debug --use-stdout -V --out-fmt-ihx












