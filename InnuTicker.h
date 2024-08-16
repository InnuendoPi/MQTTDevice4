#ifndef INNUTICKER
#define INNUTICKER

//   Author:	Innuendo
//   Version: 0.0.4

#include "Arduino.h"
// #define TICKER_LIBRARY_VERSION 0.0.4
 
// Ticker status
//
// @param STOPPED Standard, der Ticker ist gestoppt
// @param RUNNING Der Ticker läuft
// @param PAUSED Dder Ticker pausiert
//
enum status_t {STOPPED, RUNNING, PAUSED};

typedef void (*fptr)();

class InnuTicker {

public:

	// Erstelle ein Ticker Object
	//
	// @param callback Name der Callback Funktion
	// @param timer Länge Zeitintervall in ms
	// @param repeat Standard ist 0 -> endlos, repeat > 0 -> Anzahl an Wiederholungen
	//
	//
	InnuTicker(); // leerer Konstruktor - lese Konfiguration aus config.txt und setze Parameter anschließend
	InnuTicker(fptr callback, uint32_t timer, uint32_t repeat = 0);

	// Destructor für das Ticker Object
	~InnuTicker();

	// Starte Ticker
	void start();

	// Resume Ticker. Wenn der Ticker nicht gestartet ist, wird der Ticker automatisch gestartet
	void resume();

	// Pausiere den Ticker
	void pause();

	// Stoppe den Ticker
	void stop();

	// Die Funktion Update muss in der loop aufgerufen werden. Update prüft den Ticker und ruft wenn erforderlich die Callback Funktion auf
	void update();
	bool updateBack();
	void updatenow();

	// Setze Timer und Wiederholung neu
	void config(uint32_t timer, uint32_t repeat = 0);

	// Setze Callback-Funktion, Timer und Wiederholung neu
	void config(fptr callback, uint32_t timer, uint32_t repeat = 0);

	// Ändere das Zeitintervall
	//
	// @param timer Länge Zeitintervall in ms 
	//
	void interval(uint32_t timer);

	// Setze lasttime
	//
	// @rparam lastime in millis
	//

	void setLastTime(uint32_t timer);

	// Aktuell verstrichene Zeit
	//
	// @returns gibt die Zeit seit dem letzten Tick zurück
	//
	uint32_t elapsed();

	// verbleibende Zeit bis zum nächsten tick
	//
	// @returns gibt die Zeit bis zum nächsten Tick zurück 
	//
	//
	uint32_t remaining();

	// Ticker Status
	//
	// @returns gibt den aktuellen Status vom Ticker zurück: STOPPED, RUNNING oder PAUSED
	//
	status_t state();

	// Anzahl an ausgeführten Wiederholungen
	//
	// @returns Gibt die Anzahl an ausgeführten Wiederholungen zurück
	//
	uint32_t counter();

private:
	bool tick();
	bool enabled;
	uint32_t timer;
	uint32_t repeat;
	uint32_t counts;
	status_t status;
	fptr callback;
	uint32_t lastTime;
	uint32_t diffTime;
};
#endif
