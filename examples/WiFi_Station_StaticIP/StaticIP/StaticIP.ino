/**
 * configure development platform as a WiFi station with a predefined IP to get data from the mobile app
 * make sure that mobile device connected to the development platform's WiFi network
 */

#include <WiFi_Joystick_Controller.h>

// WiFi network credentials
const char* ssid = "YOUR_SSID";      // replace with SSID of your WiFi network
const char* pswd = "YOUR_PASSWORD";  // replace with password of your WiFi network
const uint16_t udpPort = 8888;       // replace with desired UDP port number

// ESP: see get_network_credentials example
// MKR1000: credentials other than static IP will be ignored during initialization
const IPAddress staticIP(192, 168, 1, 100);  // replace with desired IP. Make sure the address available on the network
const IPAddress gateway(255, 255, 255, 0);   // replace with your network credentials
const IPAddress subnet(192, 168, 1, 1);
const IPAddress primaryDNS(192, 168, 1, 1);
const IPAddress secondaryDNS(0, 0, 0, 0);

// WiFi remote controller object
WiFi_Joystick_Controller remote(udpPort);

// loop rate maintaining variables
unsigned long lastUpdated_ms;        // timestamp of last update
unsigned long lastPrinted_ms;        // timestamp of last print performed
const uint16_t updateDelay_ms = 25;  // keep this value below half of the mobile app's data send period

// remote data holding variables
int8_t leftJoystickX, leftJoystickY;
int8_t rightJoystickX, rightJoystickY;
bool buttonA1, buttonA2, buttonA3;
bool buttonB1, buttonB2, buttonB3;
uint8_t buttonGroupAValue, buttonGroupAMode;
uint8_t buttonGroupBValue, buttonGroupBMode;

void setup() {
  Serial.begin(115200);
  delay(2000);

  // initialize WiFi STA and get the status
  uint8_t wifiStatus = remote.init(ssid, pswd, staticIP, gateway, subnet, primaryDNS, secondaryDNS);

  // validate the WiFi status
  if (wifiStatus != WJC_ERR_OK) {
    Serial.print("Remote STA initialization error: ");
    Serial.print(wifiStatus);
    while (true) {
      // cannot continue with no WiFi establishment
    }
  }

  // use following data to set the "UDP Credentials" of the mobile app
  Serial.print("Remote STA initialized at IP Address ");
  Serial.print(remote.getIpAddress());
  Serial.print(" with the UDP port number ");
  Serial.println(remote.getPortNumber());

  // set timeout (milliSeconds) for data validation period
  remote.setDataValidTimeout(500);
}

void loop() {
  // make sure to run update() at least x2 speed of the mobile app's data rate to maintain performance
  // update() can use without loop rate controlling. If need control the loop rate, use time based maintainers
  if (millis() - lastUpdated_ms >= updateDelay_ms) {
    uint8_t status = remote.update();  // run this function to update the library internal buffers

    if (status == WJC_ERR_OK) {
      updateValues();  // update data holding variables using a separated function
    }

    lastUpdated_ms = millis();  // update timestamp
  }

  // rest of the loop. Replace with your own functions. Do not call delay() or time expensive functions
  if (millis() - lastPrinted_ms > updateDelay_ms * 5) {
    // validate if a valid data packet received during the given period
    if (remote.getDataValidStatus() == WJC_ERR_OK) {
      printValues();
    } else {
      Serial.print("No new data available");
    }
    Serial.println();
    lastPrinted_ms = millis();
  }

  // do not call delay()
}

void updateValues() {
  // assign joystick data to data holding variables. Range is (-100) to +100
  leftJoystickX = remote.getJoystick(WJC_LEFT_JOYSTICK, WJC_X_AXIS);
  leftJoystickY = remote.getJoystick(WJC_LEFT_JOYSTICK, WJC_Y_AXIS);

  rightJoystickX = remote.getJoystick(WJC_RIGHT_JOYSTICK, WJC_X_AXIS);
  rightJoystickY = remote.getJoystick(WJC_RIGHT_JOYSTICK, WJC_Y_AXIS);

  // assign individual button data of button groups to data holding variables
  buttonA1 = remote.getButtonValue(WJC_BTN_GROUP_A, WJC_BTN_1);
  buttonA2 = remote.getButtonValue(WJC_BTN_GROUP_A, WJC_BTN_2);
  buttonA3 = remote.getButtonValue(WJC_BTN_GROUP_A, WJC_BTN_3);

  buttonB1 = remote.getButtonValue(WJC_BTN_GROUP_B, WJC_BTN_1);
  buttonB2 = remote.getButtonValue(WJC_BTN_GROUP_B, WJC_BTN_2);
  buttonB3 = remote.getButtonValue(WJC_BTN_GROUP_B, WJC_BTN_3);

  // assign entire button group data to data holding variables
  // can use for manual calculations
  buttonGroupAValue = remote.getButtonGroupValue(WJC_BTN_GROUP_A);
  buttonGroupAMode = remote.getButtonGroupMode(WJC_BTN_GROUP_A);

  buttonGroupBValue = remote.getButtonGroupValue(WJC_BTN_GROUP_B);
  buttonGroupBMode = remote.getButtonGroupMode(WJC_BTN_GROUP_B);
}

void printValues() {
  // print left joystick data
  Serial.print(leftJoystickX);
  Serial.print('\t');
  Serial.print(leftJoystickY);
  Serial.print('\t');

  // print right joystick data
  Serial.print(rightJoystickX);
  Serial.print('\t');
  Serial.print(rightJoystickY);
  Serial.print('\t');
  Serial.print('\t');

  // print button values of group A
  Serial.print(buttonA1);
  Serial.print('\t');
  Serial.print(buttonA2);
  Serial.print('\t');
  Serial.print(buttonA3);
  Serial.print('\t');
  Serial.print('\t');

  // print button values of group B
  Serial.print(buttonB1);
  Serial.print('\t');
  Serial.print(buttonB2);
  Serial.print('\t');
  Serial.print(buttonB3);
  Serial.print('\t');
  Serial.print('\t');

  // print group values of group A
  Serial.print(buttonGroupAValue);
  Serial.print('\t');
  if (buttonGroupAMode == WJC_BTN_GROUP_MULTI) {
    Serial.print("Multi");
  } else if (buttonGroupAMode == WJC_BTN_GROUP_SINGLE) {
    Serial.print("Single");
  }
  Serial.print('\t');
  Serial.print('\t');

  // print group values of group B
  Serial.print(buttonGroupBValue);
  Serial.print('\t');
  if (buttonGroupBMode == WJC_BTN_GROUP_MULTI) {
    Serial.print("Multi");
  } else if (buttonGroupBMode == WJC_BTN_GROUP_SINGLE) {
    Serial.print("Single");
  }
}