#include <linux/module.h>       /* Needed by all modules */
#include <linux/kernel.h>       /* Needed for log level */
#include <linux/init.h>         /* Needed for the macros */
#include <linux/delay.h>
 
MODULE_AUTHOR("Jay, <Jay's email> ");
MODULE_DESCRIPTION("To test: console log level.");
MODULE_LICENSE("GPL");
MODULE_VERSION("Version-0.0.1");
 
static int __init hello_start(void)
{
	int i = 0;
	for(i = 0; i < 1024; i ++)
	{
		printk(KERN_CRIT "-------Now run the test program %d--------\n",i); 
		msleep(10);
	}
return 0;
}
 
static void __exit hello_end(void)
{
printk(KERN_INFO "Goodbye, Jay.\n");
}
 
module_init(hello_start);
module_exit(hello_end);




/*
int printk(const char *fmt, ...)
printk(KERN_ALERT "Book module exit\n");
日志级别一共有8个级别，printk的日志级别定义如下（在include/linux/kernel.h中）：
#define KERN_EMERG 0//紧急事件消息，系统崩溃之前提示，表示系统不可用
#define KERN_ALERT 1//报告消息，表示必须立即采取措施
#define KERN_CRIT 2//临界条件，通常涉及严重的硬件或软件操作失败
#define KERN_ERR 3//错误条件，驱动程序常用KERN_ERR来报告硬件的错误
#define KERN_WARNING 4//警告条件，对可能出现问题的情况进行警告
#define KERN_NOTICE 5//正常但又重要的条件，用于提醒
#define KERN_INFO 6//提示信息，如驱动程序启动时，打印硬件信息
#define KERN_DEBUG 7//调试级别的消息

*/






