/*
    This is a demo to test MutichannelGasSensor library
    This code is running on Arduino, and the I2C slave is Grove-MutichannelGasSensor
    There is a ATmega168PA on Grove-MutichannelGasSensor, it get sensors output and feed back to master.
    the data is raw resistance value, algorithm should be realized on master.
    
    please feel free to write email to me if there is any question 
    
    Jacky Zhang, Embedded Software Engineer
    email: qi.zhang@seeed.cc
    10, Apr, 2015
*/


#include <Wire.h>
#include "MutichannelGasSensor.h"

void setup()
{
    Serial.begin(9600);  // start serial for output
    Serial.println("power on!");

    mutichannelGasSensor.begin(0x04);//the default I2C address of the slave is 0x04
    //mutichannelGasSensor.changeI2cAddr(0x10);
    //mutichannelGasSensor.doCalibrate();
    //delay(8000);
    while(mutichannelGasSensor.readR0() < 0)
    {
        Serial.println("sensors init error!!");
        delay(1000);
    }
    Serial.print("Res0[0]: ");
    Serial.println(mutichannelGasSensor.res0[0]);
    Serial.print("Res0[1]: ");
    Serial.println(mutichannelGasSensor.res0[1]);
    Serial.print("Res0[2]: ");
    Serial.println(mutichannelGasSensor.res0[2]);
    mutichannelGasSensor.powerOn();
}

void loop()
{
    mutichannelGasSensor.readR();
    Serial.print("Res[0]: ");
    Serial.println(mutichannelGasSensor.res[0]);
    Serial.print("Res[1]: ");
    Serial.println(mutichannelGasSensor.res[1]);
    Serial.print("Res[2]: ");
    Serial.println(mutichannelGasSensor.res[2]);
    
    delay(1000);
    Serial.println("...");
}