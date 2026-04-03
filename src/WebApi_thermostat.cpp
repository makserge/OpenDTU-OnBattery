#include <WebApi_thermostat.h>
#include <thermostat/Controller.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <Logging.h>
#include <WebApi.h>

void WebApiThermostatClass::init(AsyncWebServer& server, Scheduler& scheduler) 
{
    server.on("/api/thermostat/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->onStatus(request);
    });

    server.on("/api/thermostat/config", HTTP_POST, [this](AsyncWebServerRequest *request){ 
        this->onAdminPost(request); 
    });
}

void WebApiThermostatClass::onStatus(AsyncWebServerRequest *request) 
{
    if (!WebApi.checkCredentialsReadonly(request)) return;

    AsyncJsonResponse *response = new AsyncJsonResponse();
    auto& root = response->getRoot();
    const CONFIG_T& config = Configuration.get();

    root["enabled"]    = config.Thermostat.Enabled;
    root["temp"]       = Thermostat.getTemperature();
    root["hum"]        = Thermostat.getHumidity();
    root["low_temp"]   = config.Thermostat.LowTemp;
    root["high_temp"]  = config.Thermostat.HighTemp;
    root["heating"]    = Thermostat.isHeatingActive();
    root["cooling"]    = Thermostat.isCoolingActive();

    response->setLength();
    request->send(response);
}

void WebApiThermostatClass::onAdminPost(AsyncWebServerRequest *request) {
    if (!WebApi.checkCredentials(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonDocument root;
    if (!WebApi.parseRequestData(request, response, root)) {
        return;
    }
    auto& retMsg = response->getRoot();

    if (!(root["enabled"].is<bool>()
            && root["low_temp"].is<float>()
            && root["high_temp"].is<float>())) {
        retMsg["message"] = "Values are missing!";
        retMsg["code"] = WebApiError::GenericValueMissing;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    if (root["low_temp"].as<float>() >= root["high_temp"].as<float>()) {
        retMsg["message"] = "Low temperature must be less than High temperature!";
        retMsg["code"] = WebApiError::GenericValueInvalid;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }
    {
        auto guard = Configuration.getWriteGuard();
        auto& config = guard.getConfig();

        config.Thermostat.Enabled = root["enabled"].as<bool>();
        config.Thermostat.LowTemp = root["low_temp"].as<float>();
        config.Thermostat.HighTemp = root["high_temp"].as<float>();
    }
    WebApi.writeConfig(retMsg);

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);

    Thermostat.updateSettings();
}
