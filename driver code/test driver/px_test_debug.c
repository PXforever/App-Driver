/* drivers/input/touchscreen/mediatek/px_test_mtk/px_test_driver.c
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


#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/proc_fs.h>	/*proc*/
#include <linux/atomic.h>
#include <linux/delay.h>

//file test
#include <asm/io.h>

#include "px_test_debug.h"
#include "mt_boot_common.h"

#define PX_CMD(num) _IO('E', num)

#define PX_READ_BUF				PX_CMD(2)
#define PX_SEND_COMMAND			PX_CMD(3)
#define PX_SET_SET_Y_OFFSET		PX_CMD(4)
#define PX_SET_SET_X_ORGIN		PX_CMD(5)
#define PX_SET_SET_Y_ORGIN		PX_CMD(6)
#define PX_SET_SET_K_X			PX_CMD(7)
#define PX_SET_SET_K_Y			PX_CMD(8)
#define PX_SET_SET_LOAD_CONFIG 	PX_CMD(9)

#include <linux/engineer_debugs.h>

#ifdef CONFIG_PX_SUPPORT_I2C_DMA
static u8 *gpDMABuf_va;
static dma_addr_t gpDMABuf_pa;
#endif

static int px_test_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int px_test_i2c_remove(struct i2c_client *client);
static int px_test_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);


static struct i2c_client *i2c_client_point;
static const struct i2c_device_id px_test_i2c_id[] = {{PX_DEVICE_NAME, 0}, {} };

#ifdef PX_USER_DT
static const struct of_device_id px_test_dt_match[] = { {.compatible = ""}, {},};
MODULE_DEVICE_TABLE(of, px_test_dt_match);
#else 
#pragma message("i2c info no use DTS")
static const struct i2c_board_info __initdata px_test_i2c_info={ I2C_BOARD_INFO(PX_DEVICE_NAME, (PX_ADDR>>1))};
#endif


static struct i2c_driver px_test_i2c_driver = {
	.probe = px_test_i2c_probe,
	.remove = px_test_i2c_remove,
	.detect = px_test_i2c_detect,
	.id_table = px_test_i2c_id,
	.driver = {
		.name = PX_DEVICE_NAME,
		.owner	= THIS_MODULE,
#ifdef PX_USER_DT
		.of_match_table = of_match_ptr(px_test_dt_match),
#endif
		}

};

static int px_test_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
		strcpy(info->type, "mtk-px_test");
		return 0;
}


#ifdef CONFIG_PX_SUPPORT_I2C_DMA
//#pragma message("use i2c DMA write")
static int i2c_write_bytes(struct i2c_client *client,u8 *txbuf, int len)
{
	int ret = -1;
	memcpy(gpDMABuf_va,txbuf,len);
	client->addr	 &= I2C_MASK_FLAG;
	client->ext_flag |= I2C_DMA_FLAG;
	client->ext_flag |= I2C_ENEXT_FLAG;
	client->timing  = 350;
	
	ret = i2c_master_send(client, (unsigned char *)(uintptr_t)gpDMABuf_pa, len);
	if(ret < 0)
		px_test_print_err("%s:read reg 0x%x, err %d\n",__func__, txbuf[0], ret);
	return ret;	
}
#else 
static int i2c_write_bytes(struct i2c_client *client,u8 *txbuf, int len)
{
	int ret = -1;

	struct i2c_msg msgs[] = {
		{.addr = client->addr, .flags = 0, .len = len, .buf = txbuf},
		//{.addr = client->addr, .flags = 1, .len = sizeof(buf[1]), .buf = &buf[1]}
	};
	//px_test_print_debug("enter %s --\n",__func__);
	
	ret = i2c_transfer(client->adapter,msgs,ARRAY_SIZE(msgs));  
	if(ret < 0)
		px_test_print_err("%s:read reg 0x%x, err %d\n",__func__, txbuf[0], ret);
	return ret;	
}
#endif

#ifdef CONFIG_PX_SUPPORT_I2C_DMA
//#pragma message("use i2c DMA read")
static int i2c_read_bytes(struct i2c_client *client, u8 *rxbuf, int len)
{
	int ret = -1;
	
	//client->addr	 &= I2C_MASK_FLAG;
	client->ext_flag |= I2C_DMA_FLAG;
	client->ext_flag |= I2C_ENEXT_FLAG;
	client->timing  = 350;
	
	ret = i2c_master_recv(client, (unsigned char *)(uintptr_t)gpDMABuf_pa, len);
	if(gpDMABuf_va == NULL)
	{	
		px_test_print_err("read err,count is :%d\n",ret);
		return ret;
	}
	memcpy(rxbuf,gpDMABuf_va,len);
	if(ret < 0)
		px_test_print_err("%s:read reg 0x%x, err %d\n",__func__, rxbuf[0], ret);
	return ret;	
}
#else 
static int i2c_read_bytes(struct i2c_client *client, u8 *rxbuf, int len)
{
	int ret = -1;
	u8 buf[len+1] ;
	//px_test_print_debug("enter %s\n", __func__);
	struct i2c_msg msgs[] = {
		{.addr = client->addr, .flags = 0, .len = sizeof(buf[0]), .buf = &buf[0]},
		{.addr = client->addr, .flags = 1, .len = len*sizeof(buf[1]), .buf = &buf[1]}
	};
	//buf[0] = addr;
	//px_test_print_debug("enter %s --\n",__func__);
	
	ret = i2c_transfer(client->adapter,msgs,ARRAY_SIZE(msgs)); 
	*rxbuf = buf[1];
	if(ret < 0)
		px_test_print_err("%s:read reg 0x%x, err %d\n",__func__, buf[0], ret);
	return ret;
}
#endif


static s32 px_test_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	px_test_print_log("enter %s\n",__func__);
	
	i2c_client_point = client;
#ifdef CONFIG_PX_SUPPORT_I2C_DMA
	client->addr &=  I2C_MASK_FLAG;	/* | I2C_DMA_FLAG; */
 	client->ext_flag |= I2C_DMA_FLAG;
	
	gpDMABuf_va = (char *)dma_alloc_coherent(&client->dev, 100, (dma_addr_t *)&gpDMABuf_pa, GFP_KERNEL);
	if (gpDMABuf_va == NULL) 
	{
		px_test_print_err("%s : failed to allocate dma gpDMABuf_va...\n", __func__);
	}		
#endif
	
	px_test_print_debug("%s, success run Done\n", __func__);
	return 0;
}


static int px_test_i2c_remove(struct i2c_client *client)
{

	return 0;
}


int a = 0;
char b = 0;
short c = 0;
bool my_swtich1 = true;
int my_func(void)
{
	printk("[px_test]run %s\n",__func__);
	return 0;
}
int print_parame(void)
{
	printk("[px_test]run %s\n",__func__);
	printk("p1:%d,p2:%d\n", a, b);
	
	return 0;
}

int my_switch2(bool status)
{
	if(status == 1)
	{
		printk("[px_test]open\n");
		return 1;
	}
	else if(status == 0)
	{
		printk("[px_test]close\n");
		return 0;
	}
	else
	{
		printk("[px_test]get status\n");
		return status;
	}
}

debug_list my_debug_list[] = {
	{{"parame1","P(i)",&a},NULL},
	{{"parame2","P(c)",&b},NULL},
	{{"func1","R(i)",&my_func},NULL},
	{{"print_parame","R(v)",&print_parame},NULL},
	{{"sw1","S(p)",&my_swtich1},NULL},
	{{"sw2","S(f)",&my_switch2},NULL},
	{{"parame3","P(c)",&b},NULL},
	{{"parame4","P(c)",&b},NULL},
	{{"parame5","P(c)",&b},NULL},
};
ENGINEER_DEBUG_INIT(px_test,my_debug_list)

static int __init px_test_driver_init(void)
{
	int ret = -1;
#ifndef PX_USER_DT
	//struct i2c_adapter *i2c_adap;    //分配一个适配器的指针
#endif
	px_test_print_debug("px_test series touch panel driver init\n");
	//px_test_get_dts_info();
#ifndef PX_USER_DT
#pragma message("no user DTS")
	if(i2c_register_board_info(BUSNUM, &px_test_i2c_info, 1)){
		px_test_print_err("%s failed to i2c_register_board_info \n",__func__);
        return -ENODEV;
    }
	px_test_print_debug("Have register px_test\n");
/* 如果adapt提前初始化了执行此注释
	i2c_adap = i2c_get_adapter(BUSNUM);
	px_test_print_debug("get a i2c adapter\n");
	if(i2c_adap == NULL)
		px_test_print_debug("i2c_adap is NULL\n");
	i2c_client_point = i2c_new_device(i2c_adap,&px_test_i2c_info);
	px_test_print_debug("add px_test i2c device success!!! \n");
*/
#endif	
	if (i2c_add_driver(&px_test_i2c_driver) != 0) {
		px_test_print_err("unable to add i2c driver.");
		return -1;
	}
	
	engineer_debugs_init("px_test", &engineer_debugs_ops_px_test);
	return 0;
}

/* should never be called */
static void __exit px_test_driver_exit(void)
{
	px_test_print_debug("GT9 series touch panel driver exit");
}

module_init(px_test_driver_init);
module_exit(px_test_driver_exit);
MODULE_LICENSE(PX_TEST v2);
MODULE_DESCRIPTION("px_test Touch Panel Driver");
