/*
ISC License

Copyright (c) 2016, Mateo Velez Rodriguez - Metavix

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

*/

#include "Ubidots.h"
/**
 * Constructor.
 */
Ubidots::Ubidots(char* token) {
    _token = token;
    _dsName = "Particle";
    currentValue = 0;
    val = (Value *)malloc(MAX_VALUES*sizeof(Value));
    String str = Particle.deviceID();
    _pId = new char[str.length() + 1];
    strcpy (_pId, str.c_str());
}
bool Ubidots::setDatasourceName(char* dsName) {
    _dsName = dsName;
    return true;
}
bool Ubidots::setDatasourceTag(char* dsTag) {
    _pId = dsTag;
    return true;
}

/** 
 * This function is to get value from the Ubidots API
 * @arg id the id where you will get the data
 * @return num the data that you get from the Ubidots API
 */
float Ubidots::getValueWithDatasource(char* dsTag, char* idName) {
  float num;
  int i = 0;
  char* allData = (char *) malloc(sizeof(char) * 300);
  String response;
  uint8_t bodyPosinit;
  sprintf(allData, "Particle/1.0|LV|%s|%s:%s|end", _token, dsTag, idName);
  while (!_client.connected() && i < 6) {
        i++;
        _client.connect(SERVER, PORT);
    }
    if (_client.connected()) {  // Connect to the server
#ifdef DEBUG_UBIDOTS
        Serial.println("Client connected");
        Serial.println(allData);
#endif
        _client.println(allData);
        _client.flush();
    }
    i = 50000;
    while (!_client.available()|| i == 0) {
        i--;
    }
    while (_client.available()) {
        char c = _client.read();
        response += c;
    }
    bodyPosinit = 3 + response.indexOf("OK|");
    response = response.substring(bodyPosinit);
    num = response.toFloat();
    currentValue = 0;
    _client.stop();
    free(allData);
    _client.stop();
    return num;
}
/**
 * Add a value of variable to save
 * @arg variable_id variable id to save in a struct
 * @arg value variable value to save in a struct
 */
void Ubidots::add(char *variable_id, double value) {
  add(variable_id, value, NULL);
}
/**
 * Add a value of variable to save
 * @arg variable_id variable id to save in a struct
 * @arg value variable value to save in a struct
 */
void Ubidots::add(char *variable_id, double value, char *ctext1) {
  (val+currentValue)->idName = variable_id;
  (val+currentValue)->idValue = value;
  (val+currentValue)->contextOne = ctext1;
  currentValue++;
  if (currentValue > MAX_VALUES) {
        Serial.println(F("You are sending more than the maximum of consecutive variables"));
        currentValue = MAX_VALUES;
  }
}
/**
 * Send all data of all variables that you saved
 * @reutrn true upon success, false upon error.
 */
bool Ubidots::sendAll() {
    int i;
    char* allData = (char *) malloc(sizeof(char) * 700);
    sprintf(allData, "%s|POST|%s|%s:%s=>", USER_AGENT, _token, _pId, _dsName);
    for (i = 0; i < currentValue; ) {
        sprintf(allData, "%s%s:%f", allData, (val + i)->idName, (val + i)->idValue);
        if ((val + i)->contextOne != NULL) {
            sprintf(allData, "%s$%s", allData, (val + i)->contextOne);
        }
        i++;
        if (i < currentValue) {
            sprintf(allData, "%s,", allData);
        }
    }
    sprintf(allData, "%s|end", allData);
#ifdef DEBUG_UBIDOTS
    Serial.println(allData);
#endif
    while (!_client.connected() && i < 6) {
        i++;
        _client.connect(SERVER, PORT);
    }
    if (_client.connected()) {        // Connect to the server
#ifdef DEBUG_UBIDOTS
        Serial.println("Client connected");
#endif
        _client.println(allData);
        _client.flush();
    }
    i = 50000;
    while (!_client.available() || i == 0) {
        i--;
    }
    while (_client.available()) {
        char c = _client.read();
#ifdef DEBUG_UBIDOTS
        Serial.write(c);
#endif
    }
    currentValue = 0;
    _client.stop();
    free(allData);
    return true;
}
