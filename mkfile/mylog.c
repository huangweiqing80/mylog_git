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

#define BUFSIZE 512
#define MAXINUM 1024*3
#define PATH "/home/Log_hwq/"


FILE *openfd;
int readsize = 0,savesize = 0;
char * buf;
int filenum = 0;
char newfile[50];
long fileindex = 0;

void makefile(char *newfile);
void writelog(char *buf);
int filesizectrl();
int removeoldfile();

int removeoldfile()
{
        DIR *d;
        struct OLDERFILE{
                long mtime;
                char filename[50];
        }olderfile;
        struct dirent *de;
        char fname[50];
        struct stat filebuf;
        int findnum = 0;
        d = opendir(PATH);
        long mtime = 0;
        if(d == NULL)
        {
                perror("prsize");
                exit(1);
        }

        while((de = readdir(d))!=NULL)
        {
		if(strncmp(de->d_name,".",1) == 0)
                        continue;
                sprintf(fname,"%s%s",PATH,de->d_name);
		if(strcmp(fname,newfile) == 0)
			continue;
                int exists = stat(fname,&filebuf);
                if(exists < 0)
                {
                        fprintf(stderr,"Could not stat %s\n",de->d_name);
                }
                else
                {
                        if(findnum == 0)
                        {
                                olderfile.mtime = filebuf.st_mtime;
                                sprintf(olderfile.filename,"%s%s",PATH,de->d_name);
                                findnum ++;
                        }
                        else
                        {
                                if(filebuf.st_mtime<olderfile.mtime)
                                {
                                        olderfile.mtime = filebuf.st_mtime;
                                        sprintf(olderfile.filename,"%s%s",PATH,de->d_name);
                                }
                                findnum ++;
                        }
                        printf("%s:file->st_mtime is%d\n",de->d_name,filebuf.st_mtime);
                }
        }      
        printf("now remove %s\n",olderfile.filename);
        remove(olderfile.filename);
        return 0;
}

void writelog(char *buf)
{
	int filesize = 0;
	char cmd[125];
	
	while(1)
	{
		readsize = klogctl(2,buf+savesize,BUFSIZE-savesize);
		printf("read %d\n",readsize);
		
		if(0>readsize)
			perror("klogctl error");
		else
		{			
			savesize += readsize; 
			if(savesize>=BUFSIZE)
			{
				printf("now write log to file\n");
                                openfd = fopen(newfile,"r+");
                                fseek(openfd,fileindex,SEEK_SET);
                                int writenum = fwrite(buf,1,savesize,openfd);
                                printf("writenum is %d\n",writenum);
                                fileindex = ftell(openfd);
				fclose(openfd);

				readsize = 0;
				savesize = 0;
				memset(buf,0,BUFSIZE);
			
				filesize = filesizectrl();
				if(filesize >= MAXINUM)
				{
					if(filenum == 1)
					{
						if(fileindex >= MAXINUM)
						{
							fileindex = 0;
						}										
					}
					else
					{
						//sprintf(cmd,"rm %s$(ls %s -rt | sed -n '1p')",PATH,PATH);
						//system(cmd);
						//printf("remove one file\n");
						removeoldfile();
					}
				}
				filenum = 0;
				
			}
		}
		sleep(1);
	}
}

void makefile(char *newfile)
{
	time_t ctime;
	struct tm *tm;
	
	umask(0);//屏蔽创建文件权限
	int fd = mkdir(PATH,777);	
	if((fd < 0) && (errno != EEXIST))
	{
		perror("mkdir file erro");
	}

	ctime = time(NULL);
	
	tm = localtime(&ctime);
	sprintf(newfile,"%s%2d_%2.2d_%2.2d-%2.2d:%2.2d:%2.2d",PATH,tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);//将时间转换为字符串
	int newfilefd = creat(newfile,777);//创建目标文件
}

int filesizectrl()
{
	int fd;
	DIR *d;
	struct dirent *de;
	struct stat filebuf;
	int exists;
	int totalsize;
	char filename[50];
	

	
	/********计算文件夹大小***********/

	d = opendir(PATH);
	
	if(d == NULL)
	{
		perror("prsize");
		exit(1);
	}

	totalsize = 0;
	while((de = readdir(d))!=NULL)
	{
		if(strncmp(de->d_name,".",1) == 0)//跳过目录.和..
			continue;
			
		sprintf(filename,"%s%s",PATH,de->d_name);
		exists = stat(filename,&filebuf);
		if(exists < 0)
		{
			fprintf(stderr,"Could not stat %s\n",de->d_name);
		}
		else
		totalsize += filebuf.st_size;
	
		filenum++;

	}
	
	printf("totalsize:%d\n",totalsize);

	close(fd);
	closedir(d);

	return totalsize;

}


void sigfunc(int a)
{
	char cmd[125];
	printf("now write log to file\n");
        openfd = fopen(newfile,"r+");
        fseek(openfd,fileindex,SEEK_SET);
        int writenum = fwrite(buf,1,savesize,openfd);
        printf("writenum is %d\n",writenum);
        fileindex = ftell(openfd);
        fclose(openfd);

        readsize = 0;
        savesize = 0;
        memset(buf,0,BUFSIZE);			

	int filesize = filesizectrl();
        if(filesize >= MAXINUM)
        {
	        if(filenum == 1)
                {
        	        if(fileindex >= MAXINUM)
                        {
               		        fileindex = 0;
                        }
                }
                else
                {
	                //sprintf(cmd,"rm %s$(ls %s -rt | sed -n '1p')",PATH,PATH);
        	        //system(cmd);
	                //printf("remove one file\n");
			removeoldfile();
                }
       }
                filenum = 0;
}

int main()
{
	makefile(newfile);
	buf = (char *)malloc((BUFSIZE));
	signal(SIGINT,sigfunc);
	writelog(buf);
			
	free(buf);
	return 0;
}
