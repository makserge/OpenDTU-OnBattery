// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <TaskSchedulerDeclarations.h>
#include <memory>
#include <mutex>
#include <Wire.h>
#include <Configuration.h>
#include <ArduinoJson.h>
#include <MqttSettings.h>
#include <Adafruit_HTU21DF.h>

namespace Thermostats {

class Controller {
public:
    void init(Scheduler& scheduler);
    void updateSettings();

    void setMQTTEnabled(bool enabled);
    void setEnabled(bool enabled);
    void setLowTemp(float temp);
    void setHighTemp(float temp);
    float getTemperature() const;
    float getHumidity() const;
    bool isHeatingActive() const;
    bool isCoolingActive() const;

private:
    void loop();
    void mqttLoop();
    void onMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len);
    
    Task _loopTask;
    Task _mqttLoopTask;
    mutable std::mutex _mutex;

    static constexpr float _hysteresis = 1.0f;

    Adafruit_HTU21DF _htu = Adafruit_HTU21DF();

    int8_t _heatingPin;
    int8_t _coolingPin;

    bool _isEnabled = false;
    float _currentTemp = 0.0f;
    float _currentHum = 0.0f;
    float _lowTemp = 20.0f;
    float _highTemp = 25.0f;
        
    bool _heatingActive = false;
    bool _coolingActive = false;   
};

} // namespace Thermostats

extern Thermostats::Controller Thermostat;