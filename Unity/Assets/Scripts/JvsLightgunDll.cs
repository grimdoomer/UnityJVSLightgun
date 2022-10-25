using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public unsafe struct JvsLightgunState
{
    public fixed byte SwitchState[3];
    public byte GPIOState;
    public short LightgunXCoord;
    public short LightgunYCoord;
}

public class JvsLightgunDll
{
    [DllImport("JvsLightgunDll.dll")]
    public static extern bool OpenSerialPort([MarshalAs(UnmanagedType.LPWStr)] string portName);

    [DllImport("JvsLightgunDll.dll")]
    public static extern void CloseSerialPort();

    [DllImport("JvsLightgunDll.dll")]
    public static extern bool JvsCmd_Reset(byte address);

    [DllImport("JvsLightgunDll.dll")]
    public static extern bool JvsCmd_AssignAddress(byte address);

    [DllImport("JvsLightgunDll.dll")]
    public static extern bool JvsCmd_RequestDeviceId(byte address, StringBuilder deviceId, int bufferLength);

    [DllImport("JvsLightgunDll.dll")]
    public static extern bool JvsCmd_GetVersion(byte address, ref byte version);

    [DllImport("JvsLightgunDll.dll")]
    public static extern bool JvsCmd_GetJvsVersion(byte address, ref byte version);

    [DllImport("JvsLightgunDll.dll")]
    public static extern bool JvsCmd_GetCommsVersion(byte address, ref byte version);

    [DllImport("JvsLightgunDll.dll")]
    public static extern bool JvsCmd_ReadSwitches(byte address, [MarshalAs(UnmanagedType.LPArray)] byte[] switches, int count);

    [DllImport("JvsLightgunDll.dll")]
    public static extern bool JvsCmd_ReadLightgun(byte address, ref short xPos, ref short yPos);

    [DllImport("JvsLightgunDll.dll")]
    public static extern bool Jvs_PollInputFast(byte address, [MarshalAs(UnmanagedType.LPArray)] byte[] switches, int count, ref short xPos, ref short yPos);

    [DllImport("JvsLightgunDll.dll")]
    public static extern bool JvsCmd_WriteGPO(byte address, byte index, byte value);

    [DllImport("JvsLightgunDll.dll")]
    public static extern IntPtr JvsAsync_CreatePollingThread(byte deviceAddress);

    [DllImport("JvsLightgunDll.dll")]
    public static extern void JvsAsync_DestroyPollingThread(IntPtr pThreadInfo);

    [DllImport("JvsLightgunDll.dll")]
    public static extern void JvsAsync_UpdateLightgunState(IntPtr pThreadInfo, ref JvsLightgunState pLightgunState);
}
