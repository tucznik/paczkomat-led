/*
 * main.cpp
 *
 *  Created on: 14 cze 2017
 *      Author: tomek
 */
#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Numeracja wg. rozmieszczenia na plytce
#define WYSW1_DDR 				DDRD
#define WYSW2_DDR 				DDRC
#define WYSW3_DDR 				DDRB
#define WYSW4_DDR 				DDRB

#define WYSW1_PORT	 			PORTD
#define WYSW2_PORT 				PORTC
#define WYSW3_PORT 				PORTB
#define WYSW4_PORT 				PORTB

#define WYSW1_PIN				(1 << 4)
#define WYSW2_PIN				(1 << 0)
#define WYSW3_PIN 				(1 << 3)
#define WYSW4_PIN				(1 << 4)

#define WYSW_SEGMENTY_DDR 		DDRA
#define WYSW_SEGMENTY_PORT 		PORTA

#define WYSW_ZNAK_ZERO 			0x40
#define WYSW_ZNAK_JEDEN			0x79
#define WYSW_ZNAK_DWA			0x24
#define WYSW__ZNAK_TRZY			0x30
#define WYSW_ZNAK_CZTERY		0x19
#define WYSW_ZNAK_PIEC			0x12
#define WYSW_ZNAK_SZESC			0x02
#define WYSW_ZNAK_SIEDEM		0x78
#define WYSW_ZNAK_OSIEM			0x00
#define WYSW_ZNAK_DZIEWIEC		0x10
#define WYSW_ZNAK_WYL			0x7F
struct Wysw
{
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	uint8_t pin;
};
Wysw tab[] =
{
	{&WYSW1_DDR, &WYSW1_PORT, WYSW1_PIN},
	{&WYSW2_DDR, &WYSW2_PORT, WYSW2_PIN},
	{&WYSW3_DDR, &WYSW3_PORT, WYSW3_PIN},
	{&WYSW4_DDR, &WYSW4_PORT, WYSW4_PIN}
};

// 0 1 2 3 4 5 6 7 8 9 zgaszone wszystkie
uint8_t tab_cyfry[11] = {WYSW_ZNAK_ZERO, WYSW_ZNAK_JEDEN, WYSW_ZNAK_DWA,
		WYSW__ZNAK_TRZY, WYSW_ZNAK_CZTERY, WYSW_ZNAK_PIEC, WYSW_ZNAK_SZESC,
		WYSW_ZNAK_SIEDEM, WYSW_ZNAK_OSIEM, WYSW_ZNAK_DZIEWIEC, WYSW_ZNAK_WYL};

uint8_t cyfra[4] = {tab_cyfry[1], tab_cyfry[2], tab_cyfry[3], tab_cyfry[4]};

void IntToLed(uint16_t liczba)
{
	cyfra[0] = tab_cyfry[liczba / 1000]; // tys
	cyfra[1] = tab_cyfry[(liczba / 100) % 10]; // setki
	cyfra[2] = tab_cyfry[(liczba % 100) / 10]; // dzies
	cyfra[3] = tab_cyfry[liczba % 10]; // jedn

}
uint8_t volatile t_flaga, t_flaga_licz = 0;
uint8_t volatile licznik = 0;

int main()
{
	for(uint8_t i = 0; i < 4; i++)
	{
		*(tab[i].ddr) |= tab[i].pin;		// jako wyjscia
		*(tab[i].port) |= tab[i].pin; 		// gaszenie wszystkich
	}
	// jako wyjscia port (za wyjatkiem najstarszego bitu) z segmentami a, b, c...
	WYSW_SEGMENTY_DDR = 0b01111111;
	// jakas maska: &0b01111111

	TCCR0A |= (1 << WGM01); 		// CTC
	TCCR0B |= (1 << CS02) | (1 << CS00);
	OCR0A = 30;
	TIMSK0  |= (1 << OCIE0A);
	sei();
// UWAGA NA NADPISANIE NAJSTARSZEGO BITU PORTA !!!!!!!!!!!!!!!!!!!!!!!

	uint8_t nr_wysw = 0;
	uint16_t l = 0;
	while(1)
	{
		if(t_flaga_licz)
		{
			t_flaga_licz = 0;
			IntToLed(l++);
			if(l == 5000) l = 0;
		}
		if(t_flaga)
		{
			t_flaga = 0;
			WYSW_SEGMENTY_PORT = cyfra[nr_wysw];				// laduj cyfre do portu
			*(tab[nr_wysw].port) &= ~tab[nr_wysw].pin;			// zaswiec modul
			if(nr_wysw == 0)
				*(tab[3].port) |= tab[3].pin;					// zgas modul
			else
				*(tab[nr_wysw-1].port) |= tab[nr_wysw-1].pin;	// zgas modul
			nr_wysw++;
			if(nr_wysw == 4) nr_wysw = 0;
		}
	}
}

ISR(TIMER0_COMPA_vect)
{
	licznik++;
	if(licznik == 20)
	{
		licznik = 0;
		t_flaga_licz = 1;
	}
	t_flaga = 1;
}
