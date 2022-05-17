/*
 * File:   lab12.c
 * Author: sando
 *
 * Created on 16 de mayo de 2022, 01:06 PM
 */
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#include <xc.h>
#include <stdint.h>

/*
 * CONSTANTES
 */
#define _XTAL_FREQ 4000000

/*
 * VARIABLES
 */

/*
 * PROTOTIPOS DE FUNCIONES
 */
void setup (void);

/*
 * INTERRUPCIONES
 */
void __interrupt() isr (void){
    
    if (INTCONbits.RBIF){
        if (PORTBbits.RB0 == 0)
            if(PIR1bits.ADIF){      //Fue interrupción del ADC?
                if(ADCON0bits.CHS == 0) //verificamos sea AN0 el canal seleccionado
                    PORTC = ADRESH;     //mostramos ADRESH en PORTC

        PIR1bits.ADIF = 0;      //Limpiamos bandera de interrupción
    }
        else if (PORTBbits.RB0 == 1)
            SLEEP();
        INTCONbits.RBIF = 0;
    }
    return;
}

/*
 * CICLO PRINCIPAL
 */
void main (void){
    setup();
    while(1){
        if(ADCON0bits.GO == 0){ //No hay proceso de conversión
          
            ADCON0bits.GO = 1;
        }
    }
    return;
}

/*
 * CONFIGURACIONES
 */
void setup(void){
    ANSEL = 0b00000001;  //AN0 como entrada analógica
    ANSELH = 0;         //I/O digitales
    
    TRISA = 0b00000001;     // AN0 como entrada
    PORTA = 0;
    
    TRISB = 0b00000001; //RB0 para habilitar el sleep
    PORTB = 0;
    
    TRISC = 0;
    PORTC = 0;

    //Configuración push button
    OPTION_REGbits.nRBPU = 0;
    WPUB = 0b00000001;
    IOCB = 0b00000001;
    
    INTCONbits.RBIF = 0;
    INTCONbits.RBIE = 1;
    INTCONbits.GIE = 1;
    
    //Configuración reloj interno
    OSCCONbits.IRCF = 0b0110; //4MHz
    OSCCONbits.SCS = 1;         //Oscilador interno
    
    //Configuración del ADC
    ADCON0bits.ADCS = 0b01;     //Fosc/8
    ADCON1bits.VCFG0 = 0;       //VDD 'Referencias internas
    ADCON1bits.VCFG1 = 0;       // voltaje de referencia
    
    ADCON0bits.CHS = 0b0000;     //Seleccionamos AN0
    ADCON1bits.ADFM = 0;         //Justificamos a la izquierda
    ADCON0bits.ADON = 1;         //Habilitamos modulo ADC
    __delay_ms(40);
    
    // Configuración de interrupciones
    PIR1bits.ADIF = 0;          //Limpiamos bandera de int. ADC
    PIE1bits.ADIE = 1;          //Habilitamos int. de ADC
    INTCONbits.PEIE = 1;         //Habilitamos int. de periféricos
    INTCONbits.GIE = 1;         //Habilitamos int. globales
    
    
   return; 
    
}