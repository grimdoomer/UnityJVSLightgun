// JvsLightgun.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <JvsInterface.h>

void PrintDeviceCapabilities(JvsCapability* capabilities, int count)
{
    // Loop and print device capabilities.
    wprintf(L"Capabilities:\n");
    for (int i = 0; i < count; i++)
    {
        // Check the capability and handle accordingly.
        switch (capabilities[i].Type)
        {
        case JvsCap_Players: wprintf(L"  PLAYERS: players=%hhu switches=%hhu\n", capabilities[i].Data[0], capabilities[i].Data[1]); break;
        case JvsCap_Coins: wprintf(L"  COINS: coins=%hhu\n", capabilities[i].Data[0]); break;
        case JvsCap_AnalogIn: wprintf(L"  ANALOG IN: channels=%hhu bits=%hhu\n", capabilities[i].Data[0], capabilities[i].Data[1]); break;
        case JvsCap_Rotary: wprintf(L"  ROTARY: channels=%hhu\n", capabilities[i].Data[0]); break;
        case JvsCap_Keypad: wprintf(L"  KEYPAD:\n"); break;
        case JvsCap_Lightgun: wprintf(L"  LIGHTGUN: xbits=%hhu ybits=%hhu channels=%hhu\n", capabilities[i].Data[0], capabilities[i].Data[1], capabilities[i].Data[2]); break;
        case JvsCap_GPI: wprintf(L"  GPI: inputs=%hhu\n", capabilities[i].Data[0]); break;
        case JvsCap_Card: wprintf(L"  CARD: %hhu\n", capabilities[i].Data[0]); break;
        case JvsCap_Hopper: wprintf(L"  HOPPER: %hhu\n", capabilities[i].Data[0]); break;
        case JvsCap_GPO: wprintf(L"  GPO: outputs=%hhu\n", capabilities[i].Data[0]); break;
        case JvsCap_AnalogOut: wprintf(L"  ANALOG OUT: channels=%hhu\n", capabilities[i].Data[0]); break;
        case JvsCap_Display: wprintf(L"  DISPLAY: columns=%hhu rows=%hhu encoding=%hhu\n", capabilities[i].Data[0], capabilities[i].Data[1], capabilities[i].Data[2]); break;
        case JvsCap_Backup: wprintf(L"  BACKUP:\n"); break;
        default: wprintf(L"  UNKNOWN CAP %hhu\n", capabilities[i].Type); break;
        }
    }
}

int wmain(int argc, WCHAR **argv)
{
    BYTE deviceAddress = 1;
    char deviceId[MAX_PATH] = { 0 };
    BYTE version, jvsVersion, commsVersion = 0;
    JvsCapability capabilities[20];
    int capabilityCount = ARRAYSIZE(capabilities);
    BYTE switchState[3] = { 0 };

    // Open the com port.
    if (OpenSerialPort(L"\\\\.\\COM3") == false)
    {
        // Failed to open serial port.
        return 0;
    }

    // Reset the bus.
    JvsCmd_Reset(JVS_BROADCAST_ADDRESS);
    JvsCmd_Reset(JVS_BROADCAST_ADDRESS);

    // Assign a device id to the client device.
    if (JvsCmd_AssignAddress(deviceAddress) == false)
    {
        wprintf(L"ASSIGN_ID failed!\n");
        goto Cleanup;
    }

    // Get the device id.
    if (JvsCmd_RequestDeviceId(deviceAddress, deviceId, sizeof(deviceId)) == false)
    {
        wprintf(L"REQUEST_ID failed!\n");
        goto Cleanup;
    }

    JvsCmd_GetVersion(deviceAddress, &version);
    JvsCmd_GetJvsVersion(deviceAddress, &jvsVersion);
    JvsCmd_GetCommsVersion(deviceAddress, &commsVersion);

    // Print device info.
    wprintf(L"Device id: %S\n", deviceId);
    wprintf(L"Version: %hhu\n", version);
    wprintf(L"Jvs Version: %hhu\n", jvsVersion);
    wprintf(L"Comms Version: %hhu\n\n", commsVersion);

    // Get device capabilities.
    if (JvsCmd_GetCapabilities(deviceAddress, capabilities, &capabilityCount) == false)
    {
        wprintf(L"GET_CAPABILITIES failed!\n");
        goto Cleanup;
    }

    PrintDeviceCapabilities(capabilities, capabilityCount);

    JvsAsync_CreatePollingThread(deviceAddress);
    Sleep(3000);

    // Test writing GPO.
    //JvsCmd_WriteGPO(deviceAddress, 0, 1);
    //JvsCmd_WriteGPO(deviceAddress, 0, 0);

    /*while (true)
    {
        WORD xpos, ypos;
        JvsCmd_ReadLightgun(deviceAddress, &xpos, &ypos);
        wprintf(L"Lightgun: %d, %d                \r", (int)xpos, (int)ypos);
        Sleep(100);
    }*/

    /*while (true)
    {
        JvsCmd_ReadSwitches(deviceAddress, switchState, 3);
        wprintf(L"Switches: 0x%hhx 0x%hhx 0x%hhx\r", switchState[0], switchState[1], switchState[2]);
        Sleep(100);
    }*/

    /*while (getchar() != 'x')
    {
        JvsCmd_WriteGPO(deviceAddress, 0, 0x80);
        Sleep(25);
        JvsCmd_WriteGPO(deviceAddress, 0, 0);
        Sleep(25);
    }*/

Cleanup:
    CloseSerialPort();
    return 0;
}