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

#define SENSOR_COUNT 2
Adafruit_BNO08x bno08x[SENSOR_COUNT];

sh2_SensorValue_t sensorValue;

// i2c pins for the Adafruit Stemma QT Py ESP32-S2 WiFi (5325)
const uint8_t I2C_ADDR[] = { 0x4A, 0x4B };

// Here is where you define the sensor outputs you want to receive
void setReports(int id = 0) {
  Serial.println("Setting desired reports");
  if (!bno08x[id].enableReport(SH2_GAME_ROTATION_VECTOR)) {
    Serial.printf("Could not enable game vector\n");
  }
}

void setup(void) {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("welcome");

  Wire1.setPins(SDA1, SCL1);

  for (int i = 0; i < SENSOR_COUNT; i++) {
    if (!bno08x[i].begin_I2C(I2C_ADDR[i], &Wire1, i)) {
      Serial.printf("Failed to find bno08x chip with address 0x%x\n", I2C_ADDR[i]);
      while (1) { delay(10); }
    } else {
      Serial.printf("Found bno08x chip with address 0x%x\n", I2C_ADDR[i]);
    }
    delay(1000);
  }

  // if (!bno08x_1.begin_I2C(I2C_ADDR[1], &Wire, 1)) {
  //   Serial.printf("Failed to find BNO08x_1 chip with address %d\n", I2C_ADDR[1]);
  //   while (1) { delay(10); }
  // }
  // delay(1000);

  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
    for (int n = 0; n < bno08x[i].prodIds.numEntries; n++) {
      Serial.print("Part ");
      Serial.print(bno08x[i].prodIds.entry[n].swPartNumber);
      Serial.print(": Version :");
      Serial.print(bno08x[i].prodIds.entry[n].swVersionMajor);
      Serial.print(".");
      Serial.print(bno08x[i].prodIds.entry[n].swVersionMinor);
      Serial.print(".");
      Serial.print(bno08x[i].prodIds.entry[n].swVersionPatch);
      Serial.print(" Build ");
      Serial.println(bno08x[i].prodIds.entry[n].swBuildNumber);
    }
    delay(1000);
  }

  setReports();

  Serial.println("Reading events");
  delay(100);
}




void loop() {
  delay(10);
  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {

    if (bno08x[i].wasReset()) {
      Serial.print("sensor was reset ");
      setReports();
    }

    if (!bno08x[i].getSensorEvent(&sensorValue)) {
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
        break;
    }
  }
}