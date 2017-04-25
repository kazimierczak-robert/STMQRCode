#include "stm32f4xx.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rng.h"
#include "misc.h"
#include "delay.h"
#include "List.h"
#include "ff.h"
#include <stdbool.h>
#include "tm_stm32f4_pcd8544.h"
#include "bmp.h"
#include "QRLib\qr_encode.h"

FATFS fatfs;
FIL file;

volatile s8 num_of_switch=-1;
volatile u8 error_state=0;
volatile bool action_button_state=0;
volatile u32 numberOfFiles=0;
struct List *first=0,*last=0,*pointer; //list of .txt file names
bool RadicalsOrText=false;

#define maxFileNameLength 64

char* readtxtFile();
bool isBMP(FILINFO fileInfo)
{
	int i=0;
	for (i=0;i<maxFileNameLength-3;i++)
	{
		if(fileInfo.fname[i]=='.')
		{
			if((fileInfo.fname[i+1]=='B'|| fileInfo.fname[i+1]=='b')
					&& (fileInfo.fname[i+2]=='M' || fileInfo.fname[i+2]=='m')
					&& (fileInfo.fname[i+3]=='P'|| fileInfo.fname[i+3]=='p'))
			{
				fileInfo.fname[i+4]='\0';
				return 1;
			}
		}
	}
	return 0;
}
bool isTXT(FILINFO fileInfo)
{
	int i=0;
	for (i=0;i<maxFileNameLength-3;i++)
	{
		if(fileInfo.fname[i]=='.')
		{
			if((fileInfo.fname[i+1]=='T'|| fileInfo.fname[i+1]=='t')
					&& (fileInfo.fname[i+2]=='X' || fileInfo.fname[i+2]=='x')
					&& (fileInfo.fname[i+3]=='T'|| fileInfo.fname[i+3]=='t'))
			{
				fileInfo.fname[i+4]='\0';
				return 1;
			}
		}
	}
	return 0;
}
/* Flashcards */
void displayFlashcard()
{
	PCD8544_Clear();
	PCD8544_Refresh();
	FRESULT fr;
	FILINFO fno;
	struct List *temporary_txt=pointer;
	TCHAR* name=(TCHAR *)calloc(maxFileNameLength+13,1);
	strcpy(name,"\\Radicals\\");
	strcat(name, temporary_txt->file.fname);
	//strcpy(name+11,);
    fr = f_stat(name, &fno);
	if( fr == FR_OK )
	{
		TCHAR* proper=(TCHAR *)calloc(maxFileNameLength,1);
		const char *p;

		p = strchr(temporary_txt->file.fname, '.');
		if (p)
		{
			int bytes=p-temporary_txt->file.fname;
			TCHAR* name2=temporary_txt->file.fname;
		    memmove(proper,name2 ,bytes);
		}


	//int radicalNameLength=strlen(temporary_txt->file.fname);
		//memmove (properName, temporary_txt->file.fname, radicalNameLength);
		QRGenerator(proper);
		ReadAndDisplayBMP(43, 2,name);
	}
	free(name);
}
void listManager() //Radicals->true, Text->false
{
	first=last=remove_all(first);
	first->next->file.fname;
	pointer=0;
	FRESULT fresult;
	DIR Dir;
	FILINFO fileInfo;
	numberOfFiles=0;
	//choose Radicals or Text QR Code
	if (RadicalsOrText==true)
	{
		fresult = f_opendir(&Dir, "\\Radicals");
	}
	else
	{
		fresult = f_opendir(&Dir, "\\");
	}

	if(fresult != FR_OK)
	{
		return(fresult);
	}


	u8 iter=0;
	bool extResult=false;
	for(;;)
	{
		fresult = f_readdir(&Dir, &fileInfo);

		if(fresult != FR_OK)
		{
			return(fresult);
		}
		if(!fileInfo.fname[0])
		{
			break;
		}

		if (RadicalsOrText==true)
		{
			extResult=isBMP(fileInfo);
		}
		else
		{
			extResult=isTXT(fileInfo);
		}
		if(extResult==1)
		{
			if(numberOfFiles==0)
			{
				first=last=add_last(last,fileInfo);
			}
			else
			{
				last=add_last(last,fileInfo);
			}
				numberOfFiles++;
			}

			for(iter=0;iter<maxFileNameLength;++iter)
			{
				fileInfo.fname[iter]='0';
			}
		}
	if (first==0)// jesli na karcie nie ma plikow .txt
		{
			error_state=3;
			PCD8544_GotoXY(12, 17);
			PCD8544_Puts("Brak plikow", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
			PCD8544_GotoXY(3, 25);
			PCD8544_Puts(".txt na karcie", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
			PCD8544_Refresh();
			TIM_Cmd(TIM4, ENABLE);
			for(;;)
			{ }
		}
		last->next=first;
		first->previous=last;
		pointer=first;
}
void display_const()
{
	//Go to x=1, y=2 position
	PCD8544_GotoXY(15, 0);
	//Print data with Pixel Set mode and Fontsize of 5x7px
	PCD8544_Puts("STMQRCode", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
	PCD8544_GotoXY(32, 40);
	PCD8544_Puts("2017", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
	PCD8544_Refresh();
}
/* QRCode */
void display_filename()
{
	PCD8544_ClearFilename();
	//Print data with Pixel Set mode and Fontsize of 5x7px
	PCD8544_Puts(pointer->file.fname, PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
	PCD8544_Refresh();
}
void displayQRCode(int side, uint8_t *bitdata)
{
	PCD8544_Clear();
	int i=0;
	int j=0;
	int a=0;
	int l=0;
	int n=0;
	int OUT_FILE_PIXEL_PRESCALER=1;
	if (side==21)
	{
		OUT_FILE_PIXEL_PRESCALER=2;
	}
		for (i = 0; i < side; i++) {

			for (j = 0; j < side; j++) {
				a = j * side + i;

				if ((bitdata[a / 8] & (1 << (7 - a % 8))))
				{
					for (l = 0; l < OUT_FILE_PIXEL_PRESCALER; l++)
					{
						for (n = 0; n < OUT_FILE_PIXEL_PRESCALER; n++)
						{
							//*(pDestData + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_B;
							PCD8544_DrawPixel(OUT_FILE_PIXEL_PRESCALER*i+l,OUT_FILE_PIXEL_PRESCALER*(j)+n,PCD8544_Pixel_Set);
						}
					}
				}
			}
		}

		 PCD8544_Refresh();
}
void ReadAndDisplayBMP(uint16_t x, uint16_t y, char* filename)
{
	bmpDrawBitmap(x, y, filename);
}
void QRGenerator(char *input)
{
	int side, i, j, a;
	uint8_t bitdata[QR_MAX_BITDATA];

	// remove newline
	if (input[strlen(input) - 1] == '\n')
	{
		input[strlen(input) - 1] = 0;
	}

	side = qr_encode(QR_LEVEL_L, 0, input, 0, bitdata);
	displayQRCode(side, bitdata);
}
void EXTI0_IRQHandler(void)
{
	// drgania stykow
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		num_of_switch=0;
		TIM_Cmd(TIM5, ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}
void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		num_of_switch=5;
		TIM_Cmd(TIM5, ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line5);
	}
	else if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
		num_of_switch=7;
		TIM_Cmd(TIM5, ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line7);
	}
	else if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		num_of_switch=8;
		TIM_Cmd(TIM5, ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	else if(EXTI_GetITStatus(EXTI_Line9) != RESET)
	{
		num_of_switch=9;
		TIM_Cmd(TIM5, ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line9);
	}

}
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		if (error_state==1)// jesli uruchomiono program bez karty SD w module, zle podpieto kable
		{
			GPIO_ToggleBits(GPIOD, GPIO_Pin_12);
		}
		if (error_state==2) //problem z odczytem pliku lub pusty plik
		{
			GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
		}
		if (error_state==3)// jesli na karcie SD nie ma plikow .txt
		{
			GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
		}
		/*if (error_state==4)// niezagospodarowane na obecna chwile
		{
			GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
		}*/
		// wyzerowanie flagi wyzwolonego przerwania
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	}
}
void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
	{
		int i_loop=0;
		u8 rand_number=0;

		if (num_of_switch==0)//user button
		{
			//ReadAndDisplayBMP(24,0,"test.bmp");
			//generuj bitmape, zapisz do pliku
		}
		else if (num_of_switch==5)// switch 5 - previous file
		{
			action_button_state=0;
			if(RadicalsOrText==true)
			{
				rand_number=RNG_GetRandomNumber()%(numberOfFiles-1)+1;
			}
			else
			{
				rand_number=1;
			}

			for(i_loop=0;i_loop<rand_number;i_loop++)
			{
				pointer=pointer->previous;
			}

			if (RadicalsOrText==true) //flashcard mode
			{
				displayFlashcard();
			}
			else
			{
				PCD8544_Clear();
				display_const();
				display_filename();
			}
		}
		else if (num_of_switch==7)// switch 7 - generate QRCode
		{
			if (RadicalsOrText==false)
			{
			if(action_button_state==0)
			{
				action_button_state=1;
				FRESULT fr;
				struct List *temporary_txt=pointer;
				char fileContent[1];
				UINT *readBytes=0;

				fr = f_open( &file, temporary_txt->file.fname, FA_READ );
				if( fr == FR_OK )
				{
					f_read(&file,fileContent,7089,readBytes);
					f_close(&file);
				}
				if (fileContent[0]!='\200')
				{
					QRGenerator(fileContent);
				}
				else
				{
					error_state=3;
					GPIO_ResetBits(GPIOD, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
					TIM_Cmd(TIM4, ENABLE);
					PCD8544_ClearFilename();
					PCD8544_GotoXY(0, 13);
					PCD8544_Puts("Plik jest pusty", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
					PCD8544_GotoXY(5, 21);
					PCD8544_Puts("lub problem z", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
					PCD8544_GotoXY(3, 29);
					PCD8544_Puts("jego odczytem", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
					PCD8544_Refresh();
				}
			}
			else
			{
				GPIO_ResetBits(GPIOD, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
				TIM_Cmd(TIM4, DISABLE);
				TIM_SetCounter(TIM4, 0);
				action_button_state=0;
				PCD8544_Clear();
				display_const();
				display_filename();
			}
			}
		}
		else if (num_of_switch==8)// switch 8 - next file
		{
			action_button_state=0;
			if(RadicalsOrText==true)
			{
				rand_number=RNG_GetRandomNumber()%(numberOfFiles-1)+1;
			}
			else
			{
				rand_number=1;
			}

			for(i_loop=0;i_loop<rand_number;i_loop++)
			{
				pointer=pointer->next;
			}
			if (RadicalsOrText==true) //flashcard mode
			{
				displayFlashcard();
			}
			else
			{
				PCD8544_Clear();
				display_const();
				display_filename();
			}

		}
		else if (num_of_switch==9)// flashcards or text
		{
			action_button_state=0;
			RadicalsOrText=!RadicalsOrText;

			listManager();

			if (RadicalsOrText==true) //flashcard mode
			{
				rand_number=RNG_GetRandomNumber()%(numberOfFiles-1)+1;
				for(i_loop=0;i_loop<rand_number;i_loop++)
				{
					pointer=pointer->next;
				}
				displayFlashcard();
			}
			else //text mode
			{
				PCD8544_Clear();
				PCD8544_Refresh();

				display_const();
				display_filename();
			}
		}
		num_of_switch=-1;
		TIM_Cmd(TIM5, DISABLE);
		TIM_SetCounter(TIM5, 0);
		// wyzerowanie flagi wyzwolonego przerwania
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	}
}
void ERROR_TIM_4()
{
	// TIMER DO OBSLUGI DIOD W PRZYPADKU BLEDU
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseInitTypeDef TIMER_4;
	/* Time base configuration */
	TIMER_4.TIM_Period = 24000-1;
	TIMER_4.TIM_Prescaler = 1000-1;
	TIMER_4.TIM_ClockDivision = TIM_CKD_DIV1;
	TIMER_4.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIMER_4);
	TIM_Cmd(TIM4,DISABLE);

	// KONFIGURACJA PRZERWAN - TIMER/COUNTER
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;// numer przerwania
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;// priorytet glowny
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;// subpriorytet
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;// uruchom dany kanal
	NVIC_Init(&NVIC_InitStructure);// zapisz wypelniona strukture do rejestrow
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);// wyczyszczenie przerwania od timera 4 (wystapilo przy konfiguracji timera)
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);// zezwolenie na przerwania od przepelnienia dla timera 4
}
void JOINT_VIBRATION()
{
	// TIMER DO ELIMINACJI DRGAN STYKOW, TIM5
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	TIM_TimeBaseInitTypeDef TIMER;
	/* Time base configuration */
	TIMER.TIM_Period = 8400-1;
	TIMER.TIM_Prescaler = 3000-1;
	TIMER.TIM_ClockDivision = TIM_CKD_DIV1;
	TIMER.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM5, &TIMER);
	TIM_Cmd(TIM5,DISABLE);

	// KONFIGURACJA PRZERWAN - TIMER/COUNTER
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;// numer przerwania
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;// priorytet glowny
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;// subpriorytet
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;// uruchom dany kanal
	NVIC_Init(&NVIC_InitStructure);// zapisz wypelniona strukture do rejestrow
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);// wyczyszczenie przerwania od timera 5 (wystapilo przy konfiguracji timera)
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);// zezwolenie na przerwania od przepelnienia dla timera 5
}
void DIODES_init()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitTypeDef  DIODES;
	/* Configure PD12, PD13, PD14 and PD15 in output pushpull mode */
	DIODES.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
	DIODES.GPIO_Mode = GPIO_Mode_OUT;// tryb wyprowadzenia, wyjcie binarne
	DIODES.GPIO_OType = GPIO_OType_PP;// wyjcie komplementarne
	DIODES.GPIO_Speed = GPIO_Speed_100MHz;// max. V przelaczania wyprowadzen
	DIODES.GPIO_PuPd = GPIO_PuPd_NOPULL;// brak podciagania wyprowadzenia
	GPIO_Init(GPIOD, &DIODES);
}
void BUTTON_init()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
	/*0 - zapis do BMP
	  5 - przewijanie wstecz
	  7 - generuj kod
	  9 - tryb fiszki lub textu
	  8 - przewijanie do przodu*/
	GPIO_InitTypeDef USER_BUTTON;
	USER_BUTTON.GPIO_Pin = GPIO_Pin_0;
	USER_BUTTON.GPIO_Mode = GPIO_Mode_IN;
	USER_BUTTON.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &USER_BUTTON);

	USER_BUTTON.GPIO_Pin = GPIO_Pin_5 |GPIO_Pin_9| GPIO_Pin_7 | GPIO_Pin_8;
	USER_BUTTON.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &USER_BUTTON);
}
void INTERRUPT_init()
{
	// KONFIGURACJA KONTROLERA PRZERWAN
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn; // numer przerwania
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;// priorytet glowny
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;// subpriorytet
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;// uruchom dany kanal
	NVIC_Init(&NVIC_InitStructure);// zapisz wypelniona strukture do rejestrow

	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;// wybor numeru aktualnie konfigurowanej linii przerwan
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;// wybor trybu - przerwanie badz zdarzenie
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;// wybor zbocza, na ktore zareaguje przerwanie
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;// uruchom dana linie przerwan
	EXTI_Init(&EXTI_InitStructure);// zapisz strukture konfiguracyjna przerwan zewnetrznych do rejestrow

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

	// KONFIGURACJA KONTROLERA PRZERWAN DLA SWITCH Pin_5, Pin_6, Pin_7
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	EXTI_InitStructure.EXTI_Line = EXTI_Line5 |EXTI_Line9| EXTI_Line7 | EXTI_Line8;
	NVIC_Init(&NVIC_InitStructure);
	EXTI_Init(&EXTI_InitStructure);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource5);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource9);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource7);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);
}

int main( void )
{
	SystemInit();
	DIODES_init();// inicjalizacja diod
	ERROR_TIM_4();
	delay_init( 80 );// wyslanie 80 impulsow zegarowych; do inicjalizacji SPI
	SPI_SD_Init();// inicjalizacja SPI pod SD

	// SysTick_CLK... >> taktowany z f = ok. 82MHz/8 = ok. 10MHz
	// Systick_Config >> generuje przerwanie co <10ms
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);// zegar 24-bitowy
	SysTick_Config(90000);

	//Initialize LCD with 0x38 software contrast
	PCD8544_Init(0x38);
	display_const();

	// SD CARD
	FRESULT fresult;

	disk_initialize(0);// inicjalizacja karty
	fresult = f_mount( &fatfs, 1,1 );// zarejestrowanie dysku logicznego w systemie
	if(fresult != FR_OK) //jesli wystapil blad tj. wlaczenie STM32 bez karty w module, zle podpiete kable
	{
		error_state=1;
		PCD8544_GotoXY(21, 13);
		PCD8544_Puts("Problem", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
		PCD8544_GotoXY(17, 21);
		PCD8544_Puts("z kablami", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
		PCD8544_GotoXY(17, 29);
		PCD8544_Puts("lub karta", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
		PCD8544_Refresh();
		TIM_Cmd(TIM4, ENABLE);
		for(;;)
		{ }
	}
	listManager();
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);


	BUTTON_init();
	JOINT_VIBRATION();
	INTERRUPT_init();
	u32 i_loop=0;

	display_filename();

	for(;;)
	{

	}

	return 0;
}
void SysTick_Handler()
{
	disk_timerproc();
}

