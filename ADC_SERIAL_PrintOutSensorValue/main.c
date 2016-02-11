#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "uart.h"

#define EN_SENSOR_PORT_DIR          DDRC
#define EN_SENSOR_STATE             PORTC
#define EN_SENSOR_PIN               PC0

#define SENSOR_PORT_DIR             DDRC
#define SENSOR_STATE                PORTC
#define SENSOR_PIN                  PC4     // ADC4

#define LED1_PORT_DIR               DDRC
#define LED1_STATE                  PORTC
#define LED1_PIN                    PC1

#define LED2_PORT_DIR               DDRC
#define LED2_STATE                  PORTC
#define LED2_PIN                    PC2


volatile uint16_t ADC_result[100];
volatile uint8_t ADC_result_index=0;

volatile uint8_t Timer0_trigger=0;

volatile uint8_t Timer0_res=0;
volatile uint8_t Timer0_prevRes=0;

volatile uint8_t timer0_ovflag_cnt = 0;

void initIO(void) {
	LED1_PORT_DIR |= (1 << LED1_PIN);  // 1 : sortie
	LED2_PORT_DIR |= (1 << LED2_PIN);  // 1 : sortie
    
    LED1_STATE |= (1 << LED1_PIN);        // 1 : OFF
    LED2_STATE |= (1 << LED2_PIN);        // 1 : OFF
    
    EN_SENSOR_PORT_DIR |= (1 << EN_SENSOR_PIN);   // 1 : sortie
    EN_SENSOR_STATE |= (1 << EN_SENSOR_PIN);      // 1 : OFF	
}

//-----------------------------------------------------------------------
//                              ADC config
//-----------------------------------------------------------------------

inline void initADC(void){
    ADMUX |= (1 << REFS0) | (1 << REFS1) | (1 << MUX2); // internal ref : 2.56V and ADC4 input selected
    //ADCSRA |= (1 << ADPS1); // ADPS2 = 0 ; ADPS1 = 1 ; ADPS0 = 0 : prescaler = 4 ==> Fcan = 250khz
    ADCSRA |= (1 << ADFR);  // Free Running mode
    ADCSRA |= (1 << ADIE);    // Enable ADC interrupt
}

inline void startADCconversion(void){
    ADCSRA |= (1 << ADEN); // enable ADC
    ADCSRA |= (1 << ADSC); // Start conversion
}

inline void stopADCconversion(void){
    ADCSRA &=~ (1<<ADEN); //stop ADC
}

//-----------------------------------------------------------------------
//                            Compter0 config
//-----------------------------------------------------------------------

inline void startCOMPTER0(void){
    TCCR0 |= (1<<CS00); //prescaler = 1 Ftimer = 1Mhz
    TCNT0 = 0x00;       //clear compter
}
inline void stopCOMPTER0(void){
    TCCR0 &=~  (1<<CS00) | (1<<CS01) | (1<<CS02);
}


//-----------------------------------------------------------------------
//                            Timer0 config
//-----------------------------------------------------------------------

inline void startTimer0(void){
    TCNT0 = 254; // config timer0 pour compter 54us
    TIMSK |= (1<<TOIE0); // enable interrupt
    TCCR0 = 0x01; //prescaler = 1 :: Ftimer = 1Mhz
}

inline void stopTimer0(void){
    timer0_ovflag_cnt = 0;
    TIMSK &=~ (1<<TOIE0); // disable interrupt
    TCCR0 &=~  (1<<CS00) | (1<<CS01) | (1<<CS02);
}

//-----------------------------------------------------------------------
//                            Sensor config
//-----------------------------------------------------------------------

inline void enableSensor(void){
    EN_SENSOR_STATE &=~ (1 << EN_SENSOR_PIN); // 0 : ON
}

inline void disableSensor(void){
    EN_SENSOR_STATE |= (1 << EN_SENSOR_PIN); // 1 : OFF
}

ISR(TIMER0_OVF_vect){

    stopTimer0();
}


ISR (ADC_vect){
    
    ADC_result[ADC_result_index] = ADCW; //store result
    ADC_result_index++;

}

int main(void) {
	
    initIO();
    initADC();
    uart_init();
    
    enableSensor();
    startADCconversion();
    
    while (ADC_result_index<100) {
	}
    
    stopADCconversion();
    ADC_result_index = 0;
    char buffer[10];
    
    while (ADC_result_index<100) {
        uart_puts(itoa(ADC_result[ADC_result_index], buffer, 10));
        uart_putc('\n');
        ADC_result_index++;
    }
    
	return 0; // never reached
}
