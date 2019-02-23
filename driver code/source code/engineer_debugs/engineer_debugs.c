#include <linux/engineer_debugs.h>

//获取链表中的某个调试项的节点，num表示第几个
debug_list* engineer_debugs_find_option(const debug_list *head,short num)
{
	debug_list *aim = head;
	//为0或小于0处理----
	num--;																	//列表从1开始，最小为1
	printk("[px_test]enter %s\n",__func__);
	while(num--)
	{
		aim = aim->next;
	}
	return aim;
}

int engineer_debugs_check_property(char *property)
{
	switch(property[0])
	{
		case 'R':
			return COMMAND_RUN;	 
			break;
		case 'P':
			return COMMAND_PARAMETER;	 
			break;
		case 'S':
			return COMMAND_SWITCH;	 
			break;
		case 'T':
			return COMMAND_TRANSFORM;	 
			break;
		case 'I':
			return COMMAND_INFO;	 
			break;
		default :
			return -1;
	}		
}

int engineer_debugs_init(char *device_name, const struct file_operations *ops)
{
	//添加到所有链表上，总体管理
	debugfs_create_file(device_name,S_IFREG | S_IRWXU |S_IRWXG|S_IRWXO, engineer_debug_root!=NULL?engineer_debug_root:NULL, (void *)0,ops);
	return 0;
}

char *engineer_debugs_get_item_info(char *device_name, struct engineer_debug_info *engineer_debug_info)
{
	char *buffer;	
	debug_list *local_run_list = engineer_debug_info->run_list;                                                                                                   
	debug_list *local_parame_list = engineer_debug_info->parame_list;                                                                                             
	debug_list *local_switch_list = engineer_debug_info->switch_list;                                                                                             
	debug_list *local_transform_list = engineer_debug_info->transform_list;                                                                                       
	debug_list *local_info_list = engineer_debug_info->info_list;                                                                                                 
	short local_run_num = engineer_debug_info->run_num;                                                                                                           
	short local_parame_num = engineer_debug_info->parame_num;                                                                                                     
	short local_switchs_num = engineer_debug_info->switchs_num;                                                                                                   
	short local_transform_num = engineer_debug_info->transform_num;                                                                                               
	short local_info_num = engineer_debug_info->info_num;   
	printk("get %s item information\n", device_name);
	buffer = kmalloc(EN_DE_READ_BUF_SIZE,GFP_KERNEL); 
	if(buffer == NULL)                                                                                                                       
	{                                                                                                                                        
		printk("[px_test]buffer alloc failed...");                                                                                             
		return "error";                                                                                                                              
	}                                                                                                                                           
	memset(buffer,'\0',EN_DE_READ_BUF_SIZE); 
	sprintf(buffer,"R=%d,P=%d,S=%d,T=%d,I=%d\n",local_run_num, local_parame_num, local_switchs_num, local_transform_num, local_info_num);  
	sprintf(buffer+strlen(buffer),"R<");
	while(local_run_num--)                                                                                                           
	{   
		if(local_run_list == NULL) return "error";
		sprintf(buffer+strlen(buffer),"%s:%s|", local_run_list->debug_option.laber, local_run_list->debug_option.property);             
		printk("[px_test]R sprintf success\n");                                                                                          
		if(local_run_list->next != NULL)                                                                                               
		{                                                                                                                              
			local_run_list = local_run_list->next;                                                                                       
		}                                                                                                                              
		printk("[px_test]next node:%s\n",local_run_list->debug_option.laber);                                                           
	}
	sprintf(buffer+strlen(buffer),"\nP<");
	while(local_parame_num--)                                                                                                           
	{    
		if(local_parame_list == NULL) return "error";
		sprintf(buffer+strlen(buffer),"%s:%s|", local_parame_list->debug_option.laber, local_parame_list->debug_option.property);             
		printk("[px_test]P sprintf success\n");                                                                                          
		if(local_parame_list->next != NULL)                                                                                               
		{                                                                                                                              
			local_parame_list = local_parame_list->next;                                                                                       
		}                                                                                                                              
		printk("[px_test]next node:%s\n",local_parame_list->debug_option.laber);                                                           
	}
	sprintf(buffer+strlen(buffer),"\nS<");
	while(local_switchs_num--)                                                                                                           
	{          
		if(local_switch_list == NULL) return "error";
		sprintf(buffer+strlen(buffer),"%s:%s|", local_switch_list->debug_option.laber, local_switch_list->debug_option.property);             
		printk("[px_test]S sprintf success\n");                                                                                          
		if(local_switch_list->next != NULL)                                                                                               
		{                                                                                                                              
			local_switch_list = local_switch_list->next;                                                                                       
		}                                                                                                                              
		printk("[px_test]next node:%s\n",local_switch_list->debug_option.laber);                                                           
	}
	sprintf(buffer+strlen(buffer),"\nT<");
	while(local_transform_num--)                                                                                                           
	{      
		if(local_transform_list == NULL) return "error";
		sprintf(buffer+strlen(buffer),"%s:%s|", local_transform_list->debug_option.laber, local_transform_list->debug_option.property);             
		printk("[px_test]T sprintf success\n");                                                                                          
		if(local_transform_list->next != NULL)                                                                                               
		{                                                                                                                              
			local_transform_list = local_transform_list->next;                                                                                       
		}                                                                                                                              
		printk("[px_test]next node:%s\n",local_transform_list->debug_option.laber);                                                           
	}
	sprintf(buffer+strlen(buffer),"\nI<");
	while(local_info_num--)                                                                                                           
	{     
		if(local_info_list == NULL) return "error";
		sprintf(buffer+strlen(buffer),"%s:%s|", local_info_list->debug_option.laber, local_info_list->debug_option.property);             
		printk("[px_test]I sprintf success\n");                                                                                          
		if(local_info_list->next != NULL)                                                                                               
		{                                                                                                                              
			local_info_list = local_info_list->next;                                                                                       
		}                                                                                                                              
		printk("[px_test]next node:%s\n",local_info_list->debug_option.laber);                                                           
	}
	//kfree(buffer);	
	return buffer;
}

int engineer_debugs_get_status(const debug_list *head, short num)
{
	debug_list *aim = engineer_debugs_find_option(head, num);
	int (*switchs_addr)(bool);
	printk("enter %s, %d\n",__func__,num);
	if(head == NULL) return -1;
	else if(aim->debug_option.property[0] == 'R')
	{
		//return times;
		printk("get switch status\n");
		return 1;
	}
	else if(aim->debug_option.property[0] == 'P')
	{
		printk("get parame status\n");
		return *(int *)aim->debug_option.addr.parame_addr;
	}
	else if(aim->debug_option.property[0] == 'S')
	{
		printk("get switch status\n");
		if(aim->debug_option.property[2] == 'p')
			return *(int *)aim->debug_option.addr.parame_addr;
		else
		{
			switchs_addr = (int *)(*aim).debug_option.addr.switchs_addr;
			return switchs_addr(3);
		}
	}
	else if(aim->debug_option.property[0] == 'T')
	{
		printk("get transform status\n");
		return 1;
	}
	else if(aim->debug_option.property[0] == 'I')
	{
		printk("get info status\n");
		return 1;
	}
	else	
		return -1;
}

struct dentry *engineer_debug_root;

static int __init engineer_debugs_local_init(void)
{
	engineer_debug_root = debugfs_create_dir("engineer_debugs", NULL);
	if(engineer_debug_root == NULL)
		printk("create engineer_debug_root dir failed...\n");
}
subsys_initcall(engineer_debugs_local_init);




