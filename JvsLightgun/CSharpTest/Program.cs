using System.Diagnostics;
using System.Runtime.InteropServices;

namespace CSharpTest
{
    internal class Program
    {
        [DllImport("JvsLightgunDll.dll")]
        public static extern bool OpenSerialPort([MarshalAs(UnmanagedType.LPWStr)] ref string portName);

        [DllImport("JvsLightgunDll.dll")]
        public static extern void CloseSerialPort();

        [DllImport("JvsLightgunDll.dll")]
        public static extern bool JvsCmd_Reset(byte address);

        [DllImport("JvsLightgunDll.dll")]
        public static extern bool JvsCmd_AssignAddress(byte address);

        [DllImport("JvsLightgunDll.dll")]
        public static extern bool JvsCmd_RequestDeviceId(byte address, ref string deviceId, int bufferLength);

        [DllImport("JvsLightgunDll.dll")]
        public static extern bool JvsCmd_GetVersion(byte address, ref byte version);

        [DllImport("JvsLightgunDll.dll")]
        public static extern bool JvsCmd_GetJvsVersion(byte address, ref byte version);

        [DllImport("JvsLightgunDll.dll")]
        public static extern bool JvsCmd_GetCommsVersion(byte address, ref byte version);

        [DllImport("JvsLightgunDll.dll")]
        public static extern bool JvsCmd_ReadSwitches(byte address, ref byte[] switches, int count);

        [DllImport("JvsLightgunDll.dll")]
        public static extern bool JvsCmd_ReadLightgun(byte address, ref short xPos, ref short yPos);

        [DllImport("JvsLightgunDll.dll")]
        public static extern bool JvsCmd_WriteGPO(byte address, byte index, byte value);

        static void Main(string[] args)
        {
            byte deviceId = 1;

            byte Version;
            byte JvsVersion;
            byte CommsVersion;

            // Try to open the com port.
            string comPort = "\\\\.\\COM3";
            if (OpenSerialPort(ref comPort) == false)
                throw new System.Exception("Failed to open COM port");

            // Assign a device id to the device.
            JvsCmd_Reset(0xFF);
            JvsCmd_Reset(0xFF);
            if (JvsCmd_AssignAddress(deviceId) == false)
            {
                // Failed to assign device id to device.
                Debug.WriteLine($"Failed to assign device id to JVS input device");
                return;
            }

            // Get the device id and version numbers.
            byte version = 0;
            JvsCmd_GetVersion(deviceId, ref version); Version = version;
            JvsCmd_GetJvsVersion(deviceId, ref version); JvsVersion = version;
            JvsCmd_GetCommsVersion(deviceId, ref version); CommsVersion = version;

        }
    }
}