#include <Arduino.h>
#include <util/delay.h>

void lgt8fx8x_init()
{
	volatile unsigned char btmp = 0;



#if defined(__LGT8FX8E__)
// store ivref calibration 
	GPIOR1 = VCAL1;
	GPIOR2 = VCAL2;

#if defined(__LGT8F_SSOP20__)
	btmp = PMXCR | 0x07;
	PMXCR = 0x80;
	PMXCR = btmp;
// for A0/1 and A4/5
	DIDR0 = 0x33;
#else
// for A0 ~ A5
	DIDR0 = 0x3F;
#endif

// enable 1KB E2PROM 
	ECCR = 0x80;
	ECCR = 0x40;

// clock source settings
	if((VDTCR & 0x0C) == 0x0C)
	{
		// switch to external crystal
		btmp = PMCR | 0x04;
		PMCR = 0x80;
		PMCR = btmp;

		for(btmp=0xf; btmp > 0; --btmp);

		btmp = PMCR | 0x20;
		PMCR = 0x80;
		PMCR = btmp;

		CLKPR = 0x80;
		CLKPR = 0x00;	
	} else {
		CLKPR = 0x80;
		CLKPR = 0x01;	
	}
#else
	CLKPR = 0x80;
	CLKPR = 0x01;
#endif


#if defined(__LGT8FX8E__)
MCUSR|=(1<<SWDD);
MCUSR|=(1<<SWDD);//close swd

DDRE|=(1<<2);//SWD
PORTE|=(1<<2);


#endif
}

int main(void)
{

#if defined(__LGT8F88A__) || defined(__LGT8F88ES__) || defined(__LGT8FX8E__)
	lgt8fx8x_init();
#endif	

	init();

#if defined(USBCON)
	USBDevice.attach();
#endif
	
	setup();
    
	for (;;) {
		loop();
		if (serialEventRun) serialEventRun();
	}
        
	return 0;
}
