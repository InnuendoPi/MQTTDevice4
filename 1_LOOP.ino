void loop()
{
  server.handleClient();

  if (WiFi.status() == WL_CONNECTED)
  {
    if (!mqttoff)
      TickerPUBSUB.update(); // Check MQTT PubSubClient
    // cbpiEventSystem(EM_MQTT); // Check MQTT

    if (startMDNS) // MDNS handle
      mdns.update();
    TickerNTP.update(); // NTP Ticker
  }
  else
  {
    cbpiEventSystem(EM_WLAN); // Check WLAN
  }

  if (numberOfSensors > 0) // Sensoren
    TickerSen.update();
  if (numberOfActors > 0) // Aktoren
    cbpiEventActors(actorsStatus);
  if (inductionStatus > 0 ) // Induktion
    TickerInd.update();
  if (useDisplay) // Display
    TickerDisp.update();

  if (autoTune && TickerPID.state() == RUNNING)
  {
    TickerPID.update();
    runAutoTune();
  }
  if (pidMode)
  {
    TickerPID.update();
    TickerMash.update();
  }

  gEM.processAllEvents(); // event queue
}
