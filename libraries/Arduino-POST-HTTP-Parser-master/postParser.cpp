#include "Arduino.h"
#include "postParser.h"

//private


String getLine(const String& data) {
  int endOfLineIndex = data.indexOf("\r\n");
  return data.substring(0, endOfLineIndex );
}

String popLine(const String& data) {
  int endOfLineIndex = data.indexOf("\r\n");
  //Serial.println(data.substring(endOfLineIndex + 2, data.length() - 1));
  return data.substring(endOfLineIndex + 2, data.length());
}

String getHeaderField(const String& data, String key) {
  const String&  bufferData = data;
  int keyIndex = bufferData.indexOf(key);
  if (keyIndex == -1) {
    return "";
  }
  int startIndex = bufferData.indexOf(": ", keyIndex);
  int stopIndex = bufferData.indexOf("\r\n", keyIndex);
  //	Serial.print("data: ");
	//Serial.println(bufferData.substring(startIndex + 2, stopIndex));
  return bufferData.substring(startIndex + 2, stopIndex);
}

String getLineFromIndex(const String& data, int index) {
  String bufferData = data;
  for (int i = 0; i < index; i++) {
    bufferData = popLine(bufferData);
  }
  return getLine(bufferData);
}

String getContentType(const String& header) {
  String contentType = getHeaderField(header, "content-type");
  if (contentType == "") {
    contentType = getHeaderField(header, "Content-Type");
  }
  return contentType;
}

String readPayLoad(WiFiClient client, int payLoadSize) {
  String payload = String("");
  for (int i = 0; i < payLoadSize; i++) {
    char c = client.read();
    payload += String(c);
  }
  return payload;
}


int getPayLoadSize(const String& header) {
  String contentLength = getHeaderField(header, "content-length");
  if (contentLength == "") {
    contentLength = getHeaderField(header, "Content-Length");
  }
  return contentLength.toInt();
}

void PostParser::addHeaderCharacter(char c) {
  _header += String(c);
}


//public
PostParser::PostParser(WiFiClient client, bool debug) {
  //save connection
  _client = client;
  _debug = debug;


  //clear data
  _header = String("");
  _payload = String("");
}

int PostParser::read(){
    boolean currentLineIsBlank = true;
    int gotHeader = -1;
    while (_client.connected()) {
      if (_client.available()) {
        char c = _client.read();
        //Serial.print(c);
        addHeaderCharacter(c); // compose the header

        //if(!client.connected()){
        if (c == '\n' && currentLineIsBlank) { // end of header
          gotHeader = 1;

          if(_debug){
            Serial.println(getHeader()); // print the header for debugging
            delay(10); //used to make sure the 2 serial prints don't overlap each other
            Serial.println(getPayload()); // print the payload for debugging
            delay(10); //used to make sure the 2 serial prints don't overlap each other
          }

          _payload = readPayLoad(_client, getPayLoadSize(_header));
          if(_debug){
              Serial.println("Payload:");
              Serial.println(_payload); // print the payload for debugging
          }

          break;
        }
        
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    return gotHeader;
}



String PostParser::getHeader() {
  return _header;
}

String PostParser::getPayload() {
  return _payload;
}

String PostParser::getField(const String& key) {

  int keyIndex = _payload.indexOf(key);
  int startIndex = _payload.indexOf("=", keyIndex);
  int stopIndex = _payload.indexOf("&", keyIndex);
  String data = _payload.substring(startIndex + 1, stopIndex);
  data.replace("+"," ");

  return data;

}

String PostParser::getRequestType(){
  int index = _header.indexOf(' ');
  return _header.substring(0, index);
}

String PostParser::getPath(){
  int startIndex = _header.indexOf(' ');
  int stopIndex = _header.indexOf(' ',startIndex+1);

  return _header.substring(startIndex + 1, stopIndex);
}





