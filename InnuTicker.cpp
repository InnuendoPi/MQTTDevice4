#include "InnuTicker.h"

InnuTicker::InnuTicker() {} 	// Konstruktor

InnuTicker::InnuTicker(fptr callback, uint32_t timer, uint32_t repeat) // Konstruktor
{
	this->timer = timer;
	this->repeat = repeat;
	this->callback = callback;
	enabled = false;
	lastTime = 0;
	counts = 0;
}

InnuTicker::~InnuTicker() {}	// Destruktor

void InnuTicker::start()
{
	if (callback == NULL)
		return;
	lastTime = millis();
	enabled = true;
	counts = 0;
	status = RUNNING;
}

void InnuTicker::resume()
{
	if (callback == NULL)
		return;
	lastTime = millis() - diffTime;
	if (status == STOPPED)
		counts = 0;
	enabled = true;
	status = RUNNING;
}

void InnuTicker::stop()
{
	enabled = false;
	counts = 0;
	status = STOPPED;
}

void InnuTicker::pause()
{
	diffTime = millis() - lastTime;
	enabled = false;
	status = PAUSED;
}

void InnuTicker::update()
{
	if (tick())
		callback();
}

void InnuTicker::updatenow()
{
	callback();
}

void InnuTicker::config(uint32_t newTimer, uint32_t newRepeat)
{
	this->timer = newTimer;
	this->repeat = newRepeat;
	lastTime = 0;
	counts = 0;
}

void InnuTicker::config(fptr callback, uint32_t timer, uint32_t repeat)
{
	this->timer = timer;
	this->repeat = repeat;
	this->callback = callback;
	enabled = false;
	lastTime = 0;
	counts = 0;
}

bool InnuTicker::tick()
{
	if (!enabled)
		return false;
	if ((millis() - lastTime) >= timer)
	{
		lastTime = millis();
		if (repeat - counts == 1)
			enabled = false;
		counts++;
		return true;
	}
	return false;
}

void InnuTicker::interval(uint32_t timer)
{
	this->timer = timer;
}

uint32_t InnuTicker::elapsed()
{
	return millis() - lastTime;
}

status_t InnuTicker::state()
{
	return status;
}

uint32_t InnuTicker::counter()
{
	return counts;
}
