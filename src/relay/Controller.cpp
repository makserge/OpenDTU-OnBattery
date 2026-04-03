#include <relay/Controller.h>
#include <ArduinoJson.h>
#include <MqttSettings.h>
#include <Configuration.h>
#include <esp_log.h>

#undef TAG
static const char *TAG = "relay";

Relays::Controller Relay;

namespace Relays
{
    void Controller::init(Scheduler &scheduler)
    {
        const auto &pin = PinMapping.get();

        int8_t relay1Pin = pin.relay_relay1;
        int8_t relay2Pin = pin.relay_relay2;
        if (relay1Pin != -1 && relay2Pin != -1)
        {
            _relay1Pin = relay1Pin;
            _relay2Pin = relay2Pin;
        
            pinMode(relay1Pin, OUTPUT);
            pinMode(relay2Pin, OUTPUT);
            
            if (Configuration.get().Mqtt.Enabled) {
                MqttSettings.subscribe(MqttSettings.getPrefix() + "relay/cmd/#", 0, std::bind(
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
            _loopTask.setInterval(Configuration.get().Mqtt.PublishInterval * TASK_SECOND);
            _loopTask.enable();
        }
        else
        {
            ESP_LOGI(TAG, "Relay: Disabled (Incomplete Pin Mapping)");
        }
    }

    void Controller::updateSettings()
    {
        std::lock_guard<std::mutex> lock(_mutex);
    
        const CONFIG_T& config = Configuration.get();
        _isRelay1Enabled = config.Relay.Relay1Enabled;
        _isRelay2Enabled = config.Relay.Relay2Enabled;
        
        digitalWrite(_relay1Pin, _isRelay1Enabled ? HIGH : LOW);
        digitalWrite(_relay2Pin, _isRelay2Enabled ? HIGH : LOW);
    }   

    void Controller::loop()
    {
        if (!(Configuration.get().Mqtt.Enabled && MqttSettings.getConnected())) {
            return;
        }
       
        JsonDocument doc;
        doc["relay1_enabled"] = _isRelay1Enabled;
        doc["relay2_enabled"] = _isRelay2Enabled;
        
        char buffer[256];
        serializeJson(doc, buffer);

        MqttSettings.publish("relay/status", buffer);
    }

    void Controller::onMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len) 
    {
        String p = "";
        for (size_t i = 0; i < len; i++) {
            p += (char)payload[i];
        }
        String t = String(topic);

        if (t.endsWith("/relay1_enabled")) {
            _isRelay1Enabled = (p == "1" || p.equalsIgnoreCase("true") || p.equalsIgnoreCase("ON"));
        } else if (t.endsWith("/relay2_enabled")) {
            _isRelay2Enabled = (p == "1" || p.equalsIgnoreCase("true") || p.equalsIgnoreCase("ON"));
        }
        {
            auto guard = Configuration.getWriteGuard();
            auto& config = guard.getConfig();

            config.Relay.Relay1Enabled = _isRelay1Enabled;
            config.Relay.Relay2Enabled = _isRelay2Enabled;
            
            digitalWrite(_relay1Pin, _isRelay1Enabled ? HIGH : LOW);
            digitalWrite(_relay2Pin, _isRelay2Enabled ? HIGH : LOW);
        }
        Configuration.write();
    }

} // namespace Relays