void loop()
{
  server.handleClient();    // Webserver handle
  cbpiEventSystem(EM_WLAN); // Überprüfe WLAN
  cbpiEventSystem(EM_MQTT); // Überprüfe MQTT
  if (startMDNS)            // MDNS handle
    cbpiEventSystem(EM_MDNS);
    
  gEM.processAllEvents(); // event queue

  if (numberOfSensors > 0) // Sensoren Ticker
    TickerSen.update();
  if (numberOfActors > 0) // Aktoren Ticker
    TickerAct.update();
  if (inductionStatus > 0) // Induktion Ticker
    TickerInd.update();
  if (useDisplay)
    TickerDisp.update();
    
  TickerNTP.update(); // NTP Ticker
}
