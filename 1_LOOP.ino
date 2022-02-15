void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    server.handleClient();
    TickerNTP.update();       // NTP Ticker
    cbpiEventSystem(EM_MQTT); // Check MQTT
    if (startMDNS)            // MDNS handle
      mdns.update();
  }
  else
  {
    cbpiEventSystem(EM_WLAN); // Check WLAN
  }
  
  if (numberOfSensors > 0)    // Sensoren
    TickerSen.update();
  if (numberOfActors > 0)     // Aktoren
    cbpiEventActors(actorsStatus);
  if (inductionStatus > 0)    // Induktion
    cbpiEventInduction(inductionStatus);
  if (useDisplay)             // Display
  {
    TickerDisp.update();
  }
  gEM.processAllEvents();     // event queue
}
