using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BulletHoleInstance : MonoBehaviour
{
    // Time the bullet stays fully transparent before fading.
    public float BulletLifeTime = 5f;

    // Time it takes the bullet to completely fade.
    public float BulletDecayTime = 2f;

    private float elapsedTime;
    public bool isFading = false;

    public void ResetInstance()
    {
        // Set values to default values.
        this.elapsedTime = 0f;
        this.isFading = false;
        this.gameObject.SetActive(true);

        // Reset opacity.
        MeshRenderer meshRenderer = this.GetComponent<MeshRenderer>();
        Color color = meshRenderer.material.color;
        color.a = 1f;
        meshRenderer.material.color = color;
    }

    // Update is called once per frame
    void Update()
    {
        // Get the mesh renderer instance for the bullet.
        MeshRenderer meshRenderer = this.GetComponent<MeshRenderer>();

        // Check if the bullet is fading.
        if (this.isFading == false)
        {
            // Check if the bullet should start fading.
            this.elapsedTime += Time.deltaTime;
            if (this.elapsedTime >= this.BulletLifeTime)
            {
                // Fade out the object.
                this.isFading = true;
                StartCoroutine(FadeTo(meshRenderer.material, 0f, this.BulletDecayTime));
            }
        }
        else
        {
            // If the object has faded out deactivate it.
            if (meshRenderer.material.color.a <= 0f)
                this.gameObject.SetActive(false);
        }
    }

    // Define an enumerator to perform our fading.
    // Pass it the material to fade, the opacity to fade to (0 = transparent, 1 = opaque),
    // and the number of seconds to fade over.
    // https://gamedev.stackexchange.com/questions/142791/how-can-i-fade-a-game-object-in-and-out-over-a-specified-duration
    IEnumerator FadeTo(Material material, float targetOpacity, float duration)
    {

        // Cache the current color of the material, and its initiql opacity.
        Color color = material.color;
        float startOpacity = color.a;

        // Track how many seconds we've been fading.
        float t = 0;

        while (t < duration)
        {
            // Step the fade forward one frame.
            t += Time.deltaTime;
            // Turn the time into an interpolation factor between 0 and 1.
            float blend = Mathf.Clamp01(t / duration);

            // Blend to the corresponding opacity between start & target.
            color.a = Mathf.Lerp(startOpacity, targetOpacity, blend);

            // Apply the resulting color to the material.
            material.color = color;

            // Wait one frame, and repeat.
            yield return null;
        }
    }
}
