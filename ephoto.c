#include<stdio.h>
#include <sys/types.h>//头文件
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include<linux/input.h>//输入子系统头文件
int x,y;
int flag=0;
int show_bmp(int x0,int y0,int w ,int h,char *picname)
{
   // 1.打开LCD驱动文件
      int lcd_fd = open("/dev/fb0",O_RDWR);
	  if(lcd_fd<0)
      {
       printf("open lcd fail\n");
	   return -1;
      }
  printf("open ok \n");
  //2申请共享内存
    char *p=mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd_fd,0);

  //3 打开bmp图片
      int bmp_fd = open(picname,O_RDWR);
	  if(bmp_fd<0)
      {
       printf("open bmp fail\n");
	   return -1;
      }
      else
		  printf("open bmp ok \n");
 //4.把bmp图片的颜色数据读取出来
  char buf24[w*h*3];
  lseek(bmp_fd,54,SEEK_SET);
  int ret=read(bmp_fd,buf24,w*h*3);
  if(ret<0)
  {
   printf("read bmp fail\n");
   return -1;
  }
   printf("read bmp ok \n");
 // 4.把24位的bmp数据转换成32位lcd数据同时放到共享内存里
   int x,y;//y 行 x 列
    for(y=y0;y<h+y0;y++)//行
      for(x=x0;x<w+x0;x++)//列
      {
		p[y*800*4+x*4+0]=buf24[(h-y+y0-1)*w*3+(x-x0)*3+0];
        p[y*800*4+x*4+1]=buf24[(h-y+y0-1)*w*3+(x-x0)*3+1];
		p[y*800*4+x*4+2]=buf24[(h-y+y0-1)*w*3+(x-x0)*3+2];
        p[y*800*4+x*4+3]=0;
	  }	  	 
  //6.关闭lcd  bmp 释放内存
    close(lcd_fd);
	close(bmp_fd);
    munmap(p, 800*480*4);
}

int start()//显示进度条、系统主界面
{
   //显示启动背景
   show_bmp(0,0,800,480,"1.bmp");
   //显示进度条
   int x=60;
   while(1)
   { 
     show_bmp(x,280,40,40,"2.bmp"); 
	 x+=5;
	 if(x>=700)
	 {
	   break;
	 }
	 usleep(10000);
   }
  //显示主界面
  sleep(1);
   show_bmp(0,0,800,480,"3.bmp"); 
   flag=0;
}
int show_main()//显示主界面
{
	flag=0;
	show_bmp(0,0,800,480,"3.bmp");
}
int get_xy()//得到触摸点XY坐标
{
   //1.打开触摸屏的驱动文件
  int ts_fd=open("/dev/input/event0",O_RDWR);
  if(ts_fd<0)
  {
    printf("open ts fail\n");
	return -1;
  }
  printf("open ts ok\n");
  //定义输入子系统模型
   struct input_event ts;
  //把触摸事件从驱动里读到输入子系统模型
  
 
  while(1)
  {
     read(ts_fd,&ts,sizeof(ts));
	 if(ts.type==EV_ABS)
	 {
	    if(ts.code==ABS_X)
		{
		   x=ts.value;
		}
		 if(ts.code==ABS_Y)
		{
		   y=ts.value;
		}
	   /* if(ts.code==ABS_PRESSURE&&ts.value==0)
	   {
	     break;
	   } */
	 }
	 if(ts.type==EV_KEY)
	 {
	    if(ts.code==BTN_TOUCH&&ts.value==0)
          break;		
	 }
  
  }  
 printf("x is %d y is %d\n",x,y);
 
}
int enter_photos()//进入电子相册
{
	int i=0;
	char picname[3][20]={
    "1.bmp",
	"2.bmp",
	"3.bmp"
} ;
	flag=1;
	show_bmp(0,0,800,480,picname[0]);
	show_bmp(720,0,80,40,"4.bmp");
	while(1){
		get_xy();
		if(x>=900&&y<=40){
			break;
		}
		else if(x<400){
			if(i==0)
				i=2;
			else
				i--;
			show_bmp(0,0,800,480,picname[i]);
			show_bmp(720,0,80,40,"2.bmp");
			continue;
		}
		else if(x>400){
			if(i==2)
				i=0;
			else
				i++;
			show_bmp(0,0,800,480,picname[i]);
			show_bmp(720,0,80,40,"1.bmp");
			continue;
		}
	}
	show_main();
	flag=0;
}
int main()
{
     start();
	while(1){
		if(flag==0){
			get_xy();
			enter_photos();
		}
		else if(flag==1){
			get_xy();
			show_main();
		}
	}	 
	 return 0;
}