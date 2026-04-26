#pragma once

#include <TaskSchedulerDeclarations.h>
#include <SerialPortManager.h>
#include <BL0942.h>
#include <memory>
#include <mutex>

namespace PowermeterBls {

class Controller {
public:
    void init(Scheduler& scheduler);
    void updateSettings();

    float getApparentPower() const;
    float getPower() const;
    float getVoltage() const;
    float getCurrent() const;
    
private:
    void loop();
    void mqttLoop();

    static constexpr uint32_t BL0942_BAUD = 4800;
    static constexpr float CURRENT_CALIBRATION = 0.0701;
    static constexpr float POWER_CALIBRATION = 0.105;

    std::unique_ptr<HardwareSerial> _serial;
    std::unique_ptr<bl0942::BL0942> _sensor;
    Task _loopTask;
    Task _mqttLoopTask;
    mutable std::mutex _mutex;

    bool _isEnabled = false;
    float _power = 0;
    float _voltage = 0;
    float _current = 0;
    float _apparentPower = 0;
};

} // namespace PowermeterBls

extern PowermeterBls::Controller PowermeterBl;