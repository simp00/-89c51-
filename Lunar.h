#ifndef _COLINLUAN_LUNAR_H_
#define _COLINLUAN_LUNAR_H_
#define uchar unsigned char
#define uint unsigned int
 
typedef struct {
	int cYear;
	uchar cMon;
	uchar cDay;
	int reserved;
} Mydate;
//typedef struct spdate
//{
//	char 	cYear;
//	char	cMon;
//	char	cDay;
//}SPDATE;


//extern SPDATE GetSpringDay(uchar GreYear,uchar GreMon,uchar GreDay); 		 //获得农历日期
extern Mydate toLunar(int year,int month,int day) ;
extern bit YearFlag(uchar cYear);											 //判断平年，闰年
extern uchar GetWeekDay(uchar cYear,uchar cMon,uchar cDay);					 //判断当前天是星期几

#endif

