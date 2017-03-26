#include "stm32f4xx.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"
#include "delay.h"
#include "List.h"
#include "ff.h"
#include <stdbool.h>
#include "tm_stm32f4_pcd8544.h"
#include "bmp.h"
#include "qrencode.h"
FATFS fatfs;
FIL file;

volatile s8 num_of_switch=-1;
volatile u16 result_of_conversion=0;
volatile u8 diode_state=0;
volatile u8 error_state=0;

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
}
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		if (error_state==1)// jesli uruchomiono program bez karty SD w module, zle podpieto kable
		{
			GPIO_ToggleBits(GPIOD, GPIO_Pin_12);
		}
		if (error_state==2)// jesli wyjeto karte SD w trakcie odtwarzania plikow
		{
			GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
		}
		if (error_state==3)// jesli na karcie SD nie ma plikow .wav
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
		// miejsce na kod wywolywany w momencie wystapienia przerwania, drgania stykow
		if (num_of_switch==0)// wcisnieto user button 0 - losowe odtwarzanie
		{

		}
		else if (num_of_switch==5)// wcisnieto switch 5 - przewijanie do przodu
		{

		}
		else if (num_of_switch==7)// wcisnieto switch 7 - pauzuj/wznow
		{

		}
		else if (num_of_switch==8)// wcisnieto switch 8 - przewijanie wstecz
		{

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
	/*0 - tryb losowy
	  5 - przewijanie wstecz
	  6 - start/stop
	  7 - przewijanie do przodu*/
	GPIO_InitTypeDef USER_BUTTON;
	USER_BUTTON.GPIO_Pin = GPIO_Pin_0;
	USER_BUTTON.GPIO_Mode = GPIO_Mode_IN;
	USER_BUTTON.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &USER_BUTTON);

	USER_BUTTON.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8;
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
	EXTI_InitStructure.EXTI_Line = EXTI_Line5 | EXTI_Line7 | EXTI_Line8;
	NVIC_Init(&NVIC_InitStructure);
	EXTI_Init(&EXTI_InitStructure);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource5);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource7);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);
}
bool fileType(FILINFO fileInfo)
{
	int i=0;
	for (i=0;i<10;i++)
	{
		if(fileInfo.fname[i]=='.')
		{
			if(fileInfo.fname[i+1]=='W' && fileInfo.fname[i+2]=='A' && fileInfo.fname[i+3]=='V')
			{
				return 1;
			}
		}
	}
	return 0;
}
void display_const()
{
	//Go to x=1, y=2 position
	PCD8544_GotoXY(1, 2);
	//Print data with Pixel Set mode and Fontsize of 5x7px
	PCD8544_Puts("STMwavPlayerMR", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
	PCD8544_GotoXY(31, 40);
	PCD8544_Puts("2016", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
	PCD8544_Refresh();
}
//	-------------------------------------------------------

//	-------------------------------------------------------
//	Main
//	-------------------------------------------------------

void ReadAndDisplayBMP(uint16_t x, uint16_t y, char* filename)
{
	bmpDrawBitmap(x, y, filename);
}
#define OUT_FILE_PIXEL_PRESCALER	2	// Prescaler (number of pixels in bmp file for each QRCode pixel, on each dimension)

#define PIXEL_COLOR_R				0	// Color of bmp pixels
#define PIXEL_COLOR_G				0
#define PIXEL_COLOR_B				0

// BMP defines

typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef signed long		LONG;

#define BI_RGB 0L

#pragma pack(push, 2)

typedef struct
{
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct
{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER;

#pragma pack(pop)
#include <stdio.h>
void CreateQRCodeToBMP(char *QRCODE_TEXT, char* OUT_FILE)
{
	char*			szSourceSring = QRCODE_TEXT;
	unsigned int	unWidth, x, y, l, n, unWidthAdjusted, unDataBytes;
	unsigned char*	pRGBData, *pSourceData, *pDestData;
	QRcode*			pQRC;
	FILE*			f;

	/*
	* Create a symbol from the string. The library automatically parses the input
	* string and encodes in a QR Code symbol.
	* @warning This function is THREAD UNSAFE when pthread is disabled.
	* @param string input string. It must be NUL terminated.
	* @param version version of the symbol. If 0, the library chooses the minimum
	*                version for the given input data.
	* @param level error correction level.
	* @param hint tell the library how non-alphanumerical characters should be
	*             encoded. If QR_MODE_KANJI is given, kanji characters will be
	*             encoded as Shif-JIS characters. If QR_MODE_8 is given, all of
	*             non-alphanumerical characters will be encoded as is. If you want
	*             to embed UTF-8 string, choose this.
	* @param casesensitive case-sensitive(1) or not(0).
	* @return an instance of QRcode class. The version of the result QRcode may
	*         be larger than the designated version. On error, NULL is returned,
	*         and errno is set to indicate the error. See Exceptions for the
	*         details.
	* @throw EINVAL invalid input object.
	* @throw ENOMEM unable to allocate memory for input objects.
	* @throw ERANGE input data is too large.
	*/

	// Compute QRCode

	if (pQRC = QRcode_encodeString(szSourceSring, 0, QR_ECLEVEL_H, QR_MODE_8, 1))
	{
		unWidth = pQRC->width;
		unWidthAdjusted = unWidth * OUT_FILE_PIXEL_PRESCALER * 3;

		if (unWidthAdjusted % 4)
		{
			unWidthAdjusted = (unWidthAdjusted / 4 + 1) * 4;
		}

		unDataBytes = unWidthAdjusted * unWidth * OUT_FILE_PIXEL_PRESCALER;

		// Allocate pixels buffer
		if (!(pRGBData = (unsigned char*)malloc(unDataBytes)))
		{
			return;
		}

		// Preset to white
		memset(pRGBData, 0xff, unDataBytes);

		// Prepare bmp headers
		BITMAPFILEHEADER kFileHeader;
		kFileHeader.bfType = 0x4d42;  // "BM"
		kFileHeader.bfSize = sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER) +
			unDataBytes;
		kFileHeader.bfReserved1 = 0;
		kFileHeader.bfReserved2 = 0;
		kFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER);

		BITMAPINFOHEADER kInfoHeader;
		kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
		kInfoHeader.biWidth = unWidth * OUT_FILE_PIXEL_PRESCALER;
		kInfoHeader.biHeight =((int)unWidth * OUT_FILE_PIXEL_PRESCALER);
		kInfoHeader.biPlanes = 1;
		kInfoHeader.biBitCount = 24;
		kInfoHeader.biCompression = BI_RGB;
		kInfoHeader.biSizeImage = 0;
		kInfoHeader.biXPelsPerMeter = 0;
		kInfoHeader.biYPelsPerMeter = 0;
		kInfoHeader.biClrUsed = 0;
		kInfoHeader.biClrImportant = 0;


		// Convert QrCode bits to bmp pixels
		pSourceData = pQRC->data;
		for (y = unWidth-1; y >=0; y--)
		{
			pDestData = pRGBData + unWidthAdjusted * y * OUT_FILE_PIXEL_PRESCALER;
			for (x = 0; x < unWidth; x++)
			{
				if (*pSourceData & 1)
				{
					for (l = 0; l < OUT_FILE_PIXEL_PRESCALER; l++)
					{
						for (n = 0; n < OUT_FILE_PIXEL_PRESCALER; n++)
						{
							*(pDestData + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_B;
							*(pDestData + 1 + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_G;
							*(pDestData + 2 + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_R;
						}
					}
				}
				pDestData += 3 * OUT_FILE_PIXEL_PRESCALER;
				pSourceData++;
			}
			if (y == 0) break;
		}


		// Output the bmp file
		/*if (!(f=fopen( OUT_FILE, "wb")))
		{
			fwrite(&kFileHeader, sizeof(BITMAPFILEHEADER), 1, f);
			fwrite(&kInfoHeader, sizeof(BITMAPINFOHEADER), 1, f);
			fwrite(pRGBData, sizeof(unsigned char), unDataBytes, f);

			fclose(f);
		}
		else
		{

			return;
		}*/


		// Free data
		free(pRGBData);
		QRcode_free(pQRC);
	}
	else
	{

		return;
	}

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
	DIR Dir;
	FILINFO fileInfo;

	struct List *first=0,*last=0,*pointer;

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
	fresult = f_opendir(&Dir, "\\");
	if(fresult != FR_OK)
	{
		return(fresult);
	}
	u32 number_of_songs=0;
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
		if(fileType(fileInfo)==1)// sprawdzenie, czy plik na karcie ma rozszerzenie .wav
		{
			if(number_of_songs==0)
			{
				first=last=add_last(last,fileInfo);
			}
			else
			{
				last=add_last(last,fileInfo);
			}
			number_of_songs++;
		}
	}
	/*if (first==0)// jesli na karcie nie ma plikow .wav
	{
		error_state=3;
		PCD8544_GotoXY(12, 17);
		PCD8544_Puts("Brak plikow", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
		PCD8544_GotoXY(3, 25);
		PCD8544_Puts(".wav na karcie", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
		PCD8544_Refresh();
		TIM_Cmd(TIM4, ENABLE);
		for(;;)
		{ }
	}
	last->next=first;
	first->previous=last;
	pointer=first;
*/
	BUTTON_init();
	JOINT_VIBRATION();
	INTERRUPT_init();
	u32 i_loop=0;

	CreateQRCodeToBMP("Test", "test.bmp");
	ReadAndDisplayBMP(24,0,"test.bmp");

	for(;;)
	{

	}

	GPIO_ResetBits(GPIOD, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
	display_const();
	PCD8544_GotoXY(8, 13);
	PCD8544_Puts("Wyjeto karte", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
	PCD8544_GotoXY(34, 21);
	PCD8544_Puts("lub", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
	PCD8544_GotoXY(5, 29);
	PCD8544_Puts("wypiety kabel", PCD8544_Pixel_Set, PCD8544_FontSize_5x7);
	PCD8544_Refresh();
	TIM_Cmd(TIM2, DISABLE);
	TIM_Cmd(TIM4, ENABLE);
	for(;;)
	{ }
	return 0;
}
void SysTick_Handler()
{
	disk_timerproc();
}

