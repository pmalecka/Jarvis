#include "CombinedHandler.h"

CombinedHandler::CombinedHandler()
    : SerialDevice(SourceType::Controlbox)
{
}

size_t CombinedHandler::write(uint8_t byte)
{ 
    return Serial.write(byte); 
}

int CombinedHandler::read()
{ 
    return Serial.read(); 
}

int CombinedHandler::available()
{ 
    return Serial.available(); 
}

void CombinedHandler::setup()
{ 
    Serial.begin(BAUDRATE);
}

void CombinedHandler::loop()
{
}
