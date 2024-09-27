#ifndef INNULOG_H
#define INNULOG_H

#ifdef ESP32
#include "esp32-hal-log.h"
#endif

#include <Arduino.h>

#define DEBUG_ESP_PORT Serial
#define LOG_COLOR_ERROR "\033[0;31m"
#define LOG_COLOR_INFO "\033[0;32m"
#define LOG_COLOR_WARN "\033[0;33m"
#define LOG_COLOR_VERBOSE "\033[0;36m"
#define LOG_COLOR_RESET "\033[0m"

#define INNU_NONE 0
#define INNU_ERROR 1
#define INNU_INFO 2
#define INNU_VERBOSE 3

#define LOG_CFG "/log_cfg.json"

#define LOGS_COUNT 6

struct InnuLogTag
{
    String tagName;
    int level;
};

#ifdef ESP8266 // required for logging on ESP8266
const char* IRAM_ATTR pathToFileName (const char* path) {
    size_t i = 0;
    size_t pos = 0;
    char* p = (char*)path;
    while (*p) {
        i++;
        if (*p == '/' || *p == '\\') {
            pos = i;
        }
        p++;
    }
    return path + pos;
}
#endif

struct InnuLogTag InnuTagLevel[LOGS_COUNT]{
    {"CFG", INNU_NONE},
    {"SEN", INNU_NONE},
    {"ACT", INNU_NONE},
    {"IND", INNU_NONE},
    {"SYS", INNU_INFO},
    {"DIS", INNU_NONE}};

#define DEBUG_ERROR(TAG, ...)                                                                                               \
    if (getTagLevel(TAG) >= INNU_ERROR)                                                                                     \
    {                                                                                                                       \
        DEBUG_ESP_PORT.printf(LOG_COLOR_ERROR);                                                                             \
        DEBUG_ESP_PORT.printf(PSTR("[%6lu][E][%s:%d] %s(): "), millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__); \
        DEBUG_ESP_PORT.printf(__VA_ARGS__);                                                                                 \
        DEBUG_ESP_PORT.println(LOG_COLOR_RESET);                                                                            \
    }

#define DEBUG_INFO(TAG, ...)                                                                                                \
    if (getTagLevel(TAG) >= INNU_INFO)                                                                                      \
    {                                                                                                                       \
        DEBUG_ESP_PORT.printf(LOG_COLOR_INFO);                                                                              \
        DEBUG_ESP_PORT.printf(PSTR("[%6lu][I][%s:%d] %s(): "), millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__); \
        DEBUG_ESP_PORT.printf(__VA_ARGS__);                                                                                 \
        DEBUG_ESP_PORT.println(LOG_COLOR_RESET);                                                                            \
    }

#define DEBUG_VERBOSE(TAG, ...)                                                                                             \
    if (getTagLevel(TAG) >= INNU_VERBOSE)                                                                                   \
    {                                                                                                                       \
        DEBUG_ESP_PORT.printf(LOG_COLOR_VERBOSE);                                                                           \
        DEBUG_ESP_PORT.printf(PSTR("[%6lu][V][%s:%d] %s(): "), millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__); \
        DEBUG_ESP_PORT.printf(__VA_ARGS__);                                                                                 \
        DEBUG_ESP_PORT.println(LOG_COLOR_RESET);                                                                            \
    }

#endif

int getTagLevel(const String &tagName)
{
  for (int i = 0; i < LOGS_COUNT; i++)
  {
    if (InnuTagLevel[i].tagName == tagName)
    {
      return InnuTagLevel[i].level;
    }
  }
  return INNU_NONE;
}

void setTagLevel(const String &tagName, int level)
{
  for (int i = 0; i < LOGS_COUNT; i++)
    if (InnuTagLevel[i].tagName == tagName)
    {
      InnuTagLevel[i].level = level;
      return;
    }
}

void saveLog()
{
  JsonDocument doc;
  doc["CFG"] = getTagLevel("CFG");
  doc["SEN"] = getTagLevel("SEN");
  doc["ACT"] = getTagLevel("ACT");
  doc["IND"] = getTagLevel("IND");
  doc["SYS"] = getTagLevel("SYS");
  doc["DIS"] = getTagLevel("DIS");
  File logFile = LittleFS.open(LOG_CFG, "w");
  if (!logFile)
  {
    DEBUG_ERROR("CFG", "error could not save log_cfg.json - permission denied");
    return;
  }
  serializeJson(doc, logFile);
  logFile.close();
  DEBUG_INFO("CFG", "CFG: %d sen: %d act: %d ind: %d sys: %d dis: %d", getTagLevel("CFG"), getTagLevel("SEN"), getTagLevel("ACT"), getTagLevel("IND"), getTagLevel("SYS"), getTagLevel("DIS"));
}

void readLog()
{
  if (!LittleFS.exists(LOG_CFG))
  {
    saveLog();
  }
  else
  {
    File logFile = LittleFS.open(LOG_CFG, "r");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, logFile);
    if (error)
    {
      DEBUG_ERROR("SYS", "error could not read log_cfg: %s - JSON error %s", LOG_CFG, error.c_str());
      return;
    }
    logFile.close();
    setTagLevel("CFG", doc["CFG"]);
    setTagLevel("SEN", doc["SEN"]);
    setTagLevel("ACT", doc["ACT"]);
    setTagLevel("IND", doc["IND"]);
    setTagLevel("SYS", doc["SYS"]);
    setTagLevel("DIS", doc["DIS"]);
    DEBUG_INFO("CFG", "read logging CFG: %d sen: %d act: %d ind: %d sys: %d dis: %d", getTagLevel("CFG"), getTagLevel("SEN"), getTagLevel("ACT"), getTagLevel("IND"), getTagLevel("SYS"), getTagLevel("DIS"));
  }
}
