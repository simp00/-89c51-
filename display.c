
#include "display.h"
#include "12864.h"
#include "model.h"
#include "ds1302.h"
#include "lunar.h"
#include "keyinput.h"

#define uchar unsigned char
#define uint unsigned int
#define NoUpLine 	1
#define UpLine   	0
#define NoUnderLine 1
#define UnderLine	0
#define FALSE	0
#define TRUE    1

uchar dispBuf[7];
uchar T0_Count=0,Tmp_Count=0;
bit T0_Flag,Tmp_Flag,Flash_Flag;
void DecToBCD();
void BCDToDEC();
SYSTIME sys;		   //系统日期
Mydate SpDat;		   //农历日期

bit Hour_Flag=TRUE,Min_Flag=TRUE,Sec_Flag=TRUE;	   //设置时间标志
bit Year_Flag=TRUE,Mon_Flag=TRUE,Day_Flag=TRUE;

uchar State_Set=0;     //设置时、分、秒、日、月、年等状态

bit State_Flag=FALSE,Inc_Flag=FALSE,Dec_Flag=FALSE;	//三个按键是否按下的标志

uchar code Mon2[2][13]={0,31,28,31,30,31,30,31,31,30,31,30,31,
				  0,31,29,31,30,31,30,31,31,30,31,30,31};

/*****************************LCD显示函数******************************
入口参数：
		cDat:		要显示的数
		X:			行数 0~7
		Y:          列数 0~127
		show_flag:  是否反白显示，0反白，1不反白 
		upline: 	上划线, 0表示带上划线
		underline:  下划线, 0表示带下划线
***********************************************************************/
void LCD_ShowTime(char cDat,uchar X,uchar Y,bit show_flag,bit up,bit under){
	uchar s[2];
   	s[0]=cDat/10+'0';
	s[1]=cDat%10+'0';
	en_disp(X,Y,2,Asc,s,show_flag,up,under);
}
																  
void Show_YMD(){	  //年、月、日、星期、显示函数
	uchar uiTempDat;
   	uiTempDat=RDS1302(0x88|0x01);
	sys.cMon=((uiTempDat&0x1f)>>4)*10+(uiTempDat&0x0f);	
	LCD_ShowTime(sys.cMon,2,5,Mon_Flag,NoUpLine,NoUnderLine);	
	hz_disp(4,5,1,uMod[1],1,NoUpLine,NoUnderLine);				//月
	Show16X32(2,27,ucNum3216[sys.cDay/10],Day_Flag);			//日
	Show16X32(2,43,ucNum3216[sys.cDay%10],Day_Flag);  			
	hz_disp(6,8,2,ucLunar[13],1,UpLine,UnderLine);					    
	if(sys.cWeek==7)
	hz_disp(6,40,1,uMod[2],1,UpLine,UnderLine);					//星期 
	else
    hz_disp(6,40,1,ucLunar[sys.cWeek],1,UpLine,UnderLine);		//星期
    LCD_ShowTime(20,0,9,1,UpLine,UnderLine);
  	LCD_ShowTime(sys.cYear,0,25,Year_Flag,UpLine,UnderLine);
    hz_disp(0,41,1,uMod[0],1,UpLine,UnderLine);					//年

	//SpDat=GetSpringDay(sys.cYear,sys.cMon,sys.cDay);			//获得农历
		SpDat=toLunar(sys.cYear+2000,sys.cMon,sys.cDay);
//		SpDat.cMon = 9;
//	SpDat.cDay= 26;
//	SpDat.cYear = 2017;
    if(SpDat.cMon==1)										   	//显示农历月
	  hz_disp(4,64,1,ucLunar[15],1,UpLine,NoUnderLine);			//"正"
	else if(SpDat.cMon==11)
	  hz_disp(4,64,1,ucLunar[16],1,UpLine,NoUnderLine);			//"冬"
	else if(SpDat.cMon==12)
	  hz_disp(4,64,1,ucLunar[17],1,UpLine,NoUnderLine);			//"腊"
	else
	  hz_disp(4,63,1,ucLunar[SpDat.cMon],1,UpLine,NoUnderLine); //"二"~"十"
	 if(SpDat.cDay/10==1 && SpDat.cDay%10>0)					//显示"十" 例如"十四"而不是"一四"
	 hz_disp(4,95,1,ucLunar[10],1,UpLine,NoUnderLine);
	 else if(SpDat.cDay/10==2 && SpDat.cDay%10>0)               //显示"廿" 例如"廿三"而不是"二四"
	 hz_disp(4,95,1,ucLunar[19],1,UpLine,NoUnderLine);
	 else
	 hz_disp(4,95,1,ucLunar[SpDat.cDay/10],1,UpLine,NoUnderLine);  //正常数字
	if(!(SpDat.cDay%10))	    							    //"十"
	 hz_disp(4,111,1,ucLunar[10],1,UpLine,NoUnderLine);
	else														//正常数字
	 hz_disp(4,111,1,ucLunar[SpDat.cDay%10],1,UpLine,NoUnderLine);

	 hz_disp(0,104,1,SX[(uint)(SpDat.cYear)%12],1,UpLine,UnderLine);   //生肖

	 hz_disp(2,95,1,TianGan[(uint)(SpDat.cYear)%10],1,NoUpLine,NoUnderLine); //天干
	 hz_disp(2,111,1,DiZhi[(uint)(SpDat.cYear)%12],1,NoUpLine,NoUnderLine);  //地支
}

void LCD_ShowWNL(){	  //万年历显示函数
	LCD_ShowTime(sys.cSec,6,111,Sec_Flag,UpLine,UnderLine);		 //秒，每秒钟刷新
	if(!sys.cSec || State_Set)								     //分，普通模式每分钟刷新
	LCD_ShowTime(sys.cMin,6,87,Min_Flag,UpLine,UnderLine);		 //    设置模式每次刷新
	 
	if(!sys.cSec && !sys.cMin || State_Set)						 //时，普通模式每小时刷新
    LCD_ShowTime(sys.cHour,6,63,Hour_Flag,UpLine,UnderLine);     //    设置模式每次刷新
	
	if(!sys.cSec && !sys.cMin && !sys.cHour || State_Set ){      //公历农历的年、月、日、星期
		Show_YMD();												 //普通模式每天刷新
		if(State_Set==7) State_Set=0;							 //设置模式每次刷新
	}	
}

//void CAL_Init(){	//日期初始化函数
////	sys.cYear=0x17;	//BCD码表示的日历时间值
////	sys.cMon=0x11;
////	sys.cDay=0x14;
////	sys.cHour=0x20;
////	sys.cMin=0x12;
////	sys.cSec=0x11;
////	sys.cWeek=GetWeekDay(sys.cYear,sys.cMon,sys.cDay);
//	
//	GetTime(&sys);				   //获得时间
//	 SetTime(sys);				   //设置时间
//	sys.cWeek=GetWeekDay(sys.cYear,sys.cMon,sys.cDay);
//}

void SFR_Init(){		 //定时器1初始化函数
	Flash_Flag=FALSE;
	TMOD=0x11;
	ET1=1;
	TH1= (-10000)/256;
	TL1= (-10000)%256;
	EA=1;
}

void GUI_Init(){	 //LCD图形初始化函数
	 LCD12864_init();
	 ClearLCD();
	 Rect(0,0,127,63,1);   //描绘框架
	 Line(62,0,62,62,1);
	 Line(0,48,127,48,1);
	 Line(0,15,127,15,1);
	 Line(24,15,24,48,1);
	 Line(63,32,128,32,1);
	
		GetTime(&sys);				   //获得时间
		DecToBCD();
	 //SetTime(sys);				   //设置时间
		sys.cWeek=GetWeekDay(sys.cYear,sys.cMon,sys.cDay);
		SetTime(sys);				   //设置时间
	//GetTime(&sys);				   //获得时间
		BCDToDEC();
		Show_YMD();
	 LCD_ShowTime(sys.cSec,6,111,Sec_Flag,UpLine,UnderLine);
	 en_disp(6,103,1,Asc,":",1,UpLine,UnderLine);
	 LCD_ShowTime(sys.cMin,6,87,Min_Flag,UpLine,UnderLine); 
	 en_disp(6,79,1,Asc,":",1,UpLine,UnderLine);	 
     LCD_ShowTime(sys.cHour,6,63,Hour_Flag,UpLine,UnderLine);

	 hz_disp(2,64,1,ucLunar[11],1,NoUpLine,NoUnderLine);	   //"农"
	 hz_disp(2,80,1,ucLunar[12],1,NoUpLine,NoUnderLine);	   //"历"
	 hz_disp(4,79,1,uMod[1],1,UpLine,NoUnderLine);			   //"月"
}

void DecToBCD(){   //十进制转换二进制
	sys.cHour=(((sys.cHour)/10)<<4)+((sys.cHour)%10);
	sys.cMin=(((sys.cMin)/10)<<4)+((sys.cMin)%10);
	sys.cSec=((sys.cSec/10)<<4)+((sys.cSec)%10);
  	sys.cYear=((sys.cYear/10)<<4)+((sys.cYear)%10);
	//sys.cMon=((sys.cMon/10)<<4)+((sys.cMon)%10);//gyz 20171120注释
	sys.cDay=((sys.cDay/10)<<4)+((sys.cDay)%10);
}
void BCDToDEC(){ //二进制转换十进制函数
	sys.cYear=(((sys.cYear)>>4)&0X0f)*10+(sys.cYear&0x0f);
	sys.cDay=(((sys.cDay)>>4)&0X0f)*10+(sys.cDay&0x0f);
	sys.cHour=(((sys.cHour)>>4)&0X0f)*10+(sys.cHour&0x0f);
	sys.cMin=(((sys.cMin)>>4)&0X0f)*10+(sys.cMin&0x0f);
	sys.cSec=(((sys.cSec)>>4)&0X0f)*10+(sys.cSec&0x0f);

}
void Time_Set(){   //时间设置函数
	if(State_Flag){					//设置键按下
		State_Flag=FALSE;
		State_Set++;
		if(State_Set==8) State_Set=0;
	}

	Hour_Flag=TRUE;Min_Flag=TRUE;Sec_Flag=TRUE;
    Year_Flag=TRUE;Mon_Flag=TRUE;Day_Flag=TRUE;

	switch(State_Set){              //设置类型
			case 0:                 //无设置
				break;
			case 1:					//设置时
				Hour_Flag=FALSE;
				break;
			case 2:	   				//设置分
				Min_Flag=FALSE;
				break;
			case 3:				 	//设置秒
				Sec_Flag=FALSE;
				break;
			case 4:					//设置天
			 	Day_Flag=FALSE;
				break;
			case 5:
			 	Mon_Flag=FALSE;     //设置月
				break;
			case 6:
			 	Year_Flag=FALSE;	//设置年
		    	break;
			case 7:					//无动作，设置此值为让"年"的反白消失。
				break;
	  }	   
	  
	if(Inc_Flag){					//加键被按下
		Inc_Flag=FALSE;
		switch(State_Set)		{
			case 0:
				break;
			case 1:	   				 //小时加
				sys.cHour++;
				(sys.cHour)%=24;
				break;
			case 2:		   			 //分加1
				sys.cMin++;
				sys.cMin%=60;
				break;
			case 3:					 //秒加1
			    sys.cSec++;
				sys.cSec%=60;
				break;
			case 4:					 //天加1
				(sys.cDay)=(sys.cDay%Mon2[YearFlag(sys.cYear)][sys.cMon])+1;		   
				break;
			case 5:					 //月加1
				sys.cMon=(sys.cMon%12)+1;
				break;
			case 6:
				sys.cYear++;		 //年加1
				sys.cYear=sys.cYear%100;
			break;
		  }	
		DecToBCD();					//转为BCD数
		sys.cWeek=GetWeekDay(sys.cYear,sys.cMon,sys.cDay);    //算出星期
		SetTime(sys);	    		//存入DS1302
	}

	if(Dec_Flag){				 	//减键按下
		Dec_Flag=FALSE;
		switch(State_Set){
			case 0:
				break;
			case 1:	   
				sys.cHour=(sys.cHour+23)%24;  //时减1
				break;
			case 2:		  					  //分减1
				sys.cMin=(sys.cMin+59)%60;
				break;
			case 3:							  //秒减1
				sys.cSec=(sys.cSec+59)%60;
				break;
			case 4:							  //天减1
				sys.cDay=((sys.cDay+Mon2[YearFlag(sys.cYear)][sys.cMon]-1)%Mon2[YearFlag(sys.cYear)][sys.cMon]);		   
				if(sys.cDay==0) sys.cDay=Mon2[YearFlag(sys.cYear)][sys.cMon];
				break;
			case 5:							  //月减1
				sys.cMon=(sys.cMon+11)%12;
				if(sys.cMon==0) sys.cMon=12;
				break;
			case 6:							  //年减1
				sys.cYear=(sys.cYear+99)%100;
				break;
		  }	
		DecToBCD();
		sys.cWeek=GetWeekDay(sys.cYear,sys.cMon,sys.cDay);     	 
		SetTime(sys);	
	}
}



void timer1() interrupt  3 {  //定时器1中断服务函数
	TH1= (-10000)/256;
	TL1= (-10000)%256;
	keyinput();					//读取按键
	if (keyvalue&0x10){	
		State_Flag=TRUE;
		keyvalue &= 0xef;		//清键值，保证一直按下只执行一次按键动作。
	}	
	if (keyvalue&0x20 ){		//加
		Inc_Flag=TRUE;
		keyvalue &= 0xdf;		//清键值，保证一直按下只执行一次按键动作。
	}	
	if (keyvalue&0x40){			//减
		Dec_Flag=TRUE;
		keyvalue &= 0xbf;		//清键值，保证一直按下只执行一次按键动作。
	}
	
}