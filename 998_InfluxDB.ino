/*
class DBServer
{
public:
    String kettle_id = "";            // Kettle ID
    String kettle_topic = "";         // Kettle Topic
    String kettle_heater_topic = "";  // Kettle Heater MQTT Topic
    float kettle_sensor_temp = 0.0;   // Kettle Sensor aktuelle Temperatur
    int kettle_target_temp = 0;       // Kettle Heater TargetTemp
    int kettle_heater_powerlevel = 0; // Kettle Heater aktueller Powerlevel
    int kettle_heater_state = 0;      // Kettle Heater Status (on/off)
    int dbEnabled = -1;               // Topic existiert nicht

    DBServer(String new_kettle_id, String new_kettle_topic)
    {
        kettle_id = new_kettle_id;
        kettle_topic = new_kettle_topic;
    }

    void mqtt_subscribe()
    {
        if (pubsubClient.connected())
        {
            char subscribemsg[50];
            kettle_topic.toCharArray(subscribemsg, 50);
            pubsubClient.subscribe(subscribemsg);
            if (!pubsubClient.subscribe(subscribemsg))
            {
                DEBUG_MSG("%s\n", "InfluxMQTT Fehler");
            }
            else
            {
                DEBUG_MSG("InfluxMQTT: Subscribing to %s\n", subscribemsg);
            }
        }
    }
    void mqtt_unsubscribe()
    {
        if (pubsubClient.connected())
        {
            char subscribemsg[50];
            kettle_topic.toCharArray(subscribemsg, 50);
            DEBUG_MSG("InfluxMQTT: Unsubscribing from %s\n", subscribemsg);
            pubsubClient.unsubscribe(subscribemsg);
        }
    }

    void handlemqtt(char *payload)
    {
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, (const char *)payload);
        if (error)
        {
            DEBUG_MSG("TCP: handlemqtt deserialize Json error %s\n", error.c_str());
            return;
        }
        kettle_id = doc["id"].as<String>();
        if (isValidInt(doc["tt"].as<String>()))
            kettle_target_temp = doc["tt"];
        else
            kettle_target_temp = 0;
        if (isValidFloat(doc["te"].as<String>()))
            kettle_sensor_temp = doc["te"];
        else
            kettle_sensor_temp = 0.0;
        kettle_heater_topic = doc["he"].as<String>();
        DEBUG_MSG("Influx handleMQTT dbEn: %d ID: %s State: %d Target: %d Temp: %f Power: %d Topic: %s\n", dbEnabled, kettle_id.c_str(), kettle_heater_state, kettle_target_temp, kettle_sensor_temp, kettle_heater_powerlevel, kettle_heater_topic.c_str());
        //if (dbEnabled == -1)
        if (dbEnabled != 0)
        {
            if (kettle_heater_topic == inductionCooker.mqtttopic)
            {
                if (inductionCooker.setGrafana)
                {
                    dbEnabled = 1;
                    kettle_heater_state = inductionCooker.isInduon;
                    kettle_heater_powerlevel = inductionCooker.power;
                    return;
                }
            }
            for (int j = 0; j < numberOfActors; j++) // Array Aktoren
            {
                if (kettle_heater_topic == actors[j].argument_actor)
                {
                    if (actors[j].setGrafana)
                    {
                        dbEnabled = 1;
                        kettle_heater_state = actors[j].actor_state;
                        kettle_heater_powerlevel = actors[j].power_actor;
                        return;
                    }
                }
            }
            if (dbEnabled == -1)
            {
                dbEnabled = 0;
                mqtt_unsubscribe();
            }
        }
    }
};

// Erstelle Array mit Kettle ID CBPi
DBServer dbInflux[numberOfDBMax] = {
    DBServer("1", "MQTTDevice/kettle/1"),
    DBServer("2", "MQTTDevice/kettle/2"),
    DBServer("3", "MQTTDevice/kettle/3")};

void sendData()
{
    for (int i = 0; i < numberOfDBMax; i++)
    {
        if (dbInflux[i].dbEnabled != 1)
            continue;

        Point dbData("mqttdevice_status");
        dbData.addTag("ID", dbInflux[i].kettle_id);
        if (dbVisTag[0] != '\0')
            dbData.addTag("Sud-ID", dbVisTag);
        dbData.addField("Temperatur", dbInflux[i].kettle_sensor_temp);
        dbData.addField("TargetTemp", dbInflux[i].kettle_target_temp);
        if (dbInflux[i].kettle_heater_state == 1)
        {
            // Test: Grafana powerlevel "no value"
            if (dbInflux[i].kettle_heater_powerlevel >= 0)
                dbData.addField("Powerlevel", dbInflux[i].kettle_heater_powerlevel);
            else
                dbData.addField("Powerlevel", 0);
        }
        else
            dbData.addField("Powerlevel", 0);
        DEBUG_MSG("Sende an InfluxDB: %s\n", dbData.toLineProtocol().c_str());

        if (!dbClient.writePoint(dbData))
        {
            DEBUG_MSG("InfluxDB Schreibfehler: %s\n", dbClient.getLastErrorMessage().c_str());
            sendAlarm(ALARM_ERROR2);
        }
    }
}

void setInfluxDB()
{
    // Setze Parameter
    dbClient.setConnectionParamsV1(dbServer, dbDatabase, dbUser, dbPass);
}

bool checkDBConnect()
{
    if (dbClient.validateConnection())
    {
        DEBUG_MSG("Verbunden mit InfluxDB: %s\n", dbClient.getServerUrl().c_str());
        return true;
    }
    else
    {
        DEBUG_MSG("Verbindung zu InfluxDB Datenbank fehlgeschlagen: %s\n", dbClient.getLastErrorMessage().c_str());
        sendAlarm(ALARM_ERROR2);
        return false;
    }
}
*/