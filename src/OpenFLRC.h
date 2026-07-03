#ifndef OPENFLRC_H
#define OPENFLRC_H

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <Adafruit_NeoPixel.h>

// =========================================================================
//  HARDWARE PINOUT - OpenFLRC v1.0 (YD-ESP32-S3 Base)
// =========================================================================
#define OPENFLRC_PIN_SCK      18
#define OPENFLRC_PIN_MISO     17
#define OPENFLRC_PIN_MOSI     16
#define OPENFLRC_PIN_NSS      15
#define OPENFLRC_PIN_RESET     8
#define OPENFLRC_PIN_BUSY      7
#define OPENFLRC_PIN_DIO1      9
#define OPENFLRC_PIN_RX_EN     6  // LNA active high
#define OPENFLRC_PIN_TX_EN     5  // PA active high
#define OPENFLRC_PIN_BUTTON    0

// Onboard RGB NeoPixel
#define OPENFLRC_LED_PIN      48
#define OPENFLRC_NUM_LEDS      1

// =========================================================================
//  FLRC DEFAULT PHYSICAL LAYER PARAMETERS
// =========================================================================
#define OPENFLRC_DEFAULT_FREQ       2480.0   // MHz
#define OPENFLRC_DEFAULT_BITRATE    1300     // kbps raw air rate (1.3 Mbps)
#define OPENFLRC_DEFAULT_CR            4     // 4 = CR 1/1 (No FEC)
#define OPENFLRC_DEFAULT_POWER         0     // dBm (E28 PA amplifies to +15 dBm)
#define OPENFLRC_DEFAULT_PREAMBLE      16    // bits 

// =========================================================================
//  RawSX1280 (DMA Optimized subclass)
// =========================================================================
class RawSX1280 : public SX1280 {
public:
    SPIClass* spi;
    RawSX1280(Module* mod, SPIClass* spi_ptr);
    void rawWriteBuffer(const uint8_t* data, uint8_t len);
    void rawSetTx();
    void rawSetRx();
    void rawClearIrqStatus();
    void rawReadBuffer(uint8_t* data, uint8_t len);
    float rawGetRSSI();
    int16_t rawSetAutoFS(bool enable);
};

// =========================================================================
//  OpenFLRC Library Class
// =========================================================================
class OpenFLRC {
public:
    OpenFLRC();
    ~OpenFLRC();

    // Initialization
    bool begin(float freq = OPENFLRC_DEFAULT_FREQ, 
               uint16_t bitrate = OPENFLRC_DEFAULT_BITRATE, 
               uint8_t cr = OPENFLRC_DEFAULT_CR);
               
    // LED Control
    void setLedColor(uint8_t r, uint8_t g, uint8_t b);

    // High level wrapper for standard API
    int transmit(uint8_t* data, size_t len);
    int receive(uint8_t* data, size_t len, uint32_t timeout = 0);
    int variablePacketLengthMode(uint8_t maxLen = 127);
    int fixedPacketLengthMode(uint8_t len = RADIOLIB_SX128X_MAX_PACKET_LENGTH);
    int setCRC(uint8_t len);
    void setPacketSentAction(void (*func)(void));
    int startTransmit(uint8_t* data, size_t len, uint8_t addr = 0);
    
    // Additional wrappers for Slave
    int setHighSensitivityMode(bool enable);
    int startReceive();
    size_t getPacketLength();
    int readData(uint8_t* data, size_t len);

    // Fast TX (Optimized DMA burst)
    void fastTransmit(const uint8_t* data, uint8_t len);
    
    // Hardware references if needed by application
    RawSX1280* radio;
    SPIClass* radioSPI;
    Adafruit_NeoPixel* pixel;

private:
    void waitBusyLow();
};

#endif // OPENFLRC_H
