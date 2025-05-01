//----------------------------------------------------------------------------------------------------------//
// Author: Peter Buckley
// Date: 5/1/25
// Class: EELE-465
// Purpose: To test out closed loop feedback with the motor seperate from other functionality
//----------------------------------------------------------------------------------------------------------//

#include <msp430.h>

#define ENCODER_A BIT4      // P1.4 C1
#define ENCODER_B BIT5      // P1.5 C2

#define IN_A BIT0           // P4.0 INA
#define IN_B BIT1           // P4.1 INB

#define PWM_OUT BIT6        // P1.6 PWM

volatile int encoder_count = 0;
unsigned char last_state = 0;
int speed = 10;

void setupUART();
void uart_send_char(char c);
void uart_send_int(int num);
void setupEncoder();
void setupMotor();
void setMotor(int dir, int pwm);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    setupUART();
    setupEncoder();
    setupMotor();

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
        // Read Encoder 
        unsigned char current_state = P1IN & (ENCODER_A | ENCODER_B);

        if (current_state != last_state)
        {
            if ((last_state == ENCODER_A) && (current_state == (ENCODER_A | ENCODER_B)))
                encoder_count++; 
            else if ((last_state == ENCODER_B) && (current_state == (ENCODER_A | ENCODER_B)))
                encoder_count--; 

            last_state = current_state;

            uart_send_int(encoder_count);
            uart_send_char('\r');
            uart_send_char('\n');
        }

        // Motor Control 
        if (encoder_count < 1000)
        {
            // Clockwise
            setMotor(1, speed);
        }
        else if (encoder_count >= 1000)
        {
            // Stop
            setMotor(0, 0);
            __delay_cycles(10);

            // Now go backward to 0
            while (encoder_count > 0)
            {
                // Read encoder during backward
                current_state = P1IN & (ENCODER_A | ENCODER_B);

                if (current_state != last_state)
                {
                    if ((last_state == ENCODER_A) && (current_state == (ENCODER_A | ENCODER_B)))
                        encoder_count++; 
                    else if ((last_state == ENCODER_B) && (current_state == (ENCODER_A | ENCODER_B)))
                        encoder_count--; 

                    last_state = current_state;

                    uart_send_int(encoder_count);
                    uart_send_char('\r');
                    uart_send_char('\n');
                }

                // Backward
                if (encoder_count > 0)
                    setMotor(-1, speed);
                else
                    setMotor(0, 0);

                __delay_cycles(10);
            }

            __delay_cycles(10);  // Pause before starting over
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

void setupMotor(){
    // PWM setup P1.6
    P1DIR |= PWM_OUT;
    P1SEL1 |= PWM_OUT;
    P1SEL0 &= ~PWM_OUT;

    TB0CCR0 = 100-1;
    TB0CCTL1 = OUTMOD_7;
    TB0CCR1 = 0;
    TB0CTL = TBSSEL_1 | MC_1 | TBCLR;

    // Direction pins P4.0 / P4.1
    P4DIR |= (IN_A | IN_B);
    P4OUT &= ~(IN_A | IN_B);
}

void setMotor(int dir, int pwm)
{
    // Direction
    if (dir > 0)
    {
        P4OUT &= ~IN_A;
        P4OUT |= IN_B;
    }
    else if (dir < 0)
    {
        P4OUT |= IN_A;
        P4OUT &= ~IN_B;
    }
    else
    {
        P4OUT &= ~(IN_A | IN_B);
    }

    // PWM Duty
    TB0CCR1 = pwm;
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
