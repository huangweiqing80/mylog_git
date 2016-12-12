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

#define BUF_SIZE 1024*5
#define Log_Path "/home/Log_hwq/"
#define KLOG_SIZE_BUFFER   10
#define Maxinum 1024*5

int open_fd;
int readSize = 0,saveSize = 0;
char new_file[50];
char buf[BUF_SIZE];
void makeFile();
void *write_log(void *arg);
int filesize_ctl();

int file_num = 0;

void *write_log(void *arg)
{
	int filesize = 0;	
	while(1)
	{
		readSize = klogctl(2,buf+saveSize,BUF_SIZE-saveSize);
		printf("%d\n",readSize);
		//write(open_fd,buf,BUF_SIZE);
		if(0>=readSize)
			perror("klogctl error");
		else
		{			
			saveSize += readSize; 
			if(saveSize>=BUF_SIZE)
			{
				//write_logfile();
				write(open_fd,buf,saveSize);
				printf("now write log to file\n");
				readSize = 0;
				saveSize = 0;
				memset(buf,0,sizeof(buf));
			
				filesize = filesize_ctl();
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
		}
	}
}
void makeFile()
{
	time_t ctime;
	struct tm *tm;
	
	memset(buf,'\0',sizeof(buf));
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
	

	
	/********计算文件夹大小***********/
	//while(1)
	//{
	char *filename = (char *)malloc(50*sizeof(char));
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
	
	printf("totalsize:%d",total_size);
	printf("file_num:%d",file_num);


	sleep(1);
	free(filename);
	closedir(d);
	close(fd);
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


void *signal_ctl(void *arg)
{
	while(1)
	{
		signal(SIGINT,mysig_func);
		pause();
	}
}
int main()
{
	pthread_t writelog_tid,signal_tid;

	
	makeFile();
	open_fd = open(new_file,O_RDWR | O_CREAT,0755);
	
	pthread_create(&writelog_tid,NULL,write_log,NULL);
	pthread_create(&signal_tid,NULL,signal_ctl,NULL);
	pthread_join(writelog_tid,NULL);
	pthread_join(signal_tid,NULL);
	return 0;
}

