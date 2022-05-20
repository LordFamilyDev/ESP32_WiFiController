#ifndef POSTPARSER_h
#define POSTPARSER_h


#include "Arduino.h"
#include "Wifi.h"
#include "WiFiClient.h"

class PostParser {
  public:
    PostParser(WiFiClient client,bool debug = false);
    String getHeader();
    String getPayload();
    String getField(const String& key);
    String getRequestType();
    String getPath();
    int read();

    
  private:
    String _header;
    String _payload;
    WiFiClient _client;
    bool _debug;

    void addHeaderCharacter(char c);
    String getHeaderField(const String& data, String key);
    String getLine(const String& data);
    String popLine(const String& data);
    String getLineFromIndex(const String& data, int index);
    String extractPayload(const String& payload, const String& key);
};

#endif
