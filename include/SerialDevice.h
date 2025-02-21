#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include "SerialMessage.h"

class SerialDevice
{
public:
    SerialDevice(uint8_t id);

    bool processData(uint8_t oktet);

    uint8_t mPartialMessage[MAX_PACKET_SIZE];
    size_t mPMSize;

private:
    enum class StateMachineState
    {
        Start,
        Id,
        Command,
        ParamSize,
        Param0,
        Param1,
        Param2,
        Param3,
        Checksum,
        End
    };
    
    uint8_t mId;
    
    StateMachineState mSMState;
};

#endif  // SERIALDEVICE_H
