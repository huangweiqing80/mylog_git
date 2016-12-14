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

#include <pthread.h>

#include <signal.h>

#define BUF_SIZE 500
#define Log_Path "/home/Log_hwq/"
#define KLOG_SIZE_BUFFER   10
#define Maxinum 1024*10

int open_fd;
int readSize = 0,saveSize = 0;
char new_file[50];
//char buf[BUF_SIZE];
char * buf;
void makeFile();
void write_log(char *buf);
int filesize_ctl();

int file_num = 0;

void write_log(char *buf)
{
	int filesize = 0;	
	while(1)
	{
		readSize = klogctl(2,buf+saveSize,BUF_SIZE-saveSize);
		printf("read %d\n",readSize);
		//write(open_fd,buf,BUF_SIZE);
		if(0>readSize)
			perror("klogctl error");
		else
		{			
			saveSize += readSize; 
			if(saveSize>=BUF_SIZE)
			{
				//write_logfile();
				printf("now write log to file\n");
				write(open_fd,buf,saveSize);
				
				readSize = 0;
				saveSize = 0;
				memset(buf,0,sizeof(BUF_SIZE));
			
				filesize = filesize_ctl();
				if(filesize > Maxinum)
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
void makeFile()
{
	time_t ctime;
	struct tm *tm;
	
	//memset(buf,'\0',sizeof(buf));
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

int filesize_ctl()
{
	int fd;
	DIR *d;
	struct dirent *de;
	struct stat file_buf;
	int exists;
	int total_size;
	char filename[50];
	

	
	/********计算文件夹大小***********/
	//while(1)
	//{
	//char *filename = (char *)malloc(50*sizeof(char));
	d = opendir(Log_Path);
	
	//d = opendir("/home/Log_hwq/");
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
		//printf("%s",filename);
		exists = stat(filename,&file_buf);
		if(exists < 0)
		{
			fprintf(stderr,"Could not stat %s\n",de->d_name);
		}
		else
		total_size += file_buf.st_size;
		//printf("totalsize:%d",total_size);
		file_num++;

	}
	
	printf("totalsize:%d\n",total_size);
	


	//sleep(1);
	//free(filename);
	close(fd);
	closedir(d);
	//printf("hello");
	return total_size;
	//}
}


void mysig_func(int a)
{
	write(open_fd,buf,saveSize);
	printf("now write log to file\n");
	readSize = 0;
	saveSize = 0;
	memset(buf,0,sizeof(buf));
			
	int filesize = filesize_ctl();
	if(filesize > Maxinum)
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
	int filesize = 0;	
	
	makeFile();
	buf = (char *)malloc((BUF_SIZE));
	open_fd = open(new_file,O_RDWR | O_CREAT,0755);
	signal(SIGINT,mysig_func);
	write_log(buf);
			
	free(buf);
	return 0;
}

