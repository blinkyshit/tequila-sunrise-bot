
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

#include "defs.h"

// ---------------------------------------------
// Drink definition. HACK ME!
// ---------------------------------------------

// the proportions of the drink expressed in ms of motoro run time
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
#define DEBUG 0

// ---------------------------------------------
// defintions for the TSB motors
// ---------------------------------------------
const int motor_count = 3;

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
    Serial.print(buffer);
#endif
}

// ---------------------------------------------
// TSB driver code
// ---------------------------------------------

void stop_all()
{
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
}

void start_all()
{
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
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
            digitalWrite(8, HIGH);

        if (cmds[i].motor == 1)
            digitalWrite(9, HIGH);

        if (cmds[i].motor == 2)
            digitalWrite(10, HIGH);
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

            delay(d);
            duration -= d;
            t += d;
            
            // If the button is pressed during dispense, STOP!
            if (digitalRead(2) == LOW || digitalRead(3) == LOW)
            {
                stop_all();
                
                // Wait until no more buttons are pressed
                while(digitalRead(2) == LOW || digitalRead(3) == LOW)
                     ; 
                return;
            }
        }
        if (cmds[i].motor == 0)
        {
            digitalWrite(8, LOW);
            dprintf("motor %d off\n", cmds[i].motor);
        }            
        if (cmds[i].motor == 1)
        {
            digitalWrite(9, LOW);
            dprintf("motor %d off\n", cmds[i].motor);
        }            
        if (cmds[i].motor == 2)
        {
            digitalWrite(10, LOW);
            dprintf("motor %d off\n", cmds[i].motor);
        }            
    }
}

void setup()
{
#if DEBUG
    Serial.begin(38400);
#endif

    // Set motor pins as outputs
    pinMode(8, OUTPUT); 
    pinMode(9, OUTPUT); 
    pinMode(10, OUTPUT); 

    // set the input pins and enable the pull up resistors
    pinMode(2, INPUT); 
    pinMode(3, INPUT); 
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
}   
    
//run all motors to clean the lines
void clean()
{
    dprintf("cleaning!\n");
    start_all();
    while(digitalRead(2) == LOW && digitalRead(3) == LOW)
        ;
    stop_all();
    dprintf("done cleaning! wait 2 seconds\n");
    delay(2000);
    dprintf("ready!\n");
}
   
void loop()
{    
    uint8_t cleaned = 0;
    
    dprintf("\nTequila Sunrise bot. What shall be your bidding?\n\n");
    for(;;)
    {
        // Let everything settle, avoid bouncy startups
        delay(100);
        if (digitalRead(2) == LOW)
        {
            cleaned = 0;
            // wait for the button to be depressed         
            while(digitalRead(2) == LOW)
                 if (digitalRead(3) == LOW)
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
        if (digitalRead(3) == LOW)
        {
            // wait for the button to be depressed         
            cleaned = 0;
            // wait for the button to be depressed         
            while(digitalRead(3) == LOW)
                 if (digitalRead(2) == LOW)
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
