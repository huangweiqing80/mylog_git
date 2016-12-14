#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/klog.h>
#include <signal.h>

#define BUF_SIZE 1024*5
#define Log_Path "/home/Log_hwq/"
#define Maxinum 1024*10

int open_fd;
int read_size = 0,save_size = 0;
char * buf;
int file_num = 0;


void makeFile(char *new_file);
void writeLog(char *buf);
int fileSizeCtl();



void writeLog(char *buf)
{
	int file_size = 0;	
	while(1)
	{
		read_size = klogctl(2,buf+save_size,BUF_SIZE-save_size);
		printf("read %d\n",read_size);
		
		if(0>read_size)
			perror("klogctl error");
		else
		{			
			save_size += read_size; 
			if(save_size>=BUF_SIZE)
			{
				printf("now write log to file\n");
				write(open_fd,buf,save_size);
				
				read_size = 0;
				save_size = 0;
				memset(buf,0,sizeof(BUF_SIZE));
			
				file_size = fileSizeCtl();
				if(file_size > Maxinum)
				{
					if(file_num == 1)
					{
				
						lseek(open_fd,0,SEEK_SET);
					}
					else
					{
						system("rm /home/Log_hwq/$(ls /home/Log_hwq/ -rt | sed -n '1p')");
						printf("remove one file\n");
					}
				}
				
				file_num = 0;
				sleep(1);
			}
		}
	}
}
void makeFile(char *new_file)
{
	time_t ctime;
	struct tm *tm;
	
	umask(0);//屏蔽创建文件权限
	int fd = mkdir(Log_Path,777);	
	if((fd < 0) && (errno != EEXIST))
	{
		perror("mkdir file erro");
	}

	ctime = time(NULL);
	
	tm = localtime(&ctime);
	sprintf(new_file,"%s%2.2d_%2.2d:%2.2d:%2.2d",Log_Path,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);//将时间转换为字符串
	int fd_newfile = creat(new_file,777);//创建目标文件
}

int fileSizeCtl()
{
	int fd;
	DIR *d;
	struct dirent *de;
	struct stat file_buf;
	int exists;
	int total_size;
	char filename[50];
	

	
	/********计算文件夹大小***********/

	d = opendir(Log_Path);
	
	if(d == NULL)
	{
		perror("prsize");
		exit(1);
	}

	total_size = 0;
	while((de = readdir(d))!=NULL)
	{
		if(strncmp(de->d_name,".",1) == 0)//跳过目录.和..
			continue;
			
		sprintf(filename,"%s%s",Log_Path,de->d_name);
		exists = stat(filename,&file_buf);
		if(exists < 0)
		{
			fprintf(stderr,"Could not stat %s\n",de->d_name);
		}
		else
		total_size += file_buf.st_size;
	
		file_num++;

	}
	
	printf("totalsize:%d\n",total_size);

	close(fd);
	closedir(d);

	return total_size;

}


void mySigFunc(int a)
{
	write(open_fd,buf,save_size);
	printf("now write log to file\n");
	read_size = 0;
	save_size = 0;
	memset(buf,0,sizeof(buf));
			
	int file_size = fileSizeCtl();
	if(file_size > Maxinum)
	{
		if(file_num == 1)
		{
			
		lseek(open_fd,0,SEEK_SET);
		}
		else
		system("rm /home/Log_hwq/$(ls /home/Log_hwq/ -rt | sed -n '1p')");
	}
	file_num = 0;
	sleep(1);
}

int main()
{
	char *new_file = (char *)malloc(50 * sizeof(char));
	makeFile(new_file);
	buf = (char *)malloc((BUF_SIZE));
	open_fd = open(new_file,O_RDWR | O_CREAT,0755);
	signal(SIGINT,mySigFunc);
	writeLog(buf);
			
	free(buf);
	free(new_file);
	return 0;
}

