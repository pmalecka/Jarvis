#include "JarvisDesk.h"
#include "Timer.h"
#include <math.h>
#include <esp_task_wdt.h>
#include "esp_err.h"
#include "esphome.h"

Timer Timer1(10000);

JarvisDesk::JarvisDesk(esphome::uart::UARTComponent *parent) : esphome::uart::UARTDevice(parent), serialDecoder(SourceType::Controlbox)
{
    mSettings.reset();
    deskInitialized = false;
}

void JarvisDesk::setup()
{
    //// warning - this never worked, no idea why it is here, lol
    //// it definitely breaks the whole thing on idf 5 (new code below)
    // set the watchdog timeout to 30 seconds
    /// old code, for esp-idf 4.4 and lower
    // esp_task_wdt_init(30, true);

    /// new code for esp-idf 5
    
    // #define CONFIG_FREERTOS_NUMBER_OF_CORES 2
    // #define WDT_TIMEOUT 30000
    
    // esp_task_wdt_config_t twdt_config = {
    //     .timeout_ms = WDT_TIMEOUT,
    //     .idle_core_mask = (1 << CONFIG_FREERTOS_NUMBER_OF_CORES) - 1,    // Bitmask of all cores
    //     .trigger_panic = false,
    // };
    // ESP_ERROR_CHECK(esp_task_wdt_init(&twdt_config));

    wakeUp();
    esphome::delay((uint32_t)70);
    
    Timer1.start();
    
    sDeskInitialized->publish_state("false");
}

void JarvisDesk::connect()
{
    ESP_LOGI("JarvisSerial", "Sending connect message: GetSettings (0x07)");
    // for (int i2 = 0; i2 < 3; i2++) {
        sendMessage(SerialMessage(CommandFromHandsetType::GetSettings));
        // esphome::delay((unsigned int)(i2*70));
    // }
    processResponse((unsigned int)(140));

    ESP_LOGI("JarvisSerial", "Sending connect message: GetAbsLimits (0x0C)");
    // for (int i1 = 0; i1 < 3; i1++) {
        sendMessage(SerialMessage(CommandFromHandsetType::GetAbsLimits));
        // esphome::delay((unsigned int)(i1*70));
    // }
    processResponse((unsigned int)(140));

    ESP_LOGI("JarvisSerial", "Sending connect message: GetUserLimits (0x20)");
    // for (int i3 = 0; i3 < 3; i3++) {
        sendMessage(SerialMessage(CommandFromHandsetType::GetUserLimits));
        // esphome::delay((unsigned int)(i3*70));
    // }
    processResponse((unsigned int)(140));

    ESP_LOGI("JarvisSerial", "Sending connect message: GET_BASIC_SETTINGS (0xFE)");
    // for (int i0 = 0; i0 < 3; i0++) {
        sendMessage(SerialMessage(CommandFromHandsetType::GET_BASIC_SETTINGS));
        // esphome::delay((unsigned int)(i0*70));
    // }
    processResponse((unsigned int)(210));
}

void JarvisDesk::loop()
{
    handleIncomingData();

    // publish desk initialization state to HA
    if(mSettings && deskInitialized == false) {
        deskInitialized = true;
        sDeskInitialized->publish_state("true");
    } else if (!mSettings && deskInitialized == true) {
        deskInitialized = false;
        sDeskInitialized->publish_state("false");
    }
    
    if (!mSettings && Timer1.isFinished())
    {
        ESP_LOGW("Jarvis",
            "Settings not complete (%s): preset 1: %u, preset2: %u, preset3: %u, preset4: %u, units: %s, touchMode: %s, killMode: %s, sensitivity: %s, sysLimitMin: %d, sysLimitMax: %d, userLimitSet: %s, userLimitMin: %d, userLimitMax: %d",
            mSettings ? "complete" : "incomplete", (unsigned int)mSettings.presetRaw1, (unsigned int)mSettings.presetRaw2, (unsigned int)mSettings.presetRaw3, (unsigned int)mSettings.presetRaw4, valToString(mSettings.units).c_str(), valToString(mSettings.touchMode).c_str(), valToString(mSettings.killMode).c_str(), valToString(mSettings.sensitivity).c_str(), mSettings.sysLimitMin, mSettings.sysLimitMax, valToString(mSettings.userLimitSet).c_str(), mSettings.userLimitMin, mSettings.userLimitMax);

        connect();
        //requestAllSettings();
        Timer1.restart();
    }
}

bool JarvisDesk::fetchMessage(SerialMessage& msg)
{
    uint8_t buffer[MAX_PACKET_SIZE]; 
    size_t bufSize = 0;

    bool isValid = false;
    while (available() > 0 && !isValid)
    {
        int r = read();
        isValid = serialDecoder.processData(r);
    }
    
    if (isValid)
    {
        memcpy(buffer, serialDecoder.mPartialMessage, serialDecoder.mPMSize);
        bufSize = serialDecoder.mPMSize;

        return msg.setPacket(buffer, bufSize);
    }

    return false;
}

void JarvisDesk::handleIncomingData()
{
    SerialMessage inMsg;
    if (not fetchMessage(inMsg))
    {
        return;
    }
    
    ESP_LOGV("JarvisSerial", "Incoming message: %s", inMsg.toString().c_str());

    extractSetting(inMsg);
}

void JarvisDesk::extractSetting(const SerialMessage& msg)
{
    switch(msg.getType())
    {
    case CommandFromControlboxType::LocPreset1:
        ESP_LOGI("JarvisSerial", "Incoming message: LocPreset1 (%02x)", msg.getType());
        mSettings.presetRaw1 = msg.getParam<uint16_t>();
        ESP_LOGD("Jarvis", "presetRaw1 %u", (unsigned int)mSettings.presetRaw1);
        break;
    case CommandFromControlboxType::LocPreset2:
        ESP_LOGI("JarvisSerial", "Incoming message: LocPreset2 (%02x)", msg.getType());
        mSettings.presetRaw2 = msg.getParam<uint16_t>();
        ESP_LOGD("Jarvis", "presetRaw2 %u", (unsigned int)mSettings.presetRaw2);
        break;
    case CommandFromControlboxType::LocPreset3:
        ESP_LOGI("JarvisSerial", "Incoming message: LocPreset3 (%02x)", msg.getType());
        mSettings.presetRaw3 = msg.getParam<uint16_t>();
        ESP_LOGD("Jarvis", "presetRaw3 %u", (unsigned int)mSettings.presetRaw3);
        break;
    case CommandFromControlboxType::LocPreset4:
        ESP_LOGI("JarvisSerial", "Incoming message: LocPreset4 (%02x)", msg.getType());
        mSettings.presetRaw4 = msg.getParam<uint16_t>();
        ESP_LOGD("Jarvis", "presetRaw4 %u", (unsigned int)mSettings.presetRaw4);
        break;
    case CommandFromControlboxType::Units:
        ESP_LOGI("JarvisSerial", "Incoming message: Units (%02x)", msg.getType());
        mSettings.units = msg.getParam<UnitsValue>();
        break;
    case CommandFromControlboxType::TouchMode:
        ESP_LOGI("JarvisSerial", "Incoming message: TouchMode (%02x)", msg.getType());
        mSettings.touchMode = msg.getParam<TouchModeValue>();
        break;
    case CommandFromControlboxType::KillMode:
        ESP_LOGI("JarvisSerial", "Incoming message: KillMode (%02x)", msg.getType());
        mSettings.killMode = msg.getParam<KillModeValue>();
        break;
    case CommandFromControlboxType::Sensitivity:
    {
        ESP_LOGI("JarvisSerial", "Incoming message: Sensitivity (%02x)", msg.getType());
        mSettings.sensitivity = msg.getParam<SensitivityValue>();
        
        ESP_LOGI("Jarvis", "Sensitivity: %s", valToString(mSettings.sensitivity).c_str());
        
        sPreset1->publish_state(mSettings.getPreset1());
        sPreset2->publish_state(mSettings.getPreset2());
        sPreset3->publish_state(mSettings.getPreset3());
        sPreset4->publish_state(mSettings.getPreset4());
        sUnits->publish_state(valToString(mSettings.units));
        sTouchMode->publish_state(valToString(mSettings.touchMode));
        sKillMode->publish_state(valToString(mSettings.killMode));
        sSensitivity->publish_state(valToString(mSettings.sensitivity));
        break;
    }
    case CommandFromControlboxType::MinMaxSet:
        ESP_LOGI("JarvisSerial", "Incoming message: MinMaxSet (%02x)", msg.getType());
        mSettings.userLimitSet = msg.getParam<UserLimitSetValue>();
        sUserLimitSet->publish_state(valToString(mSettings.userLimitSet));
        break;
    case CommandFromControlboxType::MinHeight:
        ESP_LOGI("JarvisSerial", "Incoming message: MinHeight (%02x)", msg.getType());
        mSettings.userLimitMin = msg.getParam<uint16_t>();
        sUserLimitMin->publish_state(mSettings.userLimitMin);
        break;
    case CommandFromControlboxType::MaxHeight:
        ESP_LOGI("JarvisSerial", "Incoming message: MaxHeight (%02x)", msg.getType());
        mSettings.userLimitMax = msg.getParam<uint16_t>();
        sUserLimitMax->publish_state(mSettings.userLimitMax);
        break;
    case CommandFromControlboxType::AbsLimits:
    {
        ESP_LOGI("JarvisSerial", "Incoming message: SensitAbsLimitsivity (%02x)", msg.getType());
        uint8_t params[4];
        uint8_t paramSize;
        msg.getParamArray(params, paramSize);
        
        // -1 = probably rounding error inside
        mSettings.sysLimitMax = COMBINE_BYTES(params[0], params[1]) - 1;
        mSettings.sysLimitMin = COMBINE_BYTES(params[2], params[3]) - 1;
        
        sSysLimitMin->publish_state(mSettings.sysLimitMin);
        sSysLimitMax->publish_state(mSettings.sysLimitMax);
        
        break;
    }
    case CommandFromControlboxType::Height:
        ESP_LOGI("JarvisSerial", "!!!Incoming message: Height (%02x)", msg.getType());
        lastReportedHeight = msg.getParam<uint16_t>();
        sHeight->publish_state(lastReportedHeight); break;
    default:
        ESP_LOGW("JarvisSerial", "Got unknown message: (%02x)", msg.getType());
        // ESP_LOGV("JarvisSerial", "Got unknown message: %s", msg.toString().c_str());
        break;
    }
}

void JarvisDesk::processResponse(uint32_t duration)
{
    Timer ct(duration);
    ct.start();
    while (!ct.isFinished())
    {
        handleIncomingData();
    }
}

void JarvisDesk::sendMessage(const SerialMessage& msg, uint8_t reps)
{
    ESP_LOGV("JarvisSerial", "Sending message: %s", msg.toString().c_str());

    switch (msg.getSourceId())
    {
    case SourceType::Handset:
        sendMessageRaw(msg, reps);
        break;
    case SourceType::Controlbox:
        // this will not work, we are not connected to handset, we are the handset  
        ESP_LOGE("JarvisSerial", "Error: Cannot send a message to handset!");
        break;
    default:
        break;
    }
}

void JarvisDesk::sendMessage(const SerialMessage&& msg, uint8_t reps)
{
    sendMessage(msg, reps);
}

void JarvisDesk::sendMessageRaw(const SerialMessage& msg, uint8_t repetition)
{
    uint8_t packet[MAX_PACKET_SIZE];
    msg.construct(packet);

    for (int i = 0; i < repetition; ++i)

        // call the esphome uart write_array fn directly
        write_array(packet, msg.getPacketLength());
}

void JarvisDesk::wakeUp()
{
    sendMessage(SerialMessage(CommandFromHandsetType::Wake));
}

void JarvisDesk::requestAllSettings()
{
    sendMessage(SerialMessage(CommandFromHandsetType::GET_BASIC_SETTINGS));
    processResponse(2000);
    sendMessage(SerialMessage(CommandFromHandsetType::GetAbsLimits));
    processResponse(2000);
    sendMessage(SerialMessage(CommandFromHandsetType::GetAbsLimits));
    processResponse(2000);
    sendMessage(SerialMessage(CommandFromHandsetType::GetSettings));
    processResponse(2000);
    sendMessage(SerialMessage(CommandFromHandsetType::GetSettings));
    processResponse(2000);
    sendMessage(SerialMessage(CommandFromHandsetType::GetUserLimits));
    processResponse(2000);
    // sendMessage(SerialMessage(CommandFromHandsetType::GetUserLimits));
    // processResponse(200);

    // sendMessage(SerialMessage(CommandFromHandsetType::GetUserLimits));
    // processResponse(70);
    // sendMessage(SerialMessage(CommandFromHandsetType::GetAbsLimits));
    // processResponse(70);
    // sendMessage(SerialMessage(CommandFromHandsetType::GetSettings));
    // processResponse(70);
}

void JarvisDesk::setOffset(uint16_t offset)
{
    sendMessage(SerialMessage(CommandFromHandsetType::SetOffset,
                              static_cast<uint16_t>(offset - 6)));
}

void JarvisDesk::setUnits(const char* value)
{
    UnitsValue v = UnitsValue::Unkown;
    if (std::strcmp(value, "inch") == 0)
    {
        v = UnitsValue::inch;
    }
    else if (std::strcmp(value, "cm") == 0)
    {
        v = UnitsValue::mm;
    }
    else
    {
        ESP_LOGW("Jarvis", "Unknown units value received: [%s]", value);
        return;
    }
    
    SerialMessage newcmd(CommandFromHandsetType::SetUnits);
    newcmd.setParam<UnitsValue>(v);
    sendMessage(newcmd);
    requestAllSettings();
}

void JarvisDesk::setTouchMode(const char* value)
{
    TouchModeValue v = TouchModeValue::Unkown;
    if (std::strcmp(value, "Single") == 0)
    {
        v = TouchModeValue::Single;
    }
    else if (std::strcmp(value, "Continuous") == 0)
    {
        v = TouchModeValue::Continuous;
    }
    else
    {
        ESP_LOGW("Jarvis", "Unknown touch mode value received: [%s]", value);
        return;
    }
    
    SerialMessage newcmd(CommandFromHandsetType::SetTouchMode);
    newcmd.setParam<TouchModeValue>(v);
    sendMessage(newcmd);
    requestAllSettings();
}

void JarvisDesk::setKillMode(const char* value)
{
    KillModeValue v = KillModeValue::Unkown;
    if (std::strcmp(value, "Kill") == 0)
    {
        v = KillModeValue::Kill;
    }
    else if (std::strcmp(value, "LetLive") == 0)
    {
        v = KillModeValue::LetLive;
    }
    else
    {
        ESP_LOGW("Jarvis", "Unknown kill mode value received: [%s]", value);
        return;
    }
    SerialMessage newcmd(CommandFromHandsetType::SetKillMode);
    newcmd.setParam<KillModeValue>(v);
    sendMessage(newcmd);
    requestAllSettings();
}

void JarvisDesk::setSensitivity(const char* value)
{
    SensitivityValue v = SensitivityValue::Unkown;
    if (std::strcmp(value, "High") == 0)
    {
        v = SensitivityValue::High;
    }
    else if (std::strcmp(value, "Medium") == 0)
    {
        v = SensitivityValue::Medium;
    }
    else if (std::strcmp(value, "Low") == 0)
    {
        v = SensitivityValue::Low;
    }
    else
    {
        ESP_LOGW("Jarvis", "Unknown sensitivity value received: [%s]", value);
        return;
    }
    SerialMessage newcmd(CommandFromHandsetType::SetSensitivity);
    newcmd.setParam<SensitivityValue>(v);
    sendMessage(newcmd);
    requestAllSettings();
}

void JarvisDesk::move(uint16_t height)
{
    // Hysteresis correction
    if (height > lastReportedHeight)
        height += 2;
    sendMessage(SerialMessage(CommandFromHandsetType::MoveTo, 
                              static_cast<uint16_t>(height)));
}

void JarvisDesk::goPreset1()
{
    sendMessage(SerialMessage(CommandFromHandsetType::MoveToPreset1));
}

void JarvisDesk::goPreset2()
{
    sendMessage(SerialMessage(CommandFromHandsetType::MoveToPreset2));
}

void JarvisDesk::goPreset3()
{
    sendMessage(SerialMessage(CommandFromHandsetType::MoveToPreset3));
}

void JarvisDesk::goPreset4()
{
    sendMessage(SerialMessage(CommandFromHandsetType::MoveToPreset4));
}

void JarvisDesk::setPreset1()
{
    sendMessage(SerialMessage(CommandFromHandsetType::SetPreset1));
    mSettings.presetRaw1 = 0;
    requestAllSettings();
}

void JarvisDesk::setPreset2()
{
    sendMessage(SerialMessage(CommandFromHandsetType::SetPreset2));
    mSettings.presetRaw2 = 0;
    requestAllSettings();
}

void JarvisDesk::setPreset3()
{
    sendMessage(SerialMessage(CommandFromHandsetType::SetPreset3));
    mSettings.presetRaw3 = 0;
    requestAllSettings();
}

void JarvisDesk::setPreset4()
{
    sendMessage(SerialMessage(CommandFromHandsetType::SetPreset4));
    mSettings.presetRaw4 = 0;
    requestAllSettings();
}

void JarvisDesk::setMaxHeight()
{
    sendMessage(SerialMessage(CommandFromHandsetType::SetMaxHeight));
    mSettings.userLimitMax = 0;
    mSettings.userLimitSet = UserLimitSetValue::Unkown;
}

void JarvisDesk::setMinHeight()
{
    sendMessage(SerialMessage(CommandFromHandsetType::SetMinHeight));
    mSettings.userLimitMin = 0;
    mSettings.userLimitSet = UserLimitSetValue::Unkown;
}

void JarvisDesk::clearMaxHeight()
{
    sendMessage(SerialMessage(CommandFromHandsetType::ClearMinMax,
                              static_cast<uint8_t>(0x01)));
    mSettings.userLimitMax = 0;
    mSettings.userLimitSet = UserLimitSetValue::Unkown;
}

void JarvisDesk::clearMinHeight()
{
    sendMessage(SerialMessage(CommandFromHandsetType::ClearMinMax,
                              static_cast<uint8_t>(0x02)));
    mSettings.userLimitMin = 0;
    mSettings.userLimitSet = UserLimitSetValue::Unkown;
}
 