#include "OpenFLRC.h"

// =========================================================================
//  RawSX1280 Implementation
// =========================================================================

RawSX1280::RawSX1280(Module* mod, SPIClass* spi_ptr) : SX1280(mod) {
    spi = spi_ptr;
}

void RawSX1280::rawWriteBuffer(const uint8_t* data, uint8_t len) {
    uint8_t buf[130];
    buf[0] = RADIOLIB_SX128X_CMD_WRITE_BUFFER;
    buf[1] = 0x00; 
    memcpy(buf + 2, data, len);
    
    spi->beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0)); 
    digitalWrite(OPENFLRC_PIN_NSS, LOW);
    spi->writeBytes(buf, len + 2); 
    digitalWrite(OPENFLRC_PIN_NSS, HIGH);
    spi->endTransaction();
}

void RawSX1280::rawSetTx() {
    uint8_t buf[4] = {RADIOLIB_SX128X_CMD_SET_TX, 0x02, 0x00, 0x00};
    spi->beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    digitalWrite(OPENFLRC_PIN_NSS, LOW);
    spi->writeBytes(buf, 4);
    digitalWrite(OPENFLRC_PIN_NSS, HIGH);
    spi->endTransaction();
}

void RawSX1280::rawSetRx() {
    uint8_t buf[4] = {RADIOLIB_SX128X_CMD_SET_RX, 0x02, 0xFF, 0xFF};
    spi->beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    digitalWrite(OPENFLRC_PIN_NSS, LOW);
    spi->writeBytes(buf, 4);
    digitalWrite(OPENFLRC_PIN_NSS, HIGH);
    spi->endTransaction();
}

void RawSX1280::rawClearIrqStatus() {
    uint8_t buf[3] = {RADIOLIB_SX128X_CMD_CLEAR_IRQ_STATUS, 0xFF, 0xFF};
    spi->beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    digitalWrite(OPENFLRC_PIN_NSS, LOW);
    spi->writeBytes(buf, 3);
    digitalWrite(OPENFLRC_PIN_NSS, HIGH);
    spi->endTransaction();
}

void RawSX1280::rawReadBuffer(uint8_t* data, uint8_t len) {
    spi->beginTransaction(SPISettings(18000000, MSBFIRST, SPI_MODE0));
    digitalWrite(OPENFLRC_PIN_NSS, LOW);
    spi->transfer(RADIOLIB_SX128X_CMD_READ_BUFFER);
    spi->transfer(0x00); 
    spi->transfer(0x00); 
    spi->transferBytes(NULL, data, len); 
    digitalWrite(OPENFLRC_PIN_NSS, HIGH);
    spi->endTransaction();
}

float RawSX1280::rawGetRSSI() {
    spi->beginTransaction(SPISettings(18000000, MSBFIRST, SPI_MODE0));
    digitalWrite(OPENFLRC_PIN_NSS, LOW);
    spi->transfer(RADIOLIB_SX128X_CMD_GET_PACKET_STATUS);
    spi->transfer(0x00); 
    uint8_t rssiSync = spi->transfer(0x00);
    uint8_t snr = spi->transfer(0x00);
    digitalWrite(OPENFLRC_PIN_NSS, HIGH);
    spi->endTransaction();
    return -1.0f * rssiSync / 2.0f;
}

int16_t RawSX1280::rawSetAutoFS(bool enable) {
    spi->beginTransaction(SPISettings(18000000, MSBFIRST, SPI_MODE0));
    digitalWrite(OPENFLRC_PIN_NSS, LOW);
    spi->transfer(RADIOLIB_SX128X_CMD_SET_AUTO_FS);
    spi->transfer((uint8_t)(enable ? 0x01 : 0x00));
    digitalWrite(OPENFLRC_PIN_NSS, HIGH);
    spi->endTransaction();
    return 0;
}


// =========================================================================
//  OpenFLRC Implementation
// =========================================================================

OpenFLRC::OpenFLRC() {
    radioSPI = new SPIClass(HSPI);
    Module* mod = new Module(OPENFLRC_PIN_NSS, OPENFLRC_PIN_DIO1, OPENFLRC_PIN_RESET, OPENFLRC_PIN_BUSY, *radioSPI, SPISettings(18000000, MSBFIRST, SPI_MODE0));
    radio = new RawSX1280(mod, radioSPI);
    pixel = new Adafruit_NeoPixel(OPENFLRC_NUM_LEDS, OPENFLRC_LED_PIN, NEO_GRB + NEO_KHZ800);
}

OpenFLRC::~OpenFLRC() {
    delete radio;
    delete radioSPI;
    delete pixel;
}

void OpenFLRC::setLedColor(uint8_t r, uint8_t g, uint8_t b) {
    pixel->setPixelColor(0, pixel->Color(r, g, b));
    pixel->show();
}

bool OpenFLRC::begin(float freq, uint16_t bitrate, uint8_t cr) {
    pixel->begin();
    pixel->clear();
    pixel->show();

    pinMode(OPENFLRC_PIN_RX_EN, OUTPUT);
    pinMode(OPENFLRC_PIN_TX_EN, OUTPUT);
    digitalWrite(OPENFLRC_PIN_RX_EN, LOW);
    digitalWrite(OPENFLRC_PIN_TX_EN, LOW);
    delay(100);

    radioSPI->begin(OPENFLRC_PIN_SCK, OPENFLRC_PIN_MISO, OPENFLRC_PIN_MOSI, OPENFLRC_PIN_NSS);
    radio->setRfSwitchPins(OPENFLRC_PIN_RX_EN, OPENFLRC_PIN_TX_EN);

    int state = radio->beginFLRC(freq, bitrate, cr, OPENFLRC_DEFAULT_POWER, OPENFLRC_DEFAULT_PREAMBLE, RADIOLIB_SHAPING_0_5);
    
    if (state == RADIOLIB_ERR_NONE) {
        radio->setHighSensitivityMode(true);
        return true;
    }
    return false;
}

int OpenFLRC::transmit(uint8_t* data, size_t len) {
    return radio->transmit(data, len);
}

int OpenFLRC::receive(uint8_t* data, size_t len, uint32_t timeout) {
    return radio->receive(data, len, timeout);
}

int OpenFLRC::variablePacketLengthMode(uint8_t maxLen) {
    return radio->variablePacketLengthMode(maxLen);
}

int OpenFLRC::fixedPacketLengthMode(uint8_t len) {
    return radio->fixedPacketLengthMode(len);
}

int OpenFLRC::setCRC(uint8_t len) {
    return radio->setCRC(len);
}

void OpenFLRC::setPacketSentAction(void (*func)(void)) {
    radio->setPacketSentAction(func);
}

int OpenFLRC::startTransmit(uint8_t* data, size_t len, uint8_t addr) {
    return radio->startTransmit(data, len, addr);
}

int OpenFLRC::setHighSensitivityMode(bool enable) {
    return radio->setHighSensitivityMode(enable);
}

int OpenFLRC::startReceive() {
    return radio->startReceive();
}

size_t OpenFLRC::getPacketLength() {
    return radio->getPacketLength();
}

int OpenFLRC::readData(uint8_t* data, size_t len) {
    return radio->readData(data, len);
}

void inline OpenFLRC::waitBusyLow() {
    uint64_t _t0 = esp_timer_get_time();
    while (digitalRead(OPENFLRC_PIN_BUSY)) {
        if (esp_timer_get_time() - _t0 > 50000) break; 
    }
}

void OpenFLRC::fastTransmit(const uint8_t* data, uint8_t len) {
    radio->rawWriteBuffer(data, len);
    digitalWrite(OPENFLRC_PIN_TX_EN, HIGH);
    digitalWrite(OPENFLRC_PIN_RX_EN, LOW);
    radio->rawSetTx();
    waitBusyLow();
}
