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

#define WYSW1_PIN				4
#define WYSW2_PIN				0
#define WYSW3_PIN 				3
#define WYSW4_PIN				4

#define WYSW_SEGMENTY_DDR 		DDRA
#define WYSW_SEGMENTY_PORT 		PORTA

#define WYSW_ZASWIEC_1 	WYSW_PORT_1 &= ~(1 << WYSW_PIN_1)
#define WYSW_ZGAS_1 	WYSW_PORT_1 |=  (1 << WYSW_PIN_1)

#define WYSW_ZASWIEC_2 	WYSW_PORT_2 &= ~(1 << WYSW_PIN_2)
#define WYSW_ZGAS_2 	WYSW_PORT_2 |=  (1 << WYSW_PIN_2)

#define WYSW_ZASWIEC_3 	WYSW_PORT_3 &= ~(1 << WYSW_PIN_3)
#define WYSW_ZGAS_3 	WYSW_PORT_3 |=  (1 << WYSW_PIN_3)

#define WYSW_ZASWIEC_4 	WYSW_PORT_4 &= ~(1 << WYSW_PIN_4)
#define WYSW_ZGAS_4 	WYSW_PORT_4 |=  (1 << WYSW_PIN_4)
struct Wysw
{
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	uint8_t pin;
};
Wysw tab[] =
{
	{&WYSW1_DDR, &WYSW1_PORT, (1 << 4)}
		};
volatile uint8_t *adr[4] = {&PORTD, &PORTC, &PORTB, &PORTB};
 uint8_t pin[4] = {(1 << 4), (1 << 0), (1 << 3), (1 << 4)};
// 0 1 2 3 4 5 6 7 8 9 zgaszone wszystkie
uint8_t tab_cyfry[11] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10, 0x7F};
uint8_t cyfra[4] = {tab_cyfry[0], tab_cyfry[0], tab_cyfry[0], tab_cyfry[0]};

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
	// jako wyjscia port z tranzystorami
	WYSW1_DDR |= (1 << WYSW1_PIN);
	WYSW2_DDR |= (1 << WYSW2_PIN);
	WYSW3_DDR |= (1 << WYSW3_PIN);
	WYSW4_DDR |= (1 << WYSW4_PIN);
	// jako wyjscia port (za wyjatkiem najstarszego bitu) z segmentami a, b, c...
	WYSW_SEGMENTY_DDR = 0b01111111;
	// jakas maska: &0b01111111

	TCCR0A |= (1 << WGM01); 		// CTC
	TCCR0B |= (1 << CS02) | (1 << CS00);
	OCR0A = 30;
	TIMSK0  |= (1 << OCIE0A);
	sei();
// UWAGA NA NADPISANIE NAJSTARSZEGO BITU PORTA !!!!!!!!!!!!!!!!!!!!!!!
	// gaszenie wszystkich modulow
	*adr[0]|= pin[0];
	*adr[1]|= pin[1];
	*adr[2]|= pin[2];
	*adr[3]|= pin[3];

	uint8_t ktory_wyswietlacz = 0;
	uint16_t l = 0;
	while(1)
	{
		if(t_flaga_licz)
		{
			t_flaga_licz = 0;
			IntToLed(l++);

		}
		if(t_flaga)
		{
			t_flaga = 0;
			PORTA = cyfra[ktory_wyswietlacz];
			*adr[ktory_wyswietlacz] &= ~pin[ktory_wyswietlacz];
			if(ktory_wyswietlacz == 0)	*adr[3] |= pin[3]; else *adr[ktory_wyswietlacz-1] |= pin[ktory_wyswietlacz-1];
			ktory_wyswietlacz++;
			if(ktory_wyswietlacz == 4) ktory_wyswietlacz = 0;
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
