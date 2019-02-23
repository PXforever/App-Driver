#ifndef _ENGINEER_DEBUGS_H
#define _ENGINEER_DEBUGS_H

#include <linux/debugfs.h>
#include <linux/slab.h>

#define COMMAND_RUN			1
#define COMMAND_PARAMETER	2
#define COMMAND_SWITCH		3
#define COMMAND_TRANSFORM	4
#define COMMAND_INFO		5

#define EN_DEBUG_OPEN		1
#define EN_DEBUG_CLOSE		2
#define EN_DEBUG_SET		3
#define EN_DEBUG_RUN		4
#define EN_DEBUG_MAX_IOCTL		EN_DEBUG_RUN

#define EN_DE_READ_BUF_SIZE		200
#define EN_DE_WRITE_BUF_SIZE	200

#define EN_DEBUG_SW	1
#if EN_DEBUG_SW
#define engineer_debug_print( fmt, arg...)	printk( fmt, ##arg)
#else
#define engineer_debug_print( fmt, arg...)	
#endif

extern struct dentry *engineer_debug_root;

struct debug_option{
	char *laber;
	char *property;
	union{
		void *parame_addr;
		void (*func_addr)(void);
		void (*switchs_addr)(bool);
	}addr;	
};

//typedef struct debug_list debug_list;

typedef struct debug_list{
	struct debug_option debug_option;
	struct debug_list *next;
}debug_list;

//debug的相关信息
struct engineer_debug_info{
	char *device_names;
	struct debug_list *run_list;
	struct debug_list *parame_list;
	struct debug_list *switch_list;
	struct debug_list *transform_list;
	struct debug_list *info_list;
	short run_num;
	short parame_num;
	short switchs_num;
	short transform_num;
	short info_num;
};

extern debug_list* engineer_debugs_find_option(const debug_list *head,short num);
extern int engineer_debugs_check_property(char *property);
extern char *engineer_debugs_get_item_info(char *device_name, struct engineer_debug_info *engineer_debug_info);
extern int engineer_debugs_get_status(const debug_list *head, short num);

extern int engineer_debugs_init(char *device_name, const struct file_operations *ops);

#define __engineer_debugs_init(name) engineer_debugs_init( #name, &engineer_debugs_ops_##name)


#define ENGINEER_DEBUG_INIT(device_name, list)	                                                                                                \
struct engineer_debug_info debug_info_##device_name = {																							\
	.run_list = NULL,																															\
	.parame_list = NULL,																														\
	.switch_list = NULL,																														\
	.transform_list = NULL,																														\
	.info_list = NULL,																														\
	.run_num = 0,																																\
	.parame_num = 0,																															\
	.switchs_num = 0,																															\
	.transform_num = 0,																															\
	.info_num = 0,																																\
};																																				\
static int engineer_debugs_open_##device_name(struct inode *inode, struct file *file)	                                                        \
{	                                                                                                                                            \
	short i = 0;                                                                                                                                \
	short run_end = 0;                                                                                                                          \
	short parame_end = 0;                                                                                                                       \
	short switch_end = 0;                                                                                                                       \
	short transform_end = 0;                                                                                                                    \
	short info_end = 0;                                                                                                                         \
	short list_num = 0;                                                                                           								\
	engineer_debug_print("[px_test]enter %s\n",__func__);																										\
	list_num = sizeof(list)/sizeof(debug_list);                                                                                           		\
	engineer_debug_print("[px_test]have %d list\n", list_num);																								\
	if(!list_num)	return 3;                                                                                                                  	\
	if((debug_info_##device_name.run_num+ debug_info_##device_name.parame_num+ debug_info_##device_name.switchs_num+ debug_info_##device_name.transform_num+ debug_info_##device_name.info_num) != 0)	return 0;\
	engineer_debug_print("[px_test]enter while\n");																											\
	debug_info_##device_name.device_names = #device_name ;																						\
	while(i < list_num)                                                                                                                         \
	{                                                                                                                                           \
		engineer_debug_print("[px_test]list[%d].debug_option.property\n", i);																					\
		switch(engineer_debugs_check_property(list[i].debug_option.property))                                                                  \
		{                                                                                                                                       \
			case COMMAND_RUN:if(debug_info_##device_name.run_list == NULL)																		\
							{debug_info_##device_name.run_list = &list[i]; run_end = i; debug_info_##device_name.run_num++; engineer_debug_print("[px_test]find run head\n");} \
						else                                                                                                                    \
						{                                                                                                                       \
							list[run_end].next = &list[i];                                                                                      \
							run_end = i;                                                                                                        \
							list[i].next = NULL;                                                                                                \
							debug_info_##device_name.run_num++;                                                                                  \
						}                                                                                                                       \
				break;                                                                                                                          \
			case COMMAND_PARAMETER:if(debug_info_##device_name.parame_list == NULL)																\
							{debug_info_##device_name.parame_list = &list[i]; debug_info_##device_name.parame_num++; parame_end = i;engineer_debug_print("[px_test]find parame head\n");} \
						else                                                                                                                    \
						{                                                                                                                       \
							list[parame_end].next = &list[i];                                                                                   \
							parame_end = i;                                                                                                     \
							list[i].next = NULL;                                                                                                \
							debug_info_##device_name.parame_num++;                                                                              \
						}                                                                                                                       \
				break;                                                                                                                          \
			case COMMAND_SWITCH:if(debug_info_##device_name.switch_list == NULL)																\
							{debug_info_##device_name.switch_list = &list[i]; debug_info_##device_name.switchs_num++; switch_end = i;engineer_debug_print("[px_test]find switchs head\n");} \
						else                                                                                                                    \
						{                                                                                                                       \
							list[switch_end].next = &list[i];                                                                                   \
							switch_end = i;                                                                                                     \
							list[i].next = NULL;                                                                                                \
							debug_info_##device_name.switchs_num++;                                                                        		\
						}                                                                                                                       \
				break;                                                                                                                          \
			case COMMAND_TRANSFORM:if(debug_info_##device_name.transform_list == NULL)															\
							{debug_info_##device_name.transform_list = &list[i]; debug_info_##device_name.transform_num++; transform_end = i; engineer_debug_print("[px_test]find transform head\n");} \
						else                                                                                                                    \
						{                                                                                                                       \
							list[transform_end].next = &list[i];                                                                                \
							transform_end = i;                                                                                                  \
							list[i].next = NULL;                                                                                                \
							debug_info_##device_name.transform_num++;                                                                           \
						}                                                                                                                       \
				break;                                                                                                                          \
			case COMMAND_INFO:	if(debug_info_##device_name.info_list == NULL)																	\
							{debug_info_##device_name.info_list = &list[i]; debug_info_##device_name.info_num++; info_end = i; engineer_debug_print("[px_test]find transform info\n");}   \
						else                                                                                                                    \
						{                                                                                                                       \
							list[run_end].next = &list[i];                                                                                      \
							info_end = i;                                                                                                       \
							list[i].next = NULL;                                                                                                \
							debug_info_##device_name.info_num++;                                                                               	\
						}                                                                                                                       \
				break;	                                                                                                                        \
			default:	return 2;                                                                                                              	\
		}                                                                                                                                       \
		i++;																																	\
	}                                                                                                                                           \
	engineer_debug_print("[px_test]while end\n");																												\
	return 0;																																	\
}                                                                                                                                               \
static ssize_t engineer_debugs_write_##device_name(struct file *file,	                                                                        \
			 const char __user *ubuf, size_t count, loff_t *ppos)		                                                                        \
{																		                                                                        \
	return 0;																	                                                                \
}																		                                                                        \
static ssize_t engineer_debugs_read_##device_name(struct file *file,	                                                                        \
				      char __user *ubuf, size_t count, loff_t *ppos)	                                                                        \
{																		                                                                        \
	char *buffer;																																\
	int ret = -1;																																\
	engineer_debug_print("[px_test]enter %s,run:%d,parame:%d,switchs:%d,transform:%d,info:%d\n",__func__, debug_info_##device_name.run_num, debug_info_##device_name.parame_num, debug_info_##device_name.switchs_num, debug_info_##device_name.transform_num, debug_info_##device_name.info_num);\
	engineer_debug_print("read %s \n", ubuf);																													\
	if(!strcmp( "option info", ubuf))																											\
	{																																			\
		engineer_debug_print("you want read option info\n");																									\
		buffer = engineer_debugs_get_item_info(#device_name, &debug_info_##device_name);														\
		engineer_debug_print("[px_test]buffer[%u]:%s", (unsigned int)strlen(buffer), buffer);																	\
		ret = copy_to_user(ubuf,buffer,EN_DE_READ_BUF_SIZE);                                                                                    \
		if(ret < 0) return -1;																													\
		kfree(buffer);																															\
	}																																			\
	else																																		\
	{																																			\
		switch(ubuf[0])																															\
		{																																		\
			case 1:return engineer_debugs_get_status(debug_info_##device_name.run_list,ubuf[2]);break;											\
			case 2:return engineer_debugs_get_status(debug_info_##device_name.parame_list,ubuf[2]);break;										\
			case 3:return engineer_debugs_get_status(debug_info_##device_name.switch_list,ubuf[2]);break;										\
			case 4:return engineer_debugs_get_status(debug_info_##device_name.transform_list,ubuf[2]);break;									\
			case 5:return engineer_debugs_get_status(debug_info_##device_name.info_list,ubuf[2]);break;											\
			default:return -1;																													\
		}																																		\
	}																																			\
	engineer_debug_print("[px_test]end read !");																												\
	return EN_DE_READ_BUF_SIZE;																		                                            \
}																		                                                                        \
static long engineer_debugs_ioctl_##device_name(struct file* file, unsigned int cmd, unsigned long arg)		                                  	\
{	                                                                                                                                            \
	int debug_cmd  =  cmd >> 24;																												\
	int debug_laber = cmd - (debug_cmd << 24);																									\
	struct debug_option *io_debug_option;																										\
	engineer_debug_print("[px_test]enter %s,cmd[%d]:%d,laber:%u\n", __func__, cmd, debug_cmd, debug_laber);													\
	if(debug_cmd > EN_DEBUG_MAX_IOCTL) return -1;																								\
	if(debug_cmd == EN_DEBUG_OPEN)																												\
	{																																			\
		if(debug_laber <= debug_info_##device_name.switchs_num)																					\
		{																																		\
			io_debug_option = &(engineer_debugs_find_option(debug_info_##device_name.switch_list, debug_laber)->debug_option) ;					\
			engineer_debug_print("[px_test]OPEN:%c\n",io_debug_option->property[2]);															\
			engineer_debug_print("[px_test]laber:%s\n",io_debug_option->laber);																	\
			if(io_debug_option->property[2] == 'f')																								\
				(*io_debug_option).addr.switchs_addr(1);																						\
			else if(io_debug_option->property[2] == 'p')																						\
				*(int *)io_debug_option->addr.parame_addr = true;	    																		\
			else	;																															\
		}																																		\
	}																																			\
	if(debug_cmd == EN_DEBUG_CLOSE)																												\
	{																																			\
			io_debug_option = &(engineer_debugs_find_option(debug_info_##device_name.switch_list, debug_laber)->debug_option) ;					\
			engineer_debug_print("[px_test]CLOSE:%c\n",io_debug_option->property[2]);																			\
			if(io_debug_option->property[2] == 'f')																								\
				(*io_debug_option).addr.switchs_addr(0);																						\
			else if(io_debug_option->property[2] == 'p')																						\
				*(int *)io_debug_option->addr.parame_addr = false;	    																		\
			else	;																															\
	}																																			\
	if(debug_cmd == EN_DEBUG_SET)																												\
	{																																			\
		if(debug_laber <= debug_info_##device_name.parame_num)																					\
		{																																		\
			io_debug_option = &(engineer_debugs_find_option(debug_info_##device_name.parame_list, debug_laber)->debug_option) ;					\
			*(int *)io_debug_option->addr.parame_addr = arg;																					\
		}																																		\
	}																																			\
	if(debug_cmd == EN_DEBUG_RUN)																												\
	{																																			\
		engineer_debug_print("[px_test] run list 1 func\n");																									\
		if(debug_laber <= debug_info_##device_name.run_num)																						\
		{																																		\
			io_debug_option = &(engineer_debugs_find_option(debug_info_##device_name.run_list, debug_laber)->debug_option) ;					\
			(*io_debug_option).addr.func_addr();																								\
		}																																		\
	}																																			\
	return 0;																																	\
}																		                                                                        \
static const struct file_operations engineer_debugs_ops_##device_name = {	                                                                    \
	.open = engineer_debugs_open_##device_name,                                                                                                	\
	.write = engineer_debugs_write_##device_name,						                                                                        \
	.read = engineer_debugs_read_##device_name,						                                                                        	\
	.unlocked_ioctl = engineer_debugs_ioctl_##device_name				                                                                        \
};



#endif
