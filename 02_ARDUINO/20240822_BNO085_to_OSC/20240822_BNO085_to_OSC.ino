// Board Manager URL:
// https://adafruit.github.io/arduino-board-index/package_adafruit_index.json

// select board: esp32 --> Adafruit QT Py ESP32-S2
// -----------------------------------------------

// Adafruit Stemma QT Py ESP32-S2 WiFi (5325)
// https://www.adafruit.com/product/5325

// Adafruit Stemma 9-DOF Orientation IMU Fusion Breakout - BNO085 (BNO080) (4754)
// https://www.adafruit.com/product/4754

// On the second BNO085, connect 'DI' to '3Vo'.
// This changes the I2C address from 0x4A to 0x4B

#include <Adafruit_BNO08x.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCBundle.h>

#define SENSOR_COUNT 1

// OSC-Stuff ----------------------------------------------------------
IPAddress outIp = { 192, 168, 235, 20 };  // IP Adress of Receiver PC
WiFiUDP Udp;
const unsigned int localPort = 9000;  // not needed
const unsigned int destPort = 6448;   // Wekinator Port: 6448
const char *ssid = "Spiel&&Objekt";
const char *password = "puppe2010";  // min. 8 characters

// BNO08x-Stuff -------------------------------------------------------
Adafruit_BNO08x bno08x;

sh2_SensorValue_t sensorValue;

// i2c pins for the Adafruit Stemma QT Py ESP32-S2 WiFi (5325)
// const int SDA_PIN = 41;
// const int CLK_PIN = 40;
const uint8_t I2C_ADDR = 0x4A;

// Here is where you define the sensor outputs you want to receive
void setReports() {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_GAME_ROTATION_VECTOR)) {
    Serial.printf("Could not enable game vector\n");
  }
  // if (!bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED)) {
  //   Serial.printf("Could not enable game vector\n");
  // }
}

void setup(void) {
  Serial.begin(115200);
  // while (!Serial) { delay(10); }
  Serial.println("welcome");

  // WiFi
  WiFi.setHostname("BNO085");
  //WiFi.config(staticIP, subnet, gateway, dns);
  WiFi.mode(WIFI_STA);  //WiFi mode station (connect to wifi router only)
  WiFi.begin(ssid, password);

  // Wait for connection
  int timeOut = 0;
  while ((WiFi.status() != WL_CONNECTED) && (timeOut < 10)) {
    delay(1000);
    timeOut++;
    Serial.println(timeOut);
  }
  Serial.print("local IP address: ");
  Serial.println(WiFi.localIP());
  Udp.begin(localPort);


  // BNO08x
  Wire1.setPins(SDA1, SCL1);
  if (!bno08x.begin_I2C(I2C_ADDR, &Wire1)) {
    Serial.printf("Failed to find bno08x chip with address %d\n", I2C_ADDR);
    while (1) { delay(10); }
  }
  delay(1000);


  for (int n = 0; n < bno08x.prodIds.numEntries; n++) {
    Serial.print("Part ");
    Serial.print(bno08x.prodIds.entry[n].swPartNumber);
    Serial.print(": Version :");
    Serial.print(bno08x.prodIds.entry[n].swVersionMajor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionMinor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionPatch);
    Serial.print(" Build ");
    Serial.println(bno08x.prodIds.entry[n].swBuildNumber);
  }
  delay(1000);


  setReports();

  Serial.println("Reading events");
  delay(100);
}




void loop() {
  delay(100);

  if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  switch (sensorValue.sensorId) {
    case SH2_GAME_ROTATION_VECTOR:
      Serial.print("Game Rotation Vector - r: ");
      Serial.print(sensorValue.un.gameRotationVector.real);
      Serial.print(" i: ");
      Serial.print(sensorValue.un.gameRotationVector.i);
      Serial.print(" j: ");
      Serial.print(sensorValue.un.gameRotationVector.j);
      Serial.print(" k: ");
      Serial.println(sensorValue.un.gameRotationVector.k);

      // send OSC
      if (WiFi.status() == WL_CONNECTED) {
        OSCMessage msg("/mollusc/1");
        msg.add(sensorValue.un.gameRotationVector.real);
        msg.add(sensorValue.un.gameRotationVector.i);
        msg.add(sensorValue.un.gameRotationVector.j);
        msg.add(sensorValue.un.gameRotationVector.k);
        Udp.beginPacket(outIp, destPort);
        msg.send(Udp);
        Udp.endPacket();
        msg.empty();
      }
      break;
  }
}