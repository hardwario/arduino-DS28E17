#ifndef DS28E17_h
#define DS28E17_h

#include "Arduino.h"
#include <OneWire.h>

#define ONEWIRE_TIMEOUT 50

#define DS28E17_ENABLE_SLEEP 0x1E
#define DS28E17_WRITE 0x4B
#define DS28E17_READ 0x87
#define DS28E17_MEMMORY_READ 0x2D

class DS28E17
{
  public:
    DS28E17();
    DS28E17(OneWire *oneWire);
    bool setAddress(uint8_t *sensorAddress); 
    void wakeUp();
    void enableSleepMode();
    bool write(uint8_t i2cAddress, uint8_t *data, uint8_t dataLenght);
    bool memoryWrite(uint8_t i2cAddress, uint8_t i2cRegister, uint8_t *data, uint8_t dataLenght);
    bool read(uint8_t i2cAddress, uint8_t *readed, uint8_t readedLenght);
    bool memoryRead(uint8_t i2cAddress, uint16_t i2cRegister, uint8_t *readed, uint8_t readedLenght); 
    
  private:
    OneWire *oneWire;
    uint8_t *address;
    
    bool _writeTo(uint8_t *header, uint8_t headerLenght, uint8_t *data, uint8_t dataLenght);
    bool _readFrom(uint8_t *header, uint8_t headerLenght, uint8_t *readed, uint8_t readedLenght);       
};

#endif