/**
 * This example turns the ESP32 into a Bluetooth LE mouse and USB  that continuously moves the mouse.
                             Supported    BLE    USB
ESP32                        Yes          Yes    No
ESP32-S2                     Yes          No     Yes
ESP32-S3                     Yes          Yes    Yes
ESP32-C2/ESP8684(H2/H4)      Notyet       Yes    No
ESP32-C3                     YES          Yes    No
ESP32-C5                     Notyet       Yes    Yes
ESP32-C6                     yes*         Yes    No
ESP32-C61                    Notyet       Yes    No
ESP32-H2                     yes*         Yes    No
ESP32-H4                     Notyet       Yes    No
ESP8266                      No           No     No

  *Yes is supported by Arduino but not yet by Platformio 2024-09
  *Notyet is not yet supported as i do not have the Chip or Platform software 2024-09

*/

#include <Arduino.h>

#if not defined(ESP8266)
#include <rom/rtc.h>
#endif

#if defined(CONFIG_TINYUSB_ENABLED)
#include "USBHID.h"
#endif

#if defined(CONFIG_BT_BLE_ENABLED)
#include <BleMouse.h>
BleMouse *bleMouse;
#endif
#if defined(CONFIG_TINYUSB_ENABLED)
#include "USB.h"
#include "USBHIDMouse.h"
USBHIDMouse UsbMouse;
#endif

#if not defined(ESP8266)
String reset_reason(int reason)
{
  switch (reason)
  {
  case 1:
    return ("Vbat power on reset");
    break;
  case 3:
    return ("Software reset digital core");
    break;
  case 4:
    return ("Legacy watch dog reset digital core");
    break;
  case 5:
    return ("Deep Sleep reset digital core");
    break;
  case 6:
    return ("Reset by SLC module, reset digital core");
    break;
  case 7:
    return ("Timer Group0 Watch dog reset digital core");
    break;
  case 8:
    return ("Timer Group1 Watch dog reset digital core");
    break;
  case 9:
    return ("RTC Watch dog Reset digital core");
    break;
  case 10:
    return ("Instrusion tested to reset CPU");
    break;
  case 11:
    return ("Time Group reset CPU");
    break;
  case 12:
    return ("Software reset CPU");
    break;
  case 13:
    return ("RTC Watch dog Reset CPU");
    break;
  case 14:
    return ("for APP CPU, reseted by PRO CPU");
    break;
  case 15:
    return ("Reset when the vdd voltage is not stable");
    break;
  case 16:
    return ("RTC Watch dog reset digital core and rtc module");
    break;
  default:
    return ("NO_MEAN");
  }
  return ("NO_MEAN");
}
#endif

void setup()
{
  char strTurtleName[]    = "Turtle ESP";
  char strTurtleNameUsb[] = "Turtle ESP USB";
  char strTurtleNameBLE[] = "Turtle ESP BLE";
  char strTurtleManufacturer[] = "Espressif";
#if defined(CONFIG_BT_BLE_ENABLED)
  bleMouse = new BleMouse(strTurtleNameBLE, strTurtleManufacturer , 100);
  bleMouse->begin();
#endif
#if defined(CONFIG_TINYUSB_ENABLED)
  UsbMouse.begin();
  USB.manufacturerName(strTurtleManufacturer);
  USB.productName(strTurtleNameUsb);
  USB.begin();
#endif
#if defined(LED_BUILTIN)
//  pinMode(LED_BUILTIN, OUTPUT);
//  digitalWrite(LED_BUILTIN, 1);
#endif
  Serial.begin(115200);
  String message = "Reboot from: ";
  message += strTurtleName;
#if defined(CONFIG_TINYUSB_ENABLED)
  message += " USB";
#endif
#if defined(CONFIG_BT_BLE_ENABLED)
  message += " BLE";
#endif
  message += " ";
#if defined(ESP8266)
      message += ESP.getChipId();
#else
      message += ESP.getChipModel();
#endif
  message += " CpuFreqMHz: " + String(ESP.getCpuFreqMHz());
#if not defined(ESP8266)
  message += " Total heap: " + String(ESP.getHeapSize() / 1024);
  message += " Free heap: " + String(ESP.getFreeHeap() / 1024);
  message += " Total PSRAM: " + String(ESP.getPsramSize() / 1024);
  message += " Free PSRAM: " + String(ESP.getFreePsram() / 1024);
  message += " Temperature: " + String(temperatureRead()) + " °C "; // internal TemperatureSensor
#if defined(CONFIG_IDF_TARGET_ESP32)
  message += " HallSensor: " + String(hallRead());
#endif
#else
  message += " FlashChipId: " + String(ESP.getFlashChipId());
  message += " FlashChipRealSize: " + String(ESP.getFlashChipRealSize());
#endif
  message += " FlashChipSize: " + String(ESP.getFlashChipSize());
  message += " FlashChipSpeed: " + String(ESP.getFlashChipSpeed());
  message += " FlashChipMode: ";
  switch( ESP.getFlashChipMode()){
    case FM_QIO :message += "FM_QIO"; break;
    case FM_QOUT:message += "FM_QOUT";break;
    case FM_DIO :message += "FM_DIO"; break;
    case FM_DOUT:message += "FM_DOUT";break;
    case FM_FAST_READ:message += "FM_FAST_READ";break;
    case FM_SLOW_READ:message += "FM_SLOW_READ";break;
    default:
      message += String(ESP.getFlashChipMode());
  }

#if ESP_ARDUINO_VERSION_MAJOR <= 2
  message += " Mac: ";
  uint8_t mac[6];
 // esp_efuse_mac_get_default(mac);
  esp_base_mac_addr_get(mac); 
  for (byte i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%02X", mac[i]); // added the 0 in the format for padding
    message += buf;
    if (i < 5)
      message += ':';
  }
  //message += String(((unsigned long)ESP.getEfuseMac()), (unsigned char)16U);
#endif 
#if not defined(ESP8266)
      message += " reset_reason: " + reset_reason(rtc_get_reset_reason(0));
#endif
      message += " ESP_ARDUINO_VERSION: " + String(ESP_ARDUINO_VERSION_MAJOR) + "." + String(ESP_ARDUINO_VERSION_MINOR) + "." + String(ESP_ARDUINO_VERSION_PATCH);
      message += " Build Date: " + String(__DATE__ " " __TIME__ );
      Serial.println(message);
}

int Seconds;
int CurrentSeconds;
int Minutes;
int CurrentMinutes;
bool BLEConnection = false;

void loop()
{
  delay(100);                                       // Just not to run the loop to fast
  CurrentSeconds = esp_timer_get_time() / 1000000L; // The time passed since the ESP initialization
  // CurrentSeconds=esp_timer_get_time()/50000L; //The time passed since the ESP initialization
  if (Seconds != CurrentSeconds)
  {
    Seconds = CurrentSeconds;
    CurrentMinutes = Seconds / 60;
#if defined(CONFIG_BT_BLE_ENABLED)
    if (BLEConnection && !bleMouse->isConnected())
    {
      Serial.println("BLE DISConnected");
      BLEConnection = false;
    }
    if (!BLEConnection && bleMouse->isConnected())
    {
      Serial.println("BLE Connected");
      BLEConnection = true;
    }
#endif
    if (Minutes != CurrentMinutes)
    {
      Minutes = CurrentMinutes;
#if defined(CONFIG_BT_BLE_ENABLED)
      if (bleMouse->isConnected())
      {
        switch (Minutes & 0x03)
        {
        case 0:
          bleMouse->move(-10, 0);
          break;
        case 1:
          bleMouse->move(0, -10);
          break;
        case 2:
          bleMouse->move(10, 0);
          break;
        case 3:
          bleMouse->move(0, 10);
          break;
        }
      }
#endif
#if defined(CONFIG_TINYUSB_ENABLED)
      switch (Minutes & 0x03)
      {
      case 0:
        UsbMouse.move(-10, 0);
        break;
      case 1:
        UsbMouse.move(0, -10);
        break;
      case 2:
        UsbMouse.move(10, 0);
        break;
      case 3:
        UsbMouse.move(0, 10);
        break;
      }
#endif
    }
  }
}