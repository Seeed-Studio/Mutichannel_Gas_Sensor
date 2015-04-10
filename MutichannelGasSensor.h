/*
    MutichannelGasSensor.h
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

#ifndef __MUTICHANNELGASSENSOR_H__
#define __MUTICHANNELGASSENSOR_H__

class MutichannelGasSensor{

private:

    uint8_t i2cAddress;//I2C address of this MCU
    
    void sendI2C(unsigned char dta);
    int16_t readData(uint8_t cmd);

public:

    uint16_t res0[3];//sensors res0
    uint16_t res[3];//sensors res
    
    void begin(int address);
    int16_t readR0(void);
    int16_t readR(void);
    void changeI2cAddr(uint8_t newAddr);
    void doCalibrate(void);
    void powerOn(void);
    void powerOff(void);
};

extern MutichannelGasSensor mutichannelGasSensor;

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/