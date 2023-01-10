void loop()
{
  server.handleClient();              // Webserver handle
  if (WiFi.status() == WL_CONNECTED)
  {
    TickerPUBSUB.update();          // Ticker PubSubClient
    if (startMDNS)                    // mDNS handle
    {
      mdns.update();
    }
    TickerNTP.update();               // Ticker NTP
  }
  else
  {
    EM_WLAN();                        // Event handing WLAN
  }

  if (numberOfSensors > 0)            // Ticker Sensoren
    TickerSen.update();
  if (numberOfActors > 0)             // Ticker Aktoren
    TickerAct.update();
  
  if (inductionStatus > 0)            // Ticker Induktion
      TickerInd.update();
  if (useDisplay)                     // Ticker Display
    TickerDisp.update();
}
