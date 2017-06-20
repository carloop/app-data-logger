/*
 * Data Logger app for Carloop
 * https://carloop.io
 *
 * Author: Julien Vanier
 * Copyright 2017 1000 Tools, Inc.
 */

#include "carloop.h"
#include "SdFat.h"

SdFat sd;
const auto sdChipSelect = SS;
#define GPS_LOG_FILENAME "gps_%d.log"
File gpsLogFile;

Carloop<CarloopRevision2> carloop;

// Run our code even when out of range of Wi-Fi / cellular.
SYSTEM_THREAD(ENABLED);

void setup() {
  serial.begin();

  // If you car uses different CAN bus speed, change it here
  carloop.setCANSpeed(500000);
  carloop.begin();

  openFiles();

}

void openFiles() {
  // Print an error message if there is no SD card but continue
  if (!sd.begin(sdChipSelect, SPI_FULL_SPEED)) {
    sd.initErrorPrint();
    return;
  }

  unsigned gpsFilexIndex = 0;
  while (sd.exists(String::format(GPS_LOG_FILENAME, gpsFilexIndex++).c_str())) {}

  if (!gpsLogFile.open(String::format(GPS_LOG_FILENAME, gpsFilexIndex).c_str(), O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorPrint(String::format("Error opening " GPS_LOG_FILENAME, gpsFilexIndex).c_str());
  }
  Serial.printlnf("SD card file opened " GPS_LOG_FILENAME, gpsFilexIndex);
}

void loop() {
  logGPSPosition();

  delay(1000);
}

void logGPSPosition() {
  String gpsPosition = formatGPSPosition(carloop.gps());
  Serial.println(gpsPosition);
  if (gpsLogFile.isOpen()) {
    gpsLogFile.println(gpsPosition);
    gpsLogFile.flush();
  }
}

String formatGPSPosition(TinyGPSPlus &gps) {
  String s;
  s += String::format("GPS %6d chars: ", gps.charsProcessed());
  s += formatFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  s += formatloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  s += formatateTime(gps.date, gps.time);
}

String formatFloat(float val, bool valid, int len, int prec)
{
  String s;
  if(!valid)
  {
    while(len-- > 1)
    {
      s += '*';
    }
    s += ' ';
  }
  else
  {
    char format[10];
    snprintf(format, sizeof(format), "%%%d.%df", len, prec);
    s += String::format(format, val);
  }
  return s;
}

String formatDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  String s;
  if (!d.isValid())
  {
    s += "********** ";
  }
  else
  {
    s += String::format("%02d/%02d/%02d ", d.month(), d.day(), d.year());
  }

  if (!t.isValid())
  {
    s += "******** ";
  }
  else
  {
    s += String::format("%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
  }
  return s;
}

