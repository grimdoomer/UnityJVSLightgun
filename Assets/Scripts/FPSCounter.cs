using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FPSCounter : MonoBehaviour
{
    float startingFrame = 0f;
    float framesPerSecond = 0f;
    float elapsedTime = 0.0f;

    public Color TextColor = Color.black;

    void Update()
    {
        // Check if one second has elapsed.
        this.elapsedTime += Time.unscaledDeltaTime;
        if (this.elapsedTime >= 1f)
        {
            // Update fps counters.
            this.framesPerSecond = Time.renderedFrameCount - this.startingFrame;
            this.startingFrame = Time.renderedFrameCount;
            this.elapsedTime = 0f;
        }
    }

    void OnGUI()
    {
        int w = Screen.width, h = Screen.height;

        GUIStyle style = new GUIStyle();

        Rect rect = new Rect(0, 0, w, h * 2 / 100);
        style.alignment = TextAnchor.UpperLeft;
        style.fontSize = 10;// h * 2 / 100;
        style.normal.textColor = this.TextColor;

        float msec = 1f / this.framesPerSecond;
        string text = string.Format("{0} ms ({1} fps)", msec, this.framesPerSecond);

        GUI.Label(rect, text, style);
    }
}
