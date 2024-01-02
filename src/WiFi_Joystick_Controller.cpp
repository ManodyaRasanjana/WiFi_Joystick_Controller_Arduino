/**
 * @file WiFi_Joystick_Controller.cpp
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

#include "WiFi_joystick_controller.h"

bool WiFi_Joystick_Controller::WJC_WIFI_INIT = false;

WiFi_Joystick_Controller::WiFi_Joystick_Controller(uint16_t udpPort)
{
    _port = udpPort;
}

uint8_t WiFi_Joystick_Controller::init(bool wifiInitialized)
{
    uint8_t err = WJC_ERR_OK;

    if (wifiInitialized)
    {
        WJC_WIFI_INIT = true;
    }

    // since no WiFi method in this function, check if WiFi enabled using another library function
    if (!WJC_WIFI_INIT)
    {
        err = 1;
        return err;
    }

    // enable UDP port and get status
    uint8_t udpSuccess = _initUDP();
    if (udpSuccess != WJC_ERR_OK)
    {
        err = 2;
        return err;
    }

    return err;
}

uint8_t WiFi_Joystick_Controller::init(uint8_t mode, const char *ssid, const char *password)
{
    uint8_t err = WJC_ERR_OK;

    // validate WiFi modes
    if (mode != WJC_WIFI_MODE_AP && mode != WJC_WIFI_MODE_STA)
    {
        err = 1;
        return err;
    }

    // check if WiFi already enabled using the library
    if (WJC_WIFI_INIT)
    {
        err = 2;
        return err;
    }

    // enable WiFi as an Access Point
    if (mode == WJC_WIFI_MODE_AP)
    {
        uint8_t apSucceed = _initAP(ssid, password);
        if (apSucceed != WJC_ERR_OK)
        {
            err = 3;
            return err;
        }
    }

    // enable WiFi as a Station
    if (mode == WJC_WIFI_MODE_STA)
    {
        uint8_t staSucceed = _initSTA(ssid, password);
        if (staSucceed != WJC_ERR_OK)
        {
            err = 4;
            return err;
        }
    }

    // WiFi initialized
    WJC_WIFI_INIT = true;

    // enable UDP port and get status
    uint8_t udpSuccess = _initUDP();
    if (udpSuccess != WJC_ERR_OK)
    {
        err = 5;
        return err;
    }

    return err;
}

uint8_t WiFi_Joystick_Controller::init(const char *ssid, const char *password, IPAddress staticIP, IPAddress gateway, IPAddress subnet, IPAddress primaryDNS, IPAddress secondaryDNS)
{
    uint8_t err = WJC_ERR_OK;

    // check if WiFi already enabled using the library
    if (WJC_WIFI_INIT)
    {
        err = 1;
        return err;
    }

// set WiFi configurations
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    if (!(WiFi.config(staticIP, primaryDNS, gateway, subnet)))
    {
        err = 2;
        return err;
    }
#elif defined(ARDUINO_SAMD_MKR1000)
    WiFi.config(staticIP);
#else
    // TODO: reserved for future
#endif

    // enable WiFi as a Station
    uint8_t staSucceed = _initSTA(ssid, password);
    if (staSucceed != WJC_ERR_OK)
    {
        err = 3;
        return err;
    }

    // WiFi initialized
    WJC_WIFI_INIT = true;

    // enable UDP port and get status
    uint8_t udpSuccess = _initUDP();
    if (udpSuccess != WJC_ERR_OK)
    {
        err = 4;
        return err;
    }

    return err;
}

uint8_t WiFi_Joystick_Controller::update(bool sendValidationMessage)
{
    uint8_t err = WJC_ERR_OK;
    const uint8_t bufferSize = 200;
    char pktBuffer[bufferSize];

    // check if WiFi enabled previously
    if (!WJC_WIFI_INIT)
    {
        err = 1;
        return err;
    }

    // check if a UDP data packet received and process it if received
    uint16_t pktSize = _UDP.parsePacket();
    if (pktSize)
    {
        uint16_t dataLength = _UDP.read(pktBuffer, bufferSize - 1);
        pktBuffer[dataLength] = '\0';

        StaticJsonDocument<bufferSize> jsonBuffer;
        DeserializationError jsonError = deserializeJson(jsonBuffer, pktBuffer);

        if (!jsonError)
        {
            bool dataValid = (bool)jsonBuffer["WJC"]; // validation tag

            if (dataValid)
            {
                _wjcData.leftJoystickX = (int8_t)jsonBuffer["jsLx"]; // left joystick X
                _wjcData.leftJoystickY = (int8_t)jsonBuffer["jsLy"]; // left joystick Y

                _wjcData.rightJoystickX = (int8_t)jsonBuffer["jsRx"]; // right joystick X
                _wjcData.rightJoystickY = (int8_t)jsonBuffer["jsRy"]; // right joystick Y

                _wjcData.btnGroupA.value = (uint8_t)jsonBuffer["bgA"]; // button group A value
                _wjcData.btnGroupA.mode = (bool)jsonBuffer["bgmA"];    // button group A mode

                _wjcData.btnGroupB.value = (uint8_t)jsonBuffer["bgB"]; // button group B value
                _wjcData.btnGroupB.mode = (bool)jsonBuffer["bgmB"];    // button group B mode

                _calcBtnValues();
                _lastUpdated_ms = millis();

                if (sendValidationMessage)
                {
                    sendReply(false);
                }
            }
            // data cannot validated
            else
            {
                err = 4;
            }
        }
        // cannot deserialize received packet
        else
        {
            err = 3;
        }
    }
    // no packet received since last read
    else
    {
        err = 2;
    }

    return err;
}

void WiFi_Joystick_Controller::setDataValidationTimeout(uint16_t timeout_ms)
{
    _validationTimeout_ms = timeout_ms;
}

uint8_t WiFi_Joystick_Controller::getDataValidStatus(void)
{
    uint8_t err = WJC_ERR_OK;

    if (millis() - _lastUpdated_ms >= _validationTimeout_ms)
    {
        err = 1; // timeout occurred
    }

    return err;
}

int8_t WiFi_Joystick_Controller::getJoystick(uint8_t whichJoystick, uint8_t axis)
{
    int8_t val = 0;

    if (whichJoystick == WJC_LEFT_JOYSTICK && axis == WJC_X_AXIS)
    {
        val = _wjcData.leftJoystickX;
    }
    if (whichJoystick == WJC_LEFT_JOYSTICK && axis == WJC_Y_AXIS)
    {
        val = _wjcData.leftJoystickY;
    }
    if (whichJoystick == WJC_RIGHT_JOYSTICK && axis == WJC_X_AXIS)
    {
        val = _wjcData.rightJoystickX;
    }
    if (whichJoystick == WJC_RIGHT_JOYSTICK && axis == WJC_Y_AXIS)
    {
        val = _wjcData.rightJoystickY;
    }

    return val;
}

uint8_t WiFi_Joystick_Controller::getButtonGroupValue(uint8_t whichGroup)
{
    if (whichGroup == WJC_BTN_GROUP_A)
    {
        return _wjcData.btnGroupA.value;
    }

    if (whichGroup == WJC_BTN_GROUP_B)
    {
        return _wjcData.btnGroupB.value;
    }

    return 0;
}

uint8_t WiFi_Joystick_Controller::getButtonGroupMode(uint8_t whichGroup)
{
    if (whichGroup == WJC_BTN_GROUP_A)
    {
        return (_wjcData.btnGroupA.mode) ? WJC_BTN_GROUP_MULTI : WJC_BTN_GROUP_SINGLE;
    }

    if (whichGroup == WJC_BTN_GROUP_B)
    {
        return (_wjcData.btnGroupB.mode) ? WJC_BTN_GROUP_MULTI : WJC_BTN_GROUP_SINGLE;
    }

    return 0;
}

bool WiFi_Joystick_Controller::getButtonValue(uint8_t whichGroup, uint8_t whichButton)
{
    if (whichGroup == WJC_BTN_GROUP_A && whichButton == WJC_BTN_1)
    {
        return _wjcData.btnGroupA.button1;
    }

    if (whichGroup == WJC_BTN_GROUP_A && whichButton == WJC_BTN_2)
    {
        return _wjcData.btnGroupA.button2;
    }

    if (whichGroup == WJC_BTN_GROUP_A && whichButton == WJC_BTN_3)
    {
        return _wjcData.btnGroupA.button3;
    }

    if (whichGroup == WJC_BTN_GROUP_B && whichButton == WJC_BTN_1)
    {
        return _wjcData.btnGroupB.button1;
    }

    if (whichGroup == WJC_BTN_GROUP_B && whichButton == WJC_BTN_2)
    {
        return _wjcData.btnGroupB.button2;
    }

    if (whichGroup == WJC_BTN_GROUP_B && whichButton == WJC_BTN_3)
    {
        return _wjcData.btnGroupB.button3;
    }

    return false;
}

void WiFi_Joystick_Controller::sendReply(bool sendImmediately)
{
    static uint8_t skipper = 0;
    uint8_t replyBuff[] = "{\"valid\"=1}";

    if ((skipper >= 3) || sendImmediately)
    {
        _UDP.beginPacket(_UDP.remoteIP(), _UDP.remotePort());
        _UDP.write(replyBuff, sizeof(replyBuff));
        _UDP.endPacket();
        skipper = 0;
    }
    skipper++;
}

IPAddress WiFi_Joystick_Controller::getIpAddress(void)
{
    return _ipAddress;
}

uint16_t WiFi_Joystick_Controller::getPortNumber(void)
{
    return _port;
}

uint8_t WiFi_Joystick_Controller::_initAP(const char *ssid, const char *password)
{
    uint8_t err = WJC_ERR_OK;
    bool apSucceed = false;

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    // configure WiFi mode as Access Point
    WiFi.mode(WIFI_AP);

    // enable Access Point
    apSucceed = (WiFi.softAP(ssid, password));

    // save IP address
    _ipAddress = WiFi.softAPIP();

#elif defined(ARDUINO_SAMD_MKR1000)
    // enable Access Point
    int8_t status = WiFi.beginAP(ssid);

    // validate the status
    apSucceed = (status == WL_AP_LISTENING) ? true : false;

    _ipAddress = WiFi.localIP();

#else
    // TODO: reserved for future
#endif

    if (!apSucceed)
    {
        err = 1;
        return err;
    }

    return err;
}

uint8_t WiFi_Joystick_Controller::_initSTA(const char *ssid, const char *password)
{
    uint8_t err = WJC_ERR_OK;

// configure WiFi mode as Station
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    WiFi.mode(WIFI_STA);
#elif defined(ARDUINO_SAMD_MKR1000)
    // TODO: MKR1000 WiFi configurations
#else
    // TODO: reserved for future
#endif

    // start WiFi connection
    WiFi.begin(ssid, password);

    // check if connected to a external network using provided credentials
    uint8_t staSucceed = 1;
    for (uint8_t i = 0; i < 200; i++)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            staSucceed = WJC_ERR_OK;
            break;
        }
        delay(50);
    }

    if (staSucceed != WJC_ERR_OK)
    {
        err = 1;
        return err;
    }

    _ipAddress = WiFi.localIP();

    return err;
}

uint8_t WiFi_Joystick_Controller::_initUDP(void)
{
    uint8_t err = WJC_ERR_OK;

    // validate port number
    if (_port < 0 || _port > 65535)
    {
        err = 1;
        return err;
    }

    // init the UDP socket
    if (!(_UDP.begin(_port)))
    {
        err = 2;
        return err;
    }

    return err;
}

void WiFi_Joystick_Controller::_calcBtnValues(void)
{
    if (_wjcData.btnGroupA.mode)
    {
        if (_wjcData.btnGroupA.value == 0)
        {
            _wjcData.btnGroupA.button1 = false;
            _wjcData.btnGroupA.button2 = false;
            _wjcData.btnGroupA.button3 = false;
        }
        else if (_wjcData.btnGroupA.value == 1)
        {
            _wjcData.btnGroupA.button1 = true;
            _wjcData.btnGroupA.button2 = false;
            _wjcData.btnGroupA.button3 = false;
        }
        else if (_wjcData.btnGroupA.value == 2)
        {
            _wjcData.btnGroupA.button1 = false;
            _wjcData.btnGroupA.button2 = true;
            _wjcData.btnGroupA.button3 = false;
        }
        else if (_wjcData.btnGroupA.value == 3)
        {
            _wjcData.btnGroupA.button1 = true;
            _wjcData.btnGroupA.button2 = true;
            _wjcData.btnGroupA.button3 = false;
        }
        else if (_wjcData.btnGroupA.value == 4)
        {
            _wjcData.btnGroupA.button1 = false;
            _wjcData.btnGroupA.button2 = false;
            _wjcData.btnGroupA.button3 = true;
        }
        else if (_wjcData.btnGroupA.value == 5)
        {
            _wjcData.btnGroupA.button1 = true;
            _wjcData.btnGroupA.button2 = false;
            _wjcData.btnGroupA.button3 = true;
        }
        else if (_wjcData.btnGroupA.value == 6)
        {
            _wjcData.btnGroupA.button1 = false;
            _wjcData.btnGroupA.button2 = true;
            _wjcData.btnGroupA.button3 = true;
        }
        else if (_wjcData.btnGroupA.value == 7)
        {
            _wjcData.btnGroupA.button1 = true;
            _wjcData.btnGroupA.button2 = true;
            _wjcData.btnGroupA.button3 = true;
        }
    }
    else
    {
        if (_wjcData.btnGroupA.value == 1)
        {
            _wjcData.btnGroupA.button1 = true;
            _wjcData.btnGroupA.button2 = false;
            _wjcData.btnGroupA.button3 = false;
        }
        else if (_wjcData.btnGroupA.value == 2)
        {
            _wjcData.btnGroupA.button1 = false;
            _wjcData.btnGroupA.button2 = true;
            _wjcData.btnGroupA.button3 = false;
        }
        else if (_wjcData.btnGroupA.value == 3)
        {
            _wjcData.btnGroupA.button1 = false;
            _wjcData.btnGroupA.button2 = false;
            _wjcData.btnGroupA.button3 = true;
        }
    }

    if (_wjcData.btnGroupB.mode)
    {
        if (_wjcData.btnGroupB.value == 0)
        {
            _wjcData.btnGroupB.button1 = false;
            _wjcData.btnGroupB.button2 = false;
            _wjcData.btnGroupB.button3 = false;
        }
        else if (_wjcData.btnGroupB.value == 1)
        {
            _wjcData.btnGroupB.button1 = true;
            _wjcData.btnGroupB.button2 = false;
            _wjcData.btnGroupB.button3 = false;
        }
        else if (_wjcData.btnGroupB.value == 2)
        {
            _wjcData.btnGroupB.button1 = false;
            _wjcData.btnGroupB.button2 = true;
            _wjcData.btnGroupB.button3 = false;
        }
        else if (_wjcData.btnGroupB.value == 3)
        {
            _wjcData.btnGroupB.button1 = true;
            _wjcData.btnGroupB.button2 = true;
            _wjcData.btnGroupB.button3 = false;
        }
        else if (_wjcData.btnGroupB.value == 4)
        {
            _wjcData.btnGroupB.button1 = false;
            _wjcData.btnGroupB.button2 = false;
            _wjcData.btnGroupB.button3 = true;
        }
        else if (_wjcData.btnGroupB.value == 5)
        {
            _wjcData.btnGroupB.button1 = true;
            _wjcData.btnGroupB.button2 = false;
            _wjcData.btnGroupB.button3 = true;
        }
        else if (_wjcData.btnGroupB.value == 6)
        {
            _wjcData.btnGroupB.button1 = false;
            _wjcData.btnGroupB.button2 = true;
            _wjcData.btnGroupB.button3 = true;
        }
        else if (_wjcData.btnGroupB.value == 7)
        {
            _wjcData.btnGroupB.button1 = true;
            _wjcData.btnGroupB.button2 = true;
            _wjcData.btnGroupB.button3 = true;
        }
    }
    else
    {
        if (_wjcData.btnGroupB.value == 1)
        {
            _wjcData.btnGroupB.button1 = true;
            _wjcData.btnGroupB.button2 = false;
            _wjcData.btnGroupB.button3 = false;
        }
        else if (_wjcData.btnGroupB.value == 2)
        {
            _wjcData.btnGroupB.button1 = false;
            _wjcData.btnGroupB.button2 = true;
            _wjcData.btnGroupB.button3 = false;
        }
        else if (_wjcData.btnGroupB.value == 3)
        {
            _wjcData.btnGroupB.button1 = false;
            _wjcData.btnGroupB.button2 = false;
            _wjcData.btnGroupB.button3 = true;
        }
    }
}
