void loop()
{
  server.handleClient(); // Webserver handle

  if (WiFi.status() == WL_CONNECTED)
  {
    if (!mqttoff)
      TickerPUBSUB.update(); // Ticker PubSubClient

    if (startMDNS) // MDNS handle
      mdns.update();
    TickerNTP.update(); // Ticker NTP
  }
  else
  {
    EM_WLAN();
  }
  if (numberOfSensors > 0)            // Ticker Sensoren
    TickerSen.update();
  if (numberOfActors > 0)             // Ticker Aktoren
    TickerAct.update();
  
  if (TickerMash.state() == RUNNING)  // Ticker mash
    TickerMash.update();   
  if (TickerPID.state() == RUNNING)   // Ticker calc PIDs
    TickerPID.update();    
  if (inductionStatus > 0)            // Ticker Induktion
  {
    if (!ids2AutoTune)                // AutoTune IDS2
      TickerInd.update();
  }
  if (TickerHlt.state() == RUNNING)   // Ticker HLT
  {
    if (!hltAutoTune)                 // AutoTune HLT
      TickerHlt.update();
  }
  if (useDisplay)                     // Ticker Display
    TickerDisp.update();
}
