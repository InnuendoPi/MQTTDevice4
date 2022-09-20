void loop()
{
  server.handleClient();      // Webserver handle

  if (WiFi.status() == WL_CONNECTED)
  {
    if (!mqttoff)
      TickerPUBSUB.update();  // Ticker PubSubClient

    if (startMDNS)            // MDNS handle
      mdns.update();
    TickerNTP.update();       // Ticker NTP
  }
  else
  {
    cbpiEventSystem(EM_WLAN); // Check WLAN
  }

  if (numberOfSensors > 0)    // Ticker Sensoren
    TickerSen.update();
  if (numberOfActors > 0)     // Aktoren
    cbpiEventActors(actorsStatus);
  if (inductionStatus > 0)    // Ticker Induktion
    TickerInd.update();
  if (useDisplay)             // Ticker Display
  {
    nextion.update();
    TickerDisp.update();
  }
  if (hltStatus)              // Ticker HLT
    TickerHlt.update();

  if (hltAutoTune && TickerHltPID.state() == RUNNING) // AutoTune HLT
  {
    TickerHltPID.update();
    runHltAutoTune();
  }
  else if (ids2AutoTune && TickerPID.state() == RUNNING) // AutoTune IDS2
  {
    TickerPID.update();
    runAutoTune();
  }

  if (hltStatus > 0 && TickerHltPID.state() == RUNNING) // Ticker hltPID
  {
    TickerHltPID.update();
  }

  if (pidMode)                                          // Ticker ids2PID Ticker mash
  {
    TickerPID.update();
    TickerMash.update();
  }

  gEM.processAllEvents();     // All the rest event queue
}
