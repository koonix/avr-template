/*
 * ====================================================================
 * Arduino code example for SPI Slave Mode
 * Read unsigned short (two bytes) from SPI, send word to serial port
 * On 16 MHz Arduino, can work at > 500 words per second
 * J.Beale July 19 2011
 * ====================================================================
 */

#include <stdint.h>
#include <SPI.h>

#define SCK_PIN   13  /* D13 = pin19 = PortB.5 */
#define MISO_PIN  12  /* D12 = pin18 = PortB.4 */
#define MOSI_PIN  11  /* D11 = pin17 = PortB.3 */
#define SS_PIN    10  /* D10 = pin16 = PortB.2 */

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
    Serial.println("spi.ino started successfully");
}

void loop() { /* do fuck-all here */ }

ISR (SPI_STC_vect)
{
    Serial.print((char)SPDR);
}
