#define FirmwareVersion "1.0b_dev"

//#include "FastLED.h"
//#define NUM_LEDS CRGB leds[NUM_LEDS];
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



//bool high_baudrate = false;//high_baudrate=true


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
  char bbuf[64];
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
        uint8_t recvValue  = 64;
        recvValue = recvStr.substring(11, 14).toInt();
        ReaderConfig.Game_LED_Brightness = recvValue;

        pixels.setBrightness(ReaderConfig.Game_LED_Brightness); // 修改即预览
        
        sprintf(bbuf, "ROGER SETGAMELED = %d", recvValue);
        Serial.println(bbuf);
      }
      if (recvStr.substring(0, 8) == "SAVECONF") { //SETGAMELED_xxx
        sprintf(bbuf, "ROGER SAVECONF");
        writeConf();
        getConf();
        doConf();
        Serial.println(bbuf);

      }

    }

  }

}

#include "cmd.h"

void setup() {

  getConf();

  SerialDevice.begin(115200);
  SerialDevice.dtr(0);
  display.begin(SCREEN_ADDRESS);
  delay(10);

  display.clearDisplay();
  displayTopDisp("Init", SSD1306_WHITE);

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
