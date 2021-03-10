#ifndef _ADAFRUIT_SPITFT_MACROS
#define _ADAFRUIT_SPITFT_MACROS

/*
 * Control Pins
 * */

#ifdef USE_FAST_PINIO
    #ifdef PARTICLE
    #define SPI_DC_HIGH()           pinSetFast(_dc)
    #define SPI_DC_LOW()            pinResetFast(_dc)
    #define SPI_CS_HIGH()           pinSetFast(_cs)
    #define SPI_CS_LOW()            pinResetFast(_cs)
    #else
    #define SPI_DC_HIGH()           *dcport |=  dcpinmask
    #define SPI_DC_LOW()            *dcport &= ~dcpinmask
    #define SPI_CS_HIGH()           *csport |=  cspinmask
    #define SPI_CS_LOW()            *csport &= ~cspinmask
    #endif
#else
#define SPI_DC_HIGH()           digitalWrite(_dc, HIGH)
#define SPI_DC_LOW()            digitalWrite(_dc, LOW)
#define SPI_CS_HIGH()           { if(_cs >= 0) digitalWrite(_cs, HIGH); }
#define SPI_CS_LOW()            { if(_cs >= 0) digitalWrite(_cs, LOW);  }
#endif

/*
 * Software SPI Macros
 * */

#ifdef USE_FAST_PINIO
    #ifdef PARTICLE
    #define SSPI_MOSI_HIGH()        pinSetFast(_mosi)
    #define SSPI_MOSI_LOW()         pinResetFast(_mosi)
    #define SSPI_SCK_HIGH()         pinSetFast(_sclk)
    #define SSPI_SCK_LOW()          pinResetFast(_sclk)
    #define SSPI_MISO_READ()        digitalRead(_miso)
    #else
    #define SSPI_MOSI_HIGH()        *mosiport |=  mosipinmask
    #define SSPI_MOSI_LOW()         *mosiport &= ~mosipinmask
    #define SSPI_SCK_HIGH()         *clkport  |=  clkpinmask
    #define SSPI_SCK_LOW()          *clkport  &= ~clkpinmask
    #define SSPI_MISO_READ()        ((*misoport & misopinmask) != 0)
    #endif
#else
#define SSPI_MOSI_HIGH()        digitalWrite(_mosi, HIGH)
#define SSPI_MOSI_LOW()         digitalWrite(_mosi, LOW)
#define SSPI_SCK_HIGH()         digitalWrite(_sclk, HIGH)
#define SSPI_SCK_LOW()          digitalWrite(_sclk, LOW)
#define SSPI_MISO_READ()        digitalRead(_miso)
#endif

#define SSPI_BEGIN_TRANSACTION()
#define SSPI_END_TRANSACTION()
#define SSPI_WRITE(v)           spiWrite(v)
#define SSPI_WRITE16(s)         SSPI_WRITE((s) >> 8); SSPI_WRITE(s)
#define SSPI_WRITE32(l)         SSPI_WRITE((l) >> 24); SSPI_WRITE((l) >> 16); SSPI_WRITE((l) >> 8); SSPI_WRITE(l)
#define SSPI_WRITE_PIXELS(c,l)  for(uint32_t i=0; i<(l); i+=2){ SSPI_WRITE(((uint8_t*)(c))[i+1]); SSPI_WRITE(((uint8_t*)(c))[i]); }

/*
 * Hardware SPI Macros
 * */

#if defined (__AVR__) ||  defined(TEENSYDUINO) ||  defined(ARDUINO_ARCH_STM32F1) || defined(PARTICLE)
    #define HSPI_SET_CLOCK() _spi->setClockDivider(SPI_CLOCK_DIV2);
#elif defined (__arm__)
    #define HSPI_SET_CLOCK() _spi->setClockDivider(11);
#elif defined(ESP8266) || defined(ESP32)
    #define HSPI_SET_CLOCK() _spi->setFrequency(_freq);
#elif defined(RASPI)
    #define HSPI_SET_CLOCK() _spi->setClock(_freq);
#elif defined(ARDUINO_ARCH_STM32F1)
    #define HSPI_SET_CLOCK() _spi->setClock(_freq);
#else
    #define HSPI_SET_CLOCK()
#endif

#ifdef PARTICLE
    #define SPI_HAS_TRANSACTION
#endif
#ifdef SPI_HAS_TRANSACTION
    #define HSPI_BEGIN_TRANSACTION() _spi->beginTransaction(SPISettings(_freq, MSBFIRST, SPI_MODE0))
    #define HSPI_END_TRANSACTION()   _spi->endTransaction()
#else
    #define HSPI_BEGIN_TRANSACTION() HSPI_SET_CLOCK(); _spi->setBitOrder(MSBFIRST); _spi->setDataMode(SPI_MODE0)
    #define HSPI_END_TRANSACTION()
#endif

#ifdef ESP32
    #define SPI_HAS_WRITE_PIXELS
#endif
#if defined(ESP8266) || defined(ESP32)
    // Optimized SPI (ESP8266 and ESP32)
    #define HSPI_READ()              _spi->transfer(0)
    #define HSPI_WRITE(b)            _spi->write(b)
    #define HSPI_WRITE16(s)          _spi->write16(s)
    #define HSPI_WRITE32(l)          _spi->write32(l)
    #ifdef SPI_HAS_WRITE_PIXELS
        #define SPI_MAX_PIXELS_AT_ONCE  32
        #define HSPI_WRITE_PIXELS(c,l)   _spi->writePixels(c,l)
    #else
        #define HSPI_WRITE_PIXELS(c,l)   for(uint32_t i=0; i<((l)/2); i++){ SPI_WRITE16(((uint16_t*)(c))[i]); }
    #endif
#elif defined(PARTICLE)
    // Optimized SPI (Particle Gen 3)
    #define SPI_HAS_WRITE_PIXELS
    #define HSPI_READ()              _spi->transfer(0)
    #define HSPI_WRITE(b)            _spi->transfer((uint8_t)(b))
    #define HSPI_WRITE16(s)          {uint8_t buf[]={(uint8_t)(s >> 8), (uint8_t)(s)}; _spi->transfer((void*)buf, NULL, 2, NULL);}//HSPI_WRITE((uint8_t)(s >> 8)); HSPI_WRITE((uint8_t)(s))
    #define HSPI_WRITE32(l)          {uint8_t buf[]={(uint8_t)(l >> 24), (uint8_t)(l >> 16), (uint8_t)(l >> 8), (uint8_t)(l)}; _spi->transfer((void*)buf, NULL, 4, NULL);}//HSPI_WRITE((uint8_t)(l >> 24)); HSPI_WRITE((uint8_t)(l >> 16)); HSPI_WRITE((uint8_t)(l >> 8)); HSPI_WRITE((uint8_t)(l))
    
    #define SPI_MAX_PIXELS_AT_ONCE  128
    #define HSPI_WRITE_PIXELS(c,l)   { uint8_t buf[l]; for(unsigned int i=0; i<l; i+=2){buf[i] = (*(c+i+1)); buf[i+1] = (*(c+i));}; _spi->transfer((void*)buf, NULL, l, NULL); }//_spi->writePixels(c,l)
#else
    // Standard Byte-by-Byte SPI

    #if defined (__AVR__) || defined(TEENSYDUINO)
static inline uint8_t _avr_spi_read(void) __attribute__((always_inline));
static inline uint8_t _avr_spi_read(void) {
    uint8_t r = 0;
    SPDR = r;
    while(!(SPSR & _BV(SPIF)));
    r = SPDR;
    return r;
}
        #define HSPI_WRITE(b)            {SPDR = (b); while(!(SPSR & _BV(SPIF)));}
        #define HSPI_READ()              _avr_spi_read()
    #else
        #define HSPI_WRITE(b)            _spi->transfer((uint8_t)(b))
        #define HSPI_READ()              HSPI_WRITE(0)
    #endif
    #define HSPI_WRITE16(s)          HSPI_WRITE((s) >> 8); HSPI_WRITE(s)
    #define HSPI_WRITE32(l)          HSPI_WRITE((l) >> 24); HSPI_WRITE((l) >> 16); HSPI_WRITE((l) >> 8); HSPI_WRITE(l)
    #define HSPI_WRITE_PIXELS(c,l)   for(uint32_t i=0; i<(l); i+=2){ HSPI_WRITE(((uint8_t*)(c))[i+1]); HSPI_WRITE(((uint8_t*)(c))[i]); }
#endif

  #define SPI_BEGIN()             if(_sclk < 0){_spi->begin();}
  #define SPI_BEGIN_TRANSACTION() if(_sclk < 0){HSPI_BEGIN_TRANSACTION();}
  #define SPI_END_TRANSACTION()   if(_sclk < 0){HSPI_END_TRANSACTION();}
  #define SPI_WRITE16(s)          if(_sclk < 0){HSPI_WRITE16(s);}else{SSPI_WRITE16(s);}
  #define SPI_WRITE32(l)          if(_sclk < 0){HSPI_WRITE32(l);}else{SSPI_WRITE32(l);}
  #define SPI_WRITE_PIXELS(c,l)   if(_sclk < 0){HSPI_WRITE_PIXELS(c,l);}else{SSPI_WRITE_PIXELS(c,l);}

#endif // _ADAFRUIT_SPITFT_MACROS
