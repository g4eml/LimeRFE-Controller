
//LimeRFE Stand Alone Controller

// Tested with Arduino Pro mini 3.3V version but should work on other types.
// SH1106 I2C OLED display (128 x 64) and LimeRFE on A4 A5
// push buttons on 1,2 and 3
//


// The OLED displays seem to have multiple controller types. 
// The one in this sketch used the SH1106 chip and needs the modified library from https://github.com/wonho-maker/Adafruit_SH1106
// Alternatives use the SSD1306 chip and should work with the standard Adafruit SSD1306 library. 
// The Adafruit Graphics library is also needed. 


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <EEPROM.h>

#define Up 3
#define Enter 2
#define Down 4

const char *  bandName[]= {
              "1-1000 MHz",
              "1.0-4.0 GHz",
              "1-30 MHz",
              "50-70 MHz",
              "140-150 MHz",
              "219-225 MHz",
              "400-450 MHz",
              "902-928 MHz",
              "1.2-1.4 GHz",
              "2.3-2.5 GHz",
              "3.3-3.7 GHz",
              "Cell Band1",
              "Cell Band2",
              "Cell Band3",
              "Cell Band7",
              "Cell Band38"
};

const char *  conName[]= {
              "Default",
              "J3",
              "J4",
              "J5"
};

const char *  notchName[]= {
              "Off",
              "On"
};

const char *  modeName[]= {
              "Rx",
              "Tx",
              "Off"
};


#define OLED_RESET 5      //not used but needed by library

Adafruit_SH1106 display(OLED_RESET);

int8_t rxChannel;
int8_t txChannel;
int8_t rxCon;
int8_t txCon;
int8_t notch;
int8_t rxAtt;
int8_t mode;
int8_t swrEn;
int8_t swrSrc;
int8_t fan;

int8_t selectedRow;
int8_t lastSelected;
int8_t loopCount;
bool itemSelected;

#define RFEaddress 0x51

char buffer[16];

void setup()   
{           

  // by default, we'll generate the high voltage from the 3.3v line internally
  display.begin(SH1106_SWITCHCAPVCC, 0x3c);  // initialize with the I2C addr 0x3c (for the 128x64)  This address might be different for other displays.
  // init done
  display.clearDisplay();   // clears the screen and buffer
 
  pinMode(Up,INPUT_PULLUP);
  pinMode(Enter,INPUT_PULLUP);
  pinMode(Down,INPUT_PULLUP);
  
 rxChannel=9;
 txChannel=9;
 rxCon=1;
 txCon=1;
 notch=0;
 rxAtt=7;
 mode=2;
 swrEn=0;
 swrSrc=0;
 fan=0;

 getConfig();
 
 loopCount=0;

 selectedRow=0;
 lastSelected=1;
 itemSelected=false;
 display.clearDisplay();
 display.setTextSize(2);
 display.setTextColor(WHITE);
 display.setCursor(0,0);
 display.print(F(" Lime RFE"));
 display.setCursor(0,24);
 display.print(F("Controller"));
 display.setCursor(0,48);
 display.print(F(" By G4EML")); 
 display.display();
 delay(2000);
 updateScreen();
 setRFEconfig(); 
 } 

 

void loop() 
{
  loopCount=loopCount+1;
  if (loopCount > 49)
  {
     getRFEconfig();
     updateScreen();
     loopCount=0;
  }

if(getButton() == Enter)
{
  selectRow();
}

if(getButton() == Up)
{
  if(mode != 1)
  {
    mode=1;
    setRFEmode();
    updateScreen();
  }
}

if(getButton() == Down)
{
  if(mode != 0)
  {
    mode=0;
    setRFEmode();
    updateScreen();
  }
}


delay(10);
}


void selectRow(void)
{
  uint8_t but;
  
  selectedRow=lastSelected;
  updateScreen();
  do
  {
    while(getButton()!=0)
    {
      delay(100);
    }
      while(getButton() == 0)
    {
      delay(100);
    }
    but=getButton();
    switch(but)
    {
      case(Up):
        selectedRow=selectedRow-1;
        if(selectedRow < 1) selectedRow=1;
        updateScreen();
        break;
      case(Down):
        selectedRow=selectedRow+1;
        if(selectedRow > 7) selectedRow=7;
        updateScreen();
        break;    
      case(Enter):
        itemSelected=true;
        updateScreen();
        changeSetting();
        break;
    } 
  } while(selectedRow > 0); 

   while(getButton()!=0)
    {
      delay(100);
    }
}

void changeSetting(void)
{
   uint8_t but;
  do
  { 
    while(getButton()!=0)
    {
      delay(100);
    }
    while(getButton() == 0)
    {
      delay(100);
    }
    but=getButton();
    switch(but)
    {
      case(Up):
         switch(selectedRow)
         {
          case(1):
            rxChannel = rxChannel + 1;
            if(rxChannel > 16) rxChannel = 16;
            if(rxChannel > 11) txChannel=rxChannel;
          break;
          case(2):
            txChannel = txChannel + 1;
            if(txChannel > 16) txChannel = 16;
            if(txChannel > 11) rxChannel=txChannel;
          break;
          case(3):
            rxCon = 3;
          break;
          case(4):
            txCon = 2;
          break;
          case(5):
            rxAtt = rxAtt + 1;
            if(rxAtt > 7) rxAtt = 7;
          break;
          case(6):
            notch = 1;
          case(7):
            mode=mode+1;
            if(mode > 2) mode =2;
          break;
         }
        rxCon=rxconlimit(rxCon);
        txCon=txconlimit(txCon);
        updateScreen();
        break;
      case(Down):
         switch(selectedRow)
         {
          case(1):
            rxChannel = rxChannel - 1;
            if(rxChannel < 1) rxChannel = 1;
            if(rxChannel > 10) txChannel = rxChannel;
          break;
          case(2):
            txChannel = txChannel - 1;
            if(txChannel < 1 ) txChannel = 1;
            if(txChannel > 10) rxChannel = txChannel;
          break;
          case(3):
            rxCon = 1;
          break;
          case(4):
            txCon = 1;
          break;
          case(5):
            rxAtt = rxAtt - 1;
            if(rxAtt < 0) rxAtt = 0;
          break;
          case(6):
            notch = 0;
          case(7):
            mode=mode-1;
            if(mode < 0) mode=0;
          break;
         }
        rxCon=rxconlimit(rxCon);
        txCon=txconlimit(txCon);
        updateScreen();
        break;    
      case(Enter):
        setRFEconfig();
        lastSelected=selectedRow;
        selectedRow=0;
        itemSelected=false;
        updateScreen();
        break;
    }  
  } while(selectedRow >0);
}

uint8_t rxconlimit(uint8_t val)
{
  if((rxChannel > 7) | (rxChannel == 2))
  {
    return 1;
  }
  else
  {
    if(val == 1)
    {
      return 1;
    }
    else
    {
      return 3;
    }
  }
}


uint8_t txconlimit(uint8_t val)
{
  if(txChannel > 11)
  {
    return 1;
  }

  if((txChannel == 3) | (txChannel == 4))
  {
    return 3;
  }

  if(val == 1)
  {
    return 1;
  }

  if(val >  1)
  {
    return 2;
  }
}

uint8_t getButton(void)
{
  if(digitalRead(Up) ==0) 
  {
    return Up;
  }
  else if(digitalRead(Down) ==0)
  {
    return Down;
  }
    else if(digitalRead(Enter) ==0)
  {
    return Enter;
  }
  else
  {
    return 0;    
  }
}



void updateScreen() 
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print(F("RX Band: "));
  display.print(bandName[rxChannel-1]);
  display.setCursor(0,9);
  display.print(F("TX Band: "));
  display.print(bandName[txChannel-1]);
  display.setCursor(0,18);
  display.print(F("Rx Conn: "));
  display.print(conName[rxCon]); 
  display.setCursor(0,27);
  display.print(F("Tx Conn: "));
  display.print(conName[txCon]);  
  display.setCursor(0,36);
  display.print(F("Rx Att : "));
  display.print(rxAtt << 1);
  display.print(F(" dB")); 
  display.setCursor(0,45);
  display.print(F("Notch  : "));
  display.print(notchName[notch]); 
  display.setCursor(0,54);
  display.print(F("Mode   : "));
  display.print(modeName[mode]);   


  for(uint8_t i=0;i<7;i++)
  {
    if(selectedRow == i+1)
      {
        display.setCursor(120,i*9);
        display.setCursor(120,i*9);
        display.print(F("<"));
        if(itemSelected) 
        {
          display.setCursor(46,i*9);
          display.print(F(">"));
        }
      }
  }

  display.display();
}

void getRFEconfig(void)
{
  uint8_t i;
  Wire.beginTransmission(RFEaddress);
  Wire.write(0xE3);             //get config command
  for(i=0;i<15;i++)
  {
    Wire.write(0);              //15 dummy bytes to make upp the packet
  }
  Wire.endTransmission();
  
  Wire.requestFrom(RFEaddress,16);

  for(i=0;i<16;i++)
  {
    buffer[i] = Wire.read();
  }
    rxChannel=buffer[1];
    txChannel=buffer[2];
    rxCon=buffer[3];
    txCon=buffer[4];
    mode=buffer[5];
    notch=buffer[6];
    rxAtt=buffer[7];
    swrEn=buffer[8];
    swrSrc=buffer[9];
    fan=buffer[10];

}


void setRFEconfig(void)
{
    
  uint8_t i;
  buffer[0]= 0xD2;            //set Config command
  buffer[1]=rxChannel;
  buffer[2]=txChannel;
  buffer[3]=rxCon;
  buffer[4]=txCon;
  buffer[5]=mode;
  buffer[6]=notch;
  buffer[7]=rxAtt;
  buffer[8]=swrEn;
  buffer[9]=swrSrc;
  buffer[10]=fan;
  buffer[11]=0;
  buffer[12]=0;
  buffer[13]=0;
  buffer[14]=0;
  buffer[15]=0;

  
  Wire.beginTransmission(RFEaddress);
  for(i=0;i<16;i++)
  {
    Wire.write(buffer[i]);             
  }
  Wire.endTransmission();

  saveConfig();
}

void setRFEmode(void)
{
  uint8_t i;
  buffer[0]= 0xD1;            //set Mode command
  buffer[1]=mode;
  
  Wire.beginTransmission(RFEaddress);
  for(i=0;i<2;i++)
  {
    Wire.write(buffer[i]);             
  }
  Wire.endTransmission();
  
}

void saveConfig(void)
{
  EEPROM.write(0,0x55);
  EEPROM.write(1,rxChannel);
  EEPROM.write(2,txChannel);
  EEPROM.write(3,rxCon);
  EEPROM.write(4,txCon);
  EEPROM.write(5,mode);
  EEPROM.write(6,notch);
  EEPROM.write(7,rxAtt);
  EEPROM.write(8,swrEn);
  EEPROM.write(9,swrSrc);
  EEPROM.write(10,fan);
}

void getConfig(void)
{
  if (EEPROM.read(0) == 0x55)
  {
    rxChannel=EEPROM.read(1);
    txChannel=EEPROM.read(2); 
    rxCon=EEPROM.read(3);
    txCon=EEPROM.read(4); 
    mode=EEPROM.read(5);
    notch=EEPROM.read(6); 
    rxAtt=EEPROM.read(7);
    swrEn=EEPROM.read(8); 
    swrSrc=EEPROM.read(9);
    fan=EEPROM.read(10); 
   }

}
