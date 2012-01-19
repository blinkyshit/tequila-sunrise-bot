// 
// Copyright (c) Party Robotics LLC 2012
// Written by Robert Kaye <rob@partyrobotics.com>
//
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <util/delay.h>

// Bit manipulation macros
#define sbi(a, b) ((a) |= 1 << (b))       //sets bit B in variable A
#define cbi(a, b) ((a) &= ~(1 << (b)))    //clears bit B in variable A
#define tbi(a, b) ((a) ^= 1 << (b))       //toggles bit B in variable A

// ---------------------------------------------
// defintions for the TSB motors
// ---------------------------------------------
#define MOTOR_COUNT 3
typedef struct
{
    int8_t   motor;
    uint16_t duration;
} motor_cmd;

// 
static motor_cmd drinks[2][3] =
{
    { // Normal
        { 0, 3560 },
        { 1, 21360 },
        { 2, 10680 }
    },
    { // Strong
        { 0, 3830 },
        { 1, 19150 },
        { 2, 12770 }
    }
};

// ---------------------------------------------
// serial communications definitions
// ---------------------------------------------
#define BAUD 38400
#define UBBR (F_CPU / 16 / BAUD - 1)

// ---------------------------------------------
// serial communications functions
// ---------------------------------------------
void serial_init(void)
{
    /*Set baud rate */ 
    UBRR0H = (unsigned char)(UBBR>>8); 
    UBRR0L = (unsigned char)UBBR; 
    /* Enable transmitter */ 
    UCSR0B = (1<<TXEN0)|(1<<RXEN0); 
    /* Set frame format: 8data, 1stop bit */ 
    UCSR0C = (0<<USBS0)|(3<<UCSZ00); 
}

void serial_tx(unsigned char ch)
{
    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = ch;
}

#define MAX 80 

// debugging printf function. Max MAX characters per line!!
void dprintf(const char *fmt, ...)
{
    va_list va;
    va_start (va, fmt);
    char buffer[MAX];
    char *ptr = buffer;
    vsnprintf(buffer, MAX, fmt, va);
    va_end (va);
    for(ptr = buffer; *ptr; ptr++)
    {
        if (*ptr == '\n') serial_tx('\r');
        serial_tx(*ptr);
    }
}


// ---------------------------------------------
// TSB driver code
// ---------------------------------------------

// compare function for qsort. Sorts by duration
int compare(const void *a, const void *b)
{
    return ((motor_cmd *)a)->duration > ((motor_cmd *)b)->duration;
}

// call this function to make a drink. You must pass in up to 
// MOTOR_COUNT (3) number of motor_cmds
void make_drink(uint8_t count, motor_cmd *cmds)
{
    uint8_t  i;
    uint16_t t = 0, duration;

    if (count > MOTOR_COUNT)
        return;

    // sort by duration
    qsort(cmds, count, sizeof(motor_cmd), compare);

    for(i = 0; i < count; i++)
        dprintf("%05u: motor %d on, %u ms\n", t, cmds[i].motor, cmds[i].duration);
    
    // turn motors on
    for(i = 0; i < count; i++)
    {
        if (cmds[i].motor == 0)
            sbi(PORTB, 0);
        if (cmds[i].motor == 1)
            sbi(PORTB, 1);
        if (cmds[i].motor == 2)
            sbi(PORTB, 2);
    }

    // wait the appropriate amount of time and turn off motors
    // starting with the shortest duration
    for(i = 0; i < count; i++)
    {
        duration = cmds[i].duration - t;
        dprintf("delay: %d\n", duration);
        while(duration > 0)
        {
            uint8_t j;
            uint8_t d = duration > 10 ? 10 : duration;

            for(j = 0; j < d; j++)
                _delay_ms(1);
            duration -= d;
            t += d;
        }
        if (cmds[i].motor == 0)
            cbi(PORTB, 0);
        if (cmds[i].motor == 1)
            cbi(PORTB, 1);
        if (cmds[i].motor == 2)
            cbi(PORTB, 2);
    }
}

int main(void)
{
    serial_init();

    // Set PWM pins as outputs
    DDRB |= (1<<PB0)|(1<<PB1)|(1<<PB2);

    sbi(PORTD, 2);
    sbi(PORTD, 3);

    dprintf("\nTequila Sunrise bot. What shall be your bidding?\n\n");
    for(;;)
    {
        if (!(PIND & (1<<PIND2)))
        {
            sbi(PORTB, 3);
            dprintf("making normal drink\n");
            make_drink(3, drinks[0]);
            dprintf("drink complete. bottoms up!\n");
            cbi(PORTB, 3);
        }
        if (!(PIND & (1<<PIND3)))
        {
            sbi(PORTB, 4);
            dprintf("making strong drink\n");
            make_drink(3, drinks[1]);
            dprintf("drink complete. bottoms up!\n");
            cbi(PORTB, 4);
        }
    }
    return 0;
}
