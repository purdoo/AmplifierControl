#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "time.h"
#include "math.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/flash.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "driverlib/rom.h"
#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/checkbox.h"
#include "grlib/container.h"
#include "grlib/pushbutton.h"
#include "grlib/radiobutton.h"
#include "grlib/slider.h"
#include "utils/ustdlib.h"
#include "Kentec320x240x16_ssd2119_8bit.h"
#include "touch.h"
#include "images.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"


//*****************************************************************************
//
// The DMA control structure table.
//
//*****************************************************************************
#ifdef ewarm
#pragma data_alignment=1024
tDMAControlTable sDMAControlTable[64];
#elif defined(ccs)
#pragma DATA_ALIGN(sDMAControlTable, 1024)
tDMAControlTable sDMAControlTable[64];
#else
tDMAControlTable sDMAControlTable[64] __attribute__ ((aligned(1024)));
#endif

//*****************************************************************************
//
// Forward declarations for the globals required to define the widgets at
// compile-time.
//
//*****************************************************************************
void OnPrevious(tWidget *pWidget);
void OnNext(tWidget *pWidget);
void OnIntroPaint(tWidget *pWidget, tContext *pContext);
void OnPrimitivePaint(tWidget *pWidget, tContext *pContext);
void OnCanvasPaint(tWidget *pWidget, tContext *pContext);
void OnCheckChange(tWidget *pWidget, uint32_t bSelected);
void OnButtonPress(tWidget *pWidget);
void OnRadioChange(tWidget *pWidget, uint32_t bSelected);
void OnSliderChange(tWidget *pWidget, int32_t lValue);
void Reset();
void InitADC();
void ReadTemp();
void InitSPI();
//void SPI();

extern tCanvasWidget g_psPanels[];


//*****************************************************************************
//
// Potentiometer Global Variables
//
//*****************************************************************************
#define TONAL_CONTROL_RANGE 64
#define VOLUME_CONTROL_RANGE 100
int32_t TWEETER = 32;
int32_t MIDRANGE = 32;
int32_t BASS = 32;
int32_t VOLUME = 50;
int32_t TEMP = 30;

//*****************************************************************************
//
// Temperature Sensor Global Variables
//
//*****************************************************************************

int tempF = 0;
float millivolts;
float celsius;

//*****************************************************************************
//
// Selection of push buttons.
//
//*****************************************************************************

tCanvasWidget g_psPushButtonIndicators[] =
{

	CanvasStruct(g_psPanels + 0, g_psPushButtonIndicators + 1, 0,
				 &g_sKentec320x240x16_SSD2119, 88, 35, 150, 24,
				 CANVAS_STYLE_TEXT, 0, 0, ClrSilver, &g_sFontCm20, "Tonal Presets",
				 0, 0),
	CanvasStruct(g_psPanels + 0, g_psPushButtonIndicators + 2, 0,
				 &g_sKentec320x240x16_SSD2119, 190, 55, 110, 24,
				 CANVAS_STYLE_TEXT, 0, 0, ClrSilver, &g_sFontCm20, "",
				 0, 0),

	CanvasStruct(g_psPanels + 0, g_psPushButtonIndicators + 3, 0,
				 &g_sKentec320x240x16_SSD2119, 30, 140, 110, 24,
				 CANVAS_STYLE_TEXT, 0, 0, ClrSilver, &g_sFontCm20, "Temperature",
				 0, 0),

	CanvasStruct(g_psPanels + 0, 0, 0,
				 &g_sKentec320x240x16_SSD2119, 28, 155, 110, 50,
				 CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrSilver, &g_sFontCm20, "",
				 0, 0),

};
tPushButtonWidget g_psPushButtons[] =
{

    RectangularButtonStruct(g_psPanels + 0, g_psPushButtons + 1, 0,
                            &g_sKentec320x240x16_SSD2119, 30, 70, 130, 25,
                            PB_STYLE_FILL | PB_STYLE_OUTLINE | PB_STYLE_TEXT,
                            ClrMidnightBlue, ClrBlack, ClrGray, ClrSilver,
                            &g_sFontCm22, "Pop", 0, 0, 0, 0, OnButtonPress),
	RectangularButtonStruct(g_psPanels + 0, g_psPushButtons + 2, 0,
							&g_sKentec320x240x16_SSD2119, 30, 100, 130, 25,
							PB_STYLE_FILL | PB_STYLE_OUTLINE | PB_STYLE_TEXT,
							ClrMidnightBlue, ClrBlack, ClrGray, ClrSilver,
							&g_sFontCm22, "Jazz", 0, 0, 0, 0, OnButtonPress),
	RectangularButtonStruct(g_psPanels + 0, g_psPushButtons + 3, 0,
							&g_sKentec320x240x16_SSD2119, 170, 70, 120, 25,
							PB_STYLE_FILL | PB_STYLE_OUTLINE | PB_STYLE_TEXT,
							ClrMidnightBlue, ClrBlack, ClrGray, ClrSilver,
							&g_sFontCm22, "Rock", 0, 0, 0, 0, OnButtonPress),
	RectangularButtonStruct(g_psPanels + 0, g_psPushButtons + 4, 0,
							&g_sKentec320x240x16_SSD2119, 170, 100, 120, 25,
							PB_STYLE_FILL | PB_STYLE_OUTLINE | PB_STYLE_TEXT,
							ClrMidnightBlue, ClrBlack, ClrGray, ClrSilver,
							&g_sFontCm22, "Drum+Bass", 0, 0, 0, 0, OnButtonPress),
	CircularButtonStruct(g_psPanels + 0, g_psPushButtonIndicators, 0,
							 &g_sKentec320x240x16_SSD2119, 240, 163, 27,
							 (PB_STYLE_FILL | PB_STYLE_OUTLINE | PB_STYLE_TEXT |
							  PB_STYLE_AUTO_REPEAT), ClrDarkRed, ClrBlack,
							 ClrGray, ClrSilver, &g_sFontCm22, "Reset", 0, 0, 125, 25,
							 Reset),
};

#define NUM_PUSH_BUTTONS        (sizeof(g_psPushButtons) /   \
                                 sizeof(g_psPushButtons[0]))
uint32_t g_ulButtonState;


//*****************************************************************************
//
// Sliders for Potentiometer Control
//
//*****************************************************************************
Canvas(g_sSliderValueCanvas, g_psPanels + 1, 0, 0,
       &g_sKentec320x240x16_SSD2119, 0, 0, 200, 200,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrSilver,
       &g_sFontCm24, "", 0, 0);

tSliderWidget g_psSliders[] =
{
    SliderStruct(g_psPanels + 1, g_psSliders + 1, 0,
                 &g_sKentec320x240x16_SSD2119, 10, 40, 220, 25, 0, TONAL_CONTROL_RANGE, 32,
                 (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
                  SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
                 ClrGray, ClrBlack, ClrSilver, ClrWhite, ClrWhite,
                 &g_sFontCm20, "Tweeter", 0, 0, OnSliderChange),
	SliderStruct(g_psPanels + 1, g_psSliders + 2, 0,
				 &g_sKentec320x240x16_SSD2119, 10, 80, 220, 25, 0, TONAL_CONTROL_RANGE, 32,
				 (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
				  SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
				 ClrGray, ClrBlack, ClrSilver, ClrWhite, ClrWhite,
				 &g_sFontCm20, "Mid Range", 0, 0, OnSliderChange),
	SliderStruct(g_psPanels + 1, g_psSliders + 3, 0,
				 &g_sKentec320x240x16_SSD2119, 10, 120, 220, 25, 0, TONAL_CONTROL_RANGE, 32,
				 (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
				  SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
				 ClrGray, ClrBlack, ClrSilver, ClrWhite, ClrWhite,
				 &g_sFontCm20, "Bass", 0, 0, OnSliderChange),
	SliderStruct(g_psPanels + 1, g_psSliders + 4, 0,
				 &g_sKentec320x240x16_SSD2119, 260, 40, 30, 150, 0, 100, 25,
				 (SL_STYLE_IMG | SL_STYLE_BACKG_IMG | SL_STYLE_VERTICAL |
				 SL_STYLE_OUTLINE | SL_STYLE_LOCKED), 0, ClrBlack, ClrSilver, 0, 0, 0,
				 0, g_pucGettingHotter28x148, g_pucGettingHotter28x148Mono,
				 OnSliderChange),
	 SliderStruct(g_psPanels + 1, &g_sSliderValueCanvas, 0,
				 &g_sKentec320x240x16_SSD2119, 10, 160, 220, 30, 0, VOLUME_CONTROL_RANGE, 50,
				 (SL_STYLE_FILL | SL_STYLE_BACKG_FILL | SL_STYLE_OUTLINE |
				  SL_STYLE_TEXT | SL_STYLE_BACKG_TEXT),
				  ClrDarkRed, ClrBlack, ClrSilver, ClrWhite, ClrWhite,
				 &g_sFontCm20, "Volume Control", 0, 0, OnSliderChange),

};


#define NUM_SLIDERS (sizeof(g_psSliders) / sizeof(g_psSliders[0]))

//*****************************************************************************
//
// An array of canvas widgets, one per panel.  Each canvas is filled with
// black, overwriting the contents of the previous panel.
//
//*****************************************************************************
tCanvasWidget g_psPanels[] =
{
    CanvasStruct(0, 0, g_psPushButtons, &g_sKentec320x240x16_SSD2119, 0, 24,
                 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
    CanvasStruct(0, 0, g_psSliders, &g_sKentec320x240x16_SSD2119, 0,
                 24, 320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0),
};

#define NUM_PANELS              (sizeof(g_psPanels) / sizeof(g_psPanels[0]))

char *g_pcPanelNames[] =
{
    "     Settings     ",
    "     Amp Controls    "
};

//*****************************************************************************
//
// The buttons and text across the bottom of the screen.
//
//*****************************************************************************
RectangularButton(g_sPrevious, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 210,
                  40, 30, PB_STYLE_FILL, ClrBlack, ClrBlack, 0, ClrSilver,
                  &g_sFontCm20, "-", g_pucBlue50x50, g_pucBlue50x50Press, 0, 0,
                  OnPrevious);
                  
Canvas(g_sTitle, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 50, 190, 220, 50,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, 0, 0, ClrSilver,
       &g_sFontCm20, 0, 0, 0);
       
RectangularButton(g_sNext, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 270, 210,
                  40, 30, PB_STYLE_IMG | PB_STYLE_TEXT, ClrBlack, ClrBlack, 0,
                  ClrSilver, &g_sFontCm20, "+", g_pucBlue50x50,
                  g_pucBlue50x50Press, 0, 0, OnNext);




//*****************************************************************************
//
// Handles presses of the previous panel button.
//
//*****************************************************************************
uint32_t g_ulPanel;

void OnPrevious(tWidget *pWidget)
{
    if(g_ulPanel == 0)
    {
        return;
    }

    WidgetRemove((tWidget *)(g_psPanels + g_ulPanel));

    g_ulPanel--;

    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanels + g_ulPanel));
    WidgetPaint((tWidget *)(g_psPanels + g_ulPanel));

    CanvasTextSet(&g_sTitle, g_pcPanelNames[g_ulPanel]);
    WidgetPaint((tWidget *)&g_sTitle);

    if(g_ulPanel == 0)
    {
        PushButtonImageOff(&g_sPrevious);
        PushButtonTextOff(&g_sPrevious);
        PushButtonFillOn(&g_sPrevious);
        WidgetPaint((tWidget *)&g_sPrevious);
    }

    if(g_ulPanel == (NUM_PANELS - 2))
    {
        PushButtonImageOn(&g_sNext);
        PushButtonTextOn(&g_sNext);
        PushButtonFillOff(&g_sNext);
        WidgetPaint((tWidget *)&g_sNext);
    }

}

//*****************************************************************************
//
// Handles presses of the next panel button.
//
//*****************************************************************************
void OnNext(tWidget *pWidget)
{
    if(g_ulPanel == (NUM_PANELS - 1))
    {
        return;
    }

    WidgetRemove((tWidget *)(g_psPanels + g_ulPanel));

    g_ulPanel++;

    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanels + g_ulPanel));
    WidgetPaint((tWidget *)(g_psPanels + g_ulPanel));

    CanvasTextSet(&g_sTitle, g_pcPanelNames[g_ulPanel]);
    WidgetPaint((tWidget *)&g_sTitle);

    if(g_ulPanel == 1)
    {
        PushButtonImageOn(&g_sPrevious);
        PushButtonTextOn(&g_sPrevious);
        PushButtonFillOff(&g_sPrevious);
        WidgetPaint((tWidget *)&g_sPrevious);
    }

    if(g_ulPanel == (NUM_PANELS - 1))
    {
        PushButtonImageOff(&g_sNext);
        PushButtonTextOff(&g_sNext);
        PushButtonFillOn(&g_sNext);
        WidgetPaint((tWidget *)&g_sNext);
    }

}



//*****************************************************************************
//
// Handles press notifications for the push button widgets.
//
//*****************************************************************************
void OnButtonPress(tWidget *pWidget)
{
    uint32_t ulIdx;

    for(ulIdx = 0; ulIdx < NUM_PUSH_BUTTONS; ulIdx++)
    {
        if(pWidget == (tWidget *)(g_psPushButtons + ulIdx))
        {
            break;
        }
    }

    if(ulIdx == NUM_PUSH_BUTTONS)
    {
        return;
    }
    OnNext(pWidget); // if preset tonal control is hit, cycle to next panel/page
    if(pWidget == (tWidget *)&g_psPushButtons[0]) // Pop
    {
    	// change the value of sliders, which triggers a redraw
    	g_psSliders[0].i32Value = 55;
    	g_psSliders[1].i32Value = 45;
    	g_psSliders[2].i32Value = 30;

    	// change the value of global variables used by other functions
		TWEETER = 55;
		MIDRANGE = 45;
    	BASS = 30;
    }
    if(pWidget == (tWidget *)&g_psPushButtons[1]) // Jazz
    {
    	g_psSliders[0].i32Value = 25;
    	g_psSliders[1].i32Value = 40;
    	g_psSliders[2].i32Value = 30;
    	TWEETER = 25;
		MIDRANGE = 40;
    	BASS = 30;
    }
    if(pWidget == (tWidget *)&g_psPushButtons[2]) // Rock
    {
    	g_psSliders[0].i32Value = 35;
    	g_psSliders[1].i32Value = 40;
    	g_psSliders[2].i32Value = 35;
    	TWEETER = 35;
		MIDRANGE = 40;
    	BASS = 35;
    }
    if(pWidget == (tWidget *)&g_psPushButtons[3]) // Drum + Bass
    {
    	g_psSliders[0].i32Value = 37;
    	g_psSliders[1].i32Value = 37;
    	g_psSliders[2].i32Value = 50;
    	TWEETER = 37;
		MIDRANGE = 37;
    	BASS = 50;
    }

}

//*****************************************************************************
//
// Handles notifications from the slider controls.
//
//*****************************************************************************
void OnSliderChange(tWidget *pWidget, int32_t lValue)
{
	int buffer = 0;
	if(pWidget == (tWidget *)&g_psSliders[0]) // Tweeter
	{
		//GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 239);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 224);

		SSIDataPut(SSI3_BASE, lValue);
		TWEETER = lValue;
	}

	if(pWidget == (tWidget *)&g_psSliders[1]) // Midrange
	{
		MIDRANGE = lValue;
	}

	if(pWidget == (tWidget *)&g_psSliders[2]) // Bass
	{
		BASS = lValue;
	}

	if(pWidget == (tWidget *)&g_psSliders[4]) // Volume Control
	{
		VOLUME = lValue;
	}
	SysCtlDelay(128);
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 240);
}

//*****************************************************************************
//
// Reset Function (Currently just blinks onboard LED)
//
//*****************************************************************************

bool g_RedLedOn = false;
bool onstate = true;
void Reset()
{
/*
	while(1)
	{

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 239);
		SSIDataPut(SSI3_BASE, 40);
		//SysCtlDelay(128);
		//SysCtlDelay(180);
		//GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 240);
	}
	*/
    g_RedLedOn = !g_RedLedOn;
    if(g_RedLedOn)
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x02);
    }
    else
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00);
    }
}

void InitADC()
{
	 SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
	 SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	 GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);
	 ADCSequenceConfigure(ADC1_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
	 ADCSequenceStepConfigure(ADC1_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
	 ADCSequenceEnable(ADC1_BASE, 3);
	 ADCIntClear(ADC1_BASE, 3);
}

uint32_t ADC0Value[1];
char TempText[5];
int rounded = 0;
void ReadTemp()
{
    ADCProcessorTrigger(ADC1_BASE, 3);
    while(!ADCIntStatus(ADC1_BASE, 3, false))
    {
    	// wait
    }
    ADCIntClear(ADC1_BASE, 3);
    ADCSequenceDataGet(ADC1_BASE, 3, ADC0Value);
    millivolts = (ADC0Value[0] * 3.3 * 100) / 4096;
    celsius = millivolts/10;
    rounded = (int)roundf(millivolts);
 	usprintf(TempText, "%3dC", rounded);
	CanvasTextSet(&g_psPushButtonIndicators[3], TempText);

	if(g_ulPanel == 0)
	{
		WidgetPaint((tWidget *)&g_psPushButtonIndicators[3]);
	}
	/*
	if(g_ulPanel == 1)
	{
		g_psSliders[3].i32Value = celsius;
		WidgetPaint((tWidget *)&g_psSliders[3]);
	}*/
}

void InitSPI()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	GPIOPinConfigure(GPIO_PD0_SSI3CLK);
	GPIOPinConfigure(GPIO_PD3_SSI3TX );
	GPIOPinTypeSSI(GPIO_PORTD_BASE,GPIO_PIN_0| GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	SSIConfigSetExpClk(SSI3_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000000, 16);
	SSIEnable(SSI3_BASE);
}

int main(void)
{
    tContext sContext;
    tRectangle sRect;

#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
    defined(TARGET_IS_TM4C129_RA1) ||                                         \
    defined(TARGET_IS_TM4C129_RA2)
    uint32_t ui32SysClock;
#endif
    //
    // Set the clocking to run at 20 MHz (200 MHz / 10) using the PLL.  When
    // using the ADC, you must either use the PLL or supply a 16 MHz clock
    // source.
#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
    defined(TARGET_IS_TM4C129_RA1) ||                                         \
    defined(TARGET_IS_TM4C129_RA2)
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                       SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_PLL |
                                       SYSCTL_CFG_VCO_480), 20000000);
#else
    SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
#endif

    FPUEnable();
    FPULazyStackingEnable();

    // Set the clock to 40Mhz derived from the PLL and the external oscillator
    //SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // Reset Button Bindings
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);

    // Initialize the display driver.
    Kentec320x240x16_SSD2119Init();

    // Initialize the graphics context.
    GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);

    // Fill the top 24 rows of the screen with blue to create the banner.
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 23;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);

    // Put a white box around the banner.
    GrContextForegroundSet(&sContext, ClrWhite);
    GrRectDraw(&sContext, &sRect);

    // Put the application name in the middle of the banner.
    GrContextFontSet(&sContext, &g_sFontCm20);
    GrStringDrawCentered(&sContext, "HiFi Sound System", -1, GrContextDpyWidthGet(&sContext) / 2, 8, 0);

    // Configure and enable uDMA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    SysCtlDelay(10);
    uDMAControlBaseSet(&sDMAControlTable[0]);
    uDMAEnable();
    // Initialize the touch screen driver and have it route its messages to the widget tree.
    TouchScreenInit();
    TouchScreenCallbackSet(WidgetPointerMessage);
    // Add the title block and the previous and next buttons to the widget tree.
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPrevious);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sTitle);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sNext);

    // Add the first panel to the widget tree.
    g_ulPanel = 0;
    WidgetAdd(WIDGET_ROOT, (tWidget *)g_psPanels);
    CanvasTextSet(&g_sTitle, g_pcPanelNames[0]);

    // Issue the initial paint request to the widgets.
    WidgetPaint(WIDGET_ROOT);

    //InitADC();
    InitSPI();
    int ui32Loop = 0;

    while(1)
    {
        ui32Loop++;
        if(ui32Loop == 2000000)
        {
    		//ReadTemp();
			ui32Loop = 0;
		}
        WidgetMessageQueueProcess();
    }
}
