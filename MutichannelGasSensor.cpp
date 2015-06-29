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
int16_t MutichannelGasSensor::readData(uint8_t cmd)
{
    uint16_t timeout = 0;
    uint8_t buffer[4];
    uint8_t checksum = 0;
    int16_t rtnData = 0;

    //send command
    sendI2C(cmd);
    //wait for a while
    delay(2);
    //get response
    Wire.requestFrom(i2cAddress, (uint8_t)4);    // request 4 bytes from slave device
    while(Wire.available() == 0)
    {
        if(timeout++ > 500)
            return -2;//time out
        delay(2);
    }
    if(Wire.available() != 4)
        return -3;//rtnData length wrong
    buffer[0] = Wire.read();
    buffer[1] = Wire.read();
    buffer[2] = Wire.read();
    buffer[3] = Wire.read();
    checksum = (uint8_t)(buffer[0] + buffer[1] + buffer[2]);
    if(checksum != buffer[3])
        return -4;//checksum wrong
    rtnData = ((buffer[1] << 8) + buffer[2]);
    
    return rtnData;//successful
}

/*********************************************************************************************************
** Function name:           readR0
** Descriptions:            read R0 stored in slave MCU
*********************************************************************************************************/
int16_t MutichannelGasSensor::readR0(void)
{
    int16_t rtnData = 0;

    rtnData = readData(0x11);
    if(rtnData >= 0)
        res0[0] = rtnData;
    else
        return -1;//unsuccessful

    rtnData = readData(0x12);
    if(rtnData >= 0)
        res0[1] = rtnData;
    else
        return -1;//unsuccessful

    rtnData = readData(0x13);
    if(rtnData >= 0)
        res0[2] = rtnData;
    else
        return -1;//unsuccessful

    return 0;//successful
}

/*********************************************************************************************************
** Function name:           readR
** Descriptions:            read resistance value of each channel from slave MCU
*********************************************************************************************************/
int16_t MutichannelGasSensor::readR(void)
{
    int16_t rtnData = 0;

    rtnData = readData(0x01);
    if(rtnData >= 0)
        res[0] = rtnData;
    else
        return -1;//unsuccessful

    rtnData = readData(0x02);
    if(rtnData >= 0)
        res[1] = rtnData;
    else
        return -1;//unsuccessful

    rtnData = readData(0x03);
    if(rtnData >= 0)
        res[2] = rtnData;
    else
        return -1;//unsuccessful

    return 0;//successful
}

/*********************************************************************************************************
** Function name:           readR
** Descriptions:            calculate gas concentration of each channel from slave MCU
*********************************************************************************************************/
void MutichannelGasSensor::calcGas(void)
{
    
    float ratio0 = (float)res[0] / res0[0];
    if(ratio0 < 0.04) ratio0 = 0.04;
    if(ratio0 > 0.8) ratio0 = 0.8;
    float ratio1 = (float)res[1] / res0[1];
    if(ratio1 < 0.01) ratio1 = 0.01;
    if(ratio1 > 3) ratio1 = 3;
    float ratio2 = (float)res[2] / res0[2];
    if(ratio2 < 0.07) ratio2 = 0.07;
    if(ratio2 > 40) ratio2 = 40;
    
    density_nh3 = 1 / (ratio0 * ratio0 * pow(10, 0.4));
    density_co = pow(10, 0.6) / pow(ratio1, 1.2);
    density_no2 = ratio2 / pow(10, 0.8);
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
