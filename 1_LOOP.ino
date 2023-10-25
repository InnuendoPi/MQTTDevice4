void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    server.handleClient();              // Webserver handle
    EM_MQTTUPDATE();                  // MQTT handle
    if (timeClient.update())
      Serial.printf("*** SYSINFO: %s\n", timeClient.getFormattedTime().c_str());
    if (startMDNS)                    // mDNS handle
      mdns.update();
  }

  if (numberOfSensors > 0)            // Ticker Sensoren
    TickerSen.update();
  if (numberOfActors > 0)             // Ticker Aktoren
    TickerAct.update();
  if (inductionStatus > 0)            // Ticker Induktion
      TickerInd.update();
  if (useDisplay)                     // Ticker Display
  {
    TickerDisp.update();
    nextion.update();
  }
}
