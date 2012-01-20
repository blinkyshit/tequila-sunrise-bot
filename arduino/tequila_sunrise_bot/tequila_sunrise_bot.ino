
// 
// Copyright (c) Party Robotics LLC 2012
// Written by Robert Kaye <rob@partyrobotics.com>
//

#include "defs.h"
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
    va_list va;
    va_start (va, fmt);
    char buffer[MAX];
    char *ptr = buffer;
    vsnprintf(buffer, MAX, fmt, va);
    va_end (va);
    Serial.print(buffer);
}


// the proportions of the drink expressed in ms of motoro run time
static motor_command drinks[2][3] =
{
    { // Normal
        { 0, 3560 },  // grenadine
        { 1, 21360 }, // tequila
        { 2, 10680 }  // OJ
    },
    { // Strong
        { 0, 3830 },  // grenadine
        { 1, 19150 }, // tequila
        { 2, 12770 }  // OJ
    }
};

// ---------------------------------------------
// TSB driver code
// ---------------------------------------------

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
        dprintf("%05u: motor %d on, %u ms\n", t, cmds[i].motor, cmds[i].duration);
    
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
        }
        if (cmds[i].motor == 0)
            digitalWrite(8, LOW);
        if (cmds[i].motor == 1)
            digitalWrite(9, LOW);
        if (cmds[i].motor == 2)
            digitalWrite(10, LOW);
    }
}

void setup()
{
    Serial.begin(38400);

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
    
void loop()
{

    dprintf("\nTequila Sunrise bot. What shall be your bidding?\n\n");
    for(;;)
    {
        if (digitalRead(2) == LOW)
        {
            dprintf("making normal drink\n");
            make_drink(3, drinks[0]);
            dprintf("drink complete. bottoms up!\n");
        }
        if (digitalRead(3) == LOW)
        {
            dprintf("making strong drink\n");
            make_drink(3, drinks[1]);
            dprintf("drink complete. bottoms up!\n");
        }
    }
}
