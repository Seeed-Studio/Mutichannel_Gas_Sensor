/*
    This firmware is for Xadow-MutichannelGasSensor
    There is a ATmega168PA on Xadow-MutichannelGasSensor, it get sensors output and feed back to master.
    the data is raw resistance value, algorithm should be realized on master.
    
    please feel free to write email to me if there is any question 
    
    Jacky Zhang, Embedded Software Engineer
    email: qi.zhang@seeed.cc
    10, Apr, 2015
*/

#include <Wire.h>
#include <EEPROM.h>

//EEPROM address allocation
#define EEPROM_ADDR_OK_FLAG 0x00
#define EEPROM_ADDR_I2C_ADDR 0x01
#define EEPROM_ADDR_OFFSET1_HIGH 0x10
#define EEPROM_ADDR_OFFSET1_LOW 0x11
#define EEPROM_ADDR_OFFSET2_HIGH 0x12
#define EEPROM_ADDR_OFFSET2_LOW 0x13
#define EEPROM_ADDR_OFFSET3_HIGH 0x14
#define EEPROM_ADDR_OFFSET3_LOW 0x15
//the flag of data initialized
#define OK_FLAG 0x50
//define the default I2C slave address
#define DEFAULT_I2C_ADDR 0x04
//ADC input pin
#define NH3_PIN A0
#define CO_PIN A1
#define NO2_PIN A2
#define HEAT_PIN 8
#define LED_PIN 9
#define ADCSENS_PIN A3

//functions declaration
void Calibration(void);
void R0Load(void);
void R0Save(void);
void getSensorsValue();

//I2C slave address
uint8_t i2cAddr;
//save the last command from master
uint8_t recvCmd = 0xff;
//ADC value of sensors 
uint16_t adcInput[3];
//ADC value for filter 
uint16_t adcInputFilter[3][20];
//ADC value filtered
uint16_t adcValue[3];
//sensors res0
uint16_t res0[3] = {0, 0, 0};
//res value
uint16_t res[3];
//calibrate flag
uint8_t doCali = 0;
//gas density
float density_no2 = 0, density_co = 0, density_nh3 = 0;


void setup()
{
    //GPIO initialize
    pinMode(HEAT_PIN, OUTPUT);//for sensors heat enable
    pinMode(LED_PIN, OUTPUT);//for LED
    digitalWrite(HEAT_PIN, HIGH);//not heat the sensors
    digitalWrite(LED_PIN, HIGH);//light off
    
    //get data from EEPROM
    uint8_t okFlag = EEPROM.read(EEPROM_ADDR_OK_FLAG);
    if(okFlag != OK_FLAG)//no data stored yet
    {
        i2cAddr = DEFAULT_I2C_ADDR;
        EEPROM.write(EEPROM_ADDR_I2C_ADDR, i2cAddr);
        Calibration();
        R0Save();
        EEPROM.write(EEPROM_ADDR_OK_FLAG, OK_FLAG);
    }
    else
    {
        i2cAddr = EEPROM.read(EEPROM_ADDR_I2C_ADDR);
        R0Load();
    }

    //I2C set up
    Wire.begin(i2cAddr);          // join i2c bus with address
    Wire.onReceive(receiveCallback);    // register receive callback
    Wire.onRequest(requestCallback);       // register request callback
}

void loop()
{ 
    if(doCali == 1)
    {
        Calibration();
        R0Save();
        doCali = 0;
    }
    
    getSensorsValue();

    delay(10);
}

void Calibration(void)
{
    digitalWrite(HEAT_PIN, LOW);//heat the sensors, start to work
    digitalWrite(LED_PIN, LOW);//light on
    //get R value
    delay(30000);
    for(int i = 0; i < 500; i++)
    {
        getSensorsValue();
        delay(10);
    }
    res0[0] = res[0];
    res0[1] = res[1];
    res0[2] = res[2];
    digitalWrite(HEAT_PIN, HIGH);//not heat the sensors
    digitalWrite(LED_PIN, HIGH);//light off
}

void R0Load(void)
{
    res0[0] = (EEPROM.read(EEPROM_ADDR_OFFSET1_HIGH) << 8) + EEPROM.read(EEPROM_ADDR_OFFSET1_LOW);
    res0[1] = (EEPROM.read(EEPROM_ADDR_OFFSET2_HIGH) << 8) + EEPROM.read(EEPROM_ADDR_OFFSET2_LOW);
    res0[2] = (EEPROM.read(EEPROM_ADDR_OFFSET3_HIGH) << 8) + EEPROM.read(EEPROM_ADDR_OFFSET3_LOW);
}

void R0Save(void)
{
    EEPROM.write(EEPROM_ADDR_OFFSET1_HIGH, (unsigned char)(res0[0] >> 8));
    EEPROM.write(EEPROM_ADDR_OFFSET1_LOW, (unsigned char)res0[0]);
    EEPROM.write(EEPROM_ADDR_OFFSET2_HIGH, (unsigned char)(res0[1] >> 8));
    EEPROM.write(EEPROM_ADDR_OFFSET2_LOW, (unsigned char)res0[1]);
    EEPROM.write(EEPROM_ADDR_OFFSET3_HIGH, (unsigned char)(res0[2] >> 8));
    EEPROM.write(EEPROM_ADDR_OFFSET3_LOW, (unsigned char)res0[2]);
}

void getSensorsValue()
{
    static uint8_t flag = 0;
    uint8_t i, j;
    static uint8_t ptr = 0;
    uint16_t sum;
    
    //data buffer initialize
    if(flag == 0)
    {
        for(i = 0; i < 3; i++)
            for(j = 0; j < 20; j++)
                adcInputFilter[i][j] = 0;
        flag = 1;
    }
    
    //read ADC value as raw data
    adcInput[0] = analogRead(NH3_PIN);
    adcInput[1] = analogRead(CO_PIN);
    adcInput[2] = analogRead(NO2_PIN);
    
    //filter the data
    adcInputFilter[0][ptr] = adcInput[0];
    adcInputFilter[1][ptr] = adcInput[1];
    adcInputFilter[2][ptr] = adcInput[2];
    if(ptr++ == 20) ptr = 0;
    //average
    for(i = 0; i < 3; i++)
    {
        sum = 0;
        for(j = 0; j < 20; j++)
            sum += adcInputFilter[i][j];
        adcValue[i] = sum / 20;
        res[i] = 56 * adcValue[i] / (1024 - adcValue[i]);
    }
    
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

void receiveCallback(int dataCount)
{
    if(dataCount == 1)
    {
        recvCmd  = Wire.read();
        switch(recvCmd)
        {
            case 0x20://power off
                digitalWrite(HEAT_PIN, HIGH);//not heat the sensors, stop to work
                digitalWrite(LED_PIN, HIGH);//light off      
                break;
            case 0x21://power on
                digitalWrite(HEAT_PIN, LOW);//heat the sensors, begin to work
                digitalWrite(LED_PIN, LOW);//light on
                break;
            case 0x22://calibrate sensors
                doCali = 1;
                break;
            
            default:
                break;
        }
    }
    if(dataCount == 2)
    {
        int cmd1 = Wire.read();
        int cmd2 = Wire.read();
        switch(cmd1)
        {
            case 0x23://change I2C address
                i2cAddr = cmd2;
                EEPROM.write(EEPROM_ADDR_I2C_ADDR, cmd2 & 0x7f);
                Wire.begin(i2cAddr);          // join i2c bus with address
                break;    
            default:
                break;
        }
    }
}

void requestCallback()
{
    uint8_t buffer[4];
    
    switch(recvCmd)
    {
        case 0x01://read NH3 value
            buffer[0] = 0x01;
            buffer[1] = *((uint8_t *)(&density_nh3));
            buffer[2] = *((uint8_t *)(&density_nh3) + 1);
            buffer[3] = *((uint8_t *)(&density_nh3) + 2);
            buffer[4] = *((uint8_t *)(&density_nh3) + 3);
            buffer[5] = (uint8_t)(buffer[0] + buffer[1] + buffer[2] + buffer[3] + buffer[4]);
            Wire.write(buffer, 6);
            break;
        case 0x02://read CO value
            buffer[0] = 0x02;
            buffer[1] = *((uint8_t *)(&density_co));
            buffer[2] = *((uint8_t *)(&density_co) + 1);
            buffer[3] = *((uint8_t *)(&density_co) + 2);
            buffer[4] = *((uint8_t *)(&density_co) + 3);
            buffer[5] = (uint8_t)(buffer[0] + buffer[1] + buffer[2] + buffer[3] + buffer[4]);
            Wire.write(buffer, 6);
            break;
        case 0x03://read NO2 value
            buffer[0] = 0x03;
            buffer[1] = *((uint8_t *)(&density_no2));
            buffer[2] = *((uint8_t *)(&density_no2) + 1);
            buffer[3] = *((uint8_t *)(&density_no2) + 2);
            buffer[4] = *((uint8_t *)(&density_no2) + 3);
            buffer[5] = (uint8_t)(buffer[0] + buffer[1] + buffer[2] + buffer[3] + buffer[4]);
            Wire.write(buffer, 6);
            break;
        default:
            break;
    }
}
