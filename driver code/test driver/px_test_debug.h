/* drivers/input/touchscreen/mediatek/gt9xx_mtk/include/tpd_gt9xx_common.h
 *
 * Copyright  (C)  2010 - 2016 Goodix., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Version: V2.6.0.3
 */
 
#ifndef PX_TEST_DEBUG_H__
#define PX_TEST_DEBUG_H__

#include <linux/hrtimer.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <cust_eint.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>
#include <linux/dma-mapping.h>

#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>

#define BUSNUM 2
#define PX_DEVICE_NAME "px_test_debug"
#define PX_ADDR 0x22
		
/*print define pengxuan add here */
#define DEBUGLEVEL 1		//调试打印等级，默认为2，1-err（错误），2-log（关键信息），3-debug（调试）。
#define px_test_print(n,fmt,arg...) do{\
	if(n){\
			printk("[Exc80X "fmt,##arg);\
		}\
	else\
			;\
	}while(0)

#define px_test_print_err(fmt,arg...) 	px_test_print(DEBUGLEVEL>=1?1:0,"ERROR:]"fmt,##arg)
#define px_test_print_log(fmt,arg...) 	px_test_print(DEBUGLEVEL>=2?1:0,"LOG:]"fmt,##arg)
#define px_test_print_debug(fmt,arg...)  px_test_print(DEBUGLEVEL>=3?1:0,"DEBUG:]"fmt,##arg)


#endif 
