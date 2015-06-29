/*
    This is a demo to test MutichannelGasSensor library
    This code is running on Xadow-mainboard, and the I2C slave is Xadow-MutichannelGasSensor
    There is a ATmega168PA on Xadow-MutichannelGasSensor, it get sensors output and feed back to master.
    the data is raw ADC value, algorithm should be realized on master.
    
    please feel free to write email to me if there is any question 
    
    Jacky Zhang, Embedded Software Engineer
    qi.zhang@seeed.cc
    17,mar,2015
*/

#include "xadow.h"
#include <Wire.h>
#include "MutichannelGasSensor.h"

void setup()
{
    Xadow.init();

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
    
    mutichannelGasSensor.calcGas();
    Serial.print("NH3: ");
    Serial.print(mutichannelGasSensor.density_nh3);
    Serial.println("ppm");
    Serial.print("CO: ");
    Serial.print(mutichannelGasSensor.density_co);
    Serial.println("ppm");
    Serial.print("NO2: ");
    Serial.print(mutichannelGasSensor.density_no2);
    Serial.println("ppm");
    
    Xadow.greenLed(LEDON);
    delay(500);
    Xadow.greenLed(LEDOFF);
    delay(500);
    Serial.println("...");
}