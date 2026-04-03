#pragma once

#include <TaskSchedulerDeclarations.h>
#include <espMqttClient.h>
#include <memory>
#include <mutex>

namespace Relays {

class Controller {
public:
    void init(Scheduler& scheduler);
    void updateSettings();

private:
    void loop();
    void mqttLoop();
    void onMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len);

    static constexpr uint32_t MQTT_UPDATE_INTERVAL_MS = 6000;

    Task _loopTask;
    mutable std::mutex _mutex;

    bool _isRelay1Enabled = false;
    bool _isRelay2Enabled = false;
    
    int8_t _relay1Pin;
    int8_t _relay2Pin;
};

} // namespace Relays

extern Relays::Controller Relay;