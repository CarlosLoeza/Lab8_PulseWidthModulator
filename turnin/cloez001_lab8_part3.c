#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif



volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0;
unsigned char button = 0;
unsigned char i=0;

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1;    // Start counter from here to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal clock of 1ms ticks


void set_PWM(double frequency) {
    static double current_frequency;

    if(frequency != current_frequency){
    if(!frequency) {TCCR3B &= 0x08;}
    else{TCCR3B |= 0x03;}

    if(frequency < 0.954) {OCR3A = 0xFFFF;}
    else if(frequency > 31250) {OCR3A = 0x0000;}
    else{OCR3A = (short)(8000000/(128*frequency)) - 1;}
    TCNT3 = 0;
    current_frequency = frequency;
    }
}

void PWM_on(){
    TCCR3A = (1 << COM3A0);
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);

    set_PWM(0);
}

void PWM_off(){
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}


void TimerOn(){
    // AVR timer/counter controller register TCCR1
    TCCR1B = 0x0B;    // bit3 = 0: CTC mode(clear timer on compare)
            // bit2bit1bit0 = 011;
            // 0000 1011 : 0x0B
            // So 8 MHz clock or 8,000,000 / 64  = 125,000 ticks
            // Thus TCNT1 register will count 125,000 ticks

    // AVR out compare register OCR1A
    OCR1A = 125;    // Timer interrupt will be generated when TCNT1==OCR1A
            // We want a 1ms tick. .001s * 125,000 ticks = 125
            // So when TCNT1 == 125 then that means 1ms has passed

    // AVR timer register interrupt mask register
    TIMSK1 = 0x02;    // bit1: OCIE1A -- enables compare match register

    // Initiliaze AVR counter
    TCNT1 = 0;

    _avr_timer_cntcurr = _avr_timer_M;

    // enable global interrupts
    SREG |= 0x80;    // 1000 0000
}

void TimerOff(){
    TCCR1B = 0x00;
}

void TimerISR(){
    TimerFlag = 1;
}

// In our approach C program does not touch this ISR, only TimerISR()
ISR(TIMER1_COMPA_vect){
    // CPU automatically calls when TCNT1 == OCR1 (Every 1ms per TimerOn settings)
    _avr_timer_cntcurr--;    // count down to 0
    if (_avr_timer_cntcurr == 0){
    TimerISR();    // call the ISR that the user uses
    _avr_timer_cntcurr = _avr_timer_M;
    }
}

// Set timer to tick every M ms.
void TimerSet(unsigned long M){
    _avr_timer_M = M;
    _avr_timer_cntcurr =  _avr_timer_M;
}



enum Power_States {Power_Off, Power_On_Held,Power_On} Power_State;
enum Sound_States {Sound_Wait, Sound_One, Sound_Two, Sound_Three, Sound_Four, Sound_Five} Sound_State;

unsigned char power_status;

void Power(){
    
    switch(Power_State){
        case Power_Off:
            if(button == 1)
                Power_State = Power_On_Held;
            else if(button == 0)
                Power_State = Power_Off;
            break;
            
        case Power_On_Held:
            if(button == 1)
                Power_State = Power_On_Held;
            else if(!button)
                Power_State = Power_On;
        case Power_On:
            if(i<5){
                Power_State = Power_On;
            }
            else if (!(i<5)){
                Power_State = Power_Off;
            }
            break;

                
    }
    
    switch(Power_State){
        case Power_Off:
            i =0;
            power_status =0;
            set_PWM(0);
            break;
        case Power_On:
            power_status = 1;
            i++;
            break;
    }
}


void Music_Beat(){
    
    switch(Sound_State){
        case Sound_Wait:
            if(power_status)
                Sound_State = Sound_One;
            else if (power_status)
                Sound_State = Sound_Wait;
            break;
        case Sound_One:
            Sound_State = Sound_Two;
            break;
        case Sound_Two:
            Sound_State = Sound_Three;
            break;
        
        case Sound_Three:
            Sound_State = Sound_Four;
            break;
        
        case Sound_Four:
            Sound_State = Sound_Five;
            break;
            
        case Sound_Five:
            Sound_State = Sound_Wait;
            break;
        default:
            Sound_State = Sound_Wait;
            break;
    }
    
    switch(Sound_State){
	case Sound_Wait:
	    set_PWM(0);
	    break;
        case Sound_One:
            set_PWM(261.63);
            break;
        case Sound_Two:
            set_PWM(293.66);
            break;
        
        case Sound_Three:
            set_PWM(261.63);
            break;
        
        case Sound_Four:
            set_PWM(329.6);
            break;
            
        case Sound_Five:
            set_PWM(329.6);
            break;
    }
    
    
}



int main(void){
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;

        
    PWM_on();
    set_PWM(0);
    Sound_State = Sound_Wait;

    TimerSet(1000);
    TimerOn();
    while (1)
        {
            button = ~PINA & 0x0F;
	    Power();
            Music_Beat();

            while(!TimerFlag); // wait 1 sec
                TimerFlag = 0;

        }
}

