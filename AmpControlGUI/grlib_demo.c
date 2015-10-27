#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "time.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
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
void UpdatePots(int32_t id, int32_t value); // add declaration
void Reset();
void WaitFor(unsigned int seconds);
void ADC();
void SPI();
void Test();
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

	CanvasStruct(g_psPanels + 0, g_psPushButtonIndicators + 4, 0,
				 &g_sKentec320x240x16_SSD2119, 28, 155, 110, 50,
				 CANVAS_STYLE_TEXT, ClrBlack, 0, ClrSilver, &g_sFontCm20, "",
				 0, 0),
	CanvasStruct(g_psPanels + 0, 0, 0,
				 &g_sKentec320x240x16_SSD2119, 28, 155, 110, 50,
				 CANVAS_STYLE_TEXT, ClrBlack, 0, ClrSilver, &g_sFontCm20, "",
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

#define SLIDER_TEXT_VAL_INDEX   0
#define SLIDER_LOCKED_INDEX     2
#define SLIDER_CANVAS_VAL_INDEX 4

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

//*****************************************************************************
//
// The number of panels.
//
//*****************************************************************************
#define NUM_PANELS              (sizeof(g_psPanels) / sizeof(g_psPanels[0]))

//*****************************************************************************
//
// The names for each of the panels, which is displayed at the bottom of the
// screen.
//
//*****************************************************************************
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
// The panel that is currently being displayed.
//
//*****************************************************************************
uint32_t g_ulPanel;

//*****************************************************************************
//
// Handles presses of the previous panel button.
//
//*****************************************************************************
void
OnPrevious(tWidget *pWidget)
{
    //
    // There is nothing to be done if the first panel is already being
    // displayed.
    //
    if(g_ulPanel == 0)
    {
        return;
    }

    //
    // Remove the current panel.
    //
    WidgetRemove((tWidget *)(g_psPanels + g_ulPanel));

    //
    // Decrement the panel index.
    //
    g_ulPanel--;

    //
    // Add and draw the new panel.
    //
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanels + g_ulPanel));
    WidgetPaint((tWidget *)(g_psPanels + g_ulPanel));

    //
    // Set the title of this panel.
    //
    CanvasTextSet(&g_sTitle, g_pcPanelNames[g_ulPanel]);
    WidgetPaint((tWidget *)&g_sTitle);

    //
    // See if this is the first panel.
    //
    if(g_ulPanel == 0)
    {
        //
        // Clear the previous button from the display since the first panel is
        // being displayed.
        //
        PushButtonImageOff(&g_sPrevious);
        PushButtonTextOff(&g_sPrevious);
        PushButtonFillOn(&g_sPrevious);
        WidgetPaint((tWidget *)&g_sPrevious);
    }

    //
    // See if the previous panel was the last panel.
    //
    if(g_ulPanel == (NUM_PANELS - 2))
    {
        //
        // Display the next button.
        //
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
void
OnNext(tWidget *pWidget)
{
    //
    // There is nothing to be done if the last panel is already being
    // displayed.
    //
    if(g_ulPanel == (NUM_PANELS - 1))
    {
        return;
    }

    //
    // Remove the current panel.
    //
    WidgetRemove((tWidget *)(g_psPanels + g_ulPanel));

    //
    // Increment the panel index.
    //
    g_ulPanel++;

    //
    // Add and draw the new panel.
    //
    WidgetAdd(WIDGET_ROOT, (tWidget *)(g_psPanels + g_ulPanel));
    WidgetPaint((tWidget *)(g_psPanels + g_ulPanel));

    //
    // Set the title of this panel.
    //
    CanvasTextSet(&g_sTitle, g_pcPanelNames[g_ulPanel]);
    WidgetPaint((tWidget *)&g_sTitle);

    //
    // See if the previous panel was the first panel.
    //
    if(g_ulPanel == 1)
    {
        //
        // Display the previous button.
        //
        PushButtonImageOn(&g_sPrevious);
        PushButtonTextOn(&g_sPrevious);
        PushButtonFillOff(&g_sPrevious);
        WidgetPaint((tWidget *)&g_sPrevious);
    }

    //
    // See if this is the last panel.
    //
    if(g_ulPanel == (NUM_PANELS - 1))
    {
        //
        // Clear the next button from the display since the last panel is being
        // displayed.
        //
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
void
OnButtonPress(tWidget *pWidget)
{
    uint32_t ulIdx;

    //
    // Find the index of this push button.
    //
    for(ulIdx = 0; ulIdx < NUM_PUSH_BUTTONS; ulIdx++)
    {
        if(pWidget == (tWidget *)(g_psPushButtons + ulIdx))
        {
            break;
        }
    }

    //
    // Return if the push button could not be found.
    //
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
void
OnSliderChange(tWidget *pWidget, int32_t lValue)
{

	if(pWidget == (tWidget *)&g_psSliders[0]) // Tweeter
	{
		//SliderValueSet(&g_psSliders[0], lValue);
		//SSIDataPut(SSI0_Base, lValue);
		//SysCtlDelay(128);
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



	/*
    static char pcCanvasText[5];
    static char pcSliderText[5];
	*/
}

//*****************************************************************************
//
// Passes the Slider Values to SPI Bus
//
//*****************************************************************************

void
UpdatePots(int32_t id, int32_t value)
{
}



//*****************************************************************************
//
// Reset Function (Currently just blinks onboard LED)
//
//*****************************************************************************

bool g_RedLedOn = false;
bool onstate = true;
void
Reset()
{
    g_RedLedOn = !g_RedLedOn;

    if(g_RedLedOn)
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x02);
    }
    else
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00);
    }
    Test();
}


void WaitFor (unsigned int secs) {
    unsigned int retTime = time(0) + secs;     // Get finishing time.
    while (time(0) < retTime);    // Loop until it arrives.
}


void ADC()
{
	/*
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1); // ADC Module 1 (there are 2 onboard)
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // chosing GPIO A Pins
	GPIOPinTypeADC(GPIO_PORTA_BASE, GPIO_PIN_2); // map the analog input pin to A2
	ADCSequenceConfigure(ADC1_BASE, 3, ADC_TRIGGER_PROCESSOR, 0); // configuring settings for targets and step size
	ADCSequenceStepConfigure(ADC1_BASE, 1, 0);
	ADCSequenceEnable(ADC1_BASE, 1); // enable our configured pin
	*/
}

#define NUM_SSI_DATA 8


void Test()
{
	uint32_t data = 0;
	uint32_t ulindex = 0;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	GPIOPinConfigure(GPIO_PA2_SSI0CLK);
	GPIOPinConfigure(GPIO_PA5_SSI0TX);
	GPIOPinTypeSSI(GPIO_PORTA_BASE,GPIO_PIN_5| GPIO_PIN_2);
	// Initializes the four slave select pins (3,4,6,7)
	/*
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_6);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_7);
	 */
	SSIConfigSetExpClk(SSI0_BASE,SysCtlClockGet(),SSI_FRF_MOTO_MODE_1,SSI_MODE_MASTER,10000,8);
	SSIEnable(SSI0_BASE);
	while(1)
	{
		data = 0;
		for(ulindex = 0; ulindex < NUM_SSI_DATA; ulindex++)
		{
			data = data + 1;
			SSIDataPut(SSI0_BASE, data);
			while(SSIBusy(SSI0_BASE))
			{

			}
			/*
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_3, 0);
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 128);
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 128);
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 128);*/
			if(data > 70)
			{
				data = 0;
			}
		}

	}
}

void SPI(uint32_t PotValue, uint32_t TargetPot)
{
	// Initializes the base SSI peripherals as well as the SPI_CLK and SPI_TX Pins
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	GPIOPinConfigure(GPIO_PA2_SSI0CLK);
	GPIOPinConfigure(GPIO_PA5_SSI0TX);
	GPIOPinTypeSSI(GPIO_PORTA_BASE,GPIO_PIN_5| GPIO_PIN_2);
	// Initializes the four slave select pins (3,4,6,7)
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_6);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_7);
	// Sets the SPI Clock based on the system clock in Master Mode. Data Width is set to 16 as per DigiPot Specs
	SSIConfigSetExpClk(SSI0_BASE,SysCtlClockGet(),SSI_FRF_MOTO_MODE_0,SSI_MODE_MASTER,1000000,16);
	SSIEnable(SSI0_BASE);
	if(TargetPot == 1)
	{
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_3, 0);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 1);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 1);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 1);
	}
	if(TargetPot == 2)
	{
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_3, 1);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 1);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 1);
	}
	if(TargetPot == 3)
	{
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_3, 1);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 1);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 1);
	}
	if(TargetPot == 4)
	{
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_3, 1);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 1);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 1);
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0);
	}
	SSIDataPut(SSI0_BASE, PotValue);
	while(SSIBusy(SSI0_BASE))
	{
		// wait
	}
}

	//SysCtlDelay(128);
	//GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 256);
	/*
	while(1)
	{

		data = 0;
		for(ulindex = 0; ulindex < NUM_SSI_DATA; ulindex++)
		{
			data = data + 1;
			SSIDataPut(SSI0_BASE, data);
			while(SSIBusy(SSI0_BASE))
			{

			}
		}

	}
	*/


//*****************************************************************************
//
// A simple demonstration of the features of the Stellaris Graphics Library.
//
//*****************************************************************************
int
main(void)
{
    tContext sContext;
    tRectangle sRect;

    //
    // The FPU should be enabled because some compilers will use floating-
    // point registers, even for non-floating-point code.  If the FPU is not
    // enabled this will cause a fault.  This also ensures that floating-
    // correctly and use the hardware floating-point unit.  Finally, lazy
    // stacking is enabled for interrupt handlers.  This allows floating-
    // point instructions to be used within interrupt handlers, but at the
    // expense of extra stack usage.
    //
    FPUEnable();
    FPULazyStackingEnable();

    //
    // Set the clock to 40Mhz derived from the PLL and the external oscillator
    //
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);

    // Reset Button Bindings
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
        GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);

    //
    // Initialize the display driver.
    //
    Kentec320x240x16_SSD2119Init();

    //
    // Initialize the graphics context.
    //
    GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);

    //
    // Fill the top 24 rows of the screen with blue to create the banner.
    //
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 23;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);

    //
    // Put a white box around the banner.
    //
    GrContextForegroundSet(&sContext, ClrWhite);
    GrRectDraw(&sContext, &sRect);

    //
    // Put the application name in the middle of the banner.
    //
    GrContextFontSet(&sContext, &g_sFontCm20);
    GrStringDrawCentered(&sContext, "HiFi Sound System", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 8, 0);

    //
    // Configure and enable uDMA
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    SysCtlDelay(10);
    uDMAControlBaseSet(&sDMAControlTable[0]);
    uDMAEnable();

    //
    // Initialize the touch screen driver and have it route its messages to the
    // widget tree.
    //
    TouchScreenInit();
    TouchScreenCallbackSet(WidgetPointerMessage);

    //
    // Add the title block and the previous and next buttons to the widget
    // tree.
    //
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPrevious);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sTitle);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sNext);

    //
    // Add the first panel to the widget tree.
    //
    g_ulPanel = 0;
    WidgetAdd(WIDGET_ROOT, (tWidget *)g_psPanels);
    CanvasTextSet(&g_sTitle, g_pcPanelNames[0]);

    //
    // Issue the initial paint request to the widgets.
    //
    WidgetPaint(WIDGET_ROOT);

    //
    // Loop forever handling widget messages.
    //
    int ui32Loop = 0;
    static char TempText[5];

    while(1)
    {
        // Process any messages in the widget message queue.
        WidgetMessageQueueProcess();
        //WaitFor(2);
        // Hacky Delay Function
        ui32Loop++;

        if(ui32Loop == 2000000)
        {
        	ui32Loop = 0;
        	TEMP = 25 + (rand() % 25);
			g_psSliders[3].i32Value = TEMP;
			usprintf(TempText, "%3dC", TEMP);
			CanvasTextSet(&g_psPushButtonIndicators[3], TempText);
			if(g_ulPanel == 0)
			{
				//WidgetPaint((tWidget *)&g_psPushButtonIndicators);
				WidgetPaint((tWidget *)&g_psPushButtonIndicators[4]);
				WidgetPaint((tWidget *)&g_psPushButtonIndicators[3]);
			}
			if(g_ulPanel == 1)
			{
				//WidgetPaint((tWidget *)&g_sSliderValueCanvas);
				WidgetPaint((tWidget *)&g_psSliders[3]);
			}
			//SPI(TWEETER);
		}

    }
}
