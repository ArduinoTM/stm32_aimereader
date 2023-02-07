#define FirmwareVersion "sg_1.0r_rhW"

#include <Adafruit_NeoPixel.h>

#include <EEPROM.h>


#define SerialDevice Serial
#define LED_PIN PA6
#define LED_brightness 64
#define NUMPIXELS 10
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
#include <Wire.h>
#include <PN532_I2C.h>
PN532_I2C pn532(Wire);
//---
bool radio_enabled = 0 , lastradio = 0;
uint64_t lastchange ;
bool firstpac = 0;
bool cardreading = 0 ;
bool displayscrii = 0;
int errorcnt = 0;
bool confmode = 0;
int trcol = 0;
bool lastcmdok = 1;
uint64_t confTime, tempcft1 = 0;
//---
#include "PN532.h"
PN532 nfc(pn532);

#define totalConf 4
class DeviceConfig_t {
  public:
    bool configWritten; //if it is 1 ,do not init
    char dname[32];
    uint8_t Game_LED_Brightness;
    uint8_t NGame_LED_Brightness_div;

    void initthis() {
      this->configWritten = 1;
      this->dname[0] = '\0';
      this->Game_LED_Brightness = 64;
      this->NGame_LED_Brightness_div = 16;
    }

} ReaderConfig;



typedef union {
  uint8_t block[18];
  struct {
    uint8_t IDm[8];
    uint8_t PMm[8];
    union {
      uint16_t SystemCode;
      uint8_t System_Code[2];
    };
  };
} Card;
Card card;

uint8_t AimeRKey[6] = {0x57, 0x43, 0x43, 0x46, 0x76, 0x32};
uint8_t BanaRKey[6] = {0x60, 0x90, 0xD0, 0x06, 0x32, 0xF5};
uint8_t MifareRKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#define M2F_B 1
uint16_t blockList[4] = {0x8080, 0x8081, 0x8082, 0x8083};
uint16_t serviceCodeList[1] = {0x000B};
uint8_t blockData[1][16];
//bool high_baudrate = false;//high_baudrate=true

uint8_t writeBlock2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78};
uint8_t writeBlock3[16] = {0x57, 0x43, 0x43, 0x46, 0x76, 0x32, 0x08, 0x77, 0x8F, 0x11, 0x57, 0x43, 0x43, 0x46, 0x76, 0x32};

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void displayTopDisp(char stat[], uint16_t color) {
  display.fillRect(88, 0, 128, 8, color);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("[Aime Reader]"));
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



void diplayStatus() {

  if (radio_enabled) {
    lastchange = millis() ;
    if (!displayscrii && !cardreading) {
      clear_disp_down();
      scroll_disp(24, "Put card here", 0, "Aime AmusIC Banapass");
      errorcnt = 0;
    }
  }

  if (millis() - lastchange > 1000 && firstpac) {
    errorcnt = 0;
    lastchange = millis();
    clear_disp_down();
    display.display();
  }
}

//max 21 char
void waitnfc() {
  if (!nfc.getFirmwareVersion()) {
    static_disp(18, "NFC Device Error", 9, "Check pn532 status!");
  }
  while (!nfc.getFirmwareVersion()) {

    displayTopDisp("NFCERR", SSD1306_WHITE);
    pixels.fill( pixels.Color(255, 0, 0));
    pixels.show();
    delay(500);

    displayTopDisp("NFCERR", SSD1306_BLACK);
    pixels.fill( pixels.Color(0, 0, 0));
    pixels.show();
    delay(500);
  }
  clear_disp_down();

}

int nowledpos = 0;
uint64_t ledringtime = 0;




void writeConf() {
  int eeAddress = 0;
  //woc 可以直接读写对象
  EEPROM.put(eeAddress, ReaderConfig);

}
void initConf() {
  ReaderConfig.initthis();
  writeConf();
}

void getConf() {
  int eeAddress = 0;
  //  ReaderConfig.configWritten =

  EEPROM.get(eeAddress, ReaderConfig);
  if (!ReaderConfig.configWritten)initConf();
}


void doConf() {
  pixels.setBrightness(ReaderConfig.Game_LED_Brightness);



}

void enterConfMode() {
  confmode = 1;
  static_disp(14, "Reset before game", 22, "Configure Mode");
  confTime = millis();
  ledringtime = millis();
}
void escapeConfMode() {
  getConf(); // only get,return to saved conf,because the ram conf may be confed,do not write,user will do write in conf mode
  doConf();
  confmode = 0;
  firstpac = 0;
  displayTopDisp("Ready", SSD1306_BLACK);
  scroll_disp(18, "Waiting for game", 0, "No packet yet");
  pixels.fill( pixels.Color(16, 16, 16));
  pixels.show();
}
void processConf() {
  char bbuf[128];
  while (confmode) {

    if (millis() - tempcft1 >= 1000) {
      tempcft1 = millis();
      if (trcol == 0) {
        trcol = 1;
      } else {
        trcol = 0;
      }
      displayTopDisp("Config", trcol ? SSD1306_WHITE : SSD1306_BLACK);
    }

    if (millis() - ledringtime >= 100) {
      pixels.setPixelColor(nowledpos, pixels.Color(10, 10, 0));
      nowledpos--;
      if (nowledpos < 0) nowledpos = 9;
      pixels.setPixelColor(nowledpos, pixels.Color(16, 16, 0));
      ledringtime = millis();
      pixels.show();
    }

    //substring[begin,end)
    if (SerialDevice.available()) {
      String recvStr;
      while (SerialDevice.available())
      {
        char nextread = SerialDevice.read();
        recvStr += nextread;
      }
      if (recvStr == "ESCCONF") {
        escapeConfMode();
      }
      if (recvStr == "GETVER") {
        Serial.println(FirmwareVersion);
      }
      if (recvStr.substring(0, 10) == "SETGAMELED") { //SETGAMELED_xxx
        uint8_t returncode = 0;
        uint8_t recvValue  = 64;
        recvValue = recvStr.substring(11, 14).toInt();
        ReaderConfig.Game_LED_Brightness = recvValue;

        pixels.setBrightness(ReaderConfig.Game_LED_Brightness); // 修改即预览

        sprintf(bbuf, "ROGER SETGAMELED = %d", recvValue);
        Serial.println(bbuf);
      }
      //substring 左闭右开区间
      if (recvStr.substring(0, 9) == "WRITECARD") { //WRITECARD ACCESSCODEACCESSCODE
        // 1s max
        static_disp(60, "Write Card", 0, "Converting AccessCode");
        pixels.fill( pixels.Color(100, 100, 100));
        pixels.show();
        uint8_t returncode = 0;
        for (int i = 1 ; i <= 20; i++) {
          if ('0' <= recvStr[i + 9] && recvStr[i + 9] <= '9') {
            writeBlock2[((i + 1) / 2) + 5] = (i % 2) ? ((recvStr[i + 9] - '0') * 16) : (writeBlock2[((i + 1) / 2) + 5] + (recvStr[i + 9] - '0'));
          }
          else {
            writeBlock2[((i + 1) / 2) + 5] = (i % 2) ? ((0) * 16) : (writeBlock2[((i + 1) / 2) + 5] + (0));
            returncode = 6;
          }
        }
        if (returncode) {

          sprintf(bbuf, "ROGER WRITECARD RET %d", returncode);
          Serial.println(bbuf);
          static_disp(60, "Write Card", 0, "AccessCode Error");
          pixels.fill( pixels.Color(150, 32, 0));
          pixels.show();
          delay(200);
          static_disp(14, "Reset before game", 22, "Configure Mode");
          continue;
        }
        //debug BEGIN
        //        Serial.println("");
        //        for (int i = 6; i <= 15; i++) {
        //          sprintf(bbuf, "%02X", writeBlock2[i]);
        //          Serial.print(bbuf);
        //        }
        //        Serial.println("");
        //debug END
        uint8_t uid[4], uL;
        static_disp(60, "Write Card", 0, "Find&Auth Card");
        delay(100);
        static uint64_t timewaitb = millis();
        while ((!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL, 0) ) && (millis() - timewaitb <= 100)) {
          delay(10);
        }
        if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL, 0))
        {
          sprintf(bbuf, "ROGER WRITECARD RET 4");
          Serial.println(bbuf);
          static_disp(60, "Write Card", 0, "No Card Found");
          pixels.fill( pixels.Color(150, 32, 64));
          pixels.show();
          delay(200);
          static_disp(14, "Reset before game", 22, "Configure Mode");
          continue;
        }

        //todo:自定义密钥
        returncode = 1;
        if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL) && nfc.mifareclassic_AuthenticateBlock(uid, uL, 1, 1, AimeRKey))//Aime Key
          //Aime 认证成功
          returncode = 0;
        else {
          if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL) && nfc.mifareclassic_AuthenticateBlock(uid, uL, 1, 0, BanaRKey))//Banapassport KeyA to Check Card Kind
          { //BANAPASS 内部认证KeyB
            if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL) && nfc.mifareclassic_AuthenticateBlock(uid, uL, 1, 1, BanaRKey))//Key B一般有写权，这一种，原密码
              returncode = 0;
            else {
              memset(MifareRKey, 0x00, sizeof(MifareRKey));
              if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL) && nfc.mifareclassic_AuthenticateBlock(uid, uL, 1, 1, MifareRKey))//另一种keyB，全0
                returncode = 0;
            }
            //不管是否认证成功KeyB，尽管写写试试,不汇报错误code
          }
          else {//非Banapass 认证普通卡Key
            memset(MifareRKey, 0xFF, sizeof(MifareRKey));
            if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL) && nfc.mifareclassic_AuthenticateBlock(uid, uL, 1 , 0, MifareRKey))// All-FF Factory Default Key
              returncode = 0;
            else {
              if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL) && nfc.mifareclassic_AuthenticateBlock(uid, uL, 1 , 1, MifareRKey))//NDEF
                returncode = 0;
              else {
                memset(MifareRKey, 0x00, sizeof(MifareRKey));
                if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL) && nfc.mifareclassic_AuthenticateBlock(uid, uL, 1, 0, MifareRKey))// All-Zero Key
                  returncode = 0;
              }
            }
          }
        }

        if (!returncode) {
          if (!(nfc.mifareclassic_WriteDataBlock (2, writeBlock2)))
            returncode = 3;
          if (!(nfc.mifareclassic_WriteDataBlock (3, writeBlock3)))
            returncode = 3;
        }
        else {
          static_disp(60, "Write Card", 0, "Authenticate Error");
          pixels.fill( pixels.Color(150, 0, 0));
          pixels.show();
          returncode = 2;
          sprintf(bbuf, "ROGER WRITECARD RET %u", returncode);
          Serial.println(bbuf);
          delay(200);
          static_disp(14, "Reset before game", 22, "Configure Mode");

          continue;
        }

        if (returncode) {
          sprintf(bbuf, "ROGER WRITECARD RET %u", returncode);
          Serial.println(bbuf);
          static_disp(60, "Write Card", 0, "Write Error");
          pixels.fill( pixels.Color(0, 150, 64));
          pixels.show();
          delay(200);
          static_disp(14, "Reset before game", 22, "Configure Mode");
          continue;
        }
        sprintf(bbuf, "ROGER WRITECARD RET %u", returncode);
        Serial.println(bbuf);
        static_disp(60, "Write Card", 0, "Write Successfully");
        pixels.fill( pixels.Color(0, 150, 0));
        pixels.show();
        delay(200);
        static_disp(14, "Reset before game", 22, "Configure Mode");
      }

      //todo:copy card


      if (recvStr.substring(0, 10) == "READCARD") { //SETGAMELED_xxx

        sprintf(bbuf, "ROGER READCARD" );
        Serial.println(bbuf);
        uint8_t uid[4], uL;

        if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL) && nfc.mifareclassic_AuthenticateBlock(uid, uL, 1, 1, AimeRKey)) {
          if (nfc.mifareclassic_ReadDataBlock(2, card.block)) {
            sprintf(bbuf, "READ AIME %02X %02X %02X %02X AC %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X", uid[0], uid[1], uid[2], uid[3], card.block[6], card.block[7], card.block[8], card.block[9], card.block[10], card.block[11], card.block[12], card.block[13], card.block[14], card.block[15] );
            Serial.println(bbuf);
            continue;
          }
        }
        if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL) && nfc.mifareclassic_AuthenticateBlock(uid, uL, 1, 0, BanaRKey)) {
          if (nfc.mifareclassic_ReadDataBlock(1, card.block)) {
            sprintf(bbuf, "READ BANA %02X %02X %02X %02X B2 %02X,%02X,%02X%02X%02X%02X%02X,%02X,%02X%02X%02X%02X%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3], card.block[0], card.block[1], card.block[2], card.block[3], card.block[4], card.block[5], card.block[6], card.block[7], card.block[8], card.block[9], card.block[10], card.block[11], card.block[12], card.block[13], card.block[14], card.block[15] );
            Serial.println(bbuf);
            continue;
          }
        }
        memset(MifareRKey, 0xFF, sizeof(MifareRKey));
        if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL) && nfc.mifareclassic_AuthenticateBlock(uid, uL, M2F_B, 0, MifareRKey)) {
          if (nfc.mifareclassic_ReadDataBlock(2, card.block)) {//default FF mifare
            sprintf(bbuf, "READ MFFF %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3] );
            Serial.println(bbuf);
            continue;
          }
        }
        memset(MifareRKey, 0x00, sizeof(MifareRKey));
        if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL) && nfc.mifareclassic_AuthenticateBlock(uid, uL, M2F_B, 0, MifareRKey)) {
          if (nfc.mifareclassic_ReadDataBlock(2, card.block)) {//default 00 mifare
            sprintf(bbuf, "READ MF00 %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3] );
            Serial.println(bbuf);
            continue;
          }
        }

        if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uL)) {//mifare error
          sprintf(bbuf, "READ MFER %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3] );
          Serial.println(bbuf);
          continue;
        }

        if (nfc.felica_Polling(0xFFFF, 0x01, card.IDm, card.PMm, &card.SystemCode, 200)) {
          sprintf(bbuf, "READ FELI" );
          Serial.println(bbuf);
          continue;


          card.SystemCode = card.SystemCode >> 8 | card.SystemCode << 8;

          sprintf(bbuf, "READ FELI IDM %02X %02X %02X %02X %02X %02X %02X %02X PMM %02X %02X %02X %02X %02X %02X %02X %02X SYS %02X %02X",
                  card.IDm[0], card.IDm[1], card.IDm[2], card.IDm[3], card.IDm[4], card.IDm[5], card.IDm[6], card.IDm[7] ,
                  card.PMm[0], card.PMm[1], card.PMm[2], card.PMm[3], card.PMm[4], card.PMm[5], card.PMm[6], card.PMm[7] ,
                  card.System_Code[0], card.System_Code[1]
                 );
          Serial.println(bbuf);

          //          Serial.println("FeliCa Block:");
          //          for (uint8_t i = 0; i < 4; i++) {
          //            if (nfc.felica_ReadWithoutEncryption(1, serviceCodeList, 1, &blockList[i], blockData) == 1) {
          //              Serial.println(blockList[i], HEX);
          //              nfc.PrintHex(blockData[0], 16);
          //            } else {
          //              Serial.println("error");
          //            }
          //          }
          continue;
        }

        sprintf(bbuf, "READ NULL" );
        Serial.println(bbuf);
        continue;
      }
      if (recvStr.substring(0, 8) == "SAVECONF") { //SAVECONF

        writeConf();
        getConf();
        doConf();
        sprintf(bbuf, "ROGER SAVECONF");
        Serial.println(bbuf);

      }
      if (recvStr.substring(0, 8) == "INITCONF") { //INITCONF
        sprintf(bbuf, "ROGER INITCONF");

        Serial.println(bbuf);
        ReaderConfig.configWritten = 0;
        writeConf();
        NVIC_SystemReset();
      }

    }

  }

}

#include "cmd.h"

void setup() {
  SerialDevice.begin(115200);
  delay(200);

  SerialDevice.dtr(0);
  getConf();

  //    delay(100);

  display.begin(SCREEN_ADDRESS);


  display.clearDisplay();
  displayTopDisp("Init", SSD1306_WHITE);
  static_disp(0, "FirmwareVer", 56, FirmwareVersion);
  nfc.begin();
  nfc.setPassiveActivationRetries(0x10);//设定等待次数
  nfc.SAMConfig();
  pixels.begin();
  pixels.setBrightness(ReaderConfig.Game_LED_Brightness);

  waitnfc();

  memset(&req, 0, sizeof(req.bytes));
  memset(&res, 0, sizeof(res.bytes));

  display.clearDisplay();

  displayTopDisp("Ready", SSD1306_BLACK);
  scroll_disp(18, "Waiting for game", 0, "No packet yet");

  pixels.fill( pixels.Color(16, 16, 16));
  pixels.show();
}



void loop() {
  if (!confmode) {
    diplayStatus();
    SerialCheck();
    packet_write();
  }
  else {
    processConf();

  }
}

static uint8_t len, r, checksum;
static bool escape = false;

static uint8_t packet_read() {

  while (SerialDevice.available()) {
    firstpac = 1;
    r = SerialDevice.read();
    if (r == 0xCF && lastcmdok == 1) {
      enterConfMode();
    }
    if (r == 0xE0) {
      lastcmdok = 0;
      req.frame_len = 0xFF;
      continue;
    }
    if (req.frame_len == 0xFF) {
      req.frame_len = r;
      len = 0;
      checksum = r;
      continue;
    }
    if (r == 0xD0) {
      escape = true;
      continue;
    }
    if (escape) {
      r++;
      escape = false;

    }
    req.bytes[++len] = r;
    if (len == req.frame_len && checksum == r) {
      lastcmdok = 1;
      return req.cmd;

    }
    checksum += r;
  }
  return 0;
}

static void packet_write() {
  uint8_t checksum = 0, len = 0;
  if (res.cmd == 0) {
    return;
  }
  SerialDevice.write(0xE0);
  while (len <= res.frame_len) {
    uint8_t w;
    if (len == res.frame_len) {
      w = checksum;
    } else {
      w = res.bytes[len];
      checksum += w;
    }
    if (w == 0xE0 || w == 0xD0) {
      SerialDevice.write(0xD0);
      SerialDevice.write(--w);
    } else {
      SerialDevice.write(w);
    }
    len++;
  }
  res.cmd = 0;
}

void SerialCheck() {
  switch (packet_read()) {
    case SG_NFC_CMD_RESET:
      sg_nfc_cmd_reset();
      break;
    case SG_NFC_CMD_GET_FW_VERSION:
      sg_nfc_cmd_get_fw_version();
      break;
    case SG_NFC_CMD_GET_HW_VERSION:
      sg_nfc_cmd_get_hw_version();
      break;
    case SG_NFC_CMD_POLL:
      sg_nfc_cmd_poll();
      break;
    case SG_NFC_CMD_MIFARE_READ_BLOCK:
      sg_nfc_cmd_mifare_read_block();
      break;
    case SG_NFC_CMD_FELICA_ENCAP:
      sg_nfc_cmd_felica_encap();
      break;
    case SG_NFC_CMD_AIME_AUTHENTICATE:
      sg_nfc_cmd_aime_authenticate();
      break;
    case SG_NFC_CMD_BANA_AUTHENTICATE:
      sg_nfc_cmd_bana_authenticate();
      break;
    case SG_NFC_CMD_MIFARE_SELECT_TAG:
      sg_nfc_cmd_mifare_select_tag();
      break;
    case SG_NFC_CMD_MIFARE_SET_KEY_AIME:
      sg_nfc_cmd_mifare_set_key_aime();
      break;
    case SG_NFC_CMD_MIFARE_SET_KEY_BANA:
      sg_nfc_cmd_mifare_set_key_bana();
      break;
    case SG_NFC_CMD_RADIO_ON:
      sg_nfc_cmd_radio_on();
      break;
    case SG_NFC_CMD_RADIO_OFF:
      sg_nfc_cmd_radio_off();
      break;
    case SG_RGB_CMD_RESET:
      sg_led_cmd_reset();
      break;
    case SG_RGB_CMD_GET_INFO:
      sg_led_cmd_get_info();
      break;
    case SG_RGB_CMD_SET_COLOR:
      sg_led_cmd_set_color();
      break;
    case 0:
      break;
    default:
      sg_res_init();
  }
}
