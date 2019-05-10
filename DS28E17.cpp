#include "Arduino.h"
#include "DS28E17.h"

// camelCase
// classa SoilSensor
// komentáře do anglictiny

DS28E17::DS28E17()
{
}

//kontruktor
DS28E17::DS28E17(OneWire *oneWireW)
{
  oneWire = oneWireW;
}

//nastaveni adresy senzoru
bool DS28E17::setAddress(uint8_t *sensorAddress)
{
  address = sensorAddress;  
}

//funkce pro probuzeni uspaneho prevodniku
void DS28E17::wakeUp()
{
  oneWire->depower();
  oneWire->reset();
  delay(2);
}

//uspani prevodniku, probouzi se nabeznou hranou
void DS28E17::enableSleepMode()
{
  oneWire->reset();
  oneWire->select(address);
  oneWire->write(DS28E17_ENABLE_SLEEP, 0);    
}

//spolecna cast pro zapis - vypocet CRC, odeslani a precteni stavu
// todo headerLenght
bool DS28E17::_writeTo(uint8_t *header, uint8_t headerLenght, uint8_t *data, uint8_t dataLenght)
{
  uint8_t crc[2];
  uint16_t crc16 = oneWire->crc16(&header[0], headerLenght);
  crc16 = oneWire->crc16(&data[0], dataLenght, crc16);
  crc16 = ~crc16;
  crc[1] = crc16 >> 8;                 
  crc[0] = crc16 & 0xFF;               
  
  oneWire->reset();
  oneWire->select(address);
  oneWire->write_bytes(header, headerLenght, 0);
  oneWire->write_bytes(data, dataLenght, 0);
  oneWire->write_bytes(crc, sizeof(crc), 0);

  uint8_t timeout = 0;
  while (oneWire->read_bit() == true){
    delay(1);
    timeout++;
    if (timeout > ONEWIRE_TIMEOUT){
      oneWire->depower();
      return false;
    }
  }

  uint8_t stat = oneWire->read();
  uint8_t writeStat = oneWire->read();
  
  if ((stat != 0x00) || (writeStat != 0x00)) {
    oneWire->depower();
    return false;
  }
  
  oneWire->depower(); 

  return true;    
}

//zapis libovolne dlouhych dat
bool DS28E17::write(uint8_t i2cAddress, uint8_t *data, uint8_t dataLenght)
{
  uint8_t header[3];
  uint8_t headerLenght = 3;

  header[0] = DS28E17_WRITE;                 // DS28E17 Command
  header[1] = i2cAddress << 1;      // 7 bit i2c Address
  header[2] = dataLenght;           // number of bytes to be written 

  return _writeTo(header, headerLenght, data, dataLenght);    
}

//zapis libovolne dlouhych dat na adresu dlouhou 8b nebo 16b
bool DS28E17::memoryWrite(uint8_t i2cAddress, uint8_t i2cRegister, uint8_t *data, uint8_t dataLenght)
{
  uint8_t header[5];
  uint8_t headerLenght;

  if ((i2cRegister & 0xFF00) != 0) {
    headerLenght = 5;
    header[0] = DS28E17_WRITE;                 // DS28E17 Command
    header[1] = i2cAddress << 1;      // 7 bit i2c Address
    header[2] = dataLenght + 2;       // number of bytes to be written 
    header[3] = i2cRegister >> 8;     // i2c device register address
    header[4] = i2cRegister;          // i2c device register address  
  }
  else {
    headerLenght = 4;
    header[0] = DS28E17_WRITE;                 // DS28E17 Command
    header[1] = i2cAddress << 1;      // 7 bit i2c Address
    header[2] = dataLenght + 1;       // number of bytes to be written 
    header[3] = i2cRegister;          // i2c device register address  
  }
  return _writeTo(header, headerLenght, data, dataLenght);     
}

//spolecna cast pro cteni - vypocet CRC, odeslani pozadavku, precteni stavu a dat
bool DS28E17::_readFrom(uint8_t *header, uint8_t headerLenght, uint8_t *buffer, uint8_t bufferLength)
{   
  uint8_t crc[2];
  uint16_t crc16 = oneWire->crc16(&header[0], headerLenght);
  crc16 = ~crc16;
  crc[1] = crc16 >> 8;                 
  crc[0] = crc16 & 0xFF;               
  
  oneWire->reset();
  oneWire->select(address);
  oneWire->write_bytes(header, headerLenght, 0);
  oneWire->write_bytes(crc, sizeof(crc), 0);

  uint8_t timeout = 0;
  while (oneWire->read_bit() == true){
    delay(1);
    timeout++;
    if (timeout > ONEWIRE_TIMEOUT){
      oneWire->depower();
      return false;
    }
  }
  
  uint8_t stat = oneWire->read();
  uint8_t writeStat = header[0] == DS28E17_READ ? 0 : oneWire->read();
  
  if ((stat != 0x00) || (writeStat != 0x00)) {
    oneWire->depower();
    return false;
  }

  /*Serial.print("Status =");
  Serial.println(stat,BIN);
  Serial.print("Write Status =");
  Serial.println(write_stat,BIN);*/

  for (int i=0; i<bufferLength; i++){
    buffer[i] = oneWire->read();
  }

  oneWire->depower(); 
  
  return true; 
}

//cteni dat pozadovne delky, data jsou predana pres ukazatel
bool DS28E17::read(uint8_t i2cAddress, uint8_t *buffer, uint8_t bufferLength)
{
  uint8_t header[3];
  uint8_t headerLenght = 3;

  header[0] = DS28E17_READ;           // DS28E17 Command
  header[1] = i2cAddress << 1 | 0x01; // 7 bit i2c Address
  header[2] = bufferLength;           // number of bytes to be read

  return _readFrom(header, headerLenght, buffer, bufferLength); 
}

//cteni dat pozadovne delky z adresy 8 nebo 16bit, data jsou predana pres ukazatel
//TODO buffer - buffer/bufferLength
bool DS28E17::memoryRead(uint8_t i2cAddress, uint16_t i2cRegister, uint8_t *buffer, uint8_t bufferLength) 
{
  uint8_t header[6];
  uint8_t headerLenght;

  if ((i2cRegister & 0xFF00) != 0) {
    headerLenght = 6;
    header[0] = DS28E17_MEMMORY_READ; // DS28E17 Command
    header[1] = i2cAddress << 1;      // 7 bit i2c Address
    header[2] = 0x02;                 // number of bytes to be written 
    header[3] = i2cRegister >> 8;     // i2c device register address
    header[4] = i2cRegister;          // i2c device register address  
    header[5] = bufferLength;         // number of bytes to be read
  }
  else {
    headerLenght = 5;
    header[0] = DS28E17_MEMMORY_READ; // DS28E17 Command
    header[1] = i2cAddress << 1;      // 7 bit i2c Address
    header[2] = 0x01;                 // number of bytes to be written 
    header[3] = i2cRegister;          // i2c device register address  
    header[4] = bufferLength;         // number of bytes to be read
  }

  return _readFrom(header, headerLenght, buffer, bufferLength);    
}