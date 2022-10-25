using Assets.Scripts;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using UnityEditor;
using UnityEngine;

public enum LightgunButton : int
{
    // Byte 0:
	// 0x20 gets IR sensor connected state?
	// 0x40 link index (player 1/2)
    SeviceMenuSwitch = 0x80,

    // Byte 1:
    GunTrigger = 0x01,
    EnterButton = 0x02,

    MenuDown = 0x10,
    MenuUp = 0x20,
    ServiceButton = 0x40,

    // Byte 2:
    FootPedal = 0xC0 // 0x80?
}

public enum RecoilType
{
    None,
    Active3Supress3,
    Active3Supress5,
    Active8Supress8,
}

public class LightgunController : MonoBehaviour
{
    public string COMPort = "";
    public bool Enabled = true;

    /*
        Screen bounds:		
		    192, 44		upper left
		    200, 242	bottom left
		    798, 242	bottom right
    */
    public Vector2 GunScreenOffset = new Vector2(190, 40);
    public Vector2 GunScreenSize = new Vector2(500, 200);
    public Vector2 GunCrosshairOffset = new Vector2(40, 10);

    public bool Connected { get; private set; } = false;
    
    public string DeviceId { get; private set; }
    public byte Version { get; private set; }
    public byte JvsVersion { get; private set; }
    public byte CommsVersion { get; private set; }
    public Vector2 LightgunPostion { get; private set; }
    public Vector2 LightgunScreenPosition { get; private set; }


    private IntPtr lightgunPollingThreadHandle = IntPtr.Zero;
    private JvsLightgunState lightgunState = new JvsLightgunState();

    private byte deviceId = 1;

    private Vector2 lastObservedLightgunPos = Vector2.zero;

    private int recoilFrameCounter = 0;
    private RecoilType recoilType = RecoilType.None;

    // Start is called before the first frame update
    void Start()
    {
        // Check if we should try to open the com port.
        if (this.Enabled == true && this.COMPort.Length > 0)
        {
            // Try to open the com port.
            string comPort = "\\\\.\\" + this.COMPort;
            if (JvsLightgunDll.OpenSerialPort(comPort) == false)
                throw new System.Exception("Failed to open COM port");

            // Assign a device id to the device.
            JvsLightgunDll.JvsCmd_Reset(0xFF);
            JvsLightgunDll.JvsCmd_Reset(0xFF);
            if (JvsLightgunDll.JvsCmd_AssignAddress(this.deviceId) == false)
            {
                // Failed to assign device id to device.
                Debug.LogWarning($"Failed to assign device id to JVS input device");
                return;
            }

            // Get the device id and version numbers.
            byte version = 0;
            JvsLightgunDll.JvsCmd_GetVersion(this.deviceId, ref version); this.Version = version;
            JvsLightgunDll.JvsCmd_GetJvsVersion(this.deviceId, ref version); this.JvsVersion = version;
            JvsLightgunDll.JvsCmd_GetCommsVersion(this.deviceId, ref version); this.CommsVersion = version;

            StringBuilder tempDeviceId = new StringBuilder(255);
            JvsLightgunDll.JvsCmd_RequestDeviceId(this.deviceId, tempDeviceId, 255);
            this.DeviceId = tempDeviceId.ToString();

            // Create a polling thread for async communication.
            this.lightgunPollingThreadHandle = JvsLightgunDll.JvsAsync_CreatePollingThread(this.deviceId);
            if (this.lightgunPollingThreadHandle == IntPtr.Zero)
            {
                // Failed to create polling thread.
                Debug.LogWarning("Failed to create JVS polling thread!");
                return;
            }

            this.Connected = true;
        }
    }

    private void OnApplicationQuit()
    {
        // Terminate the polling thread if one was created.
        if (this.lightgunPollingThreadHandle != IntPtr.Zero)
        {
            // Terminate the polling thread.
            JvsLightgunDll.JvsAsync_DestroyPollingThread(this.lightgunPollingThreadHandle);
            this.lightgunPollingThreadHandle = IntPtr.Zero;
        }

        // If the lightgun is enabled close the connection.
        if (this.Enabled == true)
            JvsLightgunDll.CloseSerialPort();
    }

    // Update is called once per frame
    void Update()
    {
        // If no lightgun connection is active bail out.
        if (this.lightgunPollingThreadHandle == IntPtr.Zero)
            return;

        // Update recoil state before we query for input state.
        UpdateRecoilState();

        // Update input state.
        JvsLightgunDll.JvsAsync_UpdateLightgunState(this.lightgunPollingThreadHandle, ref this.lightgunState);

        // Mask out flag value on y coord.
        this.LightgunPostion = new Vector2(this.lightgunState.LightgunXCoord & 0x7FFF, (float)(this.lightgunState.LightgunYCoord & 0x7FFF));

        // If the lightgun position is non-zero save it.
        if (this.lightgunState.LightgunXCoord != 0 || this.lightgunState.LightgunYCoord != 0)
        {
            // Set the last observed position.
            this.lastObservedLightgunPos = this.LightgunPostion;

            // Get the game window position and size so we can transform the lightgun pos to screen coordinates.
            EditorWindow gameview = EditorWindow.GetWindow(Type.GetType("UnityEditor.GameView,UnityEditor"));

            // Transform the lightgun coordinates to screen coordinates.
            Vector2 screenPos = gameview.position.min * new Vector2(1f, -1f);
            Vector2 screenSize = gameview.position.size;

            // Adjust the lightgun coordinates by the offset at which the screen is visible.
            Vector2 gunPos = this.LightgunPostion - this.GunScreenOffset;

            // Unity (more like Windows) coordinates have (0, 0) in the bottom left, TSS IO has (0, 0) in the upper left. Flip
            // the y coordinate from TSS IO coordinate grid to Unity coordinate grid.
            gunPos.y = this.GunScreenSize.y - gunPos.y;

            // Transform the lightgun coordinate to a screen coordinate.
            Vector2 temp = (gunPos / this.GunScreenSize) * screenSize;

            // Adjust for the size of the crosshair reticle.
            temp -= this.GunCrosshairOffset;

            this.LightgunScreenPosition = temp;
        }
    }

    void OnGUI()
    {
        int w = Screen.width, h = Screen.height;

        GUIStyle style = new GUIStyle();

        Rect rect = new Rect(0, 40, w, h * 2 / 100);
        style.alignment = TextAnchor.UpperLeft;
        style.fontSize = 20;// h * 2 / 100;
        style.normal.textColor = Color.black;

        string text = string.Format("Lightgun: {0}\nSwitch state: {1}\nCoords: {2} {3}\nScreen Coords: {4} {5}", (this.Connected == true ? this.DeviceId : "Not Connected"), 
            GetSwitchState(LightgunButton.GunTrigger), this.lastObservedLightgunPos.x, this.lastObservedLightgunPos.y, this.LightgunScreenPosition.x, this.LightgunScreenPosition.y);

        GUI.Label(rect, text, style);
    }

    public void ToggleRecoil(RecoilType type)
    {
        // If recoil is already active bail out.
        if (this.recoilType != RecoilType.None)
            return;

        // Set the recoil type.
        this.recoilType = type;
        this.recoilFrameCounter = 0;
    }

    private void UpdateRecoilState()
    {
        // Check if recoil state needs to be updated.
        if (this.recoilType == RecoilType.None)
            return;

        // Check the recoil type and handle accordingly.
        this.recoilFrameCounter++;
        if (this.recoilType == RecoilType.Active3Supress3)
        {
            if (this.recoilFrameCounter < 4) //3
                this.lightgunState.GPIOState |= 0x80;
            else
                this.lightgunState.GPIOState &= 0x7F;

            if (this.recoilFrameCounter == 6) //6
                this.recoilType = RecoilType.None;
        }
        else if (this.recoilType == RecoilType.Active3Supress5)
        {
            if (this.recoilFrameCounter < 3)
                this.lightgunState.GPIOState |= 0x80;
            else
                this.lightgunState.GPIOState &= 0x7F;

            if (this.recoilFrameCounter == 8)
                this.recoilType = RecoilType.None;
        }
        else if (this.recoilType == RecoilType.Active8Supress8)
        {
            if (this.recoilFrameCounter < 8)
                this.lightgunState.GPIOState |= 0x80;
            else
                this.lightgunState.GPIOState &= 0x7F;

            if (this.recoilFrameCounter == 16)
                this.recoilType = RecoilType.None;
        }
    }

    public bool GetSwitchState(LightgunButton button)
    {
        unsafe
        {
            // Check the button to determine which switch state value to use.
            if (button == LightgunButton.SeviceMenuSwitch)
                return (this.lightgunState.SwitchState[0] & (byte)button) != 0;
            else if (button == LightgunButton.FootPedal)
                return (this.lightgunState.SwitchState[2] & (byte)button) != 0;
            else
                return (this.lightgunState.SwitchState[1] & (byte)button) != 0;
        }
    }
}
