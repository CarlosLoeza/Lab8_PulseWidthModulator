/*    Author: lab
 *  Partner(s) Name:
 *    Lab Section:
 *    Assignment: Lab #  Exercise #
 *    Exercise Description: [optional - include for your own benefit]
 *  demo: 
 *    I acknowledge all content contained herein, excluding template or example
 *    code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


enum PWM_States{PW_Start, PW_OnOff, PW_Hold}PW_State;


unsigned char button; // on/off button
unsigned char count;
double sound_note;
unsigned char on_off_counter;
unsigned char button_inc_dec; // increment/decrement button

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

void On_Off_Sound(){
    switch (PW_State){
    
        case PW_Start:
            PW_State = PW_OnOff;
            break;
        case PW_OnOff:
            if(button == 0x01 && on_off_counter ==0){
                //set_PWM(261.63);
                PW_State = PW_Hold;
                on_off_counter =1;
                set_PWM(sound_note);
                
            } else if(button == 0x01 && on_off_counter == 1){
                PW_State = PW_Hold;
                on_off_counter = 0;
                set_PWM(0);
            } else if(button == 0)
		PW_State = PW_OnOff;
            break;

            
        case PW_Hold:
            while(button) 
            {
                button = ~PINA & 0x0F;
            }

            PW_State = PW_OnOff;
            break;
                
        default:
            PW_State = PW_OnOff;
            break;
        }
    
    
    switch(PW_State){
        case PW_OnOff:
            if(on_off_counter == 1){
            //set_PWM(261.63);
                if(count == 0)
            	    sound_note = 261.63;
            	else if(count == 1)
                    sound_note = 293.66;
            	else if(count == 2)
                    sound_note = 329.6;
            	else if(count == 3)
                    sound_note = 349.23;
            	else if(count ==4)
                    sound_note =392;
            	else if(count ==5)
                    sound_note = 440;
            	else if(count ==6)
                    sound_note = 493.88;
            	else if(count ==7)
                    sound_note =523.25;
            }
            break;
            
    }
}


enum Count_States {Count_Start, Count_Wait, Count_Up, Count_Up_Wait, Count_Down, Count_Down_Wait, Count_Zero, Count_Reset} Count_State;

void Increment_Decrement(){

    switch(Count_State){
    case Count_Start:
        Count_State = Count_Wait;
        break;
    case Count_Wait:
        if(button_inc_dec == 0){
            Count_State = Count_Wait;
        } else if(button_inc_dec == 0x02){
            Count_State = Count_Up;
        } else if(button_inc_dec == 0x04){
            Count_State = Count_Down;
        }
        break;
    case Count_Up:
        if(button_inc_dec == 0x02){
            Count_State = Count_Up_Wait;
        } else if (button_inc_dec == 0x04){
            Count_State = Count_Down;
        } else if(button_inc_dec == 0){
            Count_State = Count_Wait;
        }
        break;
    case Count_Up_Wait:
        if(button_inc_dec == 0){
            Count_State = Count_Wait;
        }
        else if(button_inc_dec == 0x02){
            Count_State = Count_Up_Wait;
        }else if (button_inc_dec == 0x04){
            Count_State = Count_Down;
        }
        break;
    case Count_Down:
        if(button_inc_dec == 0x02){
            Count_State = Count_Up;
        } else if(button_inc_dec == 0x04){
            Count_State = Count_Down_Wait;
        } else if(button_inc_dec == 0){
        Count_State = Count_Wait;
        }
          break;
    case Count_Down_Wait:
        if(button_inc_dec ==0){
            Count_State = Count_Wait;
        }
        else if(button_inc_dec == 0x02){
                Count_State = Count_Up;
            } else if (button_inc_dec == 0x04){
                Count_State = Count_Down_Wait;
            }
        break;
    default:
        Count_State = Count_State;
        break;
    }

    switch(Count_State){
    case Count_Up:
        if(count <9){
            count = count+1;
	    set_PWM(sound_note)
        }
        break;
    case Count_Down:
        if(count > 0){
            count = count-1;
	    set_PWM(sound_note);
        }
        break;
    }

    //PORTC = count;
}


 

int main(void)
{
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;

    
    PWM_on();
    set_PWM(0);
    sound_note = 261.63;
    PW_State = PW_Start;
    Count_State = Count_Start;
    on_off_counter = 0;
    while (1)
    {
        button = ~PINA & 0x0F;
	button_inc_dec = ~PINA & 0x0F;
        On_Off_Sound();
	Increment_Decrement();
    }
}



