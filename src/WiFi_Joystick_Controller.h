/**
 * @file WiFi_Joystick_Controller.h
 *
 * @brief Arduino library for connect WiFi compatible development boards with "WiFi Joystick Controller" mobile app
 *
 * @author Manodya Rasanjana <manodya@srqrobotics.com>
 *
 * @version 1.0.0
 *
 * @date 2024-01-01
 *
 * @url https://github.com/srqrobotics/WiFi_Joystick_Controller
 *
 * -----
 *
 * @copyright Copyright (c) 2023-2024 SRQ Robotics (https://www.srqrobotics.com)
 *
 * This file is part of the WiFi_Joystick_Controller Arduino library
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * -----
 *
 * Additionally, this library incorporates code from the ArduinoJson project,
 * which is licensed under the MIT License. The original copyright notice
 * and license terms are included below:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the “Software”), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __SRQ_WIFI_JOYSTICK_CONTROLLER_H__
#define __SRQ_WIFI_JOYSTICK_CONTROLLER_H__

#include <Arduino.h>
#include "ArduinoJson/ArduinoJson-v6.21.4.h" // special thanks to Benoit BLANCHON (https://arduinojson.org)

// WiFi libraries
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
#include <WiFi.h>
#include <WiFiUdp.h>
#elif defined(ARDUINO_SAMD_MKR1000)
#include <WiFi101.h>
#include <WiFiUdp.h>
#else
// TODO: reserved for future
#endif

// default return value if no errors detected
constexpr uint8_t WJC_ERR_OK = 0;

// WiFi modes
constexpr uint8_t WJC_WIFI_MODE_AP = 1;  // access point (hot-spot)
constexpr uint8_t WJC_WIFI_MODE_STA = 2; // station (connect to an external network)

// joystick selection
constexpr uint8_t WJC_LEFT_JOYSTICK = 1;
constexpr uint8_t WJC_RIGHT_JOYSTICK = 2;

// joystick axis selection
constexpr uint8_t WJC_X_AXIS = 1;
constexpr uint8_t WJC_Y_AXIS = 2;

// button group selection
constexpr uint8_t WJC_BTN_GROUP_A = 1;
constexpr uint8_t WJC_BTN_GROUP_B = 2;

// individual button selection
constexpr uint8_t WJC_BTN_1 = 1;
constexpr uint8_t WJC_BTN_2 = 2;
constexpr uint8_t WJC_BTN_3 = 3;

// button group data selection
constexpr uint8_t WJC_BTN_GROUP_MODE = 1;
constexpr uint8_t WJC_BTN_GROUP_VALUE = 2;

// button group mode selection
constexpr uint8_t WJC_BTN_GROUP_SINGLE = 1; // only a single button can select at a time
constexpr uint8_t WJC_BTN_GROUP_MULTI = 2;  // multiples buttons can be selected

// structure to hold button group data
typedef struct
{
    uint8_t value;
    bool mode;
    bool button1;
    bool button2;
    bool button3;
} WJC_Btn_Grp_t;

// structure to hold remote controller data
typedef struct
{
    int8_t leftJoystickX;
    int8_t leftJoystickY;
    int8_t rightJoystickX;
    int8_t rightJoystickY;
    WJC_Btn_Grp_t btnGroupA;
    WJC_Btn_Grp_t btnGroupB;
} WJC_Remote_t;

class WiFi_Joystick_Controller
{
public:
    /**
     * @fn WiFi_Joystick_Controller
     * @brief constructor
     * @param udpPort desired port number for the UDP socket
     */
    WiFi_Joystick_Controller(uint16_t udpPort);

    /**
     * @fn init
     * @brief initialize only the library instance. WiFi must enable separately (or using following init functions)
     * @param wifiInitialized current WiFi initialization status
     * @n true WiFi already initialized separately
     * @n false WiFi not initialized separately
     * @return initialization status
     * @retval 0 initialization succeeded
     * @retval 1 WiFi not initialized before
     * @retval 2 UDP socket cannot initialized
     */
    uint8_t init(bool wifiInitialized);

    /**
     * @fn init
     * @brief initialize WiFi and controller instance
     * @param mode WiFi mode to initialize
     * @n WJC_WIFI_MODE_AP WiFi Access Point mode (Other devices can connect to development board's network)
     * @n WJC_WIFI_MODE_STA WiFi Station mode (development board and devices connect to an external network)
     * @param ssid name of the WiFi network
     * @param password password of the WiFi network
     * @return initialization status
     * @retval 0 initialization succeeded
     * @retval 1 mode is not correct
     * @retval 2 WiFi already initialized using the library
     * @retval 3 AP cannot initialized
     * @retval 4 cannot connected to the external network using given credentials
     * @retval 5 UDP socket cannot initialized
     */
    uint8_t init(uint8_t mode, const char *ssid, const char *password);

    /**
     * @fn init
     * @brief initialize WiFi and connect to an external WiFi network using a predefined IP Address
     * @param ssid name of the external WiFi network
     * @param password password of the external WiFi network
     * @param staticIP desired IP address for the development board
     * @param gateway gateway of the external WiFi network
     * @param subnet subnet of the external WiFi network
     * @param primaryDNS primaryDNS of the external WiFi network
     * @param secondaryDNS secondaryDNS of the external WiFi network
     * @return initialization status
     * @retval 0 initialization succeeded
     * @retval 1 WiFi already initialized using the library
     * @retval 2 WiFi cannot configure using given credentials
     * @retval 3 cannot connected to the external network using given credentials
     * @retval 4 UDP socket cannot initialized
     */
    uint8_t init(const char *ssid, const char *password, IPAddress staticIP, IPAddress gateway, IPAddress subnet, IPAddress primaryDNS, IPAddress secondaryDNS);

    /**
     * @fn update
     * @brief read the latest received data and store in data holding variables
     * @param sendValidationMessage send a reply to the mobile app
     * @return update status
     * @retval 0 update succeeded
     * @retval 1 WiFi not initialized
     * @retval 2 no data packet received since last read
     * @retval 3 cannot deserialize received data packet
     * @retval 4 data cannot validated
     */
    uint8_t update(bool sendValidationMessage = true);

    /**
     * @fn setDataValidationTimeout
     * @brief set the timeout for the getDataValidStatus()
     * @param timeout_ms timeout in Millisecond
     */
    void setDataValidationTimeout(uint16_t timeout_ms);

    /**
     * @fn getDataValidStatus
     * @brief can use to check if new valid data received during a predefined period. Default timeout is 500mS.
     * @return validation status
     * @retval 0 data valid
     * @retval 1 data not valid
     */
    uint8_t getDataValidStatus(void);

    /**
     * @fn getJoystick
     * @brief get joystick axis values
     * @param whichJoystick selected joystick
     * @n WJC_LEFT_JOYSTICK to select left joystick
     * @n WJC_RIGHT_JOYSTICK to select joystick
     * @param axis selected axis
     * @n WJC_X_AXIS to select x-axis
     * @n WJC_Y_AXIS to select y-axis
     * @return value of the selected joystick axis (range is (-100) - 100)
     */
    int8_t getJoystick(uint8_t whichJoystick, uint8_t axis);

    /**
     * @fn getButtonGroupValue
     * @brief get value of the entire button group
     * @param whichGroup selected button group
     * @n WJC_BTN_GROUP_A to select button group A
     * @n WJC_BTN_GROUP_B to select button group B
     * @return value of the selected button group
     */
    uint8_t getButtonGroupValue(uint8_t whichGroup);

    /**
     * @fn getButtonGroupMode
     * @brief get the mode of the button group
     * @param whichGroup selected button group
     * @n WJC_BTN_GROUP_A to select button group A
     * @n WJC_BTN_GROUP_B to select button group B
     * @return mode of the selected button group
     * @retval WJC_BTN_GROUP_SINGLE button group is in single-selection mode
     * @retval WJC_BTN_GROUP_MULTI button group is in multi-selection mode
     */
    uint8_t getButtonGroupMode(uint8_t whichGroup);

    /**
     * @fn getButtonValue
     * @brief get the status of an individual button
     * @param whichGroup selected button group
     * @n WJC_BTN_GROUP_A to select button group A
     * @n WJC_BTN_GROUP_B to select button group B
     * @param whichButton select button
     * @n WJC_BTN_1 to select button 1
     * @n WJC_BTN_2 to select button 2
     * @n WJC_BTN_3 to select button 3
     * @return button status
     * @retval true button is pressed
     * @retval false button is not pressed
     */
    bool getButtonValue(uint8_t whichGroup, uint8_t whichButton);

    /**
     * @fn sendReply
     * @brief send data to mobile app
     * @param sendImmediately send data without any skipping
     */
    void sendReply(bool sendImmediately);

    /**
     * @fn getIpAddress
     * @brief get the IP address of the development board
     * @return local IP address
     */
    IPAddress getIpAddress(void);

    /**
     * @fn getPortNumber
     * @brief get port number of the UDP socket
     * @return local port number
     */
    uint16_t getPortNumber(void);

private:
    /**
     * @fn _initAP
     * @brief initialize WiFi as an Access Point
     * @param ssid desired name of the WiFi hot-spot
     * @param password desired password of the WiFi hot-spot
     * @return initialization status
     * @retval 0 initialization succeeded
     * @retval 1 initialization failed
     */
    uint8_t _initAP(const char *ssid, const char *password);

    /**
     * @fn _initAP
     * @brief initialize WiFi as an Access Point
     * @param ssid name of the external WiFi network
     * @param password password of the external WiFi network
     * @return initialization status
     * @retval 0 connected to the external network
     * @retval 1 cannot connect to the external network
     */
    uint8_t _initSTA(const char *ssid, const char *password);

    /**
     * @fn _initUDP
     * @brief initialize UDP socket
     * @return initialization status
     * @retval 0 initialization succeeded
     * @retval 1 provided port number is wrong
     * @retval 2 initialization failed
     */
    uint8_t _initUDP(void);

    /**
     * @fn _calcBtnValues
     * @brief calculate the value of each individual button
     */
    void _calcBtnValues(void);

    // joystick controller data holding variable
    WJC_Remote_t _wjcData;

    // UDP socket instance
    WiFiUDP _UDP;

    // UDP port number
    uint16_t _port = 0;

    // local IP address
    IPAddress _ipAddress = IPAddress(0, 0, 0, 0);

    // time flag of the last successful updated
    uint16_t _validationTimeout_ms = 500;
    unsigned long _lastUpdated_ms;

    // WiFi init flag. This variable will share between all of the library instances
    static bool WJC_WIFI_INIT;
};

#endif // __SRQ_WIFI_JOYSTICK_CONTROLLER_H__