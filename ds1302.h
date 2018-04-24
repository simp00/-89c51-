#ifndef _COLINLUAN_DS1302_H_
#define _COLINLUAN_DS1302_H_
#define uchar unsigned char
typedef struct systime
{
	uchar cYear;
	uchar	cMon;
	uchar	cDay;
	uchar	cHour;
	uchar	cMin;
	uchar	cSec;
	uchar	cWeek;
}SYSTIME;

extern void DS1302_Write(uchar ucData);
extern uchar DS1302_Read();
extern void DS1302_Init();
extern uchar WDS1302(uchar ucAddr,uchar ucDar);
extern uchar RDS1302(uchar ucAddr);
extern void  SetTime(SYSTIME sys);
extern void  GetTime(SYSTIME *sys);

#endif