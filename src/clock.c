#include <msp430.h>
#include <stdint.h>
#include "clock.h"

void SetVCoreUp (unsigned int level)
{

    PMMCTL0_H = 0xA5;                       // Open PMM registers for write access

//    while ((PMMIFG & SVSMHDLYIFG) == 0);    // Make sure no flags are set for
//    while ((PMMIFG & SVSMLDLYIFG) == 0);    // iterative sequences

    SVSMHCTL =                              // Set SVS/SVM high side new level
            SVSHE            +
            SVSHRVL0 * level +
            SVMHE            +
            SVSMHRRL0 * level;

    SVSMLCTL =                              // Set SVM low side to new level
            SVSLE            +
//          SVSLRVL0 * level +              // but not SVS, not yet..
            SVMLE            +
            SVSMLRRL0 * level;

    while ((PMMIFG & SVSMLDLYIFG) == 0);    // Wait till SVM is settled

    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);      // Clear already set flags

    PMMCTL0_L = PMMCOREV0 * level;          // Set VCore to new level

    if ((PMMIFG & SVMLIFG))                 // Wait till new level reached
        while ((PMMIFG & SVMLVLRIFG) == 0);

    SVSMLCTL =                              // Set SVS/SVM low side to new level
            SVSLE            +
            SVSLRVL0 * level +
            SVMLE            +
            SVSMLRRL0 * level;

    PMMCTL0_H = 0x00;                       // Lock PMM registers for write access
}


void clockInit () {
    SetVCoreUp(1);
    SetVCoreUp(2);

    // Configure crystal ports
    P5SEL |= BIT2 | BIT3 | BIT4 | BIT5;     // Configure P5 to use Crystals

    UCSCTL6 = XT2DRIVE_3      |             // XT2 and XT1 drive can be
              XT1DRIVE_3      |             // lowered to 0 (default is 3)
              XCAP_3          |             // This is the default (3).
//            XT1OFF          |             // Turns off XT1 when set
//            XT2OFF          |             // Turns off XT2 when set
//            XT1BYPASS       |             // Not using external clock source
//            XT2BYPASS       |             // Not using external clock source
//            XTS                           //
              0;

    UCSCTL0 = 0x00;                         // Let FLL manage DCO and MOD

    UCSCTL1 = DCORSEL_1       |             // Select DCO range to 1MHz
//            DISMOD          |             // Enable modulator
              0;

    UCSCTL2 = FLLD__16        |             // Set FLL dividers
              FLLN(32);

    UCSCTL3 = SELREF__XT1CLK  |             // Use Crystal 1 Oscillator divided
              FLLREFDIV__16;                // by 16 for better precision

    UCSCTL5 = DIVPA__1        |             // Output dividers to 1
              DIVA__1         |             // ACLK  divider 1
              DIVS__1         |             // SMCLK divider 4
              DIVM__1;                      // MCLK  divider 1

    UCSCTL7 = 0;                            // Clear XT2,XT1,DCO fault flags

    UCSCTL8 = SMCLKREQEN      |             // Enable conditional requests for
               MCLKREQEN      |             // SMCLK, MCLK and ACLK
               ACLKREQEN;

    do {                                    // Check if all clocks are oscillating
      UCSCTL7 &= ~( XT2OFFG   |             // Clear XT2,XT1,DCO fault flags
                    XT1LFOFFG |
                    DCOFFG);
      SFRIFG1 &= ~OFIFG;                    // Clear fault flags
    } while (SFRIFG1 & OFIFG);              // Test oscillator fault flag

    UCSCTL6 &= ~(XT1DRIVE_3   |             // Xtal is now stable,
                 XT2DRIVE_3);               // reduce drive strength (to save power)

    UCSCTL4 = SELA__XT1CLK    |             // ACLK  = Crystal 1  =>     32.768 Hz
              SELS__XT1CLK    |             // SMCLK = Crystal 1  =>     32.768 Hz
              SELM__XT2CLK;                 // MCLK  = Crystal 2  =>  1.000.000 Hz
}

