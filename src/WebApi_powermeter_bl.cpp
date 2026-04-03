#include <WebApi_powermeter_bl.h>
#include <powermeter_bl/Controller.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <WebApi.h>

void WebApiPowerMeterBlClass::init(AsyncWebServer& server, Scheduler& scheduler) 
{
    server.on("/api/powermeter_bl/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->onStatus(request);
    });

    server.on("/api/powermeter_bl/config", HTTP_POST, [this](AsyncWebServerRequest *request){ 
        this->onAdminPost(request); 
    });
}

void WebApiPowerMeterBlClass::onStatus(AsyncWebServerRequest *request) 
{
    if (!WebApi.checkCredentialsReadonly(request)) return;

    AsyncJsonResponse *response = new AsyncJsonResponse();
    auto& root = response->getRoot();
    const CONFIG_T& config = Configuration.get();

    root["enabled"] = config.PowermeterBl.Enabled;
    root["power"] = PowermeterBl.getPower();
    root["apparent_power"] = PowermeterBl.getApparentPower();
    root["voltage"] = PowermeterBl.getVoltage();
    root["current"] = PowermeterBl.getCurrent();

    response->setLength();
    request->send(response);
}

void WebApiPowerMeterBlClass::onAdminPost(AsyncWebServerRequest *request) {
    if (!WebApi.checkCredentials(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonDocument root;
    if (!WebApi.parseRequestData(request, response, root)) {
        return;
    }
    auto& retMsg = response->getRoot();

    if (!(root["enabled"].is<bool>())) {
        retMsg["message"] = "Values are missing!";
        retMsg["code"] = WebApiError::GenericValueMissing;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }
    {
        auto guard = Configuration.getWriteGuard();
        auto& config = guard.getConfig();

        config.PowermeterBl.Enabled = root["enabled"].as<bool>();
    }

    WebApi.writeConfig(retMsg);

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);

    PowermeterBl.updateSettings();
}