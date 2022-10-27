using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

public class PlayerController : MonoBehaviour
{
    public GameObject CrosshairObject;

    public Texture2D BulletHole1;
    public Texture2D BulletHole2;
    public Texture2D BulletHole3;

    public Camera MainCamera;
    public Camera FlashFrameCamera;

    public int FlashFrameCount = 2;

    // Time the bullet stays fully transparent before fading.
    public float BulletLifeTime = 5f;

    // Time it takes the bullet to completely fade.
    public float BulletDecayTime = 2f;

    public AudioClip GunShotSound;

    private LightgunController lightgun = null;
    private bool previousLightgunState = false;

    private int flashFrameCounter = 0;

    // Start is called before the first frame update
    void Start()
    {
        // Cap FPS to 30.
        Application.targetFrameRate = 30;

        // Try to get the lightgun controller instance.
        this.lightgun = GetComponent<LightgunController>();

        this.FlashFrameCamera.enabled = false;
    }

    // Update is called once per frame
    void Update()
    {
        // Check if lightgun flash is still active.
        if (this.flashFrameCounter > 0)
        {
            // Update frame counter.
            this.flashFrameCounter--;

            // If flash frame counter reaches 0 set the main camera as active.
            if (this.flashFrameCounter == 0)
            {
                this.MainCamera.enabled = true;
                this.FlashFrameCamera.enabled = false;
            }

            // Check if the bullet hit the screen.
            if (this.lightgun != null && this.lightgun.LightgunPostion.x != 0f && this.lightgun.LightgunPostion.y != 0f)
            {
                // Place the bullet on the screen.
                FireBullet(this.lightgun.LightgunScreenPosition);

                // Fire recoil.
                this.lightgun.ToggleRecoil(RecoilType.Active3Supress3);
            }
        }

        // Check for fire action.
        if (Input.GetMouseButtonDown(0) == true)
        {
            // Draw bullet on screen.
            FireBullet(new Vector2(Input.mousePosition.x, Input.mousePosition.y));

            // Draw the flash frame.
            DrawFlashFrame();
        }
        else if (this.lightgun != null && this.lightgun.GetSwitchState(LightgunButton.GunTrigger) == true && this.previousLightgunState == false)
        {
            // Set the lightgun state.
            this.previousLightgunState = true;

            // Draw the flash frame.
            DrawFlashFrame();
        }
        else if (this.lightgun != null && this.lightgun.GetSwitchState(LightgunButton.GunTrigger) == false)
        {
            // Reset lightgun trigger state.
            this.previousLightgunState = false;
        }

        // Only draw the crosshair when the flash frame isn't active.
        if (this.flashFrameCounter == 0)
        {
            // If the lightgun is enabled use it's coordinates to place the crosshair, otherwise use the mouse.
            if (this.lightgun.Enabled == true && this.lightgun.Connected == true)
            {
                this.CrosshairObject.transform.position = new Vector3(this.lightgun.LightgunScreenPosition.x, this.lightgun.LightgunScreenPosition.y, 0f);
            }
            else
            {
                this.CrosshairObject.transform.position = Input.mousePosition;
            }
        }
    }

    void DrawFlashFrame()
    {
        // Set the flash camera as active.
        this.MainCamera.enabled = false;
        this.FlashFrameCamera.enabled = true;

        // Set the flash frame counter.
        this.flashFrameCounter = this.FlashFrameCount;
    }

    void FireBullet(Vector2 pos)
    {
        // Get a bullet hole object and set the initial state.
        GameObject bulletHole = ObjectPool.Instance.GetNextFreeObject();
        BulletHoleInstance instance = bulletHole.GetComponent<BulletHoleInstance>();
        instance.ResetInstance();

        instance.BulletLifeTime = this.BulletLifeTime;
        instance.BulletDecayTime = this.BulletDecayTime;

        // Set the texture for the bullet hole renderer.
        MeshRenderer meshRenderer = bulletHole.GetComponent<MeshRenderer>();
        switch (Random.Range(0, 2))
        {
            case 0: meshRenderer.material.mainTexture = this.BulletHole1; break;
            case 1: meshRenderer.material.mainTexture = this.BulletHole2; break;
            case 2: meshRenderer.material.mainTexture = this.BulletHole3; break;
        }

        // Get the position of the mouse on the hit plane.
        Vector3 hitPos = this.MainCamera.ScreenToWorldPoint(new Vector3(pos.x, pos.y, 2f));
        bulletHole.transform.position = hitPos;

        // Play the gun shot sound.
        AudioSource audio = GetComponent<AudioSource>();
        audio.PlayOneShot(this.GunShotSound);
    }
}
