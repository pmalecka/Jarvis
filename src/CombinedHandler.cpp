#include "CombinedHandler.h"

CombinedHandler::CombinedHandler()
    : SerialDevice(SourceType::Controlbox), /*mSerial(RECEIVE_PIN, TRANSMIT_PIN),*/ mLastReportedHeight(0)
{
}

size_t CombinedHandler::write(uint8_t byte)
{
    return Serial2.write(byte); 
    // return mSerial.write(byte); 
}

int CombinedHandler::read()
{
    return Serial2.read(); 
    // return mSerial.read(); 
}

int CombinedHandler::available()
{
    return Serial2.available(); 
    // return mSerial.available(); 
}

void CombinedHandler::setup()
{
    Serial2.begin(BAUDRATE, SERIAL_8N1, RECEIVE_PIN, TRANSMIT_PIN);
    // mSerial.begin(BAUDRATE);
}

void CombinedHandler::loop()
{
}

void CombinedHandler::handleCBMessage(SerialMessage& msg)
{
    switch(msg.getType())
    {
        case CommandFromControlboxType::Height:
        {
            uint16_t height = msg.getParam<uint16_t>();

            mLastReportedHeight = height;
            break;
        }
        default:
            break;
    }
}

uint16_t CombinedHandler::getLastReportedHeight()
{
    return mLastReportedHeight;
}
