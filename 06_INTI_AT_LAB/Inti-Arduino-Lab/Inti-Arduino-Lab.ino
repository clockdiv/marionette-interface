/*
  ////////////////////////////////////////////////////////////////////////
  CODE FOR INTERFACE PUPPE
  based on "DualDistance" example from Sparkfun's I2C MUX library
  modified to read BNO055 sensors
  -hannah pw
  ////////////////////////////////////////////////////////////////////////

  Libraries needed:

  - Adafruit BNO055 (by Adafruit)
  - OSC (by Adrian Freed)
  - Sparkfun I2C Mux Arduino Library (by SparkFun)

  Board to select: ESP32 Dev Module
  Upload Speed:    115200
  Hold down **BOOT** on ESP32 when uploading

  ORIGINAL EXAMPLE COMMENTS:
  Use the Qwiic Mux to access multiple I2C devices on seperate busses.
  By: Nathan Seidle @ SparkFun Electronics
  Date: May 17th, 2020
  License: This code is public domain but you buy me a beer if you use this
  and we meet someday (Beerware license).

  Some I2C devices respond to only one I2C address. This can be a problem
  when you want to hook multiple of a device to the I2C bus. An I2C Mux
  solves this issue by allowing you to change the 'channel' or port that
  the master is talking to.

  This example shows how to hook up two VL53L1X laser distance sensors with the same address.
  You can read the VL53L1X hookup guide and get the library from https://learn.sparkfun.com/tutorials/qwiic-distance-sensor-vl53l1x-hookup-guide

  The TCA9548A is a mux. This means when you enableMuxPort(2) then the SDA and SCL lines of the master (Arduino)
  are connected to port 2. Whatever I2C traffic you do, such as distanceSensor.startRanging() will be communicated to whatever
  sensor you have on port 2. This example creates an array of objects. This increases RAM but allows for
  independent configuration of each sensor (one sensor may be configured for long range, the others for short, etc).

  Outputs two sets of distances in mm and ft.

  Hardware Connections:
  Attach the Qwiic Mux Shield to your RedBoard or Uno.
  Plug two Qwiic VL53L1X breakout boards into ports 0 and 1.
  Serial.print it out at 115200 baud to serial monitor.

  SparkFun labored with love to create this code. Feel like supporting open
  source? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14685
*/
/*
right arm?
bno.setAxisRemap(Adafruit_BNO055::REMAP_CONFIG_PUPPET);
bno.setAxisSign(Adafruit_BNO055::REMAP_SIGN_P3);

left arm?
bno.setAxisRemap(Adafruit_BNO055::REMAP_CONFIG_PUPPET);
bno.setAxisSign(Adafruit_BNO055::REMAP_SIGN_P6);

body?
bno.setAxisRemap(Adafruit_BNO055::REMAP_CONFIG_PUPPET_BODY);
bno.setAxisSign(Adafruit_BNO055::REMAP_SIGN_P4);

*/



#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

char ssid[] = "";  // your network SSID (name)
char pass[] = "";  // your network password

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const IPAddress outIp(192, 168, 103, 55);  // remote IP (not needed for receive)
const unsigned int outPort = 9999;         // remote port (not needed for receive)


Adafruit_BNO055 bno128 = Adafruit_BNO055(128, 0x28);
Adafruit_BNO055 bno129 = Adafruit_BNO055(129, 0x29);
Adafruit_BNO055 bno228 = Adafruit_BNO055(228, 0x28);
Adafruit_BNO055 bno229 = Adafruit_BNO055(229, 0x29);
Adafruit_BNO055 bno328 = Adafruit_BNO055(328, 0x28);
Adafruit_BNO055 bno329 = Adafruit_BNO055(329, 0x29);
Adafruit_BNO055 bno428 = Adafruit_BNO055(429, 0x28);
Adafruit_BNO055 bno429 = Adafruit_BNO055(429, 0x29);
Adafruit_BNO055 bno528 = Adafruit_BNO055(528, 0x28);
Adafruit_BNO055 bno529 = Adafruit_BNO055(529, 0x29);

Adafruit_BNO055 bno[] = { bno128, bno129, bno228, bno229, bno328, bno329, bno428, bno429, bno528, bno529 };

#include <SparkFun_I2C_Mux_Arduino_Library.h>

QWIICMUX myMux;

#define NUMBER_OF_SENSORS 10

int portOrder[] = { 0, 2, 3, 4, 7 };  //enter used ports here
int portNum = 0;
bool firstSensor = true;

char* sensorNames[NUMBER_OF_SENSORS] = { "/upperarm.l", "/lowerarm.l", "/body", "/head", "/upperleg.l", "/lowerleg.l", "/upperleg.r", "/lowerleg.r", "/upperarm.r", "/lowerarm.r" };

constexpr uint8_t REMAP_CONFIG_PUPPET_BODY = 0x09;  // documented within Bosch Sensor docs, not in Adafruit Library

void setup() {
  Serial.begin(115200);
  Serial.print("Setup: ");
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(WiFi.status());  // 1=NO_SSID_AVAIL, 4=CONNECT_FAILED, 6=DISCONNECTED
    delay(500);
  }

  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //Initialize Mux
  Serial.println("Qwiic Mux Shield Read Example");
  Wire.begin();

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Found device at 0x%02X\n", addr);
    }
  }

  if (myMux.begin() == false) {
    Serial.println("Mux not detected. Freezing...");
    while (1)
      ;
  }
  Serial.println("Mux detected");

  //Initialize BNOs in Ports
  bool initSuccess = true;
  for (int i = 0; i < NUMBER_OF_SENSORS; i++) {
    myMux.setPort(portOrder[portNum]);
    String portString = String(portOrder[portNum]);
    if (!bno[i].begin()) {
      // There was a problem detecting the BNO055 ... check your connections
      Serial.print(i);
      Serial.print("at Port" + portString);
      Serial.print(bno[initSuccess].getMode());
      Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
      while (1)
        ;
      initSuccess = false;
    } 
    else 
    {
      delay(100);
      bno[i].setAxisRemap(static_cast<Adafruit_BNO055::adafruit_bno055_axis_remap_config_t>(REMAP_CONFIG_PUPPET_BODY));
      bno[i].setAxisSign(Adafruit_BNO055::REMAP_SIGN_P4);
      bno[i].setMode(OPERATION_MODE_NDOF);
      delay(100);
      bno[i].setExtCrystalUse(true);
      Serial.print(i);
      Serial.print(" at Port" + portString);
      Serial.println(" configured");
    }
    if (i % 2 != 0) {
      portNum++;
    }
    delay(100);
  }
  delay(100);
  firstSensor = true;
  portNum = 0;

  if (initSuccess == false) {
    Serial.print("Freezing...");
    delay(10000);
  }
  Serial.println("Mux Shield online");
  Serial.println("Calibrating Sensors");

  firstSensor = true;
  portNum = 0;
}

void loop() {
  for (int i = 0; i < NUMBER_OF_SENSORS; i++) {
    myMux.setPort(portOrder[portNum]);
    imu::Quaternion quat;

    uint8_t sys, gyro, accel, mag = 0;

    quat = bno[i].getQuat();
    bno[i].getCalibration(&sys, &gyro, &accel, &mag);

    OSCMessage msg(sensorNames[i]);


    uint8_t calibration = sys + gyro + accel + mag;

    // Print quat data
    // Serial.print("qW: ");
    // Serial.print(quat.w(), 4);
    // Serial.print(" qX: ");
    // Serial.print(quat.x(), 4);
    // Serial.print(" qY: ");
    // Serial.print(quat.y(), 4);
    // Serial.print(" qZ: ");
    // Serial.print(quat.z(), 4);
    // Serial.print("");

    // Print body + same rotation vectors
    // vec = quat.toEuler();
    // float vecx = vec.x();
    // float vecy = vec.y();
    // float vecz = vec.z();
    // msg.add(vecx);
    // msg.add(vecy);
    // msg.add(vecz);
    // if (i > 1 && i < 8) {
    //   Serial.print(sensorNames[i]);
    //   Serial.print(" X: ");
    //   Serial.print(vecx);
    //   Serial.print(" Y: ");
    //   Serial.print(vecy);
    //   Serial.print(" Z: ");
    //   Serial.print(vecz);
    //   Serial.print("\t");
    // }

    float w = quat.w();
    float x = quat.x();
    float y = quat.y();
    float z = quat.z();

    msg.add(w);
    msg.add(x);
    msg.add(y);
    msg.add(z);

    Udp.beginPacket(outIp, outPort);
    msg.send(Udp);    // Send the bytes to the SLIP stream
    Udp.endPacket();  // Mark the end of the OSC Packet
    msg.empty();


    char buf[20];
    strcpy(buf, "/debug");
    strcat(buf, sensorNames[i]);
    OSCMessage calibMsg(buf);
    calibMsg.add(calibration);
    Udp.beginPacket(outIp, outPort);
    calibMsg.send(Udp);  // Send the bytes to the SLIP stream
    Udp.endPacket();     // Mark the end of the OSC Packet
    calibMsg.empty();

    if (i % 2 != 0) {
      portNum++;
    }
  }

  firstSensor = true;
  portNum = 0;
}
