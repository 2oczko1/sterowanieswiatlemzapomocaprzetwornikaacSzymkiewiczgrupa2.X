#include <xc.h>

// Konfiguracja mikrokontrolera
#pragma config FOSC = XT      // Oscylator XT
#pragma config WDTE = OFF     // Watchdog Timer wy??czony
#pragma config PWRTE = OFF    // Power-up Timer wy??czony
#pragma config CP = OFF       // Kod programu niezabezpieczony

// Definicje poziomów progowych
#define L_LEVEL 5   // Niski próg (przyk?adowo)
#define H_LEVEL 10  // Wysoki próg (przyk?adowo, ADC wynik 0-255)

// Definicje wyj?cia steruj?cego
#define LIGHT_PIN PORTDbits.RD0 // Pin RD0 steruje ?wiat?em
#define LIGHT_PIN2 PORTCbits.RC0

// Ustawienia cz?stotliwo?ci zegara
#define _XTAL_FREQ 4000000 // 4 MHz 

// Prototypy funkcji
void SetA_D(void);        // Konfiguracja przetwornika A/D
void IE_ENABLE(void);     // Konfiguracja przerwa?
void __interrupt() AD_Int(void); // Obs?uga przerwania A/D

// Zmienna globalna do przechowywania wyniku z ADC
unsigned char wynik = 0;
unsigned char portd_mask = 0b00000001; // Pocz?tkowa maska do cyklicznego sterowania

void main(void) {
    // Inicjalizacja systemu
    SetA_D();         // Ustawienia przetwornika A/D
    IE_ENABLE();      // W??czenie przerwa?
    TRISD = 0;        // PortD jako wyj?cie
    LIGHT_PIN = 0;    // ?wiat?o wy??czone na starcie
    
    while (1) {
        // Cycliczne sterowanie nó?kami PortD
        PORTD = portd_mask;            // Wysterowanie konkretnej nó?ki
        portd_mask = (portd_mask << 1) | (portd_mask >> 7); // Przesuni?cie
        __delay_ms(500);              // Opcjonalne opó?nienie
    }
}

// Funkcja konfiguracji przetwornika A/D
void SetA_D(void) {
    ADCON0 = 0b00000001;      // Wybór kana?u AN0, w??czenie ADC
    ADCON0bits.ADCS = 0b01;   // Fosc/8
    ADCON1 = 0b00001110;      // PCFG konfiguracja: AN0 jako wej?cie analogowe
    ADCON0bits.ADON = 1;      // W??czenie przetwornika
    __delay_ms(2);            // Czas na ustabilizowanie ADC
}

// Funkcja konfiguracji przerwa?
void IE_ENABLE(void) {
    INTCONbits.GIE = 1;       // Globalne odblokowanie przerwa?
    INTCONbits.PEIE = 1;      // W??czenie przerwa? peryferyjnych
    PIR1bits.ADIF = 0;        // Wyczyszczenie flagi przerwania ADC
    PIE1bits.ADIE = 1;        // W??czenie przerwa? od ADC
}

// Obs?uga przerwa? od przetwornika A/D
void __interrupt() AD_Int(void) {
    if (PIE1bits.ADIE && PIR1bits.ADIF) { // Sprawdzenie przerwania ADC
        wynik = ADRES;  // Odczyt wyniku 8-bitowego

        // Decyzja o w??czeniu/wy??czeniu ?wiat?a
        if (wynik < L_LEVEL) {
            LIGHT_PIN = 1; // W??cz ?wiat?o
        } else if (wynik > H_LEVEL) {
            LIGHT_PIN = 0; // Wy??cz ?wiat?o
        }

        PIR1bits.ADIF = 0;    // Wyczyszczenie flagi przerwania
        ADCON0bits.GO = 1;    // Rozpocz?cie nowej konwersji
    }
}
