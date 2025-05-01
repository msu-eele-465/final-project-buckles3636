//----------------------------------------------------------------------------------------------------------//
// Author: Peter Buckley
// Date: 5/1/25
// Class: EELE-465
// Purpose: To test out reading the encoder seperate from other subsystems
//----------------------------------------------------------------------------------------------------------//

#include "intrinsics.h"
#include <msp430.h>

#define ENCODER_A BIT4      // P1.4
#define ENCODER_B BIT5      // P1.5

volatile int encoder_count = 0;
unsigned char last_state = 0;

void setupUART();
void uart_send_char(char c);
void uart_send_int(int num);
void setupEncoder();

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    setupUART();
    setupEncoder();

    // Startup UART message
    uart_send_char('U');
    uart_send_char('A');
    uart_send_char('R');
    uart_send_char('T');
    uart_send_char(' ');
    uart_send_char('s');
    uart_send_char('t');
    uart_send_char('a');
    uart_send_char('r');
    uart_send_char('t');
    uart_send_char('\r');
    uart_send_char('\n');

    while (1)
    {
        // Read encoder pulses
        unsigned char current_state = P1IN & (ENCODER_A | ENCODER_B);

        if (current_state != last_state)
        {
            // Incriment encoder count based of encoder pulses
            if ((last_state == ENCODER_A) && (current_state == (ENCODER_A | ENCODER_B)))
                encoder_count++; 
            else if ((last_state == ENCODER_B) && (current_state == (ENCODER_A | ENCODER_B)))
                encoder_count--; 

            last_state = current_state;

            // Print encoder count
            uart_send_int(encoder_count);
            uart_send_char('\r');
            uart_send_char('\n');
        }

        __delay_cycles(10);
    }
}

void setupUART(){
    UCA1CTLW0 |= UCSWRST;
    UCA1CTLW0 |= UCSSEL__SMCLK;
    UCA1BRW = 8;
    UCA1MCTLW |= 0xD600;

    P4SEL1 &= ~BIT3;
    P4SEL0 |= BIT3;

    UCA1CTLW0 &= ~UCSWRST;
}

void setupEncoder(){
    P1DIR &= ~(ENCODER_A | ENCODER_B);
    P1REN |= (ENCODER_A | ENCODER_B);
    P1OUT |= (ENCODER_A | ENCODER_B);

    last_state = P1IN & (ENCODER_A | ENCODER_B);
}

void uart_send_char(char c)
{
    while (!(UCA1IFG & UCTXIFG));
    UCA1TXBUF = c;
}

void uart_send_int(int num)
{
    if (num < 0)
    {
        uart_send_char('-');
        num = -num;
    }

    if (num >= 100) uart_send_char('0' + (num / 100) % 10);
    if (num >= 10) uart_send_char('0' + (num / 10) % 10);
    uart_send_char('0' + (num % 10));
}


