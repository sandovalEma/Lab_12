/*
 * File:   prueba2.c
 * Author: sando
 *
 * Created on 16 de mayo de 2022, 06:14 PM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIOoscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and canbe enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pinfunction is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory codeprotection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory codeprotection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit(Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-SafeClock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin hasdigital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Resetset to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits(Write protection off)
// #pragma config statements should precede project file includes.

// Use project enums instead of #define for ON and OFF.
#include <xc.h>
#include <stdint.h>


// ---------- CONSTANTES ------------ //
#define _XTAL_FREQ 1000000

// --------- VARIABLES ---------- //
uint8_t address = 0;
uint8_t sleep = 0;
uint8_t potenciometro = 0;

// --------- PROTOTIPOS DE FUNCIONES ----------- //
void setup (void);
uint8_t read_EEPROM(uint8_t address);
void write_EEPROM(uint8_t address, uint8_t data);

// ---------- INTERRUPCIONES ---------- //
void __interrupt() isr (void){
    
    if(PIR1bits.ADIF){                  // Es interrupcion por ADC
        potenciometro = ADRESH;         // Mostramos ADC en variable potenciometro
        PORTC = potenciometro;          // Mostramos potenciometro en PORTC
        PIR1bits.ADIF = 0;              // Limpiamos bandera de interrupcion
    }
    else if(INTCONbits.RBIF){           // Es interrupción del PORTB        
        if(!PORTBbits.RB0){             // Si no es en RB0                 
            if(sleep == 1){             // Y si Sleep esta en 1
                sleep = 0;              // apagamos la bandera
            }
            else{     
                sleep = 1;}             // si no, prendemos la bandera
            }
        if(!PORTBbits.RB3){             // Si no es en RB3       
           write_EEPROM(address, potenciometro);  // Se escriben los valores en memoria
        }
        INTCONbits.RBIF = 0;            // Limpiamos bandera      
    }
    return;
}

// ---------- CICLO PRINCIPAL ---------- //
void main (void){
    setup();
    while(1){
        if(ADCON0bits.GO == 0){         // Si no hay proceso de conversión
            ADCON0bits.GO = 1;          // iniciar conversion
        }
        if(sleep == 1){                 // Si esta en modo sleep     
            PIE1bits.ADIE = 0;          // apagar interrupcion
            SLEEP();                
        }
        else if(sleep == 0){            // Si no esta en modo ON          
            PIE1bits.ADIE = 1;          // habilitar interrupcion
        }
        PORTD = read_EEPROM(address);  // Se escribe los valores de dirección de memoria al PORTD
        __delay_ms(500);
    }
    return;
}

// ---------- CONFIGURACIONES --------- //
void setup(void){
    ANSEL = 0b00000001;     //AN0 como entrada analógica
    ANSELH = 0;             //I/O digitales
    
    TRISA = 0b00000001;     // AN0 como entrada
    PORTA = 0;              // CLEAR PORTA
    TRISB = 0b00001001;     // RB0 para habilitar el sleep, RB3 para EEPROM
    PORTB = 0;              // CLEAR PORTA
    TRISC = 0;              // VALOR DE POTENCIOMETRO
    PORTC = 0;              // CLEAR PORTA
    TRISD = 0;              // VALOR DE EEPROM
    PORTD = 0;              // CLEAR PORTA

    //Configuración push button
    OPTION_REGbits.nRBPU = 0;
    WPUB = 0b1001;
    IOCB = 0b1001;
    
    INTCONbits.RBIF = 0;
    INTCONbits.RBIE = 1;
    INTCONbits.GIE = 1;
    
 // --------------- CONFIGURACION DE OSCILADOR --------------- //    
    OSCCONbits.IRCF = 0b100;    // Oscilador interno de 1 mHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    
    //Configuración del ADC
    ADCON0bits.ADCS = 0b00;     // Fosc/2
    ADCON1bits.VCFG0 = 0;       // VDD 'Referencias internas
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

// ---------- FUNCIONES ---------- //
uint8_t read_EEPROM(uint8_t address){
    EEADR = address;
    EECON1bits.EEPGD = 0;       // Lectura a la EEPROM
    EECON1bits.RD = 1;          // Obtenemos dato de la EEPROM
    return EEDAT;               // Regresamos dato 
}
void write_EEPROM(uint8_t address, uint8_t data){
    EEADR = address;
    EEDAT = data;
    EECON1bits.EEPGD = 0;       // Escritura a la EEPROM
    EECON1bits.WREN = 1;        // Habilitamos escritura en la EEPROM
    
    INTCONbits.GIE = 0;         // Deshabilitamos interrupciones
    EECON2 = 0x55;      
    EECON2 = 0xAA;
    
    EECON1bits.WR = 1;          // Iniciamos escritura
    
    EECON1bits.WREN = 0;        // Deshabilitamos escritura en la EEPROM
    INTCONbits.RBIF = 0;
    INTCONbits.GIE = 1;         // Habilitamos interrupciones
} 
 
