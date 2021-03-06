/*
  PMU.cpp - PMU library
  Copyright (c) 2014 LogicGreen  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "PMU.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * Constructors
 ******************************************************************************/

/******************************************************************************
 * User API
 ******************************************************************************/
void PowerControl::sleep(pmu_t mode, period_t period)
{
	uint8_t btmp;

	// save status
	pmcr_reg = PMCR;
	clkpr_reg = CLKPR;
	ldocr_reg = LDOCR;
	wdtcr_reg = WDTCSR;

	sreg_reg = SREG;
	cli();

	// switch clock to rc32k if necessary
	clock_switch(mode);

	// enable wdt for period sleep mode
	if(period != SLEEP_FOREVER)
		enable_wdt(period);

	// bring mcu to sleep mode
	do_sleep(mode);

	// restore setting after wakeup
	resume(mode, period);	

	// restore GIE
	SREG = sreg_reg;
}

// enable wdt for periodly wake-up mode
void PowerControl::enable_wdt(period_t period)
{
	uint8_t btmp;

	asm ("wdr");

	// disable WDE
	btmp = (WDTCSR | 0x90) & 0xF7;
	WDTCSR = 0x18 | btmp;
	WDTCSR = btmp;

	// switch wdkclk to 32k
	btmp = PMCR | 0x90;
	PMCR = 0x80;
	PMCR = btmp;

	// enable wdt
	btmp = period & 0x7;	// WPD[2:0]
	if(period > SLEEP_8S)
		btmp |= 0x20;	// WPD[3]

	btmp |= 0x40;	// WDIE enable
	btmp |= 0x10;	// WDCE enable
	WDTCSR = 0x18 | btmp;
	WDTCSR = btmp;
}

// switch main clock to rc32k
void PowerControl::clock_switch(pmu_t mode)
{
	uint8_t btmp;

	if(mode > PM_IDLE) {
		// switch main clock to rc32k
		btmp = (PMCR & 0x1F) | 0x50;
		PMCR = 0x80;
		PMCR = btmp;
		asm ("nop");
		asm ("nop");

		// w/o prescale
		CLKPR = 0x80;
		CLKPR = 0x00;

		LDOCR = 0x80;
		LDOCR = 0x05;
	}
}

// set power mode
// bring mcu to sleep mode
void PowerControl::do_sleep(pmu_t mode)
{
	SMCR = (mode << 1) | 0x1;

	asm volatile (
		"sleep	\n\t"
		"nop	\n\t"
		"nop	\t\t"
		::);
}

// resume system setings after waked up
void PowerControl::resume(pmu_t mode, period_t period)
{
	uint8_t btmp;

	// optional for power lock
	SMCR = 0x08;

	if(mode > PM_IDLE) {
		// LDO recovery
		LDOCR = 0x80;
		LDOCR = ldocr_reg;

		// CLKPR recovery 
		CLKPR = 0x80;
		CLKPR = clkpr_reg;

		// now is working by 32k clock
		// waiting for crystal stable if needed
		if((pmcr_reg & 0x20) == 0x20)
			for(btmp=0x1f; btmp>0; --btmp)
				asm("nop");

		// restore system clock settings
		PMCR = 0x80;
		PMCR = pmcr_reg;
	}

	// restore WDT in case of period mode
	if(period != SLEEP_FOREVER) {
		btmp = wdtcr_reg | 0x10;
		WDTCSR = btmp;
		WDTCSR = btmp;
	}
}

// pullup float pin to save power
void PowerControl::pullupUnusedPin()
{
	sreg_reg = SREG;
	cli();

	// disable SWD interface
	// to release PE0/2
	MCUSR = 0x80;
	MCUSR = 0x80;

	// set PE0/2 to input and pullup
	DDRE &= 0xFA;
	PORTE |= 0x05;

	// restore status
	SREG = sreg_reg;
}

ISR(WDT_vect)
{
	// disable WDE
}

PowerControl PMU;
