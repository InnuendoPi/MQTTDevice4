void tickerSenCallback() // Timer Objekt Sensoren
{
  cbpiEventSensors(sensorsStatus);
}
void tickerActCallback() // Timer Objekt Aktoren
{
  cbpiEventActors(actorsStatus);
}
void tickerIndCallback() // Timer Objekt Induktion
{
  cbpiEventInduction(inductionStatus);
}
void tickerDispCallback() // Timer Objekt Display
{
  cbpiEventSystem(EM_DISPUP);
}

void tickerMQTTCallback() // Ticker helper function calling Event MQTT Error
{
  if (TickerMQTT.counter() == 1)
  {
    switch (pubsubClient.state())
    {
    case -4: // MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
      DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECTION_TIMEOUT\n", pubsubClient.state());
      break;
    case -3: // MQTT_CONNECTION_LOST - the network connection was broken
      DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECTION_LOST\n", pubsubClient.state());
      break;
    case -2: // MQTT_CONNECT_FAILED - the network connection failed
      DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECT_FAILED\n", pubsubClient.state());
      break;
    case -1: // MQTT_DISCONNECTED - the client is disconnected cleanly
      DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_DISCONNECTED\n", pubsubClient.state());
      break;
    case 0: // MQTT_CONNECTED - the client is connected
      pubsubClient.loop();
      break;
    case 1: // MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
      DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECT_BAD_PROTOCOL\n", pubsubClient.state());
      break;
    case 2: // MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
      DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECT_BAD_CLIENT_ID\n", pubsubClient.state());
      break;
    case 3: // MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
      DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECT_UNAVAILABLE\n", pubsubClient.state());
      break;
    case 4: // MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
      DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECT_BAD_CREDENTIALS\n", pubsubClient.state());
      break;
    case 5: // MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
      DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECT_UNAUTHORIZED\n", pubsubClient.state());
      break;
    default:
      break;
    }
  }
  cbpiEventSystem(EM_MQTTER);
}

void tickerWLANCallback() // Ticker helper function calling Event WLAN Error
{
  DEBUG_MSG("%s", "tickerWLAN: callback\n");
  if (TickerWLAN.counter() == 1)
  {
    switch (WiFi.status())
    {
    case 0: // WL_IDLE_STATUS
      DEBUG_MSG("WiFi status: Fehler rc: %d WL_IDLE_STATUS");
      break;
    case 1: // WL_NO_SSID_AVAIL
      DEBUG_MSG("WiFi status: Fehler rc: %d WL_NO_SSID_AVAIL");
      break;
    case 2: // WL_SCAN_COMPLETED
      DEBUG_MSG("WiFi status: Fehler rc: %d WL_SCAN_COMPLETED");
      break;
    case 3: // WL_CONNECTED
      DEBUG_MSG("WiFi status: Fehler rc: %d WL_CONNECTED");
      break;
    case 4: // WL_CONNECT_FAILED
      DEBUG_MSG("WiFi status: Fehler rc: %d WL_CONNECT_FAILED");
      break;
    case 5: // WL_CONNECTION_LOST
      DEBUG_MSG("WiFi status: Fehler rc: %d WL_CONNECTION_LOST");
      break;
    case 6: // WL_DISCONNECTED
      DEBUG_MSG("WiFi status: Fehler rc: %d WL_DISCONNECTED");
      break;
    case 255: // WL_NO_SHIELD
      DEBUG_MSG("WiFi status: Fehler rc: %d WL_NO_SHIELD");
      break;
    default:
      break;
    }
  }
  cbpiEventSystem(EM_WLANER);
}

void tickerNTPCallback() // Ticker helper function calling Event WLAN Error
{
  timeClient.update();
  Serial.printf("*** SYSINFO: %s\n", timeClient.getFormattedTime().c_str());
}
