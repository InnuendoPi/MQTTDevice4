void startSSE()
{
    replyOK();
    handleSensors(true);
    TickerSen.setLastTime(millis());
    handleActors(true);
    TickerAct.setLastTime(millis());
    inductionSSE(true);
    TickerInd.setLastTime(millis());
    miscSSE();
}

void inductionSSE(bool val)
{
    // val true: init
    // val false: only updates

    JsonDocument ssedoc;
    ssedoc["enabled"] = (int)inductionCooker.getIsEnabled();
    ssedoc["power"] = 0;
    if (inductionCooker.getIsEnabled())
    {
        ssedoc["relayOn"] = inductionCooker.getisRelayon();
        ssedoc["power"] = inductionCooker.getPower();
        ssedoc["state"] = inductionCooker.getInductionState();
        if (inductionCooker.getisPower())
        {
            ssedoc["powerLevel"] = inductionCooker.getCMD_CUR();
        }
        else
        {
            ssedoc["powerLevel"] = max(0, inductionCooker.getCMD_CUR() - 1);
        }
    }
    else
        val = false;

    ssedoc["topic"] = inductionCooker.getTopic();
    ssedoc["pl"] = inductionCooker.getPowerLevelOnError();

    if (!val)
    {
        if (inductionCooker.getOldPower() != inductionCooker.getPower())
        {
            inductionCooker.setOldPower();
            val = true;
        }
        else if (inductionCooker.getoldisRelayon() != inductionCooker.getisRelayon())
        {
            inductionCooker.setoldisRelayon();
            val = true;
        }
        else if (inductionCooker.getoldisInduon() != inductionCooker.getisInduon())
        {
            inductionCooker.setoldisInduon();
            val = true;
        }
    }
    if (val)
    {
        char response[measureJson(ssedoc) + 2];
        serializeJson(ssedoc, response, sizeof(response));
        if (measureJson(ssedoc) > 2 && inductionCooker.getIsEnabled())
            SSEBroadcastJson(response, 2);
    }
}

void miscSSE()
{
    JsonDocument ssedoc;
    ssedoc["host"] = mqtthost;
    ssedoc["port"] = mqttport;
    ssedoc["s_mqtt"] = mqtt_state;
    ssedoc["display"] = useDisplay;
    ssedoc["lang"] = selLang;
    if (startMDNS)
        ssedoc["mdns"] = nameMDNS;
    else
        ssedoc["mdns"] = 0;

    char response[measureJson(ssedoc) + 2];
    serializeJson(ssedoc, response, sizeof(response));
    SSEBroadcastJson(response, 3);
}

void handleChannel()
{
    uint8_t channel;
    for (channel = 0; channel < SSE_MAX_CHANNELS; channel++) // Find first free slot
    {
        if (!subscription[channel].clientIP)
        {
            break;
        }
    }
    subscription[channel].clientIP = server.client().remoteIP(); // get IP address of client
    subscription[channel].check = true;

    String SSEurl = F("http://");
    SSEurl += WiFi.localIP().toString();
    SSEurl += F(":");
    SSEurl += PORT;
    SSEurl += F("/rest/events/");
    SSEurl += channel;
    server.send_P(200, "text/plain", SSEurl.c_str());
}

void SSEKeepAlive()
{
    for (uint8_t i = 0; i < SSE_MAX_CHANNELS; i++)
    {
        if (!(subscription[i].clientIP))
        {
            continue;
        }
        if (subscription[i].client.connected())
        {
            // subscription[i].client.println("data: { \"TYPE\":\"KEEP-ALIVE\" }\n"); // Extra newline required by SSE standard
            String alive = "event: alive\ndata: { \"type\":\"keep alive\", \"ip\":\"" + IPtoString(subscription[i].clientIP) + "\", \"channel\":\"" + i + "\"}\n";
            subscription[i].client.println(alive);
        }
        else
        {
            subscription[i].keepAliveTimer.detach();
            subscription[i].client.flush();
            subscription[i].client.stop();
            subscription[i].clientIP = INADDR_NONE;
            subscriptionCount--;
        }
    }
}

void SSEHandler(uint8_t channel)
{
    IPAddress clientIP = server.client().remoteIP(); // get IP address of client

    if (subscription[channel].check == true)
    {
        if (clientIP == subscription[channel].clientIP)
        {
            subscription[channel].client = server.client();
            subscription[channel].keepAliveTimer = Ticker();
            subscription[channel].check = false;
            subscriptionCount++;
        }
    }

    WiFiClient client = server.client();
    SSESubscription &s = subscription[channel];

    if (s.clientIP != client.remoteIP())
    { // IP addresses don't match, reject this client
        return handleNotFound();
    }
    client.setNoDelay(true);
    s.client = client; // capture SSE server client connection

    server.setContentLength(CONTENT_LENGTH_UNKNOWN); // the payload can go on forever
    server.sendContent_P(PSTR("HTTP/1.1 200 OK\nContent-Type: text/event-stream\nConnection: keep-alive\nCache-Control: no-cache\nAccess-Control-Allow-Origin: *\n\n"));
    s.keepAliveTimer.attach(15.0, SSEKeepAlive); // Refresh time every 15s
    initialSSE(channel);
}

void SSEBroadcastJson(const char *jsonValue, uint8_t typ)
{
    for (uint8_t i = 0; i < SSE_MAX_CHANNELS; i++)
    {
        if (!(subscription[i].clientIP))
            continue;

        WiFiClient client = subscription[i].client;
        String IPaddrstr = IPAddress(subscription[i].clientIP).toString();
        if (client)
        {
            char response[strlen(jsonValue) + 60];
            if (typ == 0) // sensors
            {
                sprintf_P(response, PSTR("event: sensors\ndata: %s\nid: %lu\nretry: 5000\n\n"), jsonValue, millis());
            }
            else if (typ == 1) // actors
            {
                sprintf_P(response, PSTR("event: actors\ndata: %s\nid: %lu\nretry: 5000\n\n"), jsonValue, millis());
                
            }
            else if (typ == 2) // induction
            {
                sprintf_P(response, PSTR("event: ids\ndata: %s\nid: %lu\nretry: 5000\n\n"), jsonValue, millis());
            }
            else if (typ == 3) // misc System
            {
                sprintf_P(response, PSTR("event: misc\ndata: %s\nid: %lu\nretry: 5000\n\n"), jsonValue, millis());
            }
            else
            {
                DEBUG_ERROR("SYS", "unknown SSE broadcast type %d", typ);
                continue;
            }
            DEBUG_VERBOSE("SYS", "%s", response);
            client.print(response);
        }
    }
}

void handleNotFound()
{
    if (loadFromLittlefs(server.uri()))
    {
        return;
    }

    String message = "Handle Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    replyNotFound(message);
}

void handleAll()
{
    const char *uri = server.uri().c_str();
    const char *restEvents = PSTR("/rest/events/");
    if (strncmp_P(uri, restEvents, strlen_P(restEvents)))
    {
        return handleNotFound();
    }
    uri += strlen_P(restEvents); // Skip the "/rest/events/" and get to the channel number
    unsigned int channel = atoi(uri);
    if (channel < SSE_MAX_CHANNELS)
        SSEHandler(channel);
    else
        handleNotFound();
}

void checkAliveSSE()
{
  IPAddress clientIP = server.client().remoteIP(); // get IP address of client
  for (uint8_t i = 0; i < SSE_MAX_CHANNELS; i++)
  {
    if (clientIP == subscription[i].clientIP)
    {
      replyResponse("1");
      return;
    }
  }
  replyResponse("-1");
}

void initialSSE(uint8_t val)
{
    String alive = "event: alive\ndata: { \"type\":\"new SSE\", \"ip\":\"" + subscription[val].clientIP.toString() + "\", \"channel\":\"" + val + "\"}\n";
    subscription[val].client.println(alive);
}
