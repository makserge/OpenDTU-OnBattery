#include <powermeter_bl/Controller.h>
#include <ArduinoJson.h>
#include <MqttSettings.h>
#include <Configuration.h>
#include <esp_log.h>
#include <memory>

#undef TAG
static const char *TAG = "powermeter_bl";

PowermeterBls::Controller PowermeterBl;

namespace PowermeterBls
{
    void Controller::init(Scheduler &scheduler)
    {
        const auto &pin = PinMapping.get();

        int8_t rxPin = pin.powermeter_bl_rx;
        int8_t txPin = pin.powermeter_bl_tx;

        if (rxPin != -1 && txPin != -1)
        {
            auto oHwSerialPort = SerialPortManager.allocatePort("PowermeterBL");
            if (!oHwSerialPort) {
                ESP_LOGI(TAG, "PowermeterBL: Unable to allocate serial port");
                return;
            }
            _serial = std::make_unique<HardwareSerial>(*oHwSerialPort);
            _serial->begin(BL0942_BAUD, SERIAL_8N1, rxPin, txPin);
            delay(1000);
            while(_serial->available()) _serial->read();
            _sensor = std::make_unique<bl0942::BL0942>(*_serial);
            _sensor->setup();
            _sensor->onDataReceived([this](bl0942::SensorData &data)
            {
                _power = std::round(data.watt * POWER_CALIBRATION  * 100.0f) / 100.0f;
                _voltage = std::round(data.voltage * 100.0f) / 100.0f;
                _current = std::round(data.current * CURRENT_CALIBRATION * 100.0f) / 100.0f;
                _apparentPower = std::round(_voltage * _current * 100.0f) / 100.0f;
            });

            scheduler.addTask(_loopTask);
            _loopTask.setCallback(std::bind(&Controller::loop, this));
            _loopTask.setIterations(TASK_FOREVER);
            _loopTask.enable();

            scheduler.addTask(_mqttLoopTask);
            _mqttLoopTask.setCallback(std::bind(&Controller::mqttLoop, this));
            _mqttLoopTask.setIterations(TASK_FOREVER);
            _mqttLoopTask.setInterval(Configuration.get().Mqtt.PublishInterval * TASK_SECOND);
            _mqttLoopTask.enable();

            updateSettings();
        }
        else
        {
            ESP_LOGI(TAG, "PowermeterBL: Disabled (Incomplete Pin Mapping)");
        }
    }

    void Controller::updateSettings()
    {
        std::lock_guard<std::mutex> lock(_mutex);
    
        const CONFIG_T& config = Configuration.get();
        _isEnabled = config.PowermeterBl.Enabled;
    }   

    float Controller::getPower() const { return _power; }
    float Controller::getApparentPower() const { return _apparentPower; }
    float Controller::getVoltage() const { return _voltage; }
    float Controller::getCurrent() const { return _current; }

    void Controller::loop()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_isEnabled && _sensor)
        {
            _sensor->loop();
        }
    }

    void Controller::mqttLoop()
    {
        if (_sensor)
        {
            _sensor->update();
        }
        if (!(Configuration.get().Mqtt.Enabled && MqttSettings.getConnected())) {
            return;
        }
        JsonDocument doc;
        doc["enabled"] = _isEnabled;
        doc["power"] = _power;
        doc["apparent_power"] = _apparentPower;
        doc["current"] = _current;
        doc["voltage"] = _voltage;

        char buffer[256];
        serializeJson(doc, buffer);

        MqttSettings.publish("powermeter_bl/status", buffer);
    }

} // namespace PowermeterBls
