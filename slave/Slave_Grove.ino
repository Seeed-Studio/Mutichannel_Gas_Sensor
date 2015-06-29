
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
#define VINSENS_PIN A3

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
uint16_t adcInput[4];
//ADC value for filter 
uint16_t adcInputFilter[4][20];
//ADC value filtered
uint16_t adcValue[3];
//sensors res0
uint16_t res0[3] = {0, 0, 0};
//res value
uint16_t res[3];
//calibrate flag
uint8_t doCali = 0;



void setup()
{
    //GPIO initialize
    pinMode(HEAT_PIN, OUTPUT);//for sensors heat enable
    pinMode(LED_PIN, OUTPUT);//for LED
    digitalWrite(HEAT_PIN, LOW);//not heat the sensors
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
    digitalWrite(HEAT_PIN, HIGH);//heat the sensors, start to work
    digitalWrite(LED_PIN, LOW);//light on
    //get R value
    for(int i = 0; i < 500; i++)
    {
        getSensorsValue();
        delay(10);
    }
    res0[0] = res[0];
    res0[1] = res[1];
    res0[2] = res[2];
    digitalWrite(HEAT_PIN, LOW);//not heat the sensors
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
    adcInput[3] = analogRead(VINSENS_PIN);
    
    //filter the data
    adcInputFilter[0][ptr] = adcInput[0];
    adcInputFilter[1][ptr] = adcInput[1];
    adcInputFilter[2][ptr] = adcInput[2];
    adcInputFilter[3][ptr] = adcInput[3];
    if(ptr++ == 20) ptr = 0;
    //average
    for(i = 0; i < 4; i++)
    {
        sum = 0;
        for(j = 0; j < 20; j++)
            sum += adcInputFilter[i][j];
        adcValue[i] = sum / 20;
    }
    
    for(i = 0; i < 3; i++)
    {
        //res[i] = 56 * adcValue[i] / (1024 - adcValue[i]);
        //R=56k * ADC_S / (2 * ADC_VIN - ADC_S)
        res[i] = 56 * adcValue[i] / (2 * adcValue[3] - adcValue[i]);
    } 
}

void receiveCallback(int dataCount)
{
    if(dataCount == 1)
    {
        recvCmd  = Wire.read();
        switch(recvCmd)
        {
            case 0x20://power off
                digitalWrite(HEAT_PIN, LOW);//not heat the sensors, stop to work
                digitalWrite(LED_PIN, HIGH);//light off      
                break;
            case 0x21://power on
                digitalWrite(HEAT_PIN, HIGH);//heat the sensors, begin to work
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
        case 0x01://read sensor1 R value
            buffer[0] = 0x01;
            buffer[1] = (uint8_t)(res[0] >> 8);
            buffer[2] = (uint8_t)res[0];
            buffer[3] = (uint8_t)(buffer[0] + buffer[1] + buffer[2]);
            Wire.write(buffer, 4);
            break;
        case 0x02://read sensor2 R value
            buffer[0] = 0x02;
            buffer[1] = (uint8_t)(res[1] >> 8);
            buffer[2] = (uint8_t)res[1];
            buffer[3] = (uint8_t)(buffer[0] + buffer[1] + buffer[2]);
            Wire.write(buffer, 4);
            break;
        case 0x03://read sensor3 R value
            buffer[0] = 0x03;
            buffer[1] = (uint8_t)(res[2] >> 8);
            buffer[2] = (uint8_t)res[2];
            buffer[3] = (uint8_t)(buffer[0] + buffer[1] + buffer[2]);
            Wire.write(buffer, 4);
            break;
            
        case 0x11://read sensor1 R0 value
            buffer[0] = 0x11;
            buffer[1] = (uint8_t)(res0[0] >> 8);
            buffer[2] = (uint8_t)res0[0];
            buffer[3] = (uint8_t)(buffer[0] + buffer[1] + buffer[2]);
            Wire.write(buffer, 4);
            break;
        case 0x12://read sensor2 R0 value
            buffer[0] = 0x12;
            buffer[1] = (uint8_t)(res0[1] >> 8);
            buffer[2] = (uint8_t)res0[1];
            buffer[3] = (uint8_t)(buffer[0] + buffer[1] + buffer[2]);
            Wire.write(buffer, 4);
            break;
        case 0x13://read sensor3 R0 value
            buffer[0] = 0x13;
            buffer[1] = (uint8_t)(res0[2] >> 8);
            buffer[2] = (uint8_t)res0[2];
            buffer[3] = (uint8_t)(buffer[0] + buffer[1] + buffer[2]);
            Wire.write(buffer, 4);
            break;
        default:
            break;
    }
}
