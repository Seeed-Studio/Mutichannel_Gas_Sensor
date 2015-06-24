/*
    MutichannelGasSensor.cpp
    2015 Copyright (c) Seeed Technology Inc.  All right reserved.

    Author: Jacky Zhang
    2015-3-17
    http://www.seeed.cc/

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Wire.h>
#include <Arduino.h>
#include "MutichannelGasSensor.h"

/*********************************************************************************************************
** Function name:           begin
** Descriptions:            initialize I2C
*********************************************************************************************************/
void MutichannelGasSensor::begin(int address)
{
    Wire.begin();
    i2cAddress = address;
}

/*********************************************************************************************************
** Function name:           sendI2C
** Descriptions:            send one byte to I2C Wire
*********************************************************************************************************/
void MutichannelGasSensor::sendI2C(unsigned char dta)
{
    Wire.beginTransmission(i2cAddress);               // transmit to device #4
    Wire.write(dta);                                    // sends one byte
    Wire.endTransmission();                             // stop transmitting
}

/*********************************************************************************************************
** Function name:           readData
** Descriptions:            read 4 bytes from I2C slave
*********************************************************************************************************/
float MutichannelGasSensor::readData(uint8_t cmd)
{
    uint16_t timeout = 0;
    uint8_t buffer[6];
    uint8_t checksum = 0;
    float rtnData = 0;

    //send command
    sendI2C(cmd);
    //wait for a while
    delay(2);
    //get response
    Wire.requestFrom(i2cAddress, (uint8_t)6);    // request 6 bytes from slave device
    while(Wire.available() == 0)
    {
        if(timeout++ > 20)
            return -2;//time out
        delay(5);
    }
    if(Wire.available() != 6)
        return -3;//rtnData length wrong
    buffer[0] = Wire.read();
    buffer[1] = Wire.read();
    buffer[2] = Wire.read();
    buffer[3] = Wire.read();
    buffer[4] = Wire.read();
    buffer[5] = Wire.read();
    checksum = (uint8_t)(buffer[0] + buffer[1] + buffer[2] + buffer[3] + buffer[4]);
    
    if(checksum != buffer[5])
        return -4;//checksum wrong
    
    if(cmd != buffer[0])
        return -5;//cmd wrong
    
    //all are right
    rtnData = *(float *)(&buffer[1]);
    
    return rtnData;//successful
}

/*********************************************************************************************************
** Function name:           changeI2cAddr
** Descriptions:            change I2C address of the slave MCU, and this address will be stored in EEPROM of slave MCU
*********************************************************************************************************/
void MutichannelGasSensor::changeI2cAddr(uint8_t newAddr)
{
    Wire.beginTransmission(i2cAddress); // transmit to device
    Wire.write(0x23);              // sends one byte
    Wire.write(newAddr);              // sends one byte
    Wire.endTransmission();    // stop transmitting
    i2cAddress = newAddr;
}

/*********************************************************************************************************
** Function name:           doCalibrate
** Descriptions:            tell slave to do a calibration, it will take about 8~10s
*********************************************************************************************************/
void MutichannelGasSensor::doCalibrate(void)
{
    sendI2C(0x22);
}

/*********************************************************************************************************
** Function name:           powerOn
** Descriptions:            power on sensor heater
*********************************************************************************************************/
void MutichannelGasSensor::powerOn(void)
{
    sendI2C(0x21);
}

/*********************************************************************************************************
** Function name:           powerOff
** Descriptions:            power off sensor heater
*********************************************************************************************************/
void MutichannelGasSensor::powerOff(void)
{
    sendI2C(0x20);
}

MutichannelGasSensor mutichannelGasSensor;
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
