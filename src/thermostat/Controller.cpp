#include <thermostat/Controller.h>
#include <esp_log.h>
#undef TAG
static const char* TAG = "thermostat";

Thermostats::Controller Thermostat;

namespace Thermostats {

void Controller::init(Scheduler& scheduler)
{
    const auto& pin = PinMapping.get();

    int8_t sensorSDAPin  = pin.thermostat_sensor_sda; 
    int8_t sensorSCLPin  = pin.thermostat_sensor_scl; 
    int8_t coolingPin = pin.thermostat_cooling;
    int8_t heatingPin = pin.thermostat_heating;
    
    if (sensorSDAPin != -1 && sensorSCLPin != -1 && (heatingPin != -1 || coolingPin != -1))
    {
        if (heatingPin != -1) {
            _heatingPin = heatingPin;
            pinMode(_heatingPin, OUTPUT);
        } 
        if (coolingPin != -1) {
            _coolingPin = coolingPin;
            pinMode(_coolingPin, OUTPUT);
        }
        if (!Wire.begin(sensorSDAPin, sensorSCLPin)) {
            ESP_LOGI(TAG, "Thermostat: Failed to start I2C bus");
        }
        if (!_htu.begin()) {
            ESP_LOGI(TAG, "Thermostat: Can't init sensor");
        }
        if (Configuration.get().Mqtt.Enabled) {
            MqttSettings.subscribe(MqttSettings.getPrefix() + "thermostat/cmd/#", 0, std::bind(
                &Controller::onMqttMessage, 
                this, 
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4
            ));
        }
        updateSettings();
        
        scheduler.addTask(_loopTask);
        _loopTask.setCallback(std::bind(&Controller::loop, this));
        _loopTask.setIterations(TASK_FOREVER);
        _loopTask.enable();

        scheduler.addTask(_mqttLoopTask);
        _mqttLoopTask.setCallback(std::bind(&Controller::mqttLoop, this));
        _mqttLoopTask.setIterations(TASK_FOREVER);
        _mqttLoopTask.setInterval(Configuration.get().Mqtt.PublishInterval * TASK_SECOND);
        _mqttLoopTask.enable();
    } else 
    {
        ESP_LOGI(TAG, "Thermostat: Disabled (Incomplete Pin Mapping)");
    }
}

void Controller::updateSettings()
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    const auto& config = Configuration.get();
    _isEnabled = config.Thermostat.Enabled;
    _lowTemp = config.Thermostat.LowTemp;
    _highTemp = config.Thermostat.HighTemp;
}    

void Controller::loop()
{
    std::lock_guard<std::mutex> lock(_mutex);
    float temp = _htu.readTemperature();
    float hum = _htu.readHumidity();
    if (isnan(temp)) return;
    _currentTemp = temp;
    _currentHum = hum;

    if (!Configuration.get().Thermostat.Enabled) {
        digitalWrite(_heatingPin, LOW);
        digitalWrite(_coolingPin, LOW);
        _heatingActive = false;
        _coolingActive = false;
        return;
    }
    if (_heatingPin != -1) {
        if (_currentTemp < _lowTemp) {
            digitalWrite(_heatingPin, HIGH);
            _heatingActive = true;
        } else {
            digitalWrite(_heatingPin, LOW);
            _heatingActive = false;
        }
    }
    if (_coolingPin != -1) {
        if (_currentTemp > _highTemp) {
            digitalWrite(_coolingPin, HIGH);
            _coolingActive = true;
        } else {
            digitalWrite(_coolingPin, LOW);
            _coolingActive = false;
        }
    }
}

void Controller::mqttLoop()
{
    if (!(Configuration.get().Mqtt.Enabled && MqttSettings.getConnected())) {
        return;
    }
    JsonDocument doc;
    doc["enabled"]   = _isEnabled;
    doc["temp"]      = String(_currentTemp).c_str();
    doc["hum"]       = String(_currentHum).c_str();
    doc["low_temp"]  = _lowTemp;
    doc["high_temp"] = _highTemp;
    doc["heating"]   = _heatingActive;
    doc["cooling"]   = _coolingActive;

    char buffer[256];
    serializeJson(doc, buffer);

    String baseTopic = "thermostat";
    MqttSettings.publish((baseTopic + "/status").c_str(), buffer);
    MqttSettings.publish((baseTopic + "/temp").c_str(), String(_currentTemp).c_str());
    MqttSettings.publish((baseTopic + "/hum").c_str(), String(_currentHum).c_str());
}    

void Controller::onMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len) 
{
    String p = "";
    for (size_t i = 0; i < len; i++) {
        p += (char)payload[i];
    }
    String t = String(topic);
    if (t.endsWith("/enabled")) {
        _isEnabled = (p == "1" || p.equalsIgnoreCase("true") || p.equalsIgnoreCase("ON"));
    } else if (t.endsWith("/low_temp")) {
        float val = p.toFloat();
        if (val >= 10.0 && val < _highTemp) {
            _lowTemp = val;
        }
    } else if (t.endsWith("/high_temp")) {
        float val = p.toFloat();
        if (val <= 40.0 && val > _lowTemp) {
            _highTemp = val;
        }
    }
    {
        auto guard = Configuration.getWriteGuard();
        auto& config = guard.getConfig();
        config.Thermostat.Enabled = _isEnabled;
        config.Thermostat.LowTemp = _lowTemp;
        config.Thermostat.HighTemp = _highTemp;
    }
    Configuration.write();
}

void Controller::setEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _isEnabled = enabled;
}

void Controller::setLowTemp(float temp) {
    std::lock_guard<std::mutex> lock(_mutex);
    _lowTemp = temp;
}

void Controller::setHighTemp(float temp)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _highTemp = temp;
}

float Controller::getTemperature() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _currentTemp;
}

float Controller::getHumidity() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _currentHum;
}

bool Controller::isHeatingActive() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _heatingActive;
}

bool Controller::isCoolingActive() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _coolingActive;
}

} // namespace Thermostats
