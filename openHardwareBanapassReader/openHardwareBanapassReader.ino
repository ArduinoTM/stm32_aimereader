#define FirmwareVersion "1.0r_rph"


#include <EEPROM.h>

#include <Adafruit_NeoPixel.h>

#define LED_PIN PA6
#define LED_Brightness 32
#define NUMPIXELS 10
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);


#include <Wire.h>
#include <PN532_I2C.h>
PN532_I2C pn532(Wire);
#include "PN532.h"
PN532 nfc(pn532);

bool isReading = 0;
uint64_t lastReport = 0;
uint64_t cardBusy = 0;
char accesscodeBuf[32];
char chipIDBuf[64];

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t displayscrii;

void displayTopDisp(char stat[], uint16_t color) {
  display.fillRect(88, 0, 128, 8, color);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("[openBana]"));
  display.setTextColor(!color);
  display.setCursor(91, 0);
  display.print(stat);
  display.display();
  display.setTextColor(SSD1306_WHITE);
}

void clear_disp_down() {
  display.stopscroll();
  display.fillRect(0, 10, 128, 32, SSD1306_BLACK);

  displayscrii = 0;
}

void scroll_disp(int xx1, char text1[], int xx2, char text2[]) {
  clear_disp_down();
  displayscrii = 1;
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(xx1, 14);
  display.print(text1);
  display.setCursor(xx2, 24);
  display.print(text2);
  display.display();
  display.startscrollleft(0x03, 0x03);
}

void static_disp(int xx1, char text1[], int xx2, char text2[]) {
  clear_disp_down();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(xx1, 14);
  display.print(text1);
  display.setCursor(xx2, 24);
  display.print(text2);
  display.display();
}

void waitnfc() {
  if (!nfc.getFirmwareVersion()) {
    static_disp(18, "NFC Device Error", 9, "Check pn532 status!");
  }
  while (!nfc.getFirmwareVersion()) {
    Serial.println("PN532 Init Error!");
    displayTopDisp("NFCERR", SSD1306_WHITE);
    pixels.fill( pixels.Color(255, 0, 0));
    pixels.show();
    delay(500);

    Serial.println("PN532 Init Error!");
    displayTopDisp("NFCERR", SSD1306_BLACK);
    pixels.fill( pixels.Color(0, 0, 0));
    pixels.show();
    delay(500);
  }
  clear_disp_down();

}

void SendCardNormal() {

  Serial.print("ReadNormalCard");
  for (int i = 0; i < 20; i++)
    Serial.print(accesscodeBuf[i]);
  Serial.print(',');
  for (int i = 0; i < 32; i++)
    Serial.print(chipIDBuf[i]);
  Serial.println("");
  return;
}

void setup() {
  // put your setup code here, to run once:
  delay(50); // for stable power
  //  getConf();

  Serial.begin(115200);
  //    delay(100);
  Serial.dtr(0);
  display.begin(SCREEN_ADDRESS);

  pixels.begin();
  pixels.setBrightness(LED_Brightness);
  pixels.fill( pixels.Color(20, 15, 17));
  pixels.show();

  display.clearDisplay();
  displayTopDisp("Init", SSD1306_WHITE);

  nfc.begin();


  waitnfc();


  nfc.setPassiveActivationRetries(0x10);//设定等待次数
  nfc.SAMConfig();


  displayTopDisp("Ready", SSD1306_BLACK);

  pixels.fill( pixels.Color(10, 15, 10));
  pixels.show();

  static_disp(0, "waiting for game", 0, "no Init Packet");
}

char bbuf[64];
void loop() {
  //  Serial.println("00");
  if (Serial.available()) {
    String recvStr;
    while (Serial.available())
    {
      char nextread = Serial.read();
      recvStr += nextread;
    }
    //    Serial.println("22");
    if (recvStr == "BNRCONNINIT") {

      static_disp(0, "Inited", 0, "Received BNRCONNINIT");

      pixels.fill( pixels.Color(16, 8, 8));
      pixels.show();
      Serial.println("ROGER INITED OBPR");
    }
    //    Serial.println("33");
    if (recvStr == "GETVER") {
      Serial.println(FirmwareVersion);
    }
    if (recvStr == "StartRead") {

      scroll_disp(0, "Reading Card", 0, "AmusIC Banapass Aime");

      pixels.fill( pixels.Color(100, 150, 64));
      pixels.show();

      isReading = 1;
      //      Serial.println("ROGER Begin Read");
    }
    //Serial.println("44");
    //    if (recvStr.substring(0, 10) == "SETGAMELED") { //SETGAMELED_xxx
    //      uint8_t recvValue  = 64;
    //      recvValue = recvStr.substring(11, 14).toInt();
    //
    //      pixels.setBrightness(LED_Brightness);
    //
    //      sprintf(bbuf, "ROGER SETGAMELED = %d", recvValue);
    //      Serial.println(bbuf);
    //    }

  }
  if (isReading) {
    //read and send
    if (millis() - lastReport < cardBusy) return;
    //Serial.println("11");
    //Serial.println("####");
    cardBusy = 0;
    static uint8_t uid[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    static uint8_t uid_t[40] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static uint8_t uidLength;

    // check for FeliCa card
    static uint8_t ret;
    static uint16_t systemCode = 0xFFFF;
    static uint8_t requestCode = 0x01;       // System Code request
    static uint8_t idm[8];
    static uint8_t pmm[8];
    static uint16_t systemCodeResponse;

    ret = nfc.felica_Polling(systemCode, requestCode, idm, pmm, &systemCodeResponse, 500);
    if (ret == 1) {
      for (int i = 0; i < 8; i++) {
        uid_t[i] = idm[i];
      }
      for (int i = 0; i < 8; i++) {
        uid_t[i + 8] = pmm[i];
      }

      formatAccessCode(uid_t, accesscodeBuf);
      formatChipID(16, uid_t, chipIDBuf);
      //      accesscodeBuf[0] &= 0x0F;
      SendCardNormal();
      lastReport = millis();
      cardBusy = 2000;
      uidLength = 0;

      static_disp(0, "Read ed", 0, "Felica");
      isReading = 0;

      pixels.fill( pixels.Color(50, 50, 200));
      pixels.show();
      delay(500);
      pixels.fill( pixels.Color(16, 8, 8));
      pixels.show();



      return;
    }
    //14443 Aime
    //Serial.println("99");
    //14443 Regular | Banapass(only uid ,not calcing real AccessCode)
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, 2000)) {
      for (int i = 0; i < 10; i++) {
        uid_t[i] = uid[i % uidLength];
      }
      //      uid_t[0] &= 0x0F;
      formatAccessCode(uid_t, accesscodeBuf);

      formatChipID(uidLength, uid_t, chipIDBuf);
      SendCardNormal();
      //Serial.println("OK");
      lastReport = millis();
      cardBusy = 2000;
      uidLength = 0;

      static_disp(0, "Read ed", 0, "14443A");
      isReading = 0;
      pixels.fill( pixels.Color(50, 50, 200));
      pixels.show();
      delay(500);

      pixels.fill( pixels.Color(16, 8, 8));
      pixels.show();


      return;
    }
    //    Serial.println("341");
    // no card detected
    lastReport = millis();
    cardBusy = 200;

  }

}




void formatAccessCode(uint8_t* ary, char* buf) {
  sprintf(buf, "%02u%02u%02u%02u%02u%02u%02u%02u%02u%02u", ary[0] % 100, ary[1] % 100, ary[2] % 100, ary[3] % 100, ary[4] % 100, ary[5] % 100, ary[6] % 100, ary[7] % 100, ary[8] % 100, ary[9] % 100);
}
void formatChipID(uint8_t aryLen, uint8_t* ary, char* buf) {
  for (int i = 0; i < 16; i++) {
    sprintf(buf + i * 2, "%02u", ary[i % aryLen] % 100);
  }
}
