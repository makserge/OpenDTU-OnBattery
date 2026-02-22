#include "ResetReason.h"
#include <Arduino.h>
#include "esp_system.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    #include "esp_rom_sys.h"
#else
    #include <rom/rtc.h>
#endif

String ResetReason::get_reset_reason_verbose(const uint8_t cpu_id) {
    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        esp_reset_reason_t reason = esp_reset_reason();
        switch (reason) {
            case ESP_RST_POWERON:   return "Vbat power on reset";
            case ESP_RST_EXT:       return "Reset by external pin";
            case ESP_RST_SW:        return "Software reset digital core";
            case ESP_RST_PANIC:     return "Software reset CPU";
            case ESP_RST_INT_WDT:   return "Interrupt Watchdog reset digital core";
            case ESP_RST_TASK_WDT:  return "Task Watchdog reset digital core";
            case ESP_RST_WDT:       return "Other Watchdog reset digital core";
            case ESP_RST_DEEPSLEEP: return "Deep Sleep reset digital core";
            case ESP_RST_BROWNOUT:  return "Brownout reset";
            case ESP_RST_SDIO:      return "Reset by SLC module";
            default:                return "Unknown reset reason";
        }
    #else
        int reason = rtc_get_reset_reason(cpu_id);
        switch (reason) {
            case 1:  return "Vbat power on reset";
            case 3:  return "Software reset digital core";
            case 4:  return "Legacy Watchdog reset digital core";
            case 5:  return "Deep Sleep reset digital core";
            case 7:  return "Timer Group0 Watchdog reset digital core";
            case 8:  return "Timer Group1 Watchdog reset digital core";
            case 9:  return "RTC Watchdog reset digital core";
            case 12: return "Software reset CPU";
            case 15: return "Brownout reset";
            default: return "Unknown (" + String(reason) + ")";
        }
    #endif
}

String ResetReason::get_reset_reason_short(const uint8_t cpu_id) {
    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        esp_reset_reason_t reason = esp_reset_reason();
        switch (reason) {
            case ESP_RST_POWERON:   return "POWERON";
            case ESP_RST_SW:        return "SW";
            case ESP_RST_PANIC:     return "PANIC";
            case ESP_RST_DEEPSLEEP: return "DSLEEP";
            case ESP_RST_BROWNOUT:  return "BROWNOUT";
            case ESP_RST_TASK_WDT:
            case ESP_RST_INT_WDT:
            case ESP_RST_WDT:       return "WDT";
            default:                return "EXT/UNK";
        }
    #else
        int reason = rtc_get_reset_reason(cpu_id);
        switch (reason) {
            case 1:  return "POWERON";
            case 3:  return "SW";
            case 5:  return "DSLEEP";
            case 7:
            case 8:
            case 9:  return "WDT";
            case 15: return "BROWNOUT";
            default: return "EXT/UNK";
        }
    #endif
}
