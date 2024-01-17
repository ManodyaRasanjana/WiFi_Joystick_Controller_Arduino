/**
 * configure development platform as a WiFi station with a predefined IP to get data from the multiple mobile apps
 * see WiFi_Access_point and WiFi_Station examples for more details
 */

#include <WiFi_Joystick_Controller.h>

// WiFi network credentials
const char* ssid = "YOUR_SSID";      // replace with SSID of your WiFi network
const char* pswd = "YOUR_PASSWORD";  // replace with password of your WiFi network
const uint16_t udpPort1 = 8888;      // replace with desired UDP port number
const uint16_t udpPort2 = 8889;      // replace with desired UDP port number

// WiFi remote controller objects
WiFi_Joystick_Controller remote1(udpPort1);
WiFi_Joystick_Controller remote2(udpPort2);

// loop rate maintaining variables
unsigned long lastUpdated_ms;        // timestamp of last update
unsigned long lastPrinted_ms;        // timestamp of last print performed
const uint16_t updateDelay_ms = 25;  // keep this value below half of the mobile app's data send period

// remote data holding variables
int8_t leftJoystickX1, leftJoystickY1;
int8_t rightJoystickX1, rightJoystickY1;

int8_t leftJoystickX2, leftJoystickY2;
int8_t rightJoystickX2, rightJoystickY2;

void setup() {
  Serial.begin(115200);
  delay(2000);

  // initialize WiFi STA and get the status. Only the first initialization should have WiFi credentials
  uint8_t wifiStatus1 = remote1.init(WJC_WIFI_MODE_STA, ssid, pswd);

  // initialize second remote. Should not include any WiFi credentials
  uint8_t wifiStatus2 = remote2.init(true);

  // validate the WiFi status
  if (wifiStatus1 != WJC_ERR_OK) {
    Serial.print("Remote STA1 initialization error: ");
    Serial.println(wifiStatus1);
    while (true) {
      // cannot continue with no WiFi establishment
    }
  }

  if (wifiStatus2 != WJC_ERR_OK) {
    Serial.print("Remote STA2 initialization error: ");
    Serial.println(wifiStatus2);
    while (true) {
      // cannot continue with no WiFi establishment
    }
  }

  // use following data to set the "UDP Credentials" of the mobile app
  Serial.print("Remote STA initialized at IP Address ");
  Serial.print(remote1.getIpAddress());
  Serial.print(" with the UDP port number ");
  Serial.print(remote1.getPortNumber());
  Serial.print(" and ");
  Serial.println(remote2.getPortNumber());

  // set timeout (milliSeconds) for data validation period
  remote1.setDataValidTimeout(500);
  remote2.setDataValidTimeout(500);
}

void loop() {
  // make sure to run update() at least x2 speed of the mobile app's data rate to maintain performance
  // update() can use without loop rate controlling. If need control the loop rate, use time based maintainers
  if (millis() - lastUpdated_ms >= updateDelay_ms) {
    uint8_t status1 = remote1.update();  // run this function to update the library internal buffers
    uint8_t status2 = remote2.update();

    if (status1 == WJC_ERR_OK) {
      updateValues1();  // update data holding variables using a separated function
    }

    if (status2 == WJC_ERR_OK) {
      updateValues2();
    }

    lastUpdated_ms = millis();  // update timestamp
  }

  // rest of the loop. Replace with your own functions. Do not call delay() or time expensive functions
  if (millis() - lastPrinted_ms > updateDelay_ms * 5) {
    // validate if a valid data packet received during the given period
    if (remote1.getDataValidStatus() == WJC_ERR_OK) {
      printValues1();
    } else {
      Serial.print("No new data available 1");
      Serial.print('\t');
    }

    if (remote2.getDataValidStatus() == WJC_ERR_OK) {
      printValues2();
    } else {
      Serial.print("No new data available 2");
    }

    Serial.println();
    lastPrinted_ms = millis();
  }

  // do not call delay()
}

void updateValues1() {
  leftJoystickX1 = remote1.getJoystick(WJC_LEFT_JOYSTICK, WJC_X_AXIS);
  leftJoystickY1 = remote1.getJoystick(WJC_LEFT_JOYSTICK, WJC_Y_AXIS);

  rightJoystickX1 = remote1.getJoystick(WJC_RIGHT_JOYSTICK, WJC_X_AXIS);
  rightJoystickY1 = remote1.getJoystick(WJC_RIGHT_JOYSTICK, WJC_Y_AXIS);
}

void updateValues2() {
  leftJoystickX2 = remote2.getJoystick(WJC_LEFT_JOYSTICK, WJC_X_AXIS);
  leftJoystickY2 = remote2.getJoystick(WJC_LEFT_JOYSTICK, WJC_Y_AXIS);

  rightJoystickX2 = remote2.getJoystick(WJC_RIGHT_JOYSTICK, WJC_X_AXIS);
  rightJoystickY2 = remote2.getJoystick(WJC_RIGHT_JOYSTICK, WJC_Y_AXIS);
}

void printValues1() {
  Serial.print(leftJoystickX1);
  Serial.print('\t');
  Serial.print(leftJoystickY1);
  Serial.print('\t');

  Serial.print(rightJoystickX1);
  Serial.print('\t');
  Serial.print(rightJoystickY1);
  Serial.print('\t');
}

void printValues2() {
  Serial.print(leftJoystickX2);
  Serial.print('\t');
  Serial.print(leftJoystickY2);
  Serial.print('\t');

  Serial.print(rightJoystickX2);
  Serial.print('\t');
  Serial.print(rightJoystickY2);
  Serial.print('\t');
}
