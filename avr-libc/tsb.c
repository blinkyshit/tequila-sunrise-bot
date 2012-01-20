/* 
   Copyright (c) Party Robotics LLC 2012
   Written by Robert Kaye <rob@partyrobotics.com>

Welcome hacker!

Not liking what the bot is churning out right now?

GOOD! Read on!

The tequila sunrise bot is nothing more than three peristaltic pumps, each 
independently driven by a motor. How long the pumps run while making a drink, 
determines how much of each ingredient is dispensed. To make a tequila sunrise 
you need to dispense:

- grenadine for 3560 milliseconds, equivalent to 1 part
- tequila for 10680 milliseconds, equivalent to 3 parts
- orange juice for 21360 milliseconds, equivalent to 6 parts

Down below is the definition of our drinks -- those initializers define the 
drinks. And there are two drinks: normal and STRONG. You get to decide on 
durations for each of these two drinks.

In order to deploy the bot for your bidding, you'll need to get the required 
booze. Please run water through the lines before you start making your new drinks.
To prime the pumps, press both buttons at the same time. This will run the
pumps until you let go of the buttons. 

Extra credit: Fix up the code us to use proportions and drink sizes not just
durations. I ran out of time to make that happen. :)

Have fun!

*/
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
// Drink definition. HACK ME!
// ---------------------------------------------
typedef struct 
{
     int8_t   motor;
     uint16_t duration;
} motor_command;

// the proportions of the drink expressed in ms of motor run time
static motor_command drinks[2][3] =
{
    { // Normal
        { 0, 21360 },  // OJ
        { 1, 10680 },  // tequila
        { 2, 3560 }    // grenadine

    },
    { // Strong
        { 0, 19150 },  // OJ
        { 1, 12770 },  // tequila
        { 2, 3830 }    // grenadine
    }
};

// ---------------------------------------------
// End of easily hackable section
// ---------------------------------------------

// ---------------------------------------------
// Print debug output to the serial port.
// ---------------------------------------------

// Set this to 0 before deploying. Sometimes the arduino will wait until a 
// USB cable is connected to start running. 
#define DEBUG 1

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
#if DEBUG
    /*Set baud rate */ 
    UBRR0H = (unsigned char)(UBBR>>8); 
    UBRR0L = (unsigned char)UBBR; 
    /* Enable transmitter */ 
    UCSR0B = (1<<TXEN0)|(1<<RXEN0); 
    /* Set frame format: 8data, 1stop bit */ 
    UCSR0C = (0<<USBS0)|(3<<UCSZ00); 
#endif
}

void serial_tx(unsigned char ch)
{
#if DEBUG
    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = ch;
#endif
}

// ---------------------------------------------
// debugging support stuff
// ---------------------------------------------
#define MAX 80 
// debugging printf function. Max MAX characters per line!!
void dprintf(const char *fmt, ...)
{
#if DEBUG
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
#endif
}

// ---------------------------------------------
// defintions for the TSB motors
// ---------------------------------------------
const int motor_count = 3;

// ---------------------------------------------
// TSB driver code
// ---------------------------------------------

void stop_all()
{
    cbi(PORTB, 0);
    cbi(PORTB, 1);
    cbi(PORTB, 2);
}

void start_all()
{
    sbi(PORTB, 0);
    sbi(PORTB, 1);
    sbi(PORTB, 2);
}

// Get the current state of the input pin on PIND
uint8_t get_pin(uint8_t pin)
{
    return (!(PIND & (1<<pin)));
}

// compare function for qsort. Sorts by duration
int compare(const void *a, const void *b)
{
    return ((motor_command *)a)->duration > ((motor_command *)b)->duration;
}

// call this function to make a drink. You must pass in up to 
// MOTOR_COUNT (3) number of motor_commands
void make_drink(uint8_t count, motor_command *cmds)
{
    uint8_t  i;
    uint16_t t = 0, duration;

    if (count > motor_count)
        return;

    // sort by duration
    qsort(cmds, count, sizeof(motor_command), compare);

    for(i = 0; i < count; i++)
        dprintf("motor %d on, %u ms\n", cmds[i].motor, cmds[i].duration);
    
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
            
            // If the button is pressed during dispense, STOP!
            if (get_pin(PIND2) || get_pin(PIND3))
            {
                stop_all();
                
                // Wait until no more buttons are pressed
                while(get_pin(PIND2) || get_pin(PIND3))
                     ; 
                return;
            }
        }
        if (cmds[i].motor == 0)
        {
            cbi(PORTB, 0);
            dprintf("motor %d off\n", cmds[i].motor);
        }            
        if (cmds[i].motor == 1)
        {
            cbi(PORTB, 1);
            dprintf("motor %d off\n", cmds[i].motor);
        }            
        if (cmds[i].motor == 2)
        {
            cbi(PORTB, 2);
            dprintf("motor %d off\n", cmds[i].motor);
        }            
    }
}

//run all motors to clean the lines
void clean()
{
    dprintf("cleaning!\n");
    start_all();
    while(get_pin(PIND2) && get_pin(PIND3))
        ;
    stop_all();
    dprintf("done cleaning! wait 2 seconds\n");
    _delay_ms(2000);
    dprintf("ready!\n");
}
   
int main(void)
{    
    uint8_t cleaned = 0;

#if DEBUG
    serial_init();
#endif

    // set pwm pins as outputs
    DDRB |= (1<<PB0)|(1<<PB1)|(1<<PB2);

    // set the pullups
    sbi(PORTD, 2);
    sbi(PORTD, 3);
    
    dprintf("\nTequila Sunrise bot. What shall be your bidding?\n\n");
    for(;;)
    {
        // Let everything settle, avoid bouncy startups
        _delay_ms(100);
        if (get_pin(PIND2))
        {
            cleaned = 0;
            // wait for the button to be depressed         
            while(get_pin(PIND2))
                 if (get_pin(PIND3))
                 {
                      clean();
                      cleaned = 1;
                 }
                 
            if (!cleaned)
            {  
                dprintf("making normal drink\n");
                make_drink(3, drinks[0]);
                dprintf("drink complete. bottoms up!\n");
            }
        }
        if (get_pin(PIND3))
        {
            // wait for the button to be depressed         
            cleaned = 0;
            // wait for the button to be depressed         
            while(get_pin(PIND3))
                 if (get_pin(PIND2))
                 {
                      clean();
                      cleaned = 1;
                 }
                 
            if (!cleaned)
            {     
                 dprintf("making strong drink\n");
                 make_drink(3, drinks[1]);
                 dprintf("drink complete. bottoms up!\n");
            }
        }
    }
}
