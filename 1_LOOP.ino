void loop()
{
  server.handleClient();              // Webserver handle
  if (WiFi.status() == WL_CONNECTED)
  {
    TickerPUBSUB.update();            // Ticker PubSubClient
    TickerNTP.update();               // Ticker NTP
    if (startMDNS)                    // mDNS handle
      mdns.update();
  }
  else
    EM_WLAN();                        // Event handing WLAN

  if (numberOfSensors > 0)            // Ticker Sensoren
    TickerSen.update();
  if (numberOfActors > 0)             // Ticker Aktoren
    TickerAct.update();
  if (inductionStatus > 0)            // Ticker Induktion
      TickerInd.update();
  if (useDisplay) // Ticker Display
  {
    TickerDisp.update();
    nextion.update();
  }
}
