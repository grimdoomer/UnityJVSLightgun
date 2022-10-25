
#include "JvsInterface.h"
#include <iostream>
#include <vector>
#include <string>

// Serial port handle for JVS communication.
HANDLE hSerialPort = INVALID_HANDLE_VALUE;

// JVS message buffer.
JVSPacket CommandPacket = { 0 };

bool __cdecl OpenSerialPort(const WCHAR* psPortName)
{
    // TODO: Check if the com port is already open.

    // Open the serial port for JVS communication.
    hSerialPort = CreateFile(psPortName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hSerialPort == INVALID_HANDLE_VALUE)
    {
        // Failed to open the serial port.
        wprintf(L"Failed to open serial port %d\n", GetLastError());
        return false;
    }

    // Get the current comm port settings.
    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);
    GetCommState(hSerialPort, &serialParams);

    // Setup the connection for 115200 baud 8 N 1.
    serialParams.BaudRate = CBR_115200;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;
    SetCommState(hSerialPort, &serialParams);

    // Port opened successfully.
    return true;
}

void __cdecl CloseSerialPort()
{
    // If the port handle is valid close it.
    if (hSerialPort != INVALID_HANDLE_VALUE)
        CloseHandle(hSerialPort);

    hSerialPort = INVALID_HANDLE_VALUE;
}

bool __cdecl ReadPacket(JVSPacket* pPacket)
{
    DWORD bytesRead = 0;
    BYTE inputBuffer[512];
    int state = STATE_WAITING_FOR_DATA;
    BYTE* pBuffer = (BYTE*)pPacket;
    int index = 0;
    bool escape = false;
    BYTE checksum = 0;

    // Clear any stale data in the packet buffer.
    memset(pPacket->Buffer, 0, ARRAYSIZE(pPacket->Buffer));

    // Loop until we receive a full packet.
    while (true)
    {
        // If no bytes are available sleep.
        COMSTAT comStat;
        DWORD error = 0;
        while (ClearCommError(hSerialPort, &error, &comStat) != FALSE && comStat.cbInQue == 0)
        {
            /*if (error == 0x10)
                ClearCommBreak(hSerialPort);*/
            Sleep(100);
        }

        // Read any incoming data from the serial port.
        if (ReadFile(hSerialPort, inputBuffer, comStat.cbInQue, &bytesRead, nullptr) == FALSE)
        {
            // Failed to read data from serial port.
            wprintf(L"Failed to read data from serial port %d\n", GetLastError());
            return false;
        }

        // Loop and process the message buffer.
        for (int i = 0; i < bytesRead; i++)
        {
            // Check if this is a sync byte.
            if (inputBuffer[i] == JVS_SYNC)
            {
                // Reset state
                state = STATE_RECEIVED_SYNC;
                pBuffer = (BYTE*)pPacket;
                index = 0;
                escape = false;
                continue;
            }

            // Check if the current value needs to be escaped.
            BYTE value = inputBuffer[i];
            if (inputBuffer[i] == JVS_ESCAPE)
            {
                escape = true;
                continue;
            }
            else if (escape == true)
            {
                value++;
                escape = false;
            }

            // Add the value to the packet and update checksum.
            pBuffer[index++] = value;
            checksum += value;

            // Check the state and handle accordingly.
            switch (state)
            {
            case STATE_RECEIVED_SYNC:
            case STATE_RECEIVED_ADDRESS:
            {
                // Next stage.
                state++;
                break;
            }
            case STATE_RECEIVED_LENGTH:
            {
                // Check if we processed the entire input buffer.
                if (index - 2 == pPacket->Length - 1)
                {
                    // Check if the checksum is correct.
                    if (checksum != inputBuffer[i + 1])
                    {
                        // Message has invalid checksum.
                        wprintf(L"Received JVS message with invalid checksum!\n");
                        return false;
                    }
                    else
                    {
                        // Messaged processed successfully.
                        return true;
                    }
                }
                break;
            }
            }
        }
    }

    // We should never make it here.
    return false;
}

bool __cdecl WritePacket(JVSPacket* pPacket, bool readReply)
{
    // Setup the comm buffer.
    int bytesToWrite = 0;
    BYTE outputBuffer[512];

    // Increment the packet length to account for the checksum byte.
    pPacket->Length++;

    // Write the sync control value.
    outputBuffer[bytesToWrite++] = JVS_SYNC;

    BYTE checksum = 0;
    BYTE* pDataPtr = (BYTE*)pPacket;

    // Loop and fill the output buffer with the data to send.
    for (int i = 0; i < pPacket->Length - 1 + 2; i++)
    {
        // Check if the current byte is a control value and handle accordingly.
        if (pDataPtr[i] == JVS_SYNC || pDataPtr[i] == JVS_ESCAPE)
        {
            // Escape the value.
            outputBuffer[bytesToWrite++] = JVS_ESCAPE;
            outputBuffer[bytesToWrite++] = pDataPtr[i] - 1;
        }
        else
        {
            // Use value as-is.
            outputBuffer[bytesToWrite++] = pDataPtr[i];
        }

        // Update checksum.
        checksum += pDataPtr[i];
    }

    // Write the checksum to the output buffer.
    if (checksum == JVS_SYNC || checksum == JVS_ESCAPE)
    {
        // Escape the checksum value.
        outputBuffer[bytesToWrite++] = JVS_ESCAPE;
        outputBuffer[bytesToWrite++] = checksum - 1;
    }
    else
        outputBuffer[bytesToWrite++] = checksum;

    // Write the output buffer to the serial port.
    DWORD bytesWritten = 0;
    if (WriteFile(hSerialPort, outputBuffer, bytesToWrite, &bytesWritten, nullptr) == FALSE)
    {
        // Failed to write buffer to the serial port.
        wprintf(L"Failed to write message to serial port %d\n", GetLastError());
        return false;
    }

    // Sleep and let the device process.
    Sleep(10);

    // Check if we should process the reply.
    if (readReply == true)
    {
        // Wait for a reply indicating the result.
        if (ReadPacket(pPacket) == false)
        {
            // Failed to receive reply message.
            wprintf(L"Failed to receive reply for command!\n");
            return false;
        }
    }

    // Message sent successfully.
    return true;
}

bool __cdecl JvsCmd_Reset(BYTE address)
{
    // Setup the reset packet.
    CommandPacket.Address = address;
    CommandPacket.Length = 2;
    CommandPacket.Buffer[0] = JVS_CMD_RESET;
    CommandPacket.Buffer[1] = 0xD9;    // Indicates the client device should reset it's address

    // Send the command and check the result.
    if (WritePacket(&CommandPacket, false) == false)
        return false;

    return true;
}

bool __cdecl JvsCmd_AssignAddress(BYTE addressToAssign)
{
    // Setup the command packet.
    CommandPacket.Address = JVS_BROADCAST_ADDRESS;
    CommandPacket.Length = 2;
    CommandPacket.Buffer[0] = JVS_CMD_ASSIGN_ADDRESS;
    CommandPacket.Buffer[1] = addressToAssign;

    // Send the command and check the result.
    if (WritePacket(&CommandPacket) == false)
        return false;

    return CommandPacket.Buffer[0] == 1;
}

bool __cdecl JvsCmd_RequestDeviceId(BYTE address, char* deviceIdBuffer, int bufferLength)
{
    // Setup the command packet.
    CommandPacket.Address = address;
    CommandPacket.Length = 1;
    CommandPacket.Buffer[0] = JVS_CMD_REQUEST_ID;

    // Send the command and check the result.
    if (WritePacket(&CommandPacket) == false || CommandPacket.Buffer[0] != 1)
        return false;

    // Get the device id from the response message.
    strncpy(deviceIdBuffer, (char*)&CommandPacket.Buffer[2], min(bufferLength, CommandPacket.Length - 3));
    return true;
}

bool __cdecl JvsCmd_GetVersion(BYTE address, BYTE* version)
{
    // Setup the command packet.
    CommandPacket.Address = address;
    CommandPacket.Length = 1;
    CommandPacket.Buffer[0] = JVS_CMD_VERSION;

    // Send the command and check the result.
    if (WritePacket(&CommandPacket) == false || CommandPacket.Buffer[0] != 1)
        return false;

    // Get the version number.
    *version = CommandPacket.Buffer[2];
    return true;
}

bool __cdecl JvsCmd_GetJvsVersion(BYTE address, BYTE* version)
{
    // Setup the command packet.
    CommandPacket.Address = address;
    CommandPacket.Length = 1;
    CommandPacket.Buffer[0] = JVS_CMD_JVS_VERSION;

    // Send the command and check the result.
    if (WritePacket(&CommandPacket) == false || CommandPacket.Buffer[0] != 1)
        return false;

    // Get the version number.
    *version = CommandPacket.Buffer[2];
    return true;
}

bool __cdecl JvsCmd_GetCommsVersion(BYTE address, BYTE* version)
{
    // Setup the command packet.
    CommandPacket.Address = address;
    CommandPacket.Length = 1;
    CommandPacket.Buffer[0] = JVS_CMD_COMMS_VERSION;

    // Send the command and check the result.
    if (WritePacket(&CommandPacket) == false || CommandPacket.Buffer[0] != 1)
        return false;

    // Get the version number.
    *version = CommandPacket.Buffer[2];
    return true;
}

bool __cdecl JvsCmd_GetCapabilities(BYTE address, JvsCapability* capabilities, int* count)
{
    // Setup the command packet.
    CommandPacket.Address = address;
    CommandPacket.Length = 1;
    CommandPacket.Buffer[0] = JVS_CMD_GET_CAPABILITIES;

    // Send the command and check the result.
    if (WritePacket(&CommandPacket) == false || CommandPacket.Buffer[0] != 1)
        return false;

    // Loop and process capabilities.
    JvsCapability* pCapPtr = (JvsCapability*)&CommandPacket.Buffer[2];
    for (int i = 0; i < *count; i++)
    {
        // Check if this is the last capability in the list.
        if (pCapPtr->Type == JvsCap_END)
        {
            *count = i;
            break;
        }

        // Copy the capability info.
        capabilities[i] = *pCapPtr;
        pCapPtr++;
    }

    return true;
}

bool __cdecl JvsCmd_ReadSwitches(BYTE address, BYTE* switches, int count)
{
    // Setup the command packet.
    CommandPacket.Address = address;
    CommandPacket.Length = 3;
    CommandPacket.Buffer[0] = JVS_CMD_READ_SWITCHES;
    CommandPacket.Buffer[1] = 1;    // switch bank?
    CommandPacket.Buffer[2] = (BYTE)count;

    // Send the command and check the result.
    if (WritePacket(&CommandPacket) == false || CommandPacket.Buffer[0] != 1)
        return false;

    // Copy switch status.
    memcpy(switches, &CommandPacket.Buffer[2], min(count, CommandPacket.Length - 2));
    return true;
}

bool __cdecl JvsCmd_ReadLightgun(BYTE address, WORD* xPos, WORD* yPos)
{
    // Setup the command packet.
    CommandPacket.Address = address;
    CommandPacket.Length = 2;
    CommandPacket.Buffer[0] = JVS_CMD_READ_LIGHTGUN;
    CommandPacket.Buffer[1] = 1;    // light gun index? IO board only serves up one gun...

    // Send the command and check the result.
    if (WritePacket(&CommandPacket) == false || CommandPacket.Buffer[0] != 1)
        return false;

    // Get lightgun pos (the data is big endian so byte flip).
    *xPos = (WORD)((WORD)CommandPacket.Buffer[2] << 8 | (WORD)CommandPacket.Buffer[3]);
    *yPos = (WORD)((WORD)CommandPacket.Buffer[4] << 8 | (WORD)CommandPacket.Buffer[5]);

    return true;
}

bool __cdecl JvsCmd_WriteGPO(BYTE address, BYTE index, BYTE value)
{
    // Setup the command packet.
    CommandPacket.Address = address;
    CommandPacket.Length = 3;
    CommandPacket.Buffer[0] = JVS_CMD_WRITE_GPO;
    CommandPacket.Buffer[1] = index;
    CommandPacket.Buffer[2] = value;

    // Send the command and check the result.
    if (WritePacket(&CommandPacket) == false || CommandPacket.Buffer[0] != 1)
        return false;

    return true;
}

bool __cdecl Jvs_PollInputFast(BYTE address, BYTE* switches, int count, WORD* xPos, WORD* yPos)
{
    // Setup the command packet.
    CommandPacket.Address = address;
    CommandPacket.Length = 3 + 2;
    CommandPacket.Buffer[0] = JVS_CMD_READ_SWITCHES;
    CommandPacket.Buffer[1] = 1;    // switch bank?
    CommandPacket.Buffer[2] = (BYTE)count;
    CommandPacket.Buffer[3] = JVS_CMD_READ_LIGHTGUN;
    CommandPacket.Buffer[4] = 1;    // light gun index? IO board only serves up one gun...

    // Send the command and check the result.
    if (WritePacket(&CommandPacket) == false || CommandPacket.Buffer[0] != 1)
        return false;

    // Copy switch status.
    memcpy(switches, &CommandPacket.Buffer[2], min(count, CommandPacket.Length - 2));

    // Get lightgun pos (the data is big endian so byte flip).
    *xPos = (WORD)((WORD)CommandPacket.Buffer[7] << 8 | (WORD)CommandPacket.Buffer[8]);
    *yPos = (WORD)((WORD)CommandPacket.Buffer[9] << 8 | (WORD)CommandPacket.Buffer[10]);

    return true;
}