/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/nicholas/Documents/Particle/I2C-MCP32F521/src/I2C-MCP32F521.ino"
// /*
//  * Project I2C-MCP32F521
//  * Description: 
//  * Author: Nicholas 
//  * Date: July 16th 2020 
//  */

#include "Wire.h"
#include "particle.h"

void wireErrors(uint8_t i2c_bus_Status);
void setup();
void loop();
#line 11 "/Users/nicholas/Documents/Particle/I2C-MCP32F521/src/I2C-MCP32F521.ino"
typedef struct MCP39F521_Data {
	uint16_t systemStatus;
	uint16_t systemVersion;
	uint16_t voltageRMS;
	uint16_t lineFrequency;
	uint16_t analogInputVoltage;
	int16_t powerFactor;
	uint32_t currentRMS;
	uint32_t activePower;
	uint32_t reactivePower;
	uint32_t apparentPower;
} MCP39F521_Data;

typedef struct MCP39F521_FormattedData {
	uint16_t systemStatus;
	uint16_t systemVersion;
	float voltageRMS;
	float lineFrequency;
	float analogInputVoltage;
	float powerFactor;
	float currentRMS;
	float activePower;
	float reactivePower;
	float apparentPower;
} UMCP39F521_FormattedData;

void wireErrors(uint8_t i2c_bus_Status){
  if(i2c_bus_Status == 0){
    Serial.print("success = "); Serial.println(i2c_bus_Status);
  }else if(i2c_bus_Status == 1){
    Serial.print("Busy timeout upon entering endTransmission() = "); Serial.println(i2c_bus_Status);
  }else if(i2c_bus_Status == 2){
    Serial.print("START bit generation timeout = "); Serial.println(i2c_bus_Status);
  }else if(i2c_bus_Status == 3){
    Serial.print("end of address transmission timeout = "); Serial.println(i2c_bus_Status);
  }else if(i2c_bus_Status == 4){
    Serial.print("Data byte transfer timeout = "); Serial.println(i2c_bus_Status);
  }else if(i2c_bus_Status == 5){
    Serial.print("Data byte transfer succeeded, busy timeout immediately after = "); Serial.println(i2c_bus_Status);
  }
}

void convertRawData(MCP39F521_Data *data, MCP39F521_FormattedData *fData)
{
  fData->voltageRMS = data->voltageRMS/10.0f;
  fData->currentRMS = data->currentRMS/10000.0f;
  fData->lineFrequency = data->lineFrequency/1000.0f;
  // Analog Input Voltage represents ADC output for 10 bit ADC
  // By trial, it's been found that it has a ref voltage of 3.3v
  // So the register value/1023 * 3.3v will give the analog input voltage in volts.
  // analogInputVoltage = RegData/1023.0 * 3.3;
  // Do this on the application side?  
  fData->analogInputVoltage = data->analogInputVoltage/1023.0f*3.3;
  float f;
  unsigned char ch;
  f = ((data->powerFactor & 0x8000)>>15) * -1.0;
  for(ch=14; ch > 3; ch--)
    f += ((data->powerFactor & (1 << ch)) >> ch) * 1.0 / (1 << (15 - ch));
  fData->powerFactor = f;
  fData->activePower = data->activePower/100.0f;
  fData->reactivePower = data->reactivePower/100.0f;
  fData->apparentPower = data->apparentPower/100.0f;
}

constexpr size_t I2C_BUFFER_SIZE = 31;
uint8_t I2C_ADDRESS = 0x74;
uint8_t numBytesToRead = 28;
uint8_t ReadDataBuf[8];
uint8_t byteArray[32];
uint32_t checksumTotal = 0;
uint8_t i2c_bus_Status = 0;
int i;

void setup() {
    WiFi.off();
    Serial.begin(115200);
    Wire.setSpeed(CLOCK_SPEED_100KHZ);
    Wire.stretchClock(true);
    Wire.begin();
}

void loop() 
{ 
  ReadDataBuf[0] = 0xA5; // Header
  ReadDataBuf[1] = 0x08; // Num bytes
  ReadDataBuf[2] = 0x41; // Command - set address pointer
  ReadDataBuf[3] = 0x00;
  ReadDataBuf[4] = 0x02;
  ReadDataBuf[5] = 0x4E; // Command - read register, N bytes
  ReadDataBuf[6] = 0x20; 
  ReadDataBuf[7] = 0x5e; // Checksum 0x05E - computed below
  for(i = 0; i < 7; i++) {
    checksumTotal += ReadDataBuf[i];
  }
  ReadDataBuf[7] = checksumTotal % 256; // 0x5E = 94 
  Serial.print("Checksum = "); Serial.println(ReadDataBuf[7]);
  Wire.beginTransmission(I2C_ADDRESS);
  for(i= 0; i < 8; i++) {
    Wire.write(ReadDataBuf[i]);
  }
  i2c_bus_Status = Wire.endTransmission(true);
  wireErrors(i2c_bus_Status);
  delay(30);
  size_t bytes_read = Wire.requestFrom(WireTransmission(I2C_ADDRESS).quantity(I2C_BUFFER_SIZE).timeout(100ms));
  //uint8_t baseline[I2C_BUFFER_SIZE];
  if ( bytes_read == I2C_BUFFER_SIZE ) {
    uint8_t index = 0;
    while( Wire.available() ) {
      // Sock away the data
      byteArray[index++] = Wire.read();
      Serial.print(byteArray[index]);
    }
  }
  Serial.print("\n");
  delay(1000);
}