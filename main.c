
#include "display.h"

extern void LCD_ShowWNL();
extern void GUI_Init();
extern void SFR_Init();
extern void Time_Set();
extern SYSTIME sys;		   //系统日期
extern  Mydate SpDat;		   //农历日期
void  main(){	//主函数
	 SFR_Init();
	
	 GUI_Init();
	 TR1=1;
	 while(1){
		GetTime(&sys);		//获得时间
		LCD_ShowWNL();		//显示万年历
		Time_Set();         //时间设置
	 }	
}