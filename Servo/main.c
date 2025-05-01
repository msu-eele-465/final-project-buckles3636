#include "intrinsics.h"
#include <msp430.h>

#define ENCODER_A BIT4
#define ENCODER_B BIT5

#define IN_A BIT0
#define IN_B BIT1

#define PWM_OUT BIT6

#define I2C_ADDR 0x45
#define COUNTS_PER_DEGREE_X10 25  // 3.5 counts per degree * 10 for fixed point math

volatile unsigned char message_buffer[2];
volatile unsigned char msg_index = 0;

volatile int encoder_count = 0;
unsigned char last_state = 0;
int speed = 5;
volatile unsigned int received_degrees = 0;
int target_position = 0;

void setupUART();
void uart_send_char(char c);
void uart_send_int(int num);
void uart_send_string(const char *str);
void setupEncoder();
void setupMotor();
void setMotor(int dir, int pwm);
void i2c_slave_setup();

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    setupUART();
    setupEncoder();
    setupMotor();
    i2c_slave_setup();

    __enable_interrupt();

    uart_send_string("UART ready\r\n");

    while (1)
    {
        // Read encoder
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

        // Motor control
        if (encoder_count < target_position - 2)
        {
            setMotor(1, speed);
        }
        else if (encoder_count > target_position + 2)
        {
            setMotor(-1, speed);
        }
        else
        {
            setMotor(0, 0);
        }
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

void i2c_slave_setup() {
    UCB1CTLW0 |= UCSWRST;

    P4SEL1 &= ~(BIT6 | BIT7);
    P4SEL0 |= (BIT6 | BIT7);

    UCB1CTLW0 = UCSWRST | UCMODE_3 | UCSYNC;
    UCB1CTLW0 &= ~UCMST;
    UCB1I2COA0 = I2C_ADDR | UCOAEN;
    UCB1CTLW1 = 0;

    UCB1CTLW0 &= ~UCSWRST;
    UCB1IE |= UCRXIE0 | UCSTPIE;
}

#pragma vector=EUSCI_B1_VECTOR
__interrupt void I2C_ISR(void) {
    switch (__even_in_range(UCB1IV, USCI_I2C_UCBIT9IFG)) {
        case USCI_I2C_UCRXIFG0:
            message_buffer[msg_index++] = UCB1RXBUF;
            break;

        case USCI_I2C_UCSTPIFG:
            if (msg_index == 2) {
                received_degrees = (message_buffer[1] & 0xFF) | (message_buffer[0] << 8);
                target_position = (received_degrees * COUNTS_PER_DEGREE_X10) / 10;
            }
            msg_index = 0;
            break;

        default: break;
    }
}

void setupEncoder(){
    P1DIR &= ~(ENCODER_A | ENCODER_B);
    P1REN |= (ENCODER_A | ENCODER_B);
    P1OUT |= (ENCODER_A | ENCODER_B);

    last_state = P1IN & (ENCODER_A | ENCODER_B);
}

void setupMotor(){
    P1DIR |= PWM_OUT;
    P1SEL1 |= PWM_OUT;
    P1SEL0 &= ~PWM_OUT;

    TB0CCR0 = 100-1;
    TB0CCTL1 = OUTMOD_7;
    TB0CCR1 = 0;
    TB0CTL = TBSSEL_1 | MC_1 | TBCLR;

    P4DIR |= (IN_A | IN_B);
    P4OUT &= ~(IN_A | IN_B);
}

void setMotor(int dir, int pwm)
{
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

    if (num >= 10000) uart_send_char('0' + (num / 10000) % 10);
    if (num >= 1000) uart_send_char('0' + (num / 1000) % 10);
    if (num >= 100) uart_send_char('0' + (num / 100) % 10);
    if (num >= 10) uart_send_char('0' + (num / 10) % 10);
    uart_send_char('0' + (num % 10));
}

void uart_send_string(const char *str)
{
    while (*str)
        uart_send_char(*str++);
}
