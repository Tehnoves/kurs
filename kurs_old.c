//*********************************************************************************
//																						
//	  Программа Kurs измеритель скорости звука версия с большими цифрами 
//    
//	  V1.0 06.07.15  start
//	       07.07.15 09:30 форматный вывод пошел индикатор
//	                12:00 вывод готов старт даллас
//	  				14:00 отладка даллас
//	  				16:30  The End
//	  V1.1	08.07.15 22:00		CRC ???    Энергосбережение *********
//			10.07.15  10:10 сборка опытной платы	
//	                  19:00 The End.......
//    V2.0  большие цифры 19.07.15
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


#include "compiler_defs.h"
#include "C8051F410_defs.h"
											//#include <c8051f410.h>                 // SFR declarations
#include <intrins.h> 
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "F350_FlashPrimitives.h"

#define  LCD_WC          (0x00)
#define  LCD_ADDR        (0x78)                 // Device addresses (7 bits, lsb is a don't care)
#define  WRITE           0x00                 	// SMBus WRITE command
#define  READ            0x01                 	// SMBus READ command
#define  SYSCLK          24500000     			// SYSCLK frequency in Hz
#define  LCD_WD          (0x40)
#define  Normal_font 1
#define  Double_font 2
#define  Triple_font 3 
#define  SMB_MTSTA      0xE0                 // (MT) start transmitted
#define  SMB_MTDB       0xC0                 // (MT) data byte transmitted
#define  SMB_MRDB       0x80                 // (MR) data byte received

//#define f109
#undef f109	
#define f10
//#undef f10
//#define test
#undef test	

	bit SMB_BUSY = 0;                           // Software flag to indicate when the
	bit SMB_SINGL_SENDCON;
 	bit SMB_SENDCON; 
 	bit SMB_SINGL_BYTE;
 	bit SMB_RW;                                  // Software flag to indicate the
                                            	 // direction of the current transfer
 	bit SMB_ACKPOLL;   
	bit flag_int0,flag_ta,flag;
	sbit LCD_RST =P1^5;  // P0^2
	sbit t_1=P2^2;  // P2^2;    P1^0;
	
	
	unsigned  char sa,a;
	union  Crr2
		{
		   int Int;
		   char Char[2];
		   
		};
	unsigned int xdata ta;	
	union Crr2 xdata tt;
	unsigned char* pSMB_DATA_IN;  
	unsigned char* pSMB_DATA_OUT;                // Global pointer for SMBus data.
            	                                 // All transmit data is read from here

	unsigned char SMB_DATA_LEN;                  // Global holder for number of bytes
    	                                          // to send or receive in the current
        	                                      // SMBus transfer
 	unsigned char xdata LCD_out_buf[12],x,y;
	unsigned char LCD_CON;
	unsigned char TARGET,len;   	// Target SMBus slave address
	unsigned char xdata buf3[30];
	unsigned char xdata buf[30];
	float velos,temper;
	unsigned char xdata crc8,crc8_ok,cr,os,bb,bb1;
	unsigned char sr[9];
	float xdata ftemper;
	char xdata i_tt;
	char xdata str[4];
	
	extern  const unsigned char code TABL [155][6];
						//char code init_lcd[12] = {0x01,0x20,0x09,0x0c,0x05,0x13,0x01,0x0a,0x0b,0x20,0x05,0xc4};
  char code init_lcd[12] = {0x01,  //  page 00
  							0x20,  //   vert,power-down
							0x09,  //  page 01
							0x0c,  //  normal mode
							0x05,  //  mux 1/34
							0x13,  //  BIAS
							0x01,  //  page 00
							0x0a,  //  page 10
							0x0b,  //  volt multipl
							0x20,  //  K of temper
							0x05,  //  VLCD LOW
							0xc4}; //  VLCD =
							
	const unsigned char code row0[] = {' ',' ','1','2','3','.','5','6','7',0};	
	const unsigned char code row1[] = {' ',' ','V','=','m','/','c','e','k',0};	
	const unsigned char code row2[] = {154,'e',151,153,'e','p','a',154,'y','p','.',0};		//
	const unsigned char code row3[] = {' ','t','=','3','4','.','6','7',179,'C',' ',' ',0};	
	const unsigned char code row4[] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0};	
	void SMBus_ISR  (void);	
	bit reset_1wire(void);
	void skip_rom(void);
	void OW_write_byte (unsigned char write_data);
	void delay(char op);
	void get_temperature(void);
	void OW_write_byte (unsigned char write_data);
	void OW_write_byte (unsigned char write_data);		
	void delay0(void);
	void OW_write_bit (unsigned char write_bit);
	void vivod_0(unsigned char x,unsigned char y,unsigned char index);
	const union Crr code BIG_TABL [12][12];
	
	union  Crr
{
   unsigned int Int;
   unsigned char Char[2];
   
};

 const unsigned char code dscrc_table[] = {
0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
157,195, 33,127,252,162, 64, 30, 95, 1,227,189, 62, 96,130,220,
35,125,159,193, 66, 28,254,160,225,191, 93, 3,128,222, 60, 98,
190,224, 2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89, 7,
219,133,103, 57,186,228, 6, 88, 25, 71,165,251,120, 38,196,154,
101, 59,217,135, 4, 90,184,230,167,249, 27, 69,198,152,122, 36,
248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91, 5,231,185,
140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
202,148,118, 40,171,245, 23, 73, 8, 86,180,234,105, 55,213,139,
87, 9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};




 unsigned char docrc8(unsigned char value)
	{
	  crc8 = dscrc_table[crc8 ^ value];
	  return crc8;
	}


//******************************
//
//   
//
//******************************




 void delay100(void)
   { unsigned char i=200;
     while(--i!=0)
     {delay(250);}
   }


void Timer_Init()
{
	TCON      = 0x01;
    TMOD      = 0x02;
    CKCON     = 0x04;
												   // TH0       = 0xAE;
													
													/*
													  TMOD      = 0x02;  
													CKCON     = 0x04;*/
#ifdef f109		
    TH0       = 0xFd; // 191406
#else  	
	TH0       = 0xec;
#endif		
	
	TR0 = 1;  
	  
}

void SMBus_Init()
{
    SMB0CF    = 0xc0;
}
	void Port_IO_Init()
{
    // P0.0  -  SDA (SMBus), Open-Drain, Digital
    // P0.1  -  SCL (SMBus), Open-Drain, Digital
    // P0.2  -  Unassigned,  Open-Drain, Digital
    // P0.3  -  Unassigned,  Open-Drain, Digital
    // P0.4  -  Unassigned,  Open-Drain, Digital
    // P0.5  -  Unassigned,  Open-Drain, Digital
    // P0.6  -  Unassigned,  Open-Drain, Digital
    // P0.7  -  Unassigned,  Open-Drain, Digital

    // P1.0  -  Unassigned,  Open-Drain, Digital
    // P1.1  -  Unassigned,  Open-Drain, Digital
    // P1.2  -  Unassigned,  Open-Drain, Digital
    // P1.3  -  Unassigned,  Open-Drain, Digital
    // P1.4  -  Unassigned,  Open-Drain, Digital
    // P1.5  -  Unassigned,  Open-Drain, Digital
    // P1.6  -  Unassigned,  Open-Drain, Digital
    // P1.7  -  Unassigned,  Open-Drain, Digital

    // P2.0  -  Unassigned,  Open-Drain, Digital
    // P2.1  -  Unassigned,  Open-Drain, Digital
    // P2.2  -  Unassigned,  Open-Drain, Digital
    // P2.3  -  Unassigned,  Open-Drain, Digital
    // P2.4  -  Unassigned,  Open-Drain, Digital
    // P2.5  -  Unassigned,  Open-Drain, Digital
    // P2.6  -  Unassigned,  Open-Drain, Digital
    // P2.7  -  Unassigned,  Open-Drain, Digital

P0MDOUT   = 0x04;
    P1MDOUT   = 0x20;
    XBR0      = 0x04;
    XBR1      = 0xC0;
	/*//
	P0MDOUT   = 0x06;
    XBR0      = 0x04;
    XBR1      = 0xC0; */
	/*
	    P0MDOUT   = 0x84;
    P1MDOUT   = 0x20;
    P0SKIP    = 0x7C;
    P1SKIP    = 0xDF;
    P2SKIP    = 0x7B;
    XBR0      = 0x04;
    XBR1      = 0xC0;  */

	
	
	
}

void Oscillator_Init()
{
#ifdef f109	 	
      // 191406
#else  
	OSCICN    = 0x87;  //2450000 
	sa= OSCICN;
	os = CLKMUL;
	//os1 =
#endif  
}
 
 

void Interrupts_Init()
{


	EIE1      = 0x01;
    IT01CF    = 0x07;
    IE        = 0x81;
}

// Initialization function for device,
// Call Init_Device() from your main program
void Init_Device(void)
{
	 Timer_Init();
    SMBus_Init();
    Port_IO_Init();
    Oscillator_Init();
    Interrupts_Init();
	flag_int0 = 0;
}
						
							
	 void SMBUS_Write_Arrey( unsigned char Len,unsigned char* dat)
			{
			   while (SMB_BUSY);                         // Wait for SMBus to be free.
			   SMB_BUSY = 1;                             // Claim SMBus (set to busy)

			   // Set SMBus ISR parameters
			   TARGET = LCD_ADDR;                     // Set target slave address
			   SMB_RW = WRITE;                           // Mark next transfer as a write
			   SMB_ACKPOLL = 1;                          // Enable Acknowledge Polling (The ISR
														 // will automatically restart the 
														 // transfer if the slave does not 
														 // acknowledge its address.

			   pSMB_DATA_OUT = dat;     // The outgoing data pointer points to
														 // the <dat> variable.

			   SMB_DATA_LEN = Len;

			   // Initiate SMBus Transfer
			   STA = 1;
			}						
							
							
	void LCD_Write_Arrey(unsigned char con,bit cycle_con ,unsigned char len,unsigned char* dat)
{
	while (SMB_BUSY);                         // Wait for SMBus to be free.
	LCD_CON = con;
	SMB_SENDCON=1;
	SMB_SINGL_SENDCON=cycle_con; 
	SMBUS_Write_Arrey(len,dat);
}						
							
							
	 void LCD_SET_XY(unsigned char X,unsigned char Y)
{
	while (SMB_BUSY);
	LCD_out_buf[0]=0x01;
	LCD_out_buf[1]=0x40|Y;
	LCD_out_buf[2]=0x80|X;
	LCD_Write_Arrey(LCD_WC,1,3,&LCD_out_buf);
}						
							
							
	 void LCD_CLR(unsigned char row)
{
    LCD_SET_XY(0,row);
	while (SMB_BUSY);                         // Wait for SMBus to be free.
	SMB_SINGL_BYTE=1;
	LCD_out_buf[0]=0x00;
	LCD_Write_Arrey(LCD_WD,0,133,&LCD_out_buf);
    LCD_SET_XY(0,row);
}

 void LCD_init(void)
{  char i=0;
   LCD_RST=1;
   LCD_RST=0;
   while(++i);
   LCD_RST=1;
  	   	 

   LCD_Write_Arrey(LCD_WC,1,12,&init_lcd);
   	    
}

void ochistka_ekrana(void)
{ LCD_CLR(0);
     LCD_CLR(1);
	 LCD_CLR(2);
	 LCD_CLR(3);
	 

}
 void LCD_print(unsigned char Line,unsigned char  Pos, const unsigned char *str,unsigned char Font,unsigned char Invers)
{   char i;
    LCD_SET_XY(Pos,Line);
	if(Font ==Normal_font)//Normal_font)
	{           
		while(*str)
		{
				while (SMB_BUSY);
				for(i=0;i<6;i++)
				{
				   if(Invers) LCD_out_buf[i]=~(TABL[*str-0x20][i]);
                   else LCD_out_buf[i]=TABL[*str-0x20][i];
				}
				*str++; 
			LCD_Write_Arrey(LCD_WD,0,6,&LCD_out_buf);
			while (SMB_BUSY);
		}
	} else if(Font ==Double_font)
	{
	 	while(*str)
		{
			switch(*str++)
			{
			 	case '0': 
				{		 while (SMB_BUSY);
						 LCD_out_buf[0]=0xFC; LCD_out_buf[1]=0xFC; LCD_out_buf[2]=0x03;
						 LCD_out_buf[3]=0x03; LCD_out_buf[4]=0xC3; LCD_out_buf[5]=0xC3;							 
						 LCD_out_buf[6]=0x33; LCD_out_buf[7]=0x33; LCD_out_buf[8]=0xFC;
						 LCD_out_buf[9]=0xFC; LCD_out_buf[10]=0; LCD_out_buf[11]=0;
						 LCD_Write_Arrey(LCD_WD,0,12,&LCD_out_buf);
						 while (SMB_BUSY);
						 LCD_SET_XY(Pos,++Line);
						 while (SMB_BUSY);
						 LCD_out_buf[0]=0x0f; LCD_out_buf[1]=0x0F; LCD_out_buf[2]=0x33;
						 LCD_out_buf[3]=0x33; LCD_out_buf[4]=0x30; LCD_out_buf[5]=0x30;							 
						 LCD_out_buf[6]=0x30; LCD_out_buf[7]=0x30; LCD_out_buf[8]=0x0F;
						 LCD_out_buf[9]=0x0F; LCD_out_buf[10]=0; LCD_out_buf[11]=0;
						 LCD_Write_Arrey(LCD_WD,0,12,&LCD_out_buf);
				break;}
			}
		}	  
	}
}		


//******************************
//
//   
//
//******************************


void delay1(int i)
{
	int j,k;
	for (k=1;k<i;k++)
	for (j=0;j<10000;j++);
} 
void convert(void)
{
//; Выдаем команду "Convert".
//		while (1)
		{
		//while (1)
			OSCICN &= 0xf8;
		while ((OSCICN & 0x40) != 0x40);
											//OSCICN |= 0x7;
											//OSCICN &= 0xf8;	
        reset_1wire(); 
											//OSCICN &= 0xf8;
			OSCICN |= 0x7;
		while ((OSCICN & 0x40) != 0x40);	
										//OSCICN &= 0xf8;
									//while (1)
			delay(2);	
        skip_rom();
		OW_write_byte(0x44);
     //   w1_io(0x44);
		}
        crc8=0;
		} 									/*
											  void int2asci(char integer)

											{
											 unsigned int celoe;

											 if (integer<0)
												{str[0]=0x0a;
												 integer =integer *(-1); }
											 else
												{ str[0]=0x0b;}

											 celoe = integer/10;
											 str[1]=celoe;
											 str[2] = (integer - celoe*10);
											} */
static void calk_temper(void)  // функция вычисления температуры
{
   int  r1,r2;

   if (tt.Char[0]>>4 == 0x0f)
       {
        ftemper = 0xffff - tt.Int;;
        ftemper = -ftemper/16;
       
       }
   else
       {
										  //  ftemper = fTEMPER_H*256+fTEMPER_L;
										//	tt.Char[0] = fTEMPER_H;
										//	tt.Char[1] = fTEMPER_L;
		
											//  ftemper = ftemper/16;
		ftemper = tt.Int;
		 ftemper = ftemper/16;
											//	 i_tt= round(ftemper);
      
       }
	 
        r1 = (int)floor(ftemper);
        r2 = (ftemper - r1)*10;
        if (r2 > 4)
    	    i_tt = (r1+1);
        else
	        i_tt = (r1);


}



 void temper1(void)
	{
	unsigned char d;
	  convert();
	   
	  OSCICN &= 0xf8;
	  while ((OSCICN & 0x40) != 0x40);  
									//delay(150);

	#ifndef f10  
	  for (d=0;d<55;d++)   //35
		delay100();
	#else  	
		delay(5);
		delay(6);

	#endif	

		OSCICN |= 0x7;
	while ((OSCICN & 0x40) != 0x40); 
		get_temperature();
	    calk_temper();
		temper=ftemper;
								  //int2asci(i_tt);
								 // calk_temper();
								 // int2asci(ftemper);
								 // regen[0]=str[0];
								 // regen[1]=str[1];
								 //regen[2]=str[2];
	}

float kk,ll,mm;
/////!!!!unsigned int tmp,tmp2,tmp3;
	void main(void)
	{
			PCA0MD &= ~0x40;
									//OSCICN &= 0xf8;
									//OSCICN |= 0x7;
									//OSCICN &= 0xf8;
									/*
		mm = 123.456;
		kk = modf(mm,&ll);
		tmp = (int)ll;
		tmp2 = (int)(kk*1000);
		
				tmp3 = (tmp2 % 10 );  // младший
				tmp3 = (tmp2 % 100/10) ;
				tmp3 = (tmp2 % 1000/100) ; // старший
				*/
			PCON = 0x0;
		_nop_();  // 5ucek 191408
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		ta = 0;
		flag = 0;
		flag_ta = 0;
		bb= 0x20;
		bb1=0x24;
			velos = 123.456;
			temper = 25.4;
	//*****************************	
			Init_Device();
			/*
			 RTC0KEY = 0xa5;		  // открываем RTClock 1 байт
			a = RTC0KEY;
			RTC0KEY = 0xf1;		  // открываем RTClock 2 байт
			a = RTC0KEY;
			// открыли и запустили таймер
				RTC0ADR = 0x07;					// RTC0XCN
				RTC0ADR=0x00;					// AGCEN+Crystal Mode
				while((RTC0ADR & 0x80) == 0x80);
				*/

				//TC0ADR = 0x06;					// RTC0CN
				//TC0DAT=0x90;					// Clock Enable+Timer Run
				//while((RTC0ADR & 0x80) == 0x80);
			
			
			
			
			
	//***********************		
			/*
						 OSCICN &= 0xf8;
					while ((OSCICN & 0x40) != 0x40);  
					 OSCICN    = 0x80;//
					sa = 0xff;
					OSCICN &= ~(1<< 5);
					
					
					PCON |= 0x01;
					PCON=PCON;
					_nop_();
					_nop_();
					_nop_();
					_nop_();
			*/
			//	PCON = 0x01;
			//sa= PCON ;
		//	while (1);
													/*
													while (1)
													{	
														OSCICN &= 0xf8;
														while ((OSCICN & 0x40) != 0x40);
														///delay0();
														reset_1wire();	
														OSCICN |= 0x7;
														while ((OSCICN & 0x40) != 0x40);
														//delay0();
													}*/
											
													//delay(0);
												//delay(2); //
	//		while (1)
	//			temper1();
			 LCD_init();
			   ochistka_ekrana();
			   LCD_print(0,0, &row1,1,0); //
			// LCD_print(2,0, &row2,1,0); 
			  
		while (1)	  
		/*	  
		while (1)
		{
			y = 1;
		x =2;
		
	    vivod_0(2,1,((int)ll % 1000/100));
		vivod_0(14,1,((int)ll % 100/10));
		vivod_0(26,1,((int)ll % 10));
		x+=12;
		vivod_0(x,y,4);	
		x+=12;
		vivod_0(x,y,2);	
			sprintf(buf3,".%3u",321);
			//strcpy(buf+2,buf3);
												//LCD_print(0,0, &row1,1,0); //
			LCD_print(2,38, &buf3,1,0); 
		}
		*/
		{
												//sprintf(regen,"%0.5u",vesi);
												//strncat
												//strncpy
			strcpy(buf,row0);	
			if ((flag_int0==0) & (!flag_ta))
				{
					LCD_Write_Arrey(LCD_WC,1,1,&bb);
					flag_ta = 1;
				}
			else if ((flag_int0==1) | (ta > 22))
				{
					LCD_Write_Arrey(LCD_WC,1,1,&bb1);
					flag_ta  = 0;
					ta=0; //
					flag= 1;
					flag_int0 = 1;
				}
			_nop_();
			_nop_();
			velos = 20.0457*sqrt(273.15+temper);//
//velos=342.001;
			
			kk = modf(velos,&ll);
			vivod_0(2,1,((int)ll % 1000/100));
		vivod_0(14,1,((int)ll % 100/10));
		vivod_0(26,1,((int)ll % 10));
		sprintf(buf3,".%-3u",(int)(kk*1000));//
			
			LCD_print(1,38, &buf3,1,0); 
					///!!kk = modf(velos,&ll);
					/*
					mm = 123.456;
		kk = modf(mm,&ll);
		tmp = (int)ll;
		tmp2 = (int)(kk*1000);
		
				tmp3 = (tmp2 % 10 );  // младший
				tmp3 = (tmp2 % 100/10) ;
				tmp3 = (tmp2 % 1000/100) ; // старший
					*/
					
			if ((flag_int0==1)| flag)
				{
					flag = 0;
					SCON0   &= ~0x10;                   // Disable UART
					TCON    &= ~0x10;  
					
						 OSCICN &= 0xf8;
					while ((OSCICN & 0x40) != 0x40);  
					 OSCICN    = 0x80;//
					sa = 0xff;
					OSCICN &= ~(1<< 5);
					 SMB0CF  &= ~0x80; 
					
					PCON |= 0x02;
					PCON=PCON;
					_nop_();
					_nop_();
					_nop_();
					_nop_();
					 SMB0CF  |= 0x80; 
					 TCON    |= 0x10;                    // Enable T0//
					
				}									// LCD_Write_Arrey(LCD_WC,1,1,&bb);
				
	
		//	sprintf(buf3,"%6.3f",velos);
		//	strcpy(buf+2,buf3);
												//LCD_print(0,0, &row1,1,0); //
		//	LCD_print(1,0, &buf,1,0); 
												//LCD_print(2,0, &row2,1,0); 
		
#ifdef test	
#else   
			temper1();
#endif	  			
	
			strcpy(buf,row3);
			sprintf(buf3,"%3.1f",temper);	//
											//	len = strlen(buf3);
			strcpy(buf+3,buf3);//
											//		memcpy(buf+3,buf3,len);	//
			len = strlen(buf);
			strcat(buf,row3+8);
			LCD_print(3,0, &buf,1,0); 
			ta++;
#ifdef test	
			temper=temper+.1;
			if (temper > 70.0)
				temper = -30.0;
#else  
#endif		


//delay(150);
	 OSCICN &= 0xf8;
	while ((OSCICN & 0x40) != 0x40);  
#ifndef f10  
	delay1(100);
#else  	
	delay1(2);

#endif	
		OSCICN |= 0x7;
		while ((OSCICN & 0x40) != 0x40);		
													/*
											#ifdef f109
														delay1(1);
											#else  
														delay1(100);
											#endif				
												*/		
							   
		}	   
	}
	
	
	
void delay0(void)
			{
				
			}	



void irq0_int(void) interrupt  INTERRUPT_INT0 
	 	{
			//unsigned char i;
			 if (IT01CF    == 0x0F)   // 1
				 {
					IT01CF    = 0x07;  // 0
					flag_int0 = ~flag_int0;
				 } 
			 else if (IT01CF    == 0x07) 
				 {
					IT01CF    = 0x0F; 
				   
												//flag_int0 = 0;
				 }	
										//  IE0 = 0;                        // Clear the SPIF flag
										  //flag_int0 = 1;
		}
		
//******************************
//
//   
//
//******************************



void delay(char op)
{
	unsigned char q;
	for (q=op;q>0;q--);


}	
//***********************************
//
//
//
//***********************************



bit reset_1wire(void)
{
  bit err;
 // EA = 0;
  t_1  = 0;
					// 480usec

#ifndef f10	 
   delay(100);
   
   delay(100);
   delay(100);
   delay(100);
   delay(100);
   delay(100);
   delay(100);
   delay(100);
   delay(100);
   delay(100);
   delay(75);
	
#else  
	
	delay(6);	
	delay(0);

#endif	   
    //delay(200); 
	  t_1= 1;

						// 120usec
#ifndef f10							
  delay(100);
  delay(100);
 #else   
	delay0();
	delay(0);
		
 #endif	 
																 // t_1wread = 1;
																  //t_1wone = 0;
																 // t_1wone = 1;
																  //delay(100);
																  // delay(35);
																 //   delay(235);
	 
  err = t_1;
// EA = 1;
						// 360usec
						
#ifndef f10							
	delay(100);
	delay(100);
	delay(100);
	delay(100);
	delay(100);
	delay(100);
	delay(100);
	delay(80); 
#else  	
	
	delay(5);
//#endif	  
  return err;      //| !t_1
}	

 void OW_write_bit (unsigned char write_bit)
{
	if (write_bit)
	{
										//writing a bit '1'  ~~~~~     5usec    (55usec)
		t_1 = 0;	
#ifndef f109										//	drive_OW_low(); 				// Drive the bus low
		delay(25);
#else  	
		_nop_();
#endif										//wait(DELAY_6Us);				// delay 6 microsecond (us)
		t_1 = 1;	
#ifndef f109			
		delay(150);
#else  		
		delay0();
		_nop_();
		_nop_();
#endif										//drive_OW_high ();  				// Release the bus
										//wait(DELAY_64Us);				// delay 64 microsecond (us)
	}
	else
	{
										//writing a bit '0'    ~~~~~~ (60)     (5)
		t_1 = 0;	
#ifndef f109											//	drive_OW_low(); 				// Drive the bus low
		delay(150);
#else  
		delay0();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
#endif											//wait(DELAY_60Us);				// delay 60 microsecond (us)
		t_1 = 1;	
#ifdef f109											//drive_OW_high ();  				// Release the bus

		_nop_();
		
#else  		
		delay(25);
	
#endif											//wait(DELAY_10Us);				// delay 10 microsecond for recovery (us)
	}
}


void OW_write_byte (unsigned char write_data)
{
	unsigned char loop;
	
	for (loop = 0; loop < 8; loop++)
	{
		OW_write_bit(write_data & 0x01); 	//Sending LS-bit first
		write_data >>= 1;					// shift the data byte for the next bit to send
	}	
}
unsigned char OW_read_bit (void)
{
	unsigned char read_data; 
	//reading a bit          5     10    45
	t_1 = 0;	
#ifndef f109											//	drive_OW_low(); 						// Drive the bus low
	delay(15);
#else  	
	_nop_();
#endif											//wait(DELAY_6Us);						// delay 6 microsecond (us)
	t_1 = 1;
#ifndef f109											//drive_OW_high ();  						// Release the bus
	delay(20);
#else  	
	_nop_();
	_nop_();
#endif											//wait(DELAY_9Us);						// delay 9 microsecond (us)

	read_data = t_1;					//Read the status of OW_PIN
#ifndef f109		
	delay(130);
#else  	
	delay0();
#endif	
										//wait(DELAY_55Us);						// delay 55 microsecond (us)	
	return read_data;
}
 //***********************************
//
//
//
//***********************************


void skip_rom(void)
{
OW_write_byte(0xcc);
  //w1_io(0xCC);
  crc8=0;
}

 unsigned char OW_read_byte (void)
{
	unsigned char loop, result=0;
	
	for (loop = 0; loop < 8; loop++)
	{
		
		result >>= 1; 				// shift the result to get it ready for the next bit to receive
		if (OW_read_bit())
		result |= 0x80;				// if result is one, then set MS-bit
	}
	return result;					
}	

 //***********************************
//
//
//
//***********************************



void get_temperature(void)
{
        crc8=0;
        crc8_ok=0;
 //; Сбрасываем все приборы.
	OSCICN &= 0xf8;
	while ((OSCICN & 0x40) != 0x40);
										//OSCICN |= 0x7;
										//OSCICN &= 0xf8;
        reset_1wire();
										//OSCICN &= 0xf8;
	OSCICN |= 0x7;
	while ((OSCICN & 0x40) != 0x40);//

	//OSCICN &= 0xf8;		
        skip_rom();
//; Выводим команду транспортного уровня "Чтение блокнотной памяти".
		OW_write_byte(0xbe);
        /////w1_io(0xbe);
//; Считываем данные и вычисляем CRC.        mov      CRC,#0
       // docrc8(tt.Char[1]=w1_io(0xff));// byte 0.; Младший байт температуры.
		 docrc8(tt.Char[1]= OW_read_byte());// byte 0.; Младший байт температуры.
		sr[0] = tt.Char[1];
        docrc8(tt.Char[0]=OW_read_byte());// byte 1.; Старший байт температуры.
		sr[1] = tt.Char[0];

//; Далее 4 ненужных байта (читаем только из-за CRC).
		sr[2] =OW_read_byte(); 
        docrc8(sr[2]);// byte 2.
		sr[3] =OW_read_byte(); 
        docrc8(sr[3]);// byte 2.
        //docrc8(w1_io(0xff));// byte 3.
			sr[4] =OW_read_byte(); 
        docrc8(sr[4]);// byte 2.
        //docrc8(w1_io(0xff));// byte 4.
			sr[5] =OW_read_byte(); 
        docrc8(sr[5]);// byte 2.
        //docrc8(w1_io(0xff));// byte 5
//; Далее данные, необходимые для более точного вычисления температуры.
			sr[6] =OW_read_byte(); 
        docrc8(sr[6]);// byte 2.
//        docrc8(fCNT_REM  =w1_io(0xff));// byte 6.
	sr[7] =OW_read_byte(); 
        docrc8(sr[7]);// byte 2.
        //docrc8(fCNT_PER_C=w1_io(0xff));// byte 7.
	sr[8] =OW_read_byte(); 
       // docrc8(sr[8]);// byte 2.
		//cr = w1_io(0xff);
//; Читаем CRC.
        if(crc8 == cr) // byte 8 (CRC).
        crc8_ok=1;

}	
//------------------------------------------------------------------------------------
// SMBus Interrupt Service Routine (ISR)
//------------------------------------------------------------------------------------
//
// SMBus ISR state machine
// - Master only implementation - no slave or arbitration states defined
// - All incoming data is written starting at the global pointer <pSMB_DATA_IN>
// - All outgoing data is read from the global pointer <pSMB_DATA_OUT>
//
void SMBus_ISR (void) interrupt 7
{
   bit FAIL = 0;                             // Used by the ISR to flag failed
                                             // transfers

   static unsigned char i;                            // Used by the ISR to count the
                                             // number of data bytes sent or
                                             // received

   static bit SEND_START = 0;                // Send a start

   switch (SMB0CN & 0xF0)                    // Status vector
   {
      // Master Transmitter/Receiver: START condition transmitted.
      case SMB_MTSTA:
         SMB0DAT = TARGET;                   // Load address of the target slave
         SMB0DAT |= SMB_RW;                  // Load R/W bit
         STA = 0;                            // Manually clear START bit
         i = 0;                              // reset data byte counter
         break;

      // Master Transmitter: Data byte (or Slave Address) transmitted
      case SMB_MTDB:
         if (ACK)                            // Slave Address or Data Byte 
         {                                   // Acknowledged?
            if (SEND_START)
            {
               STA = 1;
               SEND_START = 0;
               break;
            }
            if(SMB_SENDCON)                  // Are we sending the control byte?
            {
               SMB_SENDCON = 0;              // Clear flag
               SMB0DAT = LCD_CON;            // send control byte
               break;
             }

            if (SMB_RW==WRITE)               // Is this transfer a WRITE?
            {

               if (i < SMB_DATA_LEN)         // Is there data to send?
               {
			      if(SMB_SINGL_BYTE) SMB0DAT = pSMB_DATA_OUT[0];

				  else	SMB0DAT = pSMB_DATA_OUT[i]; // send data byte
                  i++; 
			//	  if(SMB_SINGL_SENDCON)SMB_SENDCON = 1;              
               }
               else
               {
                 STO = 1;                    // set STO to terminte transfer
                 SMB_BUSY = 0;               // clear software busy flag
				 SMB_SINGL_BYTE=0;
				 SMB_SENDCON = 0;              
               }
            }
            else {}                          // If this transfer is a READ,
                                             // then take no action. Slave
                                             // address was transmitted. A
                                             // separate 'case' is defined
                                             // for data byte recieved.
         }
         else                                // If slave NACK,
         {
            if(SMB_ACKPOLL)
            {
               STA = 1;                      // Restart transfer
            }
            else
            {
               FAIL = 1;                     // Indicate failed transfer
            }                                // and handle at end of ISR
         }
         break;

      // Master Receiver: byte received
      case SMB_MRDB:
         if ( i < SMB_DATA_LEN )             // Is there any data remaining?
         {
            *pSMB_DATA_IN = SMB0DAT;         // Store received byte
            pSMB_DATA_IN++;                  // Increment data in pointer
            i++;                             // Increment number of bytes received
            ACK = 1;                         // Set ACK bit (may be cleared later
                                             // in the code)

         }

         if (i == SMB_DATA_LEN)              // This is the last byte
         {
            SMB_BUSY = 0;                    // Free SMBus interface
            ACK = 0;                         // Send NACK to indicate last byte
                                             // of this transfer
            STO = 1;                         // Send STOP to terminate transfer
		 }

         break;

      default:
         FAIL = 1;                           // Indicate failed transfer
                                             // and handle at end of ISR
         break;
   }

   if (FAIL)                                 // If the transfer failed,
   {
      SMB0CN &= ~0x40;                       // Reset communication
      SMB0CN |= 0x40;
      SMB_BUSY = 0;                          // Free SMBus
   }

   SI=0;                                     // clear interrupt flag
}	

	const unsigned char code TABL [155][6]=
                {{ 0x00,0x000,0x000,0x000,0x00, 00},
                  {0x00,0x000,0x04f,0x000,0x00, 00},  //;!
                  {0x00,0x007,0x000,0x007,0x00, 00},  //;"
                  {0x14,0x07f,0x014,0x07f,0x14, 00},  //;#
                  {0x24,0x02A,0x07F,0x02A,0x12, 00},  //;$
                  {0x23,0x013,0x008,0x064,0x62, 00},  //;%
                  {0x36,0x049,0x055,0x022,0x40, 00},  //;&
                  {0x00,0x005,0x003,0x000,0x00, 00},  //;'								 
                  {0x00,0x01c,0x022,0x041,0x00, 00},  //;(
                  {0x00,0x041,0x022,0x01c,0x00, 00},  //;)
                  {0x14,0x008,0x03e,0x008,0x14, 00},  //;*
                  {0x08,0x008,0x03e,0x008,0x08, 00},  //;+
                  {0x00,0x050,0x030,0x000,0x00, 00},  //;,
                  {0x08,0x008,0x008,0x008,0x08, 00},  //;-
                  {0x00,0x060,0x060,0x000,0x00, 00},  //;,
                  {0x20,0x010,0x008,0x004,0x02, 00},  //;/
                  {0x3E,0x051,0x049,0x045,0x3E, 00},  //;0
                  {0x00,0x042,0x07f,0x040,0x00, 00},  //;1
                  {0x42,0x061,0x051,0x049,0x46, 00},  //;2
                  {0x21,0x041,0x045,0x04b,0x31, 00},  //;3
                  {0x18,0x014,0x012,0x07f,0x10, 00},  //;4
                  {0x27,0x045,0x045,0x045,0x39, 00},  //;5
                  {0x3c,0x04a,0x049,0x049,0x30, 00},  //;6
                  {0x01,0x071,0x009,0x005,0x03, 00},  //;7
                  {0x36,0x049,0x049,0x049,0x36, 00},  //;8
                  {0x06,0x049,0x049,0x025,0x1e, 00},  //;9
                  {0x00,0x036,0x036,0x000,0x00, 00},  //;:
                  {0x00,0x056,0x036,0x000,0x00, 00},  //;;
                  {0x08,0x014,0x022,0x041,0x00, 00},  //;<
                  {0x14,0x014,0x014,0x014,0x14, 00},  //;=
                  {0x00,0x041,0x022,0x014,0x08, 00},  //;>
                  {0x02,0x001,0x051,0x009,0x06, 00},  //;?
                  {0x32,0x049,0x079,0x041,0x3e, 00},  //;@
                  {0x7e,0x011,0x011,0x011,0x7e, 00},  //;A
                  {0x7f,0x049,0x049,0x049,0x36, 00},  //;B
                  {0x3e,0x041,0x041,0x041,0x22, 00},  //;C
                  {0x7f,0x041,0x041,0x022,0x1c, 00},  //;D
                  {0x7f,0x049,0x049,0x049,0x41, 00},  //;E
                  {0x7f,0x009,0x009,0x009,0x01, 00},  //;F
                  {0x3e,0x041,0x049,0x049,0x3a, 00},  //;G40
                  {0x7f,0x008,0x008,0x008,0x7f, 00},  //;H
                  {0x00,0x041,0x07f,0x041,0x00, 00},  //;I
                  {0x20,0x040,0x041,0x03f,0x01, 00},  //;J
                  {0x7f,0x008,0x014,0x022,0x41, 00},  //;K
                  {0x7f,0x040,0x040,0x040,0x40, 00},  //;L
                  {0x7f,0x002,0x00c,0x002,0x7f, 00},  //;M
                  {0x7f,0x004,0x008,0x010,0x7f, 00},  //;N
                  {0x3e,0x041,0x041,0x041,0x3e, 00},  //;O
                  {0x7f,0x009,0x009,0x009,0x06, 00},  //;P
                  {0x3e,0x041,0x051,0x021,0x5e, 00},  //;Q50
                  {0x7f,0x009,0x019,0x029,0x46, 00},  //;R
                  {0x46,0x049,0x049,0x049,0x31, 00},  //;S
                  {0x01,0x001,0x07f,0x001,0x01, 00},  //;T
                  {0x3f,0x040,0x040,0x040,0x3f, 00},  //;U
                  {0x1f,0x020,0x040,0x020,0x1f, 00},  //;V
                  {0x3f,0x040,0x070,0x040,0x3f, 00},  //;W
                  {0x63,0x014,0x008,0x014,0x63, 00},  //;X
                  {0x07,0x008,0x070,0x008,0x07, 00},  //;Y
                  {0x61,0x051,0x049,0x045,0x43, 00},  //;Z
                  {0x00,0x07f,0x041,0x041,0x00, 00},  //;[60
                  {0x02,0x004,0x008,0x010,0x20, 00},  //;
                  {0x00,0x041,0x041,0x07f,0x00, 00},  //;]
                  {0x04,0x002,0x001,0x002,0x04, 00},  //;~
                  {0x40,0x040,0x040,0x040,0x40, 00},  //;_
                  {0x00,0x001,0x002,0x000,0x00, 00},  //;`
                  {0x20,0x054,0x054,0x054,0x78, 00},  //;a
                  {0x7f,0x044,0x044,0x044,0x38, 00},  //;b
                  {0x38,0x044,0x044,0x044,0x20, 00},  //;c
                  {0x38,0x044,0x044,0x048,0x7f, 00},  //;d
                  {0x38,0x054,0x054,0x054,0x18, 00},  //;e70
                  {0x08,0x07e,0x009,0x001,0x02, 00},  //;f
                  {0x0c,0x052,0x052,0x052,0x3e, 00},  //;g
                  {0x7f,0x008,0x004,0x004,0x78, 00},  //;h
                  {0x00,0x044,0x07d,0x040,0x00, 00},  //;i
                  {0x20,0x040,0x044,0x03d,0x00, 00},  //;j
                  {0x7f,0x010,0x028,0x044,0x00, 00},  //;k
                  {0x00,0x041,0x07f,0x040,0x00, 00},  //;l
                  {0x7c,0x004,0x018,0x004,0x78, 00},  //;m
                  {0x7c,0x008,0x004,0x004,0x7c, 00},  //;n
                  {0x38,0x044,0x044,0x044,0x38, 00},  //;o80
                  {0x7c,0x014,0x014,0x014,0x08, 00},  //;p
                  {0x08,0x014,0x014,0x014,0x7c, 00},  //;q
                  {0x7c,0x008,0x004,0x004,0x08, 00},  //;r
                  {0x48,0x054,0x054,0x054,0x20, 00},  //;s
                  {0x04,0x03f,0x044,0x040,0x20, 00},  //;t
                  {0x3c,0x040,0x040,0x020,0x7c, 00},  //;u
                  {0x1c,0x020,0x040,0x020,0x1c, 00},  //;v
                  {0x3c,0x040,0x020,0x040,0x3c, 00},  //;w
                  {0x44,0x028,0x010,0x028,0x44, 00},  //;x
                  {0x0c,0x050,0x050,0x050,0x3c, 00},  //;y90
                  {0x44,0x064,0x054,0x04C,0x44, 00},  //;z
                  {0x7f,0x049,0x049,0x049,0x33, 00},  //;Ѓ
                  {0x7f,0x001,0x001,0x001,0x03, 00},  //;ѓ
                  {0x7c,0x055,0x054,0x055,0x00, 00},  //;…
                  {0x77,0x008,0x07f,0x008,0x77, 00},  //;†
                  {0x41,0x049,0x049,0x049,0x36, 00},  //;‡
                  {0x7f,0x010,0x008,0x004,0x7f, 00},  //;€
                  {0x7c,0x021,0x012,0x009,0x7c, 00},  //;©
                  {0x20,0x041,0x03f,0x001,0x7f, 00},  //;‹
                  {0x7f,0x001,0x001,0x001,0x7f, 00},  //;Џ100
                  {0x47,0x028,0x010,0x008,0x07, 00},  //;“
                  {0x1c,0x022,0x07f,0x022,0x1c, 00},  //;”
                  {0x07,0x008,0x008,0x008,0x7f, 00},  //;—
                  {0x7f,0x040,0x07f,0x040,0x7f, 00},  //;
                  {0x01,0x07f,0x048,0x048,0x30, 00},  //;љ
                  {0x7f,0x048,0x030,0x000,0x7f, 00},  //;›
                  {0x22,0x041,0x049,0x049,0x3e, 00},  //;ќ
                  {0x7f,0x008,0x03e,0x041,0x3e, 00},  //;ћ
                  {0x46,0x029,0x019,0x009,0x7f, 00},  //;џ
                  {0x3C,0x04A,0x04A,0x049,0x31, 00},  //;Ў110
                  {0x7c,0x054,0x054,0x028,0x00, 00},  //;ў
                  {0x7c,0x004,0x004,0x004,0x0c, 00},  //;Ј
                  {0x38,0x055,0x054,0x055,0x18, 00},  //;Ґ
                  {0x6c,0x010,0x07c,0x010,0x6c, 00},  //;¦
                  {0x44,0x044,0x054,0x054,0x28, 00},  //;§
                  {0x7c,0x020,0x010,0x008,0x7c, 00},  //;Ё
                  {0x78,0x042,0x024,0x012,0x78, 00},  //;©
                  {0x7c,0x010,0x028,0x044,0x00, 00},  //;Є
                  {0x20,0x044,0x03c,0x004,0x7c, 00},  //;«
                  {0x7c,0x008,0x010,0x008,0x7c, 00},  //;¬120
                  {0x7c,0x010,0x010,0x010,0x7c, 00},  //;­
                  {0x7c,0x004,0x004,0x004,0x7c, 00},  //;Ї
                  {0x04,0x004,0x07c,0x004,0x04, 00},  //;в
                  {0x0c,0x010,0x010,0x010,0x7c, 00},  //;з
                  {0x7c,0x040,0x07c,0x040,0x7c, 00},  //;и
                  {0x04,0x07c,0x050,0x050,0x20, 00},  //;к
                  {0x7c,0x050,0x050,0x020,0x7c, 00},  //;л
                  {0x7c,0x050,0x050,0x020,0x00, 00},  //;м
                  {0x28,0x044,0x054,0x054,0x38, 00},  //;н
                  {0x7c,0x010,0x038,0x044,0x38, 00},  //;о130
                  {0x08,0x054,0x034,0x014,0x7C, 00},  //;п
                  {0x10,0x028,0x044,0x010,0x28, 00},  //;<<
                  {0x28,0x010,0x044,0x028,0x10, 00},  //;>>
                  {0x04,0x002,0x07f,0x002,0x04, 00},  //;  ‘’ђ…‹ЉЂ ‚‚…ђ•
                  {0x10,0x020,0x07f,0x020,0x10, 00},  //;  бваҐ«Є  ў­Ё§
                  {0xe0,0x051,0x04f,0x041,0xff, 00}, //;//„
                  {0x7f,0x040,0x040,0x040,0xff, 00}, //;//–
                  {0x7f,0x040,0x07f,0x040,0xff, 00}, //;//™
                  {0xe0,0x054,0x04c,0x044,0xfc, 00}, //;//¤
                  {0x30,0x048,0x0fe,0x048,0x30, 00}, //;//д140
                  {0x7c,0x040,0x040,0x040,0xfc, 00}, //;//ж
                  {0x7C,0x040,0x07C,0x040,0xFC, 00}, //;й
                  {0x08,0x01c,0x02a,0x008,0x0F, 00}, //;       ўЄ
                  {0x17,0x008,0x014,0x01a,0x7d, 00}, //;1/4
                  {0x17,0x008,0x044,0x056,0x7d, 00}, //;1/3
                  {0x17,0x008,0x034,0x056,0x5d, 00}, //;1/2
                  {0x00,0x0d8,0x0a8,0x098,0x00, 00}, //;   2       Ё­¤ҐЄб
                  {0x0e,0x00a,0x00e,0x000,0x00, 00 }, //   gradus
                  {0x40,0x32,0x09,0x3E,0x40,0x00},   //ALFA
                  {0x90,0x0f8,0x080,0x000,0x00, 00  }, //  1         Ё­¤ҐЄб
                  {0x70,0x028,0x024,0x028,0x70, 00 }, // 150    a    181    д § 
                  {0x7c,0x054,0x054,0x06c,0x00, 00},// ;   b  182      д § 
                  {0x7c,0x044,0x044,0x044,0x00, 00},// ;   c  183      д § 
                  {0x7C,0x044,0x044,0x044,0x00, 00},// ;   0 Ё­ўҐаб­®Ґ
				  {0x30,0x028,0x024,0x028,0x30, 00}};//	
				  
//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
	void vivod_0(unsigned char x,unsigned char y,unsigned char index)
{ //LCD_SET_ONE();
	unsigned char ij;
for (ij =0; ij<12;ij++)
	//for (i=0;i<16;i++)
	{ 	LCD_SET_XY(ij+x,y);
			while (SMB_BUSY);
		 LCD_Write_Arrey(LCD_WD,0,1,&BIG_TABL[index][ij].Char[1]);
		   	while (SMB_BUSY);
		  	LCD_SET_XY(ij+x,y+1);
		 	while (SMB_BUSY);
		 LCD_Write_Arrey(LCD_WD,0,1,&BIG_TABL[index][ij].Char[0]);
		  	while (SMB_BUSY);
 }
}


const union Crr code BIG_TABL [12][12]=
                {{ 0x0ffc,0x1ffe,0x3703,0x3383,0x31c3,0x30e3,0x3073,0x303b,0x1ffe,0x0ffc,0x0000,0x0000},   //0
				 { 0x0000,0x0000,0x3004,0x3006,0x3fff,0x3fff,0x3000,0x3000,0x0000,0x0000,0x0000,0x0000},   //1
				 { 0x300c,0x380e,0x3c03,0x3e03,0x3703,0x3383,0x31c3,0x30e3,0x307e,0x303c,0x0000,0x0000},   //2
				 { 0x0c00,0x1c03,0x3003,0x3023,0x3073,0x30fb,0x30df,0x30cf,0x1f87,0x0f03,0x0000,0x0000},   //3
				 { 0x03c0,0x03e0,0x0370,0x0338,0x031c,0x030e,0x3fff,0x3fff,0x0300,0x0300,0x0000,0x0000},   //4
				 { 0x0c3f,0x1c3f,0x3033,0x3033,0x3033,0x3033,0x3033,0x3033,0x1fe3,0x0fc0,0x0000,0x0000},   //5
				 { 0x0ff8,0x1ffc,0x30ce,0x30c7,0x30c3,0x30c3,0x30c3,0x30c3,0x1f80,0x0f00,0x0000,0x0000},   //6
				 { 0x0003,0x0003,0x3f83,0x3fc3,0x00e3,0x0073,0x003b,0x001f,0x000f,0x0000,0x0000,0x0000},   //7
				 { 0x0e1c,0x1f3e,0x30c3,0x30c3,0x30c3,0x30c3,0x30c3,0x30c3,0x1f3e,0x0e1c,0x0000,0x0000},   //8
				 { 0x0c3c,0x1c7e,0x30c3,0x30c3,0x30c3,0x30c3,0x38c3,0x1cc3,0x0ffe,0x07fc,0x0000,0x0000},   //9
				 { 0x00c0,0x00c0,0x00c0,0x00c0,0x0ffc,0x0ffc,0x00c0,0x00c0,0x00c0,0x00c0,0x0000,0x0000},   //+
				 { 0x00c0,0x00c0,0x00c0,0x00c0,0x00c0,0x00c0,0x00c0,0x00c0,0x00c0,0x00c0,0x0000,0x0000},   //-
				 };  