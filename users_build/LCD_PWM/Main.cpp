#include "wio.h"
#include "suli2.h"
#include "Main.h"
#include <Wire.h>
//#include "grove_oled_12864.h"
#include "LiquidCrystal_I2C.h"
#include "grove_generic_digital_in.h"
int pinsda = 4;
int pinscl = 5;
int pcf8591Addr = 0;

//GroveOLED12864 *oled;
#define PCF8591 0x48
#define LCD2004 0x3F
#define LCD1602 0x27

void findI2CDevice();

/********** for PCF8591 ADC DAC I2C ***********/
#include <Wire.h>

#define PCF8591ADC0 0x00 // control bytes for reading individual ADCs
#define PCF8591ADC1 0x01
#define PCF8591ADC2 0x02
#define PCF8591ADC3 0x03
#define PCF8591DAC0 0x40

#define KEYX_ADC    PCF8591ADC0
#define KEYY_ADC    PCF8591ADC1
#define KEYZ_ADC    PCF8591ADC2
#define SOUND_DAC   PCF8591DAC0

#define key_none  0
#define key_click 1
#define key_up    2
#define key_down  3
#define key_left  4
#define key_right 5
#define key_notfound 6
#define key_max   6

//#define DBG
//#define LCD_DBG

int Gtarget_duty_percent = 0;


int flag = 0;

uint32_t last_time;
LiquidCrystal_I2C lcd1602(LCD1602,16,2);
LiquidCrystal_I2C lcd2004(LCD2004,20,4);
LiquidCrystal_I2C *lcd = &lcd1602;


void PCF8591_setup()
{
 Wire.begin();
}

byte PCF8591_ReadADC(uint8_t cmd)
{
    byte value, error;
    Wire.beginTransmission(pcf8591Addr);
    error = Wire.write(cmd); // control byte - read ADC
    Wire.endTransmission(); // end tranmission
//    if (error == 0) {
        Wire.requestFrom(pcf8591Addr, 2);
        value=Wire.read();
        value=Wire.read();

        return value;
//    }
//    Serial1.println("write command error ");
//    Serial1.println(pcf8591Addr);
//    Serial1.println(cmd);
//    pcf8591Addr = 0;
 //   return 0x90; // return a undetect value if error
}

void PCF8591_WriteDAC(uint8_t value)
{
    byte error;
    Serial1.print(value);
    Wire.beginTransmission(pcf8591Addr);
    error = Wire.write(PCF8591DAC0);
    Wire.write(value);
    Wire.endTransmission();
}

typedef struct _sSoundBee {
    uint8 type;
    uint8 stage;
    uint8 count;
    uint8 value;
} sSoundBee;

typedef struct _sSoundBeeType {
    uint8 off;
    uint8 on;
} sSoundBeeType;
/* */
#define BEE_LONG1   0x0A10
#define BEE_SHORT1  0x0304
#define MAX_SOUND_TYPE 4
#define MAX_SOUND_STAGE 3
uint16 GSoundType[MAX_SOUND_TYPE][MAX_SOUND_STAGE] = 
    {{0x0103,0,0}, // Long - short - short
     {0x0204,0,0}, // Lone
     {0x0304,0,0}, // short short
     {0x0405,0,0}  // short
    };

sSoundBee GSoundBee = {0,0,0,0};
sSoundBee GSoundBeeNext;

void SoundBee_Set(uint8 type, uint8 value)
{
    sSoundBee *pSoundBee;
    if(GSoundBee.stage != 0)
        pSoundBee = &GSoundBeeNext;
    else
        pSoundBee = &GSoundBee;
    memset(pSoundBee, 0, sizeof(sSoundBee));
    if(type >= MAX_SOUND_TYPE)
        type = 3;
    pSoundBee->type = type;
    pSoundBee->stage = 1;
    pSoundBee->value = value;
}

void SoundBee_Routine(void)
{
    sSoundBeeType *pSoundStage;
    if(GSoundBee.stage != 0) {
       // play sound
       if((GSoundBee.stage > MAX_SOUND_STAGE) || (GSoundBee.type > (MAX_SOUND_TYPE-1))) {
         Serial1.print("GSoundBee Error!!");
         memset(&GSoundBee, 0, sizeof(sSoundBee));
         return;
       }
       GSoundBee.count++;
       pSoundStage = (sSoundBeeType *)&GSoundType[GSoundBee.type][GSoundBee.stage-1];

       if(pSoundStage->on != pSoundStage->off) {
            if(GSoundBee.count > pSoundStage->off) {
            // change to next sound stage
                
                if(GSoundBee.stage == MAX_SOUND_STAGE) {
                    // play finish
                    memset(&GSoundBee, 0, sizeof(sSoundBee));
                } else {
                    // into next stage
                    GSoundBee.stage++;
                }
            } else if(GSoundBee.count > pSoundStage->on) {
                // Stop Sound
                PCF8591_WriteDAC(0);
            } else {
                // Start Sound
                PCF8591_WriteDAC(GSoundBee.value);
            }  
       } else {
            // play finish
            memset(&GSoundBee, 0, sizeof(sSoundBee));
       }
        
    } else if(GSoundBeeNext.stage != 0) {
      // prepare next sound
      memcpy(&GSoundBee, &GSoundBeeNext, sizeof(sSoundBee));
      memset(&GSoundBeeNext, 0, sizeof(sSoundBee));
    }


}


byte PCF8691_ADC_Scan_Key()
{
    byte KeyX, KeyY, KeyZ;
    if(pcf8591Addr == 0) {
        findI2CDevice();
        if (pcf8591Addr == 0) {
            return key_notfound;
        }
    }

    KeyZ = PCF8591_ReadADC(KEYZ_ADC);
    KeyX = PCF8591_ReadADC(KEYX_ADC);
    KeyY = PCF8591_ReadADC(KEYY_ADC);
#ifdef DBG
    Serial1.print("KeyZ=");
    Serial1.print(KeyZ);
    Serial1.print(",X=");
    Serial1.print(KeyX);
    Serial1.print(",Y=");
    Serial1.println(KeyY);
#endif

    if(KeyX < 30 || KeyY < 30) {
        if (KeyY < KeyX)
            return key_up;
        else
            return key_left;
    }
    if(KeyX > 0xE0 || KeyY > 0xE0) {
        if (KeyY > KeyX)
            return key_down;
        else
            return key_right;
    }
    if(KeyZ == 0) {
           return key_click;
    }
    return key_none;
}

void PCF8691_ADC_Key_debug(byte Key)
{
#ifdef LCD_DBG
    int value = Key;
    lcd->setCursor(9,1);
    lcd->print("Key:");
    lcd->print(value);
    lcd->print("  ");
#endif

    if(Key == key_none)
        return;
    Serial1.print("Key:");
    switch(Key) {
        case key_click:
            Serial1.println("click");
            break;
        case key_up:
            Serial1.println("up");
            break;
        case key_down:
            Serial1.println("down");
            break;
        case key_left:
            Serial1.println("left");
            break;
        case key_right:
            Serial1.println("right");
            break;
        case key_notfound:
            Serial1.println("Key Not Found");
            break;
        default:
            Serial1.println("Unknow Key");
            break;
    }
}

/************End of PCF8591  *****************/

int key_MapToVol(byte key)
{
    int value = 0;
    if(key == key_notfound) {
        lcd->setCursor(9,1);
        lcd->print("No Key!");
        value = 0;
    }
    else {
        if(key != key_none) {
            switch(key) {
                case key_up:
                    value = 10;
                    break;
                case key_down:
                    value = -10;
                    break;
                case key_right:
                    value = 1;
                    break; 
                case key_left:
                    value = -1;
                    break;
                case key_click:
                    value = 0;
                    break;
            }
        }
    }
    return value;
}

void Menu_Speed_Entry(void)
{
    lcd->backlight();
    lcd->setCursor(0,0);
    lcd->print("Adjust PWM value ");
    lcd->setCursor(0,1);
    lcd->print("PWM:   %");
}

void Menu_Speed_Adj(byte key)
{
    int value;
    float duty_percent;
    static int pause_value = 0;
    /*change speed*/
    GenericPWMOutD0_ins->read_pwm(&duty_percent, NULL);
    if(key == key_click) {
        if(duty_percent > 0) {
            pause_value = Gtarget_duty_percent;
            Gtarget_duty_percent = 0;
        } else {
            Gtarget_duty_percent = pause_value;
        }
    } else {
        Gtarget_duty_percent += key_MapToVol(key);
    }

    if(Gtarget_duty_percent < 0)
        Gtarget_duty_percent = 0;
    else if (Gtarget_duty_percent > 100)
        Gtarget_duty_percent = 100;

    /*display on lcd*/
    value = Gtarget_duty_percent;

    lcd->setCursor(4,1);
    lcd->print("   ");
    lcd->setCursor(4,1);
    lcd->print(value);
}

#define MAX_ADJ_SPEED    5
void Speed_Routine(void)
{
    int value;
    float duty_percent;
    GenericPWMOutD0_ins->read_pwm(&duty_percent, NULL);
    value = (int)duty_percent;
    if(Gtarget_duty_percent != value) {
        if(Gtarget_duty_percent == 0) {
            value = 0;
        } else if(abs(Gtarget_duty_percent - value) > MAX_ADJ_SPEED) {
            if(Gtarget_duty_percent > value)
                value += MAX_ADJ_SPEED;
            else
                value -= MAX_ADJ_SPEED;
        } else {
            value = Gtarget_duty_percent;
        }
#ifdef LCD_DBG
    lcd->setCursor(4,2);
    lcd->print("   ");
    lcd->setCursor(4,2);
    lcd->print(value);
#endif
        duty_percent = (float)value;
        GenericPWMOutD0_ins->write_pwm(duty_percent);
    }
}
void setup()
{
    float duty_percent = 0.0f;
    uint32_t freq = 8000;
    findI2CDevice();
    GenericPWMOutD0_ins->write_pwm_with_freq(duty_percent, freq);
    lcd->init();
    Menu_Speed_Entry();
    last_time = millis();
    Serial1.println("\n***********setup done****************\n");
}

void loop()
{
    int value;
    byte key;
    static byte last_key = key_none;
    static uint8_t key_rep = 0;
    uint8_t u8value;
    uint8_t u8Key;
    uint32_t t = millis();
    // 0.1 Sec base
    if (t - last_time > 100)
    {
        last_time = t;
        /*ScanKey*/
        key = PCF8691_ADC_Scan_Key();
        /*Repeat Key if more than 1 Second*/
        if((key > key_click) && (key < key_notfound))
        {
            key_rep++;
            if(key_rep > 9) {
                last_key = key_none;
            }
        }
        if(key != last_key) {
#if 1
            PCF8691_ADC_Key_debug(key);
#endif
            Menu_Speed_Adj(key);
            PCF8591_WriteDAC(0xFF);

//            SoundBee_Set(1,0xFF);

            last_key = key;
            key_rep = 0;
        }
//        GroveLCD2004I2C0_ins->write_float(0, 5, duty_percent, 2);
        if(flag % 2 == 0) {
            Speed_Routine();
 //           SoundBee_Routine();
        }
        flag++;

 //       Serial1.print("Key: ");
  //      Serial1.println(u8Key,HEX);
    }
}

void findI2CDevice()
{
  byte error, address;
  int nDevices = 0;
  Wire.begin();
  for(address = 0x20; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial1.print("\n----------I2C device found at address 0x");
      if (address<16) 
        Serial1.print("0");
      Serial1.print(address,HEX);
      Serial1.println("  !");
      if(address == 0x3f) {
        Serial1.print("\n----------Find LCD 2004a");
        lcd = &lcd2004;
      }
      else if(address == PCF8591)
      {
        Serial1.print("\n----------Find PCF8591 ");
        pcf8591Addr = PCF8591;
      }
      nDevices++;
    }
    else if (error==4) 
    {
      Serial1.print("\n------------Unknow error at address 0x");
      if (address<16) 
        Serial1.print("0");
      Serial1.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial1.println("\n****************No I2C devices found**********************\n");
  else
    Serial1.println("\n****************I2C devices done **************************\n");
}
/* The following is an exmaple for ULB */
/*int var1 = 10;
float f = 123.45;
String s;
uint32_t time;

void setup()
{
    s = "this is a string...";
    wio.registerVar("var1", var1);
    wio.registerVar("var2", f);
    wio.registerVar("var3", s);
    time = millis();
}

void loop()
{
    if (millis() - time > 1000)
    {
        time = millis();
        Serial1.println(var1);
    }
}*/
