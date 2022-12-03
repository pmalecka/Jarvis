#ifndef CONTROLBOXHANDLER_H
#ifndef COMBINEDHANDLER_H
#define COMBINEDHANDLER_H

#include "SerialDevice.h"
#include "Timer.h"
#include "utils.h"

class CombinedHandler : public SerialDevice
{
public:
    CombinedHandler();

    void setup() override;
    void loop() override;

    void requestSettings();

protected:
    size_t  write(uint8_t byte) override;
    int read() override;
    int available() override;
    using Print::write;

private:
    
};

#endif  // COMBINEDHANDLER_H

