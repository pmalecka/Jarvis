#include "SerialDevice.h"
#include "utils.h"

#include "esphome.h"

SerialDevice::SerialDevice(uint8_t id) 
    : mId(id), mPMSize(0), mSMState(StateMachineState::Start)
{
}

SerialDevice::~SerialDevice()
{
}

void SerialDevice::sendPacket(uint8_t* packet, size_t packetSize)
{
    write(packet, packetSize);
}

void SerialDevice::sendMessage(const SerialMessage& msg, uint8_t repetition)
{
    uint8_t packet[MAX_PACKET_SIZE];
    msg.construct(packet);

    for (int i = 0; i < repetition; ++i)
        sendPacket(packet, msg.getPacketLength());
}

void SerialDevice::sendMessage(const SerialMessage&& msg, uint8_t repetition)
{
    sendMessage(msg, repetition);
}

bool SerialDevice::fetchMessage(SerialMessage& msg)
{
    uint8_t buffer[MAX_PACKET_SIZE]; 
    size_t bufSize = 0;

    bool succ = fetchNextCommand(buffer, bufSize);
    succ = msg.setPacket(buffer, bufSize);   

    return succ;
}

bool SerialDevice::fetchNextCommand(uint8_t* array, size_t& arraySize)
{
    bool isValid = false;
    while (available() > 0 && !isValid)
    {
        int r = read();
        isValid = processData(r);
    }
    
    if (isValid)
    {
        memcpy(array, mPartialMessage, mPMSize);
        arraySize = mPMSize;
    }
    return isValid;
}

bool SerialDevice::processData(uint8_t oktet)
{
    // esphome::ESP_LOGD("JarvisStateMachine", "Processing byte: %02X, current state: %02x", oktet, (int)mSMState);
    switch (mSMState)
    {
        case StateMachineState::Start:
        {
            mPMSize = 0;
            mPartialMessage[mPMSize++] = oktet;
            if (oktet == mId)
            {
                mSMState = StateMachineState::Id;
            }
            else
            {
                mSMState = StateMachineState::Start;
                esphome::ESP_LOGE("JarvisStateMachine", "Got UNexpected start octet %02x", oktet);
            }
            break;
        }
        case StateMachineState::Id:
        {
            if (oktet == mId)
            {
                mPartialMessage[mPMSize++] = oktet;
                mSMState = StateMachineState::Command;
            }
            else
            {
                mSMState = StateMachineState::Start;
                esphome::ESP_LOGE("JarvisStateMachine", "Got UNexpected Id octet %02x", oktet);
            }
            break;
        }
        case StateMachineState::Command:
        {
            mPartialMessage[mPMSize++] = oktet;
            mSMState = StateMachineState::ParamSize;
            break;
        }
        case StateMachineState::ParamSize:
        {
            mPartialMessage[mPMSize++] = oktet;
            switch (oktet)
            {
                case 0x00: mSMState = StateMachineState::Checksum; break;
                case 0x01: mSMState = StateMachineState::Param0; break;
                case 0x02: mSMState = StateMachineState::Param1; break;
                case 0x03: mSMState = StateMachineState::Param2; break;
                case 0x04: mSMState = StateMachineState::Param3; break;
                default: 
                {
                    esphome::ESP_LOGE("JarvisSerial", "ParamSize Validation Failed: %s --faulty byte: %s", array2String(mPartialMessage, mPMSize).c_str(), char2hex(oktet).c_str());
                    mSMState = StateMachineState::Start;
                }
            }
            break;
        }
        case StateMachineState::Param0:
        {
            mPartialMessage[mPMSize++] = oktet;
            mSMState = StateMachineState::Checksum; 
            break;
        }
        case StateMachineState::Param1:
        {
            mPartialMessage[mPMSize++] = oktet;
            mSMState = StateMachineState::Param0; 
            break;
        }
        case StateMachineState::Param2:
        {
            mPartialMessage[mPMSize++] = oktet;
            mSMState = StateMachineState::Param1;
            break;
        }
        case StateMachineState::Param3:
        {
            mPartialMessage[mPMSize++] = oktet;
            mSMState = StateMachineState::Param2;
            break;
        }
        case StateMachineState::Checksum:
        {
            uint8_t cmd = mPartialMessage[2];
            uint8_t paramSize = mPartialMessage[3];

            uint8_t params[MAX_PARAM_SIZE];
            memcpy(params, mPartialMessage + 4, paramSize);
            
            uint8_t chk = SerialMessage::computeChecksum(cmd, paramSize, params);
            if (chk == oktet)
            {
                mPartialMessage[mPMSize++] = oktet;
                mSMState = StateMachineState::End;
            }
            else
            {
                esphome::ESP_LOGE("JarvisSerial", "Checksum failed for message: %s expected: %s, got %s", array2String(mPartialMessage, mPMSize).c_str(), char2hex(chk).c_str(), char2hex(oktet).c_str());
                mSMState = StateMachineState::Start;
            }
            break;
        }
        case StateMachineState::End:
        {
            mPartialMessage[mPMSize++] = oktet;
            mSMState = StateMachineState::Start;
            return oktet == 0x7E;
        }
    }
    return false;
}
