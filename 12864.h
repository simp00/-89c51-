/*-----------------------------------------------------------*/
#define LCD_OFF 0x3E  
#define LCD_ON  0x3F

#define Add_X 0xB8     //the start address of the page 0 ;(0~7)
#define Add_Y 0x40     //the start address of the Y counter ; (0~64)
#define Add_Z 0xC0     //the start address of the DDRAM ; (0~64)

#define UPLINE      0x01
#define UNDERLINE	0x80	    
/*-----------------------------------------------------------*/
#define  LCD12864_DATA_PORT  P0
/*仿真电路驱动*/
sbit  LCD12864_EN         =P2^4;
sbit  LCD12864_RW         =P2^3;       //0:write ; 1:read 
sbit  LCD12864_RS         =P2^2;	   //0:the command .1:the data
sbit  LCD12864_CS_L       =P2^1;       //select the left of the lcd when 1
sbit  LCD12864_CS_R       =P2^0;  
sbit  LCD12864_RST 	   	  =P2^5;

////炜煌开发板驱动
//sbit  LCD12864_EN         =P2^3;
//sbit  LCD12864_RW         =P2^4;       //0:write ; 1:read 
//sbit  LCD12864_RS         =P2^5;	   //0:the command .1:the data
//sbit  LCD12864_CS_L       =P2^1;       //select the left of the lcd when 1
//sbit  LCD12864_CS_R       =P2^2;  
//sbit  LCD12864_RST 	   	  =P2^0;

/*------------------------------------------------------------*/
void delayus(unsigned int us){
   while(us--);
}
/*

while(1)
{
	us--;
if(us==0)break;
}

*/
/*--------------------------select the LCD--------------------*/
void LCDSel(unsigned char sel){
    switch(sel)	{
	   case 0: LCD12864_CS_L=0;LCD12864_CS_R=0;break;	  
	   case 1: LCD12864_CS_L=1;LCD12864_CS_R=0;break;       //left 
	   case 2: LCD12864_CS_L=0;LCD12864_CS_R=1;break;		//right
	   default:;
	};
}

/*------------------------------------------------------------*/
void WaitLCD(){
   unsigned char flag;

   LCD12864_DATA_PORT=0xFF;
   LCD12864_RW=1;
   LCD12864_RS=0;
   LCD12864_EN=1;
   LCD12864_EN=1;
   LCD12864_EN=0;

   LCD12864_DATA_PORT=0xFF;		//读有效数据
   LCD12864_RW=1;
   LCD12864_RS=0;

   LCD12864_EN=1;
   do{
      flag=LCD12864_DATA_PORT;
	  LCD12864_DATA_PORT=0xFF;
   }while(!((flag&0x80)==0x80));

   LCD12864_EN=0;
}				

/*-------------------------------------------------------------*/
void WriteDatToLCD12864(unsigned char dat){
//   WaitLCD();
   LCD12864_RS=1;   //the data
   LCD12864_RW=0;   //write
   LCD12864_DATA_PORT=dat;
   LCD12864_EN=1;
   LCD12864_EN=0;
}

/*-------------------------------------------------------------*/
void WriteCmdToLCD12864(unsigned char cmd){
//   WaitLCD();
   LCD12864_RS=0;   //the command
   LCD12864_RW=0;	//write
   LCD12864_DATA_PORT=cmd;
   LCD12864_EN=1;
   LCD12864_EN=0;
}

/*-------------------------------------------------------------*/
unsigned char ReadDatFromLCD12864(void){
   unsigned char dat;
   WaitLCD();
   LCD12864_DATA_PORT=0xFF;		      //读空操作
   LCD12864_RS=1;    //the data
   LCD12864_RW=1;     //read
   LCD12864_EN=1;
   LCD12864_EN=1;
   LCD12864_EN=0;

   LCD12864_DATA_PORT=0xFF;			  //读有效数据
   LCD12864_RS=1;    
   LCD12864_RW=1;     
   LCD12864_EN=1;
   dat=LCD12864_DATA_PORT;
   LCD12864_EN=0;
   return dat;
}	

/*--------------------------------------------------------------*/
//from the chip manual
void LCD12864_init(void){
    LCD12864_RST=0;
	delayus(50);
	LCD12864_RST=1;
	LCDSel(0);       //left and right 炜煌选择为2 仿真选择为0
	WriteCmdToLCD12864(LCD_OFF);
	WriteCmdToLCD12864(LCD_ON);
}

/*------------------------------------------------------------*/
//x:0~7
void SetX(unsigned char x){
    WriteCmdToLCD12864(Add_X+x);
}

/*------------------------------------------------------------*/
//y:0~127
void SetY(unsigned char y){
    WriteCmdToLCD12864(Add_Y+y);
}	 

/*------------------------------------------------------------*/
//z:0~63
void SetZ(unsigned char z){
    WriteCmdToLCD12864(Add_Z+z);
}

/*------------------------------------------------------------*/
void ClearLCD(){
    int i,j;
    LCDSel(0);
	for(j=0;j<8;j++)	{
	   WriteCmdToLCD12864(LCD_ON);
       SetX(j);  
       WriteCmdToLCD12864(Add_Y);  //diffrent from SetY(0),SetY(64);     
	   SetZ(0);
	   for (i=0;i<64;i++)	   {
	      WriteDatToLCD12864(0x00);
	   }
	}	
}

/*------------------------------------------------------------*/
//左上角第一个点为原点，向下Y为轴，向右为X轴
//x:0~63~127    y:0~63
//flag :  0：擦除某个点
//        1：显示某个点 
unsigned char code Tab[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
void Dot(char x,char y,bit flag){
    unsigned char dat;
    if(x<64){
	    LCDSel(1);
		SetX(y/8);   //set the page address
		SetY(x);         //set the Y adress
		dat=ReadDatFromLCD12864();
		if(flag)
		  dat=dat|(Tab[y%8]);
		else
		  dat=dat&(~(Tab[y%8]));
		SetY(x);         //set the Y adress
		WriteDatToLCD12864(dat);
	}
	else if(x<128){
	    LCDSel(2);
		SetX(y/8);
		SetY(x-64);
		dat=ReadDatFromLCD12864();
		if(flag)
		  dat=dat|(Tab[y%8]);
		else
		  dat=dat&(~(Tab[y%8]));
		SetY(x-64);
		WriteDatToLCD12864(dat);
	}     
}
	   
/*------------------------------------------------------------*/
//draw a line between point(x1,y1) and point(x2,y2)
//flag   0:erase the line    1:draw a line
void Line(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2,bit flag){
    unsigned char i;
	unsigned char temp;
	float k;
	if(x1==x2){
	   if(y1>y2){
	   	   temp=x1;x1=x2;x2=temp;
    	   temp=y1;y1=y2;y2=temp;
		}
	   for(i=y1;i<=y2;i++){
	       Dot(x1,i,flag);
	   }
	}else{
    	if(x1>x2){
    	   temp=x1;x1=x2;x2=temp;
    	   temp=y1;y1=y2;y2=temp;
     	 }
         k=(float)(y2-y1)/(float)(x2-x1);
         for(i=0;i<x2-x1;i++){
           Dot(x1+i,(unsigned char)(y1+k*i),flag);
         }
	}
}	   

/*-------------------------------------------------------------*/
void Rect(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2,bit flag){
    Line(x1,y1,x2,y1,flag);
	Line(x2,y1,x2,y2,flag);
	Line(x2,y2,x1,y2,flag);
	Line(x1,y2,x1,y1,flag);
} 

/*-------------------------------------------------------------*/
//16*16
//x行  0~7      y列 0~127
//flag 0:汉字反白显示
//upline: 0表示有上划线
//underline: 0表示有下划线
void hz_disp(unsigned char x,unsigned char y,unsigned char n,unsigned char code * hz,bit flag,bit upline,bit underline){
    unsigned char i,j;
	for (j=0;j<n;j++){
	    //显示上半个汉字
	    for(i=0;i<16;i++){
		    //点的位置是在左还是右
			if(y+16*j+i<64){
			    LCDSel(1);
				WriteCmdToLCD12864(LCD_ON);
				SetX(x);
				SetZ(0);
				SetY(y+16*j+i);
				if(upline){
					if(flag)
				 	   WriteDatToLCD12864(hz[32*j+i]);
			    	else
				   	 WriteDatToLCD12864(~hz[32*j+i]);
				}
				else{
					if(flag)
				 	   WriteDatToLCD12864(hz[32*j+i]|UPLINE);
			    	else
				   	 WriteDatToLCD12864(~hz[32*j+i]|UPLINE);	
				}
			}
			else if(y+16*j+i<128){
			    LCDSel(2);
				WriteCmdToLCD12864(LCD_ON);
				SetX(x);
				SetZ(0);
				SetY(y+16*j+i-64);
				if(upline){
					if(flag)
				 	   WriteDatToLCD12864(hz[32*j+i]);
			    	else
				   	 WriteDatToLCD12864(~hz[32*j+i]);
				}
				else{
					if(flag)
				 	   WriteDatToLCD12864(hz[32*j+i]|UPLINE);
			    	else
				   	 WriteDatToLCD12864(~hz[32*j+i]|UPLINE);	
				}
			}
		}
		//显示下半个汉字
		for(i=16;i<32;i++){
		    //先判断点是在左还是在右
			if(y+16*j+i-16<64){
				if(x+1<8){	       //最后一行显示上半个字
			        LCDSel(1);
			    	WriteCmdToLCD12864(LCD_ON);
				    SetX(x+1);
				    SetZ(0);
				    SetY(y+16*j+i-16);
				if(underline){
					if(flag)
				 	   WriteDatToLCD12864(hz[32*j+i]);
			    	else
				   	 WriteDatToLCD12864(~hz[32*j+i]);
				}
				else{
					if(flag)
				 	   WriteDatToLCD12864(hz[32*j+i]|UNDERLINE);
			    	else
				   	 WriteDatToLCD12864(~hz[32*j+i]|UNDERLINE);	
				}
				}
			}else if(y+16*j+i-16<127){
			    if(x+1<8){ 	      //最后一行
			        LCDSel(2);
				    WriteCmdToLCD12864(LCD_ON);
				    SetX(x+1);
				    SetZ(0);
				    SetY(y+16*j+i-16-64);
				if(underline){
					if(flag)
				 	   WriteDatToLCD12864(hz[32*j+i]);
			    	else
				   	 WriteDatToLCD12864(~hz[32*j+i]);
				}
				else{
					if(flag)
				 	   WriteDatToLCD12864(hz[32*j+i]|UNDERLINE);
			    	else
				   	 WriteDatToLCD12864(~hz[32*j+i]|UNDERLINE);	
				}
				}
			}
		}
	}
}				  
/*-------------------------------------------------------------*/
//x:行0~7
//y:列0~127
//asc: 指向标准交换码
//string: 指向要显示的字符串
//flag:  0 反白显示
//online: 0 带上划线
//underline : 0带下划线
//n: the number of the string
void en_disp(unsigned char x,unsigned char y,unsigned char n,unsigned char code *asc,unsigned char *string,bit flag,bit online,bit underline){
    unsigned char i,j,loc;
	for (j=0;j<n;j++){
	    loc=string[j]-0x20;	    //确定要显示的字符在asc表中的位置（乘上16）
	    //显示上半个字母
	    for(i=0;i<8;i++){
		    //点的位置是在左还是右
			if(y+8*j+i<64){
			    LCDSel(1);
				WriteCmdToLCD12864(LCD_ON);
				SetX(x);
				SetZ(0);
				SetY(y+8*j+i);
				if(online){
					if(flag)
					    WriteDatToLCD12864(asc[16*loc+i]);
				    else
					    WriteDatToLCD12864(~asc[16*loc+i]);
				}
				else{
					if(flag)
					    WriteDatToLCD12864(asc[16*loc+i]|UPLINE);
				    else
					    WriteDatToLCD12864(~asc[16*loc+i]|UPLINE);
				}
			}
			else if(y+8*j+i<128){
			    LCDSel(2);
				WriteCmdToLCD12864(LCD_ON);
				SetX(x);
				SetZ(0);
				SetY(y+8*j+i-64);
				if(online){
					if(flag)
					    WriteDatToLCD12864(asc[16*loc+i]);
				    else
					    WriteDatToLCD12864(~asc[16*loc+i]);
				}
				else {
				  	if(flag)
					    WriteDatToLCD12864(asc[16*loc+i]|UPLINE);
				    else
					    WriteDatToLCD12864(~asc[16*loc+i]|UPLINE);
				}
			}
		}
		//显示下半个字母
		for(i=8;i<16;i++){
		    //先判断点是在左还是在右
			if(y+8*j+i-8<64){  
			    if(x+1<8){	//最后一行
			       LCDSel(1);
				   WriteCmdToLCD12864(LCD_ON);
				   SetX(x+1);
				   SetZ(0);
				   SetY(y+8*j+i-8);
	 				if(underline){
						if(flag)
						    WriteDatToLCD12864(asc[16*loc+i]);
					    else
						    WriteDatToLCD12864(~asc[16*loc+i]);
					}
					else{
					  	if(flag)
						    WriteDatToLCD12864(asc[16*loc+i]|UNDERLINE);
					    else
						    WriteDatToLCD12864(~asc[16*loc+i]|UNDERLINE);
					}

				}
			}
			else if(y+8*j+i-8<128){
			    if(x+1<8){ 	      //最后一行
			        LCDSel(2);
				    WriteCmdToLCD12864(LCD_ON);
				    SetX(x+1);
				    SetZ(0);
				    SetY(y+8*j+i-8-64);
					if(underline){
						if(flag)
						    WriteDatToLCD12864(asc[16*loc+i]);
					    else
						    WriteDatToLCD12864(~asc[16*loc+i]);
					}
					else{
					  	if(flag)
						    WriteDatToLCD12864(asc[16*loc+i]|UNDERLINE);
					    else
						    WriteDatToLCD12864(~asc[16*loc+i]|UNDERLINE);
					}
				}
			}
		}
	}  
}


//亦木明 2008.2.23
//显示一个16X32的字符
void Show16X32(unsigned char line,unsigned char column, unsigned char code *pt,bit flag){
	unsigned char i,column_temp1,column_temp2,column_temp3;
	column_temp1=column;
	column_temp2=column;
	column_temp3=column;
	LCDSel(1);                         //选通左屏
	SetY(column);                      //设置起始列
	SetX(line);                          //设置行
	for(i=0;i<16;i++){
		if(column>=64){                      //如果列>=64，则选通右屏
			LCDSel(2);
			column=column%64;               //设置右屏起始列
			SetY(column);
			SetX(line);
		}
		if(flag)	
		WriteDatToLCD12864(*pt);                     //写数据
		else
		WriteDatToLCD12864(~(*pt)); 
		*pt++;
		column++;
	}
	LCDSel(1);
	SetX(line+1);
	column=column_temp1;
	SetY(column);
	for(i=0;i<16;i++){
		if(column>=64){
			LCDSel(2);
			column=column%64;
			SetY(column);
			SetX(line+1);
		}	
		if(flag)	
		WriteDatToLCD12864(*pt);                     //写数据
		else
		WriteDatToLCD12864(~(*pt)); 
		*pt++;
		column++;
	}
	LCDSel(1);
	SetX(line+2);
	column=column_temp2;
	SetY(column);
	for(i=0;i<16;i++){
		if(column>=64){
			LCDSel(2);
			column=column%64;
			SetY(column);
			SetX(line+2);
		}	
		if(flag)	
		WriteDatToLCD12864(*pt);                     //写数据
		else
		WriteDatToLCD12864(~(*pt)); 
		*pt++;
		column++;
	}
	LCDSel(1);
	SetX(line+3);
	column=column_temp3;
	SetY(column);
	for(i=0;i<16;i++){
		if(column>=64){
			LCDSel(2);
			column=column%64;
			SetY(column);
			SetX(line+3);
		}	
		if(flag)	
		WriteDatToLCD12864(*pt);                     //写数据
		else
		WriteDatToLCD12864(~(*pt)); 
		*pt++;
		column++;
	}
}
/*-------------------------------------------------------*/