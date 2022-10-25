# UnityJVSLightgun
JVS lightgun integration with Unity. This is a sample Unity game that will let you use a Namco TSS-IO based arcade light gun as input for light gun games.
This project requires a CRT TV/monitor and complicated hardware setup and is only a proof of concept. If you're serious about making a light gun based Unity game I highly recommend using an IR based light gun.
[]()

# Unity Game
The Unity project is located in the 'Unity' directory and features a shooting range style demo. You can use the mouse or light gun as input, moving the mouse/light gun will move the on-screen crosshair, and pressing the left mouse button or pulling the light gun trigger will place a bullet hole on screen where the crosshair was aimed.
[]()

## Required settings
In order to work with the Namco TSS-IO board the video output of the PC must be set to 15kHz 640x480 @ 30Hz. See the hardware setup section for addtional details on how to do this. The Unity game can run at whatever resolution you want, though it's recommended to match the output video resolution of the PC for best results. It will also need to be capped to 30fps with vsync enabled to make sure frames are drawn on sync intervals or else the light gun may not be able to properly detect flash frames. These settings have already been configured in the Unity project as follows:
- Project Settings/Player/Resolution and Presentation/Default Screen Width/Height = 640x480
- Project Settings/Quality/VSync Count = Every V Blank
- In Assets/Scripts/PlayerController.cs the FPS is limited to 30

## Object hierarchy

## PlayerController script

## Lightgun Controller script

# JvsLightgun Dll
The JvsLightgun directory contains the native code to talk to the TSS-IO board over a COM port. There are three projects in this directory:
- JvsLightgunDll - Dll that handles JVS communication and exposes APIs for querying device state
- JvsLightgun - Console application used to test light gun communication is working correctly
- CSharpTest - C# wrapper for the JvsLightgunDll project to test C# interop outside of Unity

# Hardware Setup
