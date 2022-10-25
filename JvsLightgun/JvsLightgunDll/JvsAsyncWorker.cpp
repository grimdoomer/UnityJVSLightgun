
#include "JvsInterface.h"

DWORD JvsWorkerThreadProc(JvsWorkerThreadParam* pParam)
{
    // Loop until the thread is terminated.
    while (true)
    {
        // Setup the command packet.
        pParam->CommandPacket.Address = pParam->DeviceAddress;
        pParam->CommandPacket.Length = 3 + 2 + 3;
        pParam->CommandPacket.Buffer[0] = JVS_CMD_READ_SWITCHES;
        pParam->CommandPacket.Buffer[1] = 1;    // switch bank?
        pParam->CommandPacket.Buffer[2] = 3;
        pParam->CommandPacket.Buffer[3] = JVS_CMD_READ_LIGHTGUN;
        pParam->CommandPacket.Buffer[4] = 1;    // light gun index? IO board only serves up one gun...
        pParam->CommandPacket.Buffer[5] = JVS_CMD_WRITE_GPO;
        pParam->CommandPacket.Buffer[6] = 0;
        pParam->CommandPacket.Buffer[7] = pParam->LightgunState.GPIOState;

        // Send the command and check the result.
        if (WritePacket(&pParam->CommandPacket) == false || pParam->CommandPacket.Buffer[0] != 1)
        {
            // Failed to query device state.
            Sleep(10);
            continue;
        }

        {
            // Acquire the device state lock.
            std::lock_guard<std::mutex> deviceInfoLockGuard(pParam->JvsDeviceInfoLock);

            // TODO: We should probably check response codes for each command, but if the packet validated successfully it's probably fine...

            // Update switch state.
            memcpy(pParam->LightgunState.SwitchState, &pParam->CommandPacket.Buffer[2], 3);

            // Get lightgun pos (the data is big endian so byte flip).
            pParam->LightgunState.LightgunXCoord = (WORD)((WORD)pParam->CommandPacket.Buffer[7] << 8 | (WORD)pParam->CommandPacket.Buffer[8]);
            pParam->LightgunState.LightgunYCoord = (WORD)((WORD)pParam->CommandPacket.Buffer[9] << 8 | (WORD)pParam->CommandPacket.Buffer[10]);
        }

        // Sleep.
        Sleep(10);
    }

    return 0;
}

JvsWorkerThreadParam* __cdecl JvsAsync_CreatePollingThread(BYTE deviceAddress)
{
    // Create the worker thread param.
    JvsWorkerThreadParam* pThreadParam = new JvsWorkerThreadParam();
    pThreadParam->DeviceAddress = deviceAddress;

    // Create the worker thread.
    pThreadParam->hWorkerThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)JvsWorkerThreadProc, pThreadParam, 0, nullptr);
    if (pThreadParam->hWorkerThread == NULL)
    {
        // Failed to create worker thread.
        delete pThreadParam;
        return nullptr;
    }

    // Return the address of the thread info struct.
    return pThreadParam;
}

void __cdecl JvsAsync_DestroyPollingThread(JvsWorkerThreadParam* pThreadInfo)
{
    // Stop the polling thread.
    if (pThreadInfo->hWorkerThread != NULL)
        TerminateThread(pThreadInfo->hWorkerThread, 0);

    // Free the thread info struct.
    delete pThreadInfo;
}

void __cdecl JvsAsync_UpdateLightgunState(JvsWorkerThreadParam* pThreadInfo, JvsLightgunState* pLightgunState)
{
    // Acquire the device state lock.
    std::lock_guard<std::mutex> deviceInfoLockGuard(pThreadInfo->JvsDeviceInfoLock);

    // Copy out the switch states and lightgun coords.
    memcpy(pLightgunState->SwitchState, pThreadInfo->LightgunState.SwitchState, 3);
    pLightgunState->LightgunXCoord = pThreadInfo->LightgunState.LightgunXCoord;
    pLightgunState->LightgunYCoord = pThreadInfo->LightgunState.LightgunYCoord;

    // Copy in the GPIO state.
    pThreadInfo->LightgunState.GPIOState = pLightgunState->GPIOState;
}