#ifndef COMBINEDHANDLER_H
#define COMBINEDHANDLER_H

#include "SerialDevice.h"
#include "Timer.h"
#include "utils.h"

// #include <SoftwareSerial.h>

#define RECEIVE_PIN 17
#define TRANSMIT_PIN 18

class CombinedHandler : public SerialDevice
{
public:
    CombinedHandler();

    void setup() override;
    void loop() override;

    void handleCBMessage(SerialMessage& msg);
    uint16_t getLastReportedHeight();

protected:
    size_t  write(uint8_t byte) override;
    int read() override;
    int available() override;
    using Print::write;

private:
    // SoftwareSerial mSerial;
    uint16_t mLastReportedHeight;
    
};

#endif  // COMBINEDHANDLER_H

