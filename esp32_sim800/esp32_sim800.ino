#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER 1024

#include <TinyGsmClient.h>

#define RESET 12
#define Rx 13
#define Tx 15
#define Temp 14
#define BTN 2

#define SerialMon Serial
#define SerialAT Serial1
#define GSM_PIN ""

#define uS_TO_S_FACTOR 1000000UL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60         /* Time ESP32 will go to sleep (in seconds) 3600 seconds = 1 hour */

#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 57600

const char apn[] = "net";
const char gprsUser[] = "";
const char gprsPass[] = "";

const char server[] = "soil.mn";
const char resource[] = "/app/device";
const int port = 8080;

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

void setup() {
  SerialMon.begin(115200);
  delay(10);
  SerialAT.begin(115200, SERIAL_8N1, Rx, Tx);
  delay(3000);
  SerialMon.println("Initializing modem...");
  modem.restart();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}
void loop() {
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println("Failed to connect to the cellular network.");
  } else {
    SerialMon.println("Connected to the cellular network");

    if (!client.connect(server, port)) {
      SerialMon.println("Failed to connect to the server");
    } else {
      SerialMon.println("Performing HTTP POST request...");

      String httpRequestData = "";
      client.print(String("POST ") + resource + " HTTP/1.1\r\n");
      client.print(String("Host: ") + server + "\r\n");
      client.println("Connection: close");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(httpRequestData.length());
      client.println();
      client.println(httpRequestData);
      unsigned long timeout = millis();
      while (client.connected() && millis() - timeout < 10000L) {
        while (client.available()) {
          char c = client.read();
          SerialMon.print(c);
          timeout = millis();
        }
      }
      if (millis() - timeout < 10000L) {
        SerialMon.println("Connection timed out.");
      }
    }
    SerialMon.println();
    client.stop();
    SerialMon.println(F("Server disconnected"));
    modem.gprsDisconnect();
    SerialMon.println(F("GPRS disconnected"));
  }
  esp_deep_sleep_start();
}
