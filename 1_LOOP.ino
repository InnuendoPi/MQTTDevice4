void loop()
{
  server.handleClient(); // Webserver handle
  if (WiFi.status() == WL_CONNECTED)
  {
    TickerPUBSUB.update(); // Ticker PubSubClient
#ifdef ESP8266
    if (startMDNS) // mDNS handle
      mdns.update();
#endif
  }

  TickerTime.update();

  if ((numberOfSensors > 0) && (millis() - lastRequestSensors >= timeoutSensors)) // Ticker Sensoren
  {
    // if (DS18B20.isConversionComplete())
      TickerSen.update();
  }

  if (numberOfActors > 0) // Ticker Aktoren
    TickerAct.update();
  if (inductionStatus > 0) // Ticker Induktion
    TickerInd.update();
  if (useDisplay) // Ticker Display
  {
    TickerDisp.update();
    nextion.checkNex();
  }
}
