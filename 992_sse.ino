void startSSE()
{
    server.send(200, "text/plain", "ok");
    handleSensors(true);
    handleActors(true);
    inductionSSE(true);
    miscSSE();
}

void inductionSSE(bool val)
{
    // val true: init
    // val false: only updates

    JsonDocument doc;
    doc["enabled"] = (int)inductionCooker.getIsEnabled();
    doc["power"] = 0;
    if (inductionCooker.getIsEnabled())
    {
        doc["relayOn"] = inductionCooker.getisRelayon();
        doc["power"] = inductionCooker.getPower();
        doc["state"] = inductionCooker.getInductionState();
        if (inductionCooker.getisPower())
        {
            doc["powerLevel"] = inductionCooker.getCMD_CUR();
        }
        else
        {
            doc["powerLevel"] = max(0, inductionCooker.getCMD_CUR() - 1);
        }
    }
    else
        val = false;

    doc["topic"] = inductionCooker.getTopic();
    doc["pl"] = inductionCooker.getPowerLevelOnError();

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
        String response;
        serializeJson(doc, response);
        if (measureJson(doc) > 2 && inductionCooker.getIsEnabled())
            SSEBroadcastJson(response.c_str(), 2);
    }
}

void miscSSE()
{
    JsonDocument doc;
    doc["host"] = mqtthost;
    doc["port"] = mqttport;
    doc["s_mqtt"] = mqtt_state;
    doc["display"] = useDisplay;
    doc["lang"] = selLang;
    if (startMDNS)
        doc["mdns"] = nameMDNS;
    else
        doc["mdns"] = 0;
    String response;

    serializeJson(doc, response);
    if (measureJson(doc) > 2)
        SSEBroadcastJson(response.c_str(), 3); // only broadcast if state is different
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
    // log_e("Unregistered client with IP %s tries to listen", server.client().remoteIP().toString().c_str());
    return handleNotFound();
  }
  client.setNoDelay(true);
  s.client = client; // capture SSE server client connection

  server.setContentLength(CONTENT_LENGTH_UNKNOWN); // the payload can go on forever
  server.sendContent_P(PSTR("HTTP/1.1 200 OK\nContent-Type: text/event-stream\nConnection: keep-alive\nCache-Control: no-cache\nAccess-Control-Allow-Origin: *\n\n"));
  s.keepAliveTimer.attach(15.0, SSEKeepAlive); // Refresh time every 30s - WebUpdate benötigt bei langsamer Leitung über 60s
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
            String response = "";
            if (typ == 0)
                response += "event: sensors\n";
            else if (typ == 1)
                response += "event: actors\n";
            else if (typ == 2)
                response += "event: ids\n";
            else if (typ == 3)
                response += "event: misc\n";
            else if (typ == 4)
                response += "event: alive\n"; // retry: 30000\n";
            else
                return;

            response += "data: ";
            response += jsonValue;
            response += "\n";
            client.println(response);
        }
        // else
        //   log_e("SSEBroadcastState - client %s registered on channel %d but not listening", IPaddrstr.c_str(), i);
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
    server.send(404, "text/plain", message);
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
    String checkSSE = F("-1");
    for (uint8_t i = 0; i < SSE_MAX_CHANNELS; i++)
    {
        if (clientIP == subscription[i].clientIP)
        {
            checkSSE = F("1");
            continue;
        }
    }
    server.send_P(200, "text/plain", checkSSE.c_str());
}

void initialSSE(uint8_t val)
{
    String alive = "event: alive\ndata: { \"type\":\"new SSE\", \"ip\":\"" + subscription[val].clientIP.toString() + "\", \"channel\":\"" + val + "\"}\n";
    subscription[val].client.println(alive);
}
