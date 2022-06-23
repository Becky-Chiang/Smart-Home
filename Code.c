#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <Servo.h>
#include <TimeLib.h>

SoftwareSerial BT(A0, A1);

char hInput[7]; //輸入
char hMode[6] = "HOME"; //功能模式

char lSwitch[4] = "OFF"; //電燈開關

char fSwitch[4] = "OFF"; //風扇開關
char fMode[7] = "MANUAL"; //風扇模式
char fPower[7] = "LOW"; //風扇轉速
float fNormalTemp = 25.00f; //預期溫度

char dPWRead[5];
char dMode[5];

//智能照明設置
const byte lSensor = 4;
const byte lLedOut = 5;
const byte lLedIn = 6;

unsigned long lTime;

//溫控風扇設置
U8G2_SSD1306_128X64_NONAME_1_HW_I2C fOled(U8G2_R0, U8X8_PIN_NONE);
#define fTemp_Pin 2
OneWire oneWire(fTemp_Pin);
DallasTemperature fTemp(&oneWire);
const byte fFan = 3;

char fTempReadString[6];
const byte fLow = 63; //風扇低速強度
const byte fMedium = 127; //風扇中速強度
const byte fHigh = 255; //風扇高速強度

//智慧門禁設置
U8G2_SSD1306_128X64_NONAME_1_HW_I2C dOled(U8G2_R0, U8X8_PIN_NONE);
#define DOOR_RST 9
#define DOOR_SS 10
#define DOOR_SERVO 8
MFRC522 dRfid;
Servo dServo;
const byte dBuzz = 7;

char dLock[9] = "Locked";
const char dPW[5] = "0000";
char card1[9], card2[9], card3[9];
unsigned long dTime;

void setup() {
  BT.begin(9600);
  SPI.begin();
  
  //智能照明初始化
  pinMode(lSensor, INPUT);
  pinMode(lLedOut, OUTPUT);
  pinMode(lLedIn, OUTPUT);

  //溫控風扇初始化
  fOled.setI2CAddress(0x3C*2);
  fOled.begin();
  showLoading(fOled);
  fTemp.begin();
  pinMode(fFan, OUTPUT);

  //智慧門禁初始化
  dOled.setI2CAddress(0x3D*2);
  dOled.begin();
  showLoading(dOled);
  dRfid.PCD_Init(DOOR_SS, DOOR_RST);
  dServo.attach(DOOR_SERVO, 500, 2400);
  dServo.write(90);
  pinMode(dBuzz, OUTPUT);
}

//OLED熒幕初始化函式
void showLoading(U8G2_SSD1306_128X64_NONAME_1_HW_I2C oled) {
  char loading[] = "...Loading..."; //Loading字元陣列
  char title[14]; //顯示字元陣列
  
  for (byte i=0; i<sizeof(loading); i++) {
    title[i] = loading[i]; //將Loading字元陣列內元素逐一加入顯示字元陣列
    oled.firstPage(); //OLED頁面0
    do {
      oled.setFont(u8g2_font_fur14_tf); //設定OLED顯示字體
      oled.drawStr(2, 39, title); //顯示文字
    } while (oled.nextPage()); //OLED下一頁
    delay(150);
  }
}

void loop() {
  if (BT.available()) { //若有輸入
    ifInput(); //輸入字串函式
    
    if (!strcmp(hInput, "HOME")) { //若輸入為HOME
      strcpy(hMode, hInput); //功能模式切換為電燈
    } else if (!strcmp(hInput, "LIGHT")) { //若輸入為LIGHT
      strcpy(hMode, hInput); //功能模式切換為電燈
    } else if (!strcmp(hInput, "FAN")) { //若輸入為FAN
      strcpy(hMode, hInput); //功能模式切換為風扇
    } else if (!strcmp(hInput, "DOOR")) { //若輸入為DOOR
      strcpy(hMode, hInput); //功能模式切換為門禁
      
    } else if (!strcmp(hMode, "LIGHT")) { //若功能模式為電燈
      if (!strcmp(hInput, "ON")) //若輸入為ON
        strcpy(lSwitch, hInput); //電燈開關切換為ON
      else if (!strcmp(hInput, "OFF")) //若輸入為OFF
        strcpy(lSwitch, hInput); //電燈開關切換為ON
        
    } else if (!strcmp(hMode, "FAN")) { //若功能模式為風扇
      if (!strcmp(hInput, "ON")) //若輸入為ON
        strcpy(fSwitch, hInput); //風扇開關切換為ON
      else if (!strcmp(hInput, "OFF")) //若輸入為OFF
        strcpy(fSwitch, hInput); //風扇開關切換為ON
      else if (!strcmp(hInput, "AUTO")) //若輸入為AUTO
        strcpy(fMode, hInput); //風扇模式切換為自動
      else if (hInput[0] >= '0' && hInput[0] <= '9') //若輸入為0-9
        fNormalTemp = atof(hInput); //預期溫度改為輸入溫度
      else if (!strcmp(hInput, "MANUAL")) //若輸入為MANUAL
        strcpy(fMode, hInput); //風扇模式切換為手動
      else if (!strcmp(hInput, "LOW")) //若輸入為LOW
        strcpy(fPower, hInput); //風扇轉速切換為低速
      else if (!strcmp(hInput, "MEDIUM")) //若輸入為MEDIUM
        strcpy(fPower, hInput); //風扇轉速切換為中速
      else if (!strcmp(hInput, "HIGH")) //若輸入為HIGH
        strcpy(fPower, hInput); //風扇轉速切換為高速
        
    } else if (!strcmp(hMode, "DOOR")) { //若功能模式為門禁
      if (hInput[0] >= '0' && hInput[0] <= '9') //若輸入為0-9
        strcpy(dPWRead, hInput); //讀取輸入密碼
      else if (!strcmp(hInput, "ADD1")) //若輸入為ADD1
        strcpy(dMode, hInput); //門禁功能切換為添加卡1
      else if (!strcmp(hInput, "ADD2")) //若輸入為ADD2
        strcpy(dMode, hInput); //門禁功能切換為添加卡2
      else if (!strcmp(hInput, "ADD3")) //若輸入為ADD3
        strcpy(dMode, hInput); //門禁功能切換為添加卡3
      else if (!strcmp(hInput, "DEL1")) //若輸入為DEL1
        strcpy(dMode, hInput); //門禁功能切換為刪除卡1
      else if (!strcmp(hInput, "DEL2")) //若輸入為DEL2
        strcpy(dMode, hInput); //門禁功能切換為刪除卡2
      else if (!strcmp(hInput, "DEL3")) //若輸入為DEL3
        strcpy(dMode, hInput); //門禁功能切換為刪除卡3
    }
  }

  //手動控制
  if (!strcmp(hMode, "LIGHT")) //若功能模式為電燈
    manualLight();
  else if (!strcmp(hMode, "FAN")) { //若功能模式為風扇
    if (!strcmp(fSwitch, "OFF")) //若風扇開關為OFF
      analogWrite(fFan, 0);
    if (!strcmp(fMode, "MANUAL")) //若風扇模式為AUTO
      manualFan();
  } else if (!strcmp(hMode, "DOOR") && dTime<now()) { //若功能模式為門禁
    pwUnlock();
    if (!strcmp(dMode, "DEL1")) //若門禁功能為DEL1
      delCard(card1);
    else if (!strcmp(dMode, "DEL2")) //若門禁功能為DEL2
      delCard(card2);
    else if (!strcmp(dMode, "DEL3")) //若門禁功能為DEL3
      delCard(card3);
  }
  
  //自動控制
  autoLight();
  showFanOled();
  if (!strcmp(fSwitch, "ON") && !strcmp(fMode, "AUTO")) //若風扇開關為ON和風扇模式為AUTO
    autoFan();
  if (dTime < now()) { //若小於現在時間
    if (!strcmp(dLock, "UnLocked")) closeDoor(); //若門鎖為解鎖
    showOled(dOled, dLock, "..Scan RFID..");

    //檢查有沒有新卡片
    if (!dRfid.PICC_IsNewCardPresent())
      return;
    if (!dRfid.PICC_ReadCardSerial())
      return;
      
    char dCard[9];
    for (byte i=0; i<9; i++)
      dCard[i] = NULL; //將存取卡號內容先設為空白
    readCard(dCard, dRfid.uid.uidByte, dRfid.uid.size);

    if (!strcmp(hMode, "DOOR")) { //若功能模式為門禁
      if (!strcmp(dMode, "ADD1")) { //若門禁功能為添加卡1
        strcpy(card1, dCard); //存取卡號添加到預設卡1
        checkInput();
      } else if (!strcmp(dMode, "ADD2")) { //若門禁功能為添加卡2
        strcpy(card2, dCard); //存取卡號添加到預設卡2
        checkInput();
      } else if (!strcmp(dMode, "ADD3")) { //若門禁功能為添加卡3
        strcpy(card3, dCard); //存取卡號添加到預設卡3
        checkInput();
      } else checkRFID(dCard);
      strcpy(dMode, "NONE"); //門禁模式切換為無
    } else checkRFID(dCard);

    //卡片進入停止模式
    dRfid.PICC_HaltA();
    dRfid.PCD_StopCrypto1();
  }
}

//室外紅外線感測電燈
void autoLight() {
  unsigned short lSensorRead = digitalRead(lSensor); //讀取紅外線感測器
  if (lSensorRead == 0) { //若有人經過
    digitalWrite(lLedOut, HIGH); //開啟室外電燈
    lTime = now() + 5; //將現在的時間增加5秒
  }
  if (lTime < now()) //5秒後，會小於現在時間
    digitalWrite(lLedOut, LOW); //關閉室外電燈
}

//室內手動控制電燈
void manualLight() {
  if (!strcmp(lSwitch, "ON")) //若電燈開關為ON
    digitalWrite(lLedIn, HIGH); //開啟室內電燈
  else if (!strcmp(lSwitch, "OFF")) //若電燈開關為OFF
    digitalWrite(lLedIn, LOW); //關閉室內電燈
}

//溫控風扇
void autoFan() {
  fTemp.requestTemperatures(); //偵測環境
  float fTempRead = fTemp.getTempCByIndex(0); //讀取環境溫度
  dtostrf(fTempRead, 2, 2, fTempReadString); //將環境溫度轉為字串
  if (fTempRead <= fNormalTemp) //若環境溫度<=預期溫度
    analogWrite(fFan, fLow); //風扇轉速設為低速
  else if (fTempRead > fNormalTemp+2) //若環境溫度>預期溫度+2
    analogWrite(fFan, fHigh); //風扇轉速設為高速
  else 
    analogWrite(fFan, fMedium); //風扇轉速設為中速
}

//手動控制風扇
void manualFan() {
  if (!strcmp(fSwitch, "ON")) { //若風扇開關為ON
    if (!strcmp(fPower, "LOW")) //若風扇轉速為LOW
      analogWrite(fFan, fLow); //風扇轉速設為低速
    else if (!strcmp(fPower, "MEDIUM")) //若風扇轉速為MEDIUM
      analogWrite(fFan, fMedium); //風扇轉速設為中速
    else if (!strcmp(fPower, "HIGH")) //若風扇轉速為HIGH
      analogWrite(fFan, fHigh); //風扇轉速設為高速
  }
}

//密碼解開門鎖
void pwUnlock() {
  if (strcmp(dPWRead, "")) {
    if (!strcmp(dPWRead, dPW)) { //若輸入密碼等於設置密碼
      strcpy(dPWRead, ""); //清除輸入密碼
      openDoor();
      showOled(dOled, dLock, "|PW Open|");
    } else {
      strcpy(dPWRead, ""); //清除輸入密碼
      buzzLock();
      showOled(dOled, dLock, "|Wrong PW|");
    }
  }
}

//檢查卡號是否等於預設卡函式
void checkRFID(char card[]) {
  if (!strcmp(card1, card) || !strcmp(card2, card) || !strcmp(card2, card)) { //若卡號等於預設卡1/2/3
    openDoor();
    showOled(dOled, dLock, "|Card Open|");
  } else {
    buzzLock();
    showOled(dOled, dLock, "|Wrong Card|");
  }
}

//輸入字串函式
void ifInput() {
  byte inputSize = BT.available();
  for (byte i=0; i<inputSize; i++) { //讀取字串
    char a = BT.read(); //讀取輸入字元
    if (a != '\r') //若不為換行字元
      hInput[i] = toupper(a); //將英文字母轉為大寫
    hInput[i+1] = '\0'; //結尾添加空白字元
  }
}

//OLED顯示函式
void showOled(U8G2_SSD1306_128X64_NONAME_1_HW_I2C oled, 
char* string1, char* string2) {
  oled.firstPage(); //OLED頁面0
  do {
    oled.setFont(u8g2_font_fur17_tf); //設定OLED顯示字體
    oled.drawStr(2, 27, string1); //顯示文字1
    oled.setFont(u8g2_font_fur14_tf); //設定OLED顯示字體
    oled.drawStr(2,55, string2); //顯示文字2
  } while (oled.nextPage()); //OLED下一頁
}

//風扇OLED顯示函式
void showFanOled() {
  if (!strcmp(fSwitch, "OFF")) //若風扇開關為OFF
    showOled(fOled, fSwitch, "");
  else if (!strcmp(fSwitch, "ON")) { //若風扇開關為ON
    if (!strcmp(fMode, "AUTO")) { //若風扇模式為AUTO
      char showAuto[11];
      strcpy(showAuto, fMode); //將風扇模式字元添加到showAuto
      strcat(showAuto, " "); //將空白字元添加到showAuto
      strcat(showAuto, fTempReadString); //將環境溫度字元添加到showAuto
      showOled(fOled, fSwitch, showAuto);
    } else if (!strcmp(fMode, "MANUAL")) //若風扇模式為MANUAL
      showOled(fOled, fSwitch, fPower);
  }
}

//檢查輸入卡片是否成功函式
void checkInput() {
  digitalWrite(dBuzz, HIGH); //蜂鳴器開啟
  delay(500); //延遲0.5秒
  digitalWrite(dBuzz, LOW); //蜂鳴器關閉

  BT.write(1); //藍牙發送信號

  showOled(dOled, "Card", "Recorded");
}

//刪除卡片函式
void delCard(char card[]) {
  strcpy(card, NULL); //清除欲刪除之預設卡號
  strcpy(dMode, "NONE"); //門禁模式切換為無
}

//蜂鳴器解鎖函式
void buzzUnlock() {
  digitalWrite(dBuzz, HIGH); //蜂鳴器開啟
  delay(1000); //延遲1秒
  digitalWrite(dBuzz, LOW); //蜂鳴器關閉
}

//蜂鳴器封鎖函式
void buzzLock() {
  digitalWrite(dBuzz, HIGH); //蜂鳴器開啟
  delay(500); //延遲0.5秒
  digitalWrite(dBuzz, LOW); //蜂鳴器關閉
  delay(100); //延遲0.1秒
  digitalWrite(dBuzz, HIGH); //蜂鳴器開啟
  delay(500); //延遲0.5秒
  digitalWrite(dBuzz, LOW); //蜂鳴器關閉
  delay(100); //延遲0.1秒
}

//開門函式
void openDoor() {
  buzzUnlock();

  strcpy(dLock, "UnLocked"); //門鎖切換為開鎖
  dServo.write(0); //伺服馬達開門
  dTime = now() + 7; //將現在的時間增加7秒
}

//關門函式
void closeDoor() {
    strcpy(dLock, "Locked"); //門鎖切換為關鎖
    dServo.write(90); //伺服馬達關門
}

//讀取卡號函式
void readCard(char card[], byte* rfidUid, byte rfidUidSize) {
  int a;
  char b[2];
  for (byte i=0; i<rfidUidSize; i++) {
    a = rfidUid[i]; //依序將卡號存入a
    strcat(card, itoa(a, b, 16)); //將a以16進制轉為字元存進card
  }
  for (byte i=0; i<8; i++) {
    card[i] = toupper(card[i]); //將card內英文大寫
  }
}