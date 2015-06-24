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

float nh3 = 0, co = 0, no2 = 0;
void setup()
{
    Serial.begin(9600);  // start serial for output
    Serial.println("power on!");

    mutichannelGasSensor.begin(0x04);//the default I2C address of the slave is 0x04
    //mutichannelGasSensor.changeI2cAddr(0x10);
    Serial.println("calibrate");
    mutichannelGasSensor.doCalibrate();
    delay(35000);
    Serial.println("done!");
    mutichannelGasSensor.powerOn();
}

void loop()
{
    nh3 = mutichannelGasSensor.readData(0x01);
    delay(10);
    co = mutichannelGasSensor.readData(0x02);
    delay(10);
    no2 = mutichannelGasSensor.readData(0x03);
    delay(10);
    Serial.print("nh3: ");
    Serial.print(nh3);
    Serial.println("ppm");
    Serial.print("co: ");
    Serial.print(co);
    Serial.println("ppm");
    Serial.print("no2: ");
    Serial.print(no2);
    Serial.println("ppm");
    
    delay(1000);
    Serial.println("...");
}
