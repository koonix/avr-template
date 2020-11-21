/*
 * ====================================================================
 * Arduino code for my spi-based protocol.
 * ====================================================================
 */

#include <stdint.h>
#include <SPI.h>

#define SCK_PIN   13  /* D13 = pin19 = PortB.5 */
#define MISO_PIN  12  /* D12 = pin18 = PortB.4 */
#define MOSI_PIN  11  /* D11 = pin17 = PortB.3 */
#define SS_PIN    10  /* D10 = pin16 = PortB.2 */

volatile static bool int_mode = true;

void SlaveInit(void)
{
    pinMode ( SCK_PIN,  INPUT  );
    pinMode ( MOSI_PIN, INPUT  );
    pinMode ( SS_PIN,   INPUT  );
/*  pinMode ( MISO_PIN, OUTPUT ); */

    SPCR =
    ( 1 << SPE  ) |
    ( 1 << SPIE ) |
    ( 0 << DORD ) |
    ( 0 << MSTR ) |
    ( 0 << CPOL ) |
    ( 0 << CPHA);
}

void setup()
{
    Serial.begin(115200);
    SlaveInit();
    SPI.attachInterrupt(); /* enable spi interrups */
    delay(10);
    Serial.println("SPI port reader v0.1");
}

void loop() { /* do fuck-all here */ }

ISR (SPI_STC_vect)
{
    static bool    special  = false;
    static uint8_t num      = 0;
    static char    buff[2];

    buff[num] = SPDR;

    /* if 0 received, next reception is a special character */
    if(buff[num] == 0x07)
    {
        num = 0;
        special = true;
    }

    /* if special character received, checkout what it is */
    else if (special == true)
    {
        if (buff[num] == 0x21)
            int_mode = true;

        else if (buff[num] == 0x24)
            int_mode = false;

        num = 0;
        special = false;
    }

    /* if it's not a special character or 0x07, just increment num */
    else
    {
        num++;
    }

    /* if we received 2 proper bytes, process it and print it */
    if (num >= 2)
    {
        /* if we're in int mode, print it as an integer. */
        if (int_mode == true)
        {
            uint32_t concat = (buff[1] << 8) | buff[0];
            Serial.println(concat, DEC);
        }

        /* otherwise, print it as two characters. */
        else
        {
            Serial.print(buff[0]);
            Serial.print(buff[1]);
        }

        /* reset and get ready for the next byte(s). */
        num = 0;
    }
}
