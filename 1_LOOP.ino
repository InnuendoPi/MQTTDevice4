void loop()
{
  server.handleClient(); // Webserver handle
  if (WiFi.status() == WL_CONNECTED)
  {
    TickerPUBSUB.update(); // Ticker PubSubClient
#ifdef ESP8266
    if (startMDNS) // ESP8266 mDNS handle
      mdns.update();
#endif
  }

  TickerTime.update();

  if (numberOfSensors > 0) // Ticker Sensoren
    TickerSen.update();
  if (numberOfActors > 0) // Ticker Aktoren
    TickerAct.update();
  if (inductionStatus) // Ticker Induktion
    TickerInd.update();
  if (useDisplay) // Ticker Display
  {
    TickerDisp.update();
    nextion.checkNex();
  }
}
