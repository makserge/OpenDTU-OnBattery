#include <WebApi_thermostat.h>
#include <relay/Controller.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <Logging.h>
#include <WebApi.h>

void WebApiRelayClass::init(AsyncWebServer& server, Scheduler& scheduler) 
{
    server.on("/api/relay/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->onStatus(request);
    });

    server.on("/api/relay/config", HTTP_POST, [this](AsyncWebServerRequest *request){ 
        this->onAdminPost(request); 
    });
}

void WebApiRelayClass::onStatus(AsyncWebServerRequest *request) 
{
    if (!WebApi.checkCredentialsReadonly(request)) return;

    AsyncJsonResponse *response = new AsyncJsonResponse();
    auto& root = response->getRoot();
    const CONFIG_T& config = Configuration.get();

    root["relay1_enabled"] = config.Relay.Relay1Enabled;
    root["relay2_enabled"] = config.Relay.Relay2Enabled;
    
    response->setLength();
    request->send(response);
}

void WebApiRelayClass::onAdminPost(AsyncWebServerRequest *request) {
    if (!WebApi.checkCredentials(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonDocument root;
    if (!WebApi.parseRequestData(request, response, root)) {
        return;
    }
    auto& retMsg = response->getRoot();

    if (!(root["relay1_enabled"].is<bool>()
            && root["relay2_enabled"].is<bool>())) {
        retMsg["message"] = "Values are missing!";
        retMsg["code"] = WebApiError::GenericValueMissing;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    {
        auto guard = Configuration.getWriteGuard();
        auto& config = guard.getConfig();

        config.Relay.Relay1Enabled = root["relay1_enabled"].as<bool>();
        config.Relay.Relay2Enabled = root["relay2_enabled"].as<bool>();
    }
    WebApi.writeConfig(retMsg);

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);

    Relay.updateSettings();
}
