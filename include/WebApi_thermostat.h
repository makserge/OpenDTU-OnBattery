#pragma once

#include <ESPAsyncWebServer.h>
#include <TaskSchedulerDeclarations.h>

class WebApiThermostatClass {
public:
    void init(AsyncWebServer& server, Scheduler& scheduler);

private:
    void onStatus(AsyncWebServerRequest* request);
    void onAdminPost(AsyncWebServerRequest* request);
};