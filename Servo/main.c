#include <msp430.h>

// Encoder inputs (P1)
#define ENCODER_A BIT4      // P1.4
#define ENCODER_B BIT5      // P1.5

// Motor control outputs (P4)
#define IN_A BIT0           // P4.0
#define IN_B BIT1           // P4.1

// PWM output (P1)
#define PWM_OUT BIT6        // P1.6

// Encoder parameters
#define COUNTS_PER_REV 28   // 7 PPR quadrature -> 28 counts per revolution

volatile int encoder_count = 0;
volatile unsigned char last_encoder_state = 0;

int pwm_duty = 0;
int target_counts = 0;
int target_degrees = 360;

void setupPWM();
void setPWM(int duty);
void setupEncoder();
void setupMotorPins();
void delay_ms(unsigned int ms);
void rotate(int direction, int counts);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // ==== UNLOCK GPIO (FR2355 requires this after power-up) ====
    PM5CTL0 &= ~LOCKLPM5;

    setupPWM();
    setupEncoder();
    setupMotorPins();

    __enable_interrupt();

    // Calculate target counts from degrees
    target_counts = (COUNTS_PER_REV * target_degrees) / 360;

    while (1)
    {
        // Rotate forward
        rotate(1, target_counts);

        // Rotate backward
        rotate(-1, target_counts);
    }
}

//---------------------------
// Rotate specified counts
//---------------------------
void rotate(int direction, int counts)
{
    encoder_count = 0;

    // Set motor direction
    if (direction > 0) {
        P4OUT &= ~IN_A;
        P4OUT |= IN_B;
    } else {
        P4OUT |= IN_A;
        P4OUT &= ~IN_B;
    }

    // Ramp up PWM
    for (pwm_duty = 0; pwm_duty <= 40; pwm_duty++)  
    {
        setPWM(pwm_duty);
        delay_ms(50);
    }

    // Run until target counts reached
    if (direction > 0) {
        while (encoder_count < counts);
    } else {
        while (encoder_count > -counts);
    }

    // Ramp down PWM
    for (pwm_duty = 40; pwm_duty >= 0; pwm_duty--)  
    {
        setPWM(pwm_duty);
        delay_ms(50);
    }
}

//---------------------------
// PWM Setup (TB0 / P1.6)
//---------------------------
void setupPWM(){
    TB0CCR0 = 100-1;                  // PWM period
    TB0CCTL1 = OUTMOD_7;              // Reset/set mode
    TB0CCR1 = 0;                      // Duty initially 0
    TB0CTL = TBSSEL_1 | MC_1 | TBCLR; // ACLK, up mode

    P1DIR |= PWM_OUT;
    P1SEL1 |= PWM_OUT;
    P1SEL0 &= ~PWM_OUT;
}

void setPWM(int duty){
    if (duty < 0) duty = 0;
    if (duty > 100) duty = 100;
    TB0CCR1 = duty;
}

//---------------------------
// Encoder input + interrupt setup
//---------------------------
void setupEncoder(){
    P1DIR &= ~(ENCODER_A | ENCODER_B);
    P1REN |= (ENCODER_A | ENCODER_B);
    P1OUT |= (ENCODER_A | ENCODER_B);

    P1IES ^= (ENCODER_A | ENCODER_B); // Interrupt on both edges
    P1IE |= (ENCODER_A | ENCODER_B);
    P1IFG &= ~(ENCODER_A | ENCODER_B);

    last_encoder_state = P1IN & (ENCODER_A | ENCODER_B);
}

//---------------------------
// Motor driver direction control pins
//---------------------------
void setupMotorPins(){
    P4DIR |= (IN_A | IN_B);
    P4OUT &= ~(IN_A | IN_B);
}

//---------------------------
// Delay
//---------------------------
void delay_ms(unsigned int ms){
    while (ms--)
        __delay_cycles(1000);
}

//---------------------------
// Encoder ISR (Quadrature decode)
//---------------------------
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    unsigned char current_state = P1IN & (ENCODER_A | ENCODER_B);
    unsigned char combined = (last_encoder_state << 2) | current_state;

    switch (combined) {
        case 0b0001:
        case 0b0111:
        case 0b1110:
        case 0b1000:
            encoder_count++;
            break;

        case 0b0010:
        case 0b0100:
        case 0b1101:
        case 0b1011:
            encoder_count--;
            break;

        default:
            break;
    }

    last_encoder_state = current_state;

    if (P1IFG & ENCODER_A) P1IES ^= ENCODER_A;
    if (P1IFG & ENCODER_B) P1IES ^= ENCODER_B;

    P1IFG &= ~(ENCODER_A | ENCODER_B);
}
