using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LightgunInfo : MonoBehaviour
{
    public Color TextColor = Color.red;

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    void OnGUI()
    {
        int w = Screen.width, h = Screen.height;

        GUIStyle style = new GUIStyle();

        Rect rect = new Rect(0, 20, w, h * 2 / 100);
        style.alignment = TextAnchor.UpperLeft;
        style.fontSize = h * 2 / 100;
        style.normal.textColor = this.TextColor;

        string text = string.Format("Coords: {0} {1}", Input.mousePosition.x, Input.mousePosition.y);

        GUI.Label(rect, text, style);
    }
}
