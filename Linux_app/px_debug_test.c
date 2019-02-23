#include <linux/input.h>
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <linux/ioctl.h>
#include <unistd.h>
#include <dirent.h>
//#include <linux/engineer_debugs.h>

//该文件在没有创建目标文件时会自动创建文件
//在没有参数时默认写入数据到驱动
//在有数据是写入数据至文件

struct debug_option{
	char laber[30];
	char *property;
	void *data;
};
typedef struct debug_list{
	struct debug_option debug_option;
	struct debug_list *next;
}debug_list;
//debug的相关信息
typedef struct engineer_debug_info{
	char *device_names;
	debug_list *run_list;
	debug_list *parame_list;
	debug_list *switch_list;
	debug_list *transform_list;
	debug_list *info_list;
	short run_num;
	short parame_num;
	short switchs_num;
	short transform_num;
	short info_num;
}engineer_debug_info;


#define DEVICEPATH "/sys/kernel/debug/engineer_debugs/\0"

#define GET_INFO "option info\0"

#define EN_DEBUG_OPEN		1
#define EN_DEBUG_CLOSE		2
#define EN_DEBUG_SET		3
#define EN_DEBUG_RUN		4
#define EN_DEBUG_MAX_IOCTL		EN_DEBUG_RUN

//#define PRINT_LIST

engineer_debug_info *my_engineer_debug_info;
int device_fd = -1; int config_fd = -1;

int engineer_debug_init(char *device_name);
unsigned int get_cmd_form_property_laber(char property, char laber, unsigned int cmd);
engineer_debug_info* get_option_info(const char *raw_data);
void printf_debug_list(const debug_list *debug_list_head);
int get_laber_list_num(char *laber, const debug_list *debug_list_head);
int get_status( char property, char *laber);
int readFileList(char *basePath);
int recognize_command_and_run(char *cmd);
int get_status_from_num( char property, int num);

int main(int argc, char * const argv[])
{

    int ret = 0;
	char device_name[25];
	char cmd_line[30] = "\0";
	printf("welcome to engineer debug driver!\n");
CHOSE_DEVICE:
	readFileList(DEVICEPATH);
	printf("which device do you want chose? q exit\n>");
	scanf( "%[^\n]", device_name);
	scanf("%*c"); 
	if(!strcmp( cmd_line, "q")) exit(1);
	if(engineer_debug_init( device_name) < 0)
		exit(1);
	
	while(1)
	{
		printf("please input command,quit is exit program,back is cback to chose device\n>");
		memset( cmd_line, '\0', 30);
		scanf( "%[^\n]", cmd_line);
		scanf("%*c"); 
		if(!strcmp( cmd_line, "quit")) break;
		if(!strcmp( cmd_line, "back")) { close(device_fd); goto CHOSE_DEVICE;}
		//if(!strcmp( cmd_line, "exit")) { close(device_fd); goto CLOSE_PROGRAM;}
		ret = recognize_command_and_run(cmd_line);
		if(ret < 0)
		{
			printf("cmd failed:%d\n", ret); 
		}
	}
	printf("ioctl successful:%d\n", ret); 	
CLOSE_PROGRAM:
	close(device_fd);
	return 0;

}


int engineer_debug_init(char *device_name)
{
	char get_parmer[200];
	char device_path[55] = DEVICEPATH;
	stpcpy( get_parmer, GET_INFO);
	strcat( device_path, device_name);
	device_fd = open( device_path, O_RDWR);
	//printf("device_fd:%d\n", device_fd);
	if(device_fd < 0)
	{
		printf("device open failed...\n");
		return -1;
	}
	printf("open %s successful!\n", device_name);
	if(read(device_fd, get_parmer, 200 ) < 0)
	{
		printf("read failed...\n"); 
		return -1;
	}
	printf("----------------------command list----------------------\n");
	printf("%s\n", get_parmer);
	printf("--------------------------------------------------------\n");
	my_engineer_debug_info = get_option_info( get_parmer);
	if(my_engineer_debug_info == NULL)
		return -1;
	return 0;
}

//获取ioctl命令通过property和laber
unsigned int get_cmd_form_property_laber(char property, char laber, unsigned int cmd)
{
	//laber与列表对比
	switch(property)
	{
		case 'R':	return((EN_DEBUG_RUN << 24) + laber);break;
		case 'P':	return((EN_DEBUG_SET << 24) + laber);break;
		case 'S':	return(((cmd?EN_DEBUG_OPEN:EN_DEBUG_CLOSE) << 24) + laber);break;
		case 'T':	return((cmd << 24) + laber);break;
		case 'I':	return((cmd << 24) + laber);break;
		default:	;break;
	}
	return 0;
}

//读取状态,通过属性和标签
//返回值:一个数值
//参数:property-类型，laber-标签名。这里应该使用重载，使得返回值可以是任何值，比如int char string
int get_status( char property, char *laber)
{
	char local_parmer[4] = "000"; //local_parmer[0]-属性号, local_parmer[1]-保留, local_parmer[2]-表中的序号
	int ret = -1;
	//stpcpy( local_parmer, "312");
	switch(property)
	{
		case 'R':	local_parmer[0] = 1;local_parmer[2] = get_laber_list_num( laber, my_engineer_debug_info->run_list);break;
		case 'P':	local_parmer[0] = 2;local_parmer[2] = get_laber_list_num( laber, my_engineer_debug_info->parame_list);break;
		case 'S':	local_parmer[0] = 3;local_parmer[2] = get_laber_list_num( laber, my_engineer_debug_info->switch_list);break;
		case 'T':	local_parmer[0] = 4;local_parmer[2] = get_laber_list_num( laber, my_engineer_debug_info->transform_list);break;
		case 'I':	local_parmer[0] = 5;local_parmer[2] = get_laber_list_num( laber, my_engineer_debug_info->info_list);break;
		default:	local_parmer[0] = 0;break;
	}
	local_parmer[1] = 0;	//保留
	
	ret = read(device_fd, local_parmer, 3);	
	if(ret < 0)
	{
		printf("read failed:%d\n", ret); 
	}
	return ret;
}

//读取状态,通过属性和序号
//返回值:一个数值
//参数:property-类型，num-序列号。这里应该使用重载，使得返回值可以是任何值，比如int char string
int get_status_from_num( char property, int num)
{
	char local_parmer[4] = "000"; //local_parmer[0]-属性号, local_parmer[1]-保留, local_parmer[2]-表中的序号
	int ret = -1;
	//stpcpy( local_parmer, "312");
	switch(property)
	{
		case 'R':	local_parmer[0] = 1;local_parmer[2] = num;break;
		case 'P':	local_parmer[0] = 2;local_parmer[2] = num;break;
		case 'S':	local_parmer[0] = 3;local_parmer[2] = num;break;
		case 'T':	local_parmer[0] = 4;local_parmer[2] = num;break;
		case 'I':	local_parmer[0] = 5;local_parmer[2] = num;break;
		default:	local_parmer[0] = 0;break;
	}
	local_parmer[1] = 0;	//保留
	
	ret = read(device_fd, local_parmer, 3);	//BUG,使用一个char去接受序列号会产生只能有255个数据大小
	if(ret < 0)
	{
		printf("read failed:%d\n", ret); 
	}
	return ret;
}

//识别指令并执行
//BUG,使用一个char去接受序列号会产生只能有9个数据大小
int recognize_command_and_run(char *cmd)
{
	int ret = -1;
	int value = -1;		//默认值-1
	//printf("enter %s,cmd:%s\n", __func__, cmd);
	int cmd_num = 0;
	if(!strncmp( cmd, "open ", 5))
	{
		cmd_num = cmd[6] - '0';
		ret = ioctl(device_fd, get_cmd_form_property_laber( cmd[5], cmd_num, 1), 2);
		if(ret < 0) printf("open failed\n");
	}
	else if(!strncmp( cmd, "close ", 6))
	{
		cmd_num = cmd[7] - '0';
		ret = ioctl(device_fd, get_cmd_form_property_laber( cmd[6], cmd_num, 0), 2);
		if(ret < 0) printf("close failed\n");
	}
	else if(!strncmp( cmd, "run ", 4))
	{
		cmd_num = cmd[5] - '0';
		ret = ioctl(device_fd, get_cmd_form_property_laber( cmd[4], cmd_num, 1), 2);
		if(ret < 0) printf("run failed\n");
	}
	else if(!strncmp( cmd, "get ", 4))
	{
		if(cmd[4] == 'P')
		{
			cmd_num = cmd[5] - '0';
			ret = cmd_num;
			printf("%d\n", get_status_from_num( 'P', cmd_num));	//BUG,使用一个char去接受序列号会产生只能有9个数据大小
		}
		else if(cmd[4] == 'S')
		{
			cmd_num = cmd[5] - '0';
			ret = get_status_from_num( cmd[4], cmd_num);	//BUG,使用一个char去接受序列号会产生只能有9个数据大小
			printf("ret = %d\n", ret);
			if(ret == 3) 
				printf("Don`t Know\n");
			else
				printf("%s\n", ret?"ON":"OFF");
		}
		
	}
	else if(!strncmp( cmd, "set ", 4))
	{
		//set: P2 253
		cmd_num = cmd[5] - '0';
		value = atoi(cmd+7);
		ret = ioctl(device_fd, get_cmd_form_property_laber( cmd[4], cmd_num, 1), value);
	}
	else if(!strncmp( cmd, "help", 4))
	{
		ret = -3;
		printf("[cmd] [type][num] [other]\n");
		printf("such as: set P1 222\n");
		printf("such as: open S1\n");
		printf("such as: close S2\n");
		printf("such as: run R1\n");
		printf("such as: get R1\n");
	}
	else 
		printf("bad cmd\n");
	return ret;
}


//目标字符串至少两个。
char* find_string(const char* source, char* goal)		//在字符串中寻找某串字符，返回寻找到源数据中目标字段的最末尾指针,无则返回NULL
{
	char *ptr_pos = (char *)source;
	unsigned char state = 0;
	int i = 1;
	int goal_len = strlen(goal);
	//printf("enter find_string\n");
	if(source == NULL||goal == NULL)
		return (char *)source;
	
	while(ptr_pos != NULL)
	{
		if(state == 0)
		{
			//printf("enter strchr\n");
			ptr_pos = strchr(ptr_pos, goal[0]);
			//printf("ptr_pos:%c\n",*ptr_pos==NULL?'0':*ptr_pos);
			if(ptr_pos == NULL)
			{
				//printf("ptr_pos NULL\n");
				break;
			}
		}
		else 
		{
			//printf("ptr_pos:%c\n",*ptr_pos);
			return ptr_pos; 
		}
		for(i = 1; i < goal_len; i++)		//比较后续几个字符
		{
			//printf("enter for,ptr_pos[%d]=%c,goal[%d]=%c\n",i,ptr_pos[i],i,goal[i]);
			if(ptr_pos[i] != goal[i]) 
			{
				ptr_pos+=i; state = 0; 
				//printf("ptr_pos:%c,goal:%c\n", *ptr_pos, goal[i]);
				break;
			}
			if(i == (goal_len-1)) 
			{
				state = 1; ptr_pos = (char *)&ptr_pos[goal_len-1];
				//printf("find it,ptr_pos:%s\n",ptr_pos);
				//printf("find it\n");
			}
			//printf("i = %d\n",i);
		}
	}
	//printf("no find\n");
	return ptr_pos;
}

//获取某个测试项的第n个laber
char *get_option_laber(const char *raw_data, char property, int num)
{
	static char laber_temp[30];
	int num_temp = 0;
	char *laber_head;
	char *laber_end;
	char aim_property[3] = "R<";
	char aim_property_head[3] = ":R";
	aim_property[0] = property;
	aim_property_head[1] = property;
	//printf("aim_property:%s,aim_property_head:%s\n", aim_property, aim_property_head);
	//printf("raw_data:%s\n",raw_data);
	//获取laber复制给结构体
	laber_head = find_string( raw_data, aim_property);
	laber_head++;
	do{
		laber_end = find_string( laber_head, aim_property_head);
		if(laber_end == NULL) break;
		//printf("laber_head:%s,laber_end:%s\n", laber_head, laber_end);
		num_temp++;
		laber_end--;
		memset( laber_temp, '\0', 30);
		strncpy( laber_temp, laber_head, laber_end - laber_head);
		//printf("get laber[%d]:%s\n", num_temp, laber_temp);
		if(num_temp == num) return laber_temp;
		laber_head = find_string( laber_end, ")|");
		laber_head++;
	}while( *laber_head != '\n' || laber_head == NULL);
	return NULL;
}

//获取所有的信息构建成链表,通过底层构建的字符串表格提取各个信息。
engineer_debug_info* get_option_info(const char *raw_data)
{
	char *string_run_num;
	char *string_parame_num;
	char *string_switch_num;
	char *string_transform_num;
	char *string_info_num;
	int num_temp = 1;
	char *laber_temp;
	static engineer_debug_info loacl_engineer_debug_info;
	debug_list *debug_list_buff;
	debug_list *debug_list_last_buff;
	
	if(strlen(raw_data) < 19) return NULL;	//至少有19个字符
	string_run_num = find_string( raw_data, "R=");
	string_parame_num = find_string( raw_data, "P=");
	string_switch_num = find_string( raw_data, "S=");
	string_transform_num = find_string( raw_data, "T=");
	string_info_num = find_string( raw_data, "I=");
	if(string_run_num == NULL || string_parame_num== NULL || string_switch_num== NULL || string_transform_num== NULL || string_info_num== NULL)
		return NULL; //raw_data表格信息错误
	string_run_num++;
	string_parame_num++;
	string_switch_num++;
	string_transform_num++;
	string_info_num++;
	//printf("get pre option num:R=%c,P=%c,S=%c,T=%c,I=%c\n", *string_run_num, *string_parame_num, *string_switch_num, *string_transform_num, *string_info_num);
	loacl_engineer_debug_info.run_num = *string_run_num - '0';
	loacl_engineer_debug_info.parame_num = *string_parame_num - '0';
	loacl_engineer_debug_info.switchs_num = *string_switch_num - '0';
	loacl_engineer_debug_info.transform_num = *string_transform_num - '0';
	loacl_engineer_debug_info.info_num = *string_info_num - '0';

	//获取run表，并组建链表
	do{
		laber_temp = get_option_laber(raw_data, 'R', num_temp);
		if(laber_temp != NULL)
		{
			debug_list_buff = malloc(sizeof(debug_list));
			//printf("debug_list_buff addr :%#p\n", debug_list_buff);
			stpcpy( debug_list_buff->debug_option.laber, laber_temp);
			if(num_temp == 1)
			{
				loacl_engineer_debug_info.run_list = debug_list_buff;
			}
			else
			{
				debug_list_last_buff->next =  debug_list_buff;
			}
			debug_list_last_buff = debug_list_buff;
			debug_list_buff->next = NULL;
			//printf("laber_temp[%d]:%s\n", num_temp, debug_list_buff->debug_option.laber);
		}
		num_temp++;
	}while( laber_temp != NULL);
	
	
	//获取parame表，并组建链表
	num_temp = 1;
	do{
		laber_temp = get_option_laber(raw_data, 'P', num_temp);
		if(laber_temp != NULL)
		{
			debug_list_buff = malloc(sizeof(debug_list));
			//printf("debug_list_buff addr :%#p\n", debug_list_buff);
			stpcpy( debug_list_buff->debug_option.laber, laber_temp);
			if(num_temp == 1)
			{
				loacl_engineer_debug_info.parame_list = debug_list_buff;
			}
			else
			{
				debug_list_last_buff->next =  debug_list_buff;
			}
			debug_list_last_buff = debug_list_buff;
			debug_list_buff->next = NULL;
			//printf("laber_temp[%d]:%s\n", num_temp, debug_list_buff->debug_option.laber);
		}
		num_temp++;
	}while( laber_temp != NULL);
	
	
	//获取switch表，并组建链表
	num_temp = 1;
	do{
		laber_temp = get_option_laber(raw_data, 'S', num_temp);
		if(laber_temp != NULL)
		{
			debug_list_buff = malloc(sizeof(debug_list));
			//printf("debug_list_buff addr :%#p\n", debug_list_buff);
			stpcpy( debug_list_buff->debug_option.laber, laber_temp);
			if(num_temp == 1)
			{
				loacl_engineer_debug_info.switch_list = debug_list_buff;
			}
			else
			{
				debug_list_last_buff->next =  debug_list_buff;
			}
			debug_list_last_buff = debug_list_buff;
			debug_list_buff->next = NULL;
			//printf("laber_temp[%d]:%s\n", num_temp, debug_list_buff->debug_option.laber);
		}
		num_temp++;
	}while( laber_temp != NULL);
	
	
	//获取transform表，并组建链表
	num_temp = 1;
	do{
		laber_temp = get_option_laber(raw_data, 'T', num_temp);
		if(laber_temp != NULL)
		{
			debug_list_buff = malloc(sizeof(debug_list));
			//printf("debug_list_buff addr :%#p\n", debug_list_buff);
			stpcpy( debug_list_buff->debug_option.laber, laber_temp);
			if(num_temp == 1)
			{
				loacl_engineer_debug_info.transform_list = debug_list_buff;
			}
			else
			{
				debug_list_last_buff->next =  debug_list_buff;
			}
			debug_list_last_buff = debug_list_buff;
			debug_list_buff->next = NULL;
			//printf("laber_temp[%d]:%s\n", num_temp, debug_list_buff->debug_option.laber);
		}
		num_temp++;
	}while( laber_temp != NULL);
	
	
	//获取info表，并组建链表
	num_temp = 1;
	do{
		laber_temp = get_option_laber(raw_data, 'I', num_temp);
		if(laber_temp != NULL)
		{
			debug_list_buff = malloc(sizeof(debug_list));
			//printf("debug_list_buff addr :%#p\n", debug_list_buff);
			stpcpy( debug_list_buff->debug_option.laber, laber_temp);
			if(num_temp == 1)
			{
				loacl_engineer_debug_info.info_list = debug_list_buff;
			}
			else
			{
				debug_list_last_buff->next =  debug_list_buff;
			}
			debug_list_last_buff = debug_list_buff;
			debug_list_buff->next = NULL;
			//printf("laber_temp[%d]:%s\n", num_temp, debug_list_buff->debug_option.laber);
		}
		num_temp++;
	}while( laber_temp != NULL);
	
#ifdef PRINT_LIST
	printf_debug_list(loacl_engineer_debug_info.run_list);
	printf_debug_list(loacl_engineer_debug_info.parame_list);
	printf_debug_list(loacl_engineer_debug_info.switch_list);
	printf_debug_list(loacl_engineer_debug_info.transform_list);
	printf_debug_list(loacl_engineer_debug_info.info_list);
#endif	

	
	return &loacl_engineer_debug_info;
}

//打印出一张链表所有的laber
void printf_debug_list(const debug_list *debug_list_head)
{
	debug_list *local_debug_list = (debug_list *)debug_list_head;
	int list_length = 0;
	//printf("enter %s\n",__func__);
	while(local_debug_list != NULL)
	{
		list_length++;
		printf("node[%d]->laber:%s\n", list_length, local_debug_list->debug_option.laber);
		local_debug_list = local_debug_list->next;
	}
	printf("This list have %d node\n", list_length);
}

//通过属性和laber来获取在链表中的序列
int get_laber_list_num(char *laber, const debug_list *debug_list_head)
{
	debug_list *local_debug_list = (debug_list *)debug_list_head;
	int list_length = 0;
	//printf("enter %s\n",__func__);
	while(local_debug_list != NULL)
	{
		list_length++;
		if(!strcmp(local_debug_list->debug_option.laber, laber))
			return list_length;
		local_debug_list = local_debug_list->next;
	}
	return list_length;
}

//获得某个链表中的第x个节点
debug_list* get_list_x_node(const debug_list *head, int num)
{
	debug_list *aim = (debug_list *)head;
	//为0或小于0处理----
	num--;																	//列表从1开始，最小为1
	printf("[px_test]enter %s\n",__func__);
	while(num--)
	{
		aim = head->next;
	}
	return aim;
}

/**************************************************************
函数功能：readFileList,遍历一个文件夹内的所有文件，返回文件个数
输入参数：
输出参数：文件个数
备    注：
****************************************************/
int readFileList(char *basePath)
{
    DIR *dir;
    struct dirent *ptr;
	int file_num = 0;
    char base[1000];

    if ((dir=opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;
        else if(ptr->d_type == 8)    ///file
		{
            printf("device[%d]:%s\n", file_num, ptr->d_name);
			file_num++;
		}
        else if(ptr->d_type == 10)    ///link file
            printf("d_name:%s%s\n",basePath,ptr->d_name);
        else if(ptr->d_type == 4)    ///dir
        {
            memset(base,'\0',sizeof(base));
            strcpy(base,basePath);
            strcat(base,"/");
            strcat(base,ptr->d_name);
            readFileList(base);
        }
    }
    closedir(dir);
    return file_num;
}






