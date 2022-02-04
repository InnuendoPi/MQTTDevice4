void loop()
{
  cbpiEventSystem(EM_WLAN); // Check WLAN
  cbpiEventSystem(EM_WEB);  // Webserver handle
  cbpiEventSystem(EM_MQTT); // Check MQTT
  if (startMDNS)            // MDNS handle
    cbpiEventSystem(EM_MDNS);

  if (numberOfSensors > 0) // Sensoren
    TickerSen.update();
  if (numberOfActors > 0) // Aktoren
    cbpiEventActors(actorsStatus);
  if (inductionStatus > 0) // Induktion
    cbpiEventInduction(inductionStatus);
  if (useDisplay) // Display
  {
    // nextion.update();
    TickerDisp.update();
  }

  TickerNTP.update();     // NTP Ticker
  gEM.processAllEvents(); // event queue
}
