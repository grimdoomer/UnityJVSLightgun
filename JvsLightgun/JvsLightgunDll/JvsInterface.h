
#pragma once
#include <Windows.h>
#include <mutex>

#define JVS_SYNC        0xE0
#define JVS_ESCAPE      0xD0

#define STATE_WAITING_FOR_DATA  0
#define STATE_RECEIVED_SYNC     1
#define STATE_RECEIVED_ADDRESS  2
#define STATE_RECEIVED_LENGTH   3

#define JVS_CMD_REQUEST_ID          0x10
#define JVS_CMD_VERSION             0x11
#define JVS_CMD_JVS_VERSION         0x12
#define JVS_CMD_COMMS_VERSION       0x13
#define JVS_CMD_GET_CAPABILITIES    0x14

#define JVS_CMD_READ_SWITCHES       0x20

#define JVS_CMD_READ_LIGHTGUN       0x25

#define JVS_CMD_WRITE_GPO           0x32

#define JVS_CMD_RESET               0xF0
#define JVS_CMD_ASSIGN_ADDRESS      0xF1

#define JVS_BROADCAST_ADDRESS       0xFF

struct JVSPacket
{
    BYTE Address;
    BYTE Length;
    BYTE Buffer[255];
};

enum JvsCapabilityType : BYTE
{
    JvsCap_END = 0,
    JvsCap_Players = 1,
    JvsCap_Coins = 2,
    JvsCap_AnalogIn = 3,
    JvsCap_Rotary = 4,
    JvsCap_Keypad = 5,
    JvsCap_Lightgun = 6,
    JvsCap_GPI = 7,

    JvsCap_Card = 0x10,
    JvsCap_Hopper = 0x11,
    JvsCap_GPO = 0x12,
    JvsCap_AnalogOut = 0x13,
    JvsCap_Display = 0x14,
    JvsCap_Backup = 0x15
};

struct JvsCapability
{
    /* 0x00 */ JvsCapabilityType    Type;
    /* 0x01 */ BYTE                 Data[3];
};
static_assert(sizeof(JvsCapability) == 4, "JvsCapability struct incorrect size");

struct JvsLightgunState
{
    BYTE SwitchState[3];
    BYTE GPIOState;
    WORD LightgunXCoord;
    WORD LightgunYCoord;
};

struct JvsWorkerThreadParam
{
    // Worker thread handle.
    HANDLE hWorkerThread;

    // JVS message buffer.
    JVSPacket CommandPacket;

    // Address of the lightgun IO board.
    BYTE DeviceAddress;

    // Lightgun info structure and lock.
    JvsLightgunState LightgunState;
    std::mutex JvsDeviceInfoLock;
};

bool __cdecl OpenSerialPort(const WCHAR* psPortName);
void __cdecl CloseSerialPort();

bool __cdecl ReadPacket(JVSPacket* pPacket);
bool __cdecl WritePacket(JVSPacket* pPacket, bool readReply = true);

bool __cdecl JvsCmd_Reset(BYTE address);
bool __cdecl JvsCmd_AssignAddress(BYTE addressToAssign);
bool __cdecl JvsCmd_RequestDeviceId(BYTE address, char* deviceIdBuffer, int bufferLength);
bool __cdecl JvsCmd_GetVersion(BYTE address, BYTE* version);
bool __cdecl JvsCmd_GetJvsVersion(BYTE address, BYTE* version);
bool __cdecl JvsCmd_GetCommsVersion(BYTE address, BYTE* version);
bool __cdecl JvsCmd_GetCapabilities(BYTE address, JvsCapability* capabilities, int* count);
bool __cdecl JvsCmd_ReadSwitches(BYTE address, BYTE* switches, int count);
bool __cdecl JvsCmd_ReadLightgun(BYTE address, WORD* xPos, WORD* yPos);
bool __cdecl JvsCmd_WriteGPO(BYTE address, BYTE index, BYTE value);

JvsWorkerThreadParam* __cdecl JvsAsync_CreatePollingThread(BYTE deviceAddress);
void __cdecl JvsAsync_DestroyPollingThread(JvsWorkerThreadParam* pThreadInfo);
void __cdecl JvsAsync_UpdateLightgunState(JvsWorkerThreadParam* pThreadInfo, JvsLightgunState* pLightgunState);