using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;

public class ObjectPool : MonoBehaviour
{
    public static ObjectPool Instance = null;

    public List<GameObject> PooledObejcts;

    public GameObject ObjectToPool;
    public int PoolSize;

    private int nextObject = 0;

    private void Awake()
    {
        // Set the singleton instance.
        Instance = this;
    }

    private void Start()
    {
        // Initialize the object pool.
        this.PooledObejcts = new List<GameObject>(this.PoolSize);

        // Loop and create object instances.
        for (int i = 0; i < this.PoolSize; i++)
        {
            // Create the object in a deactivated state.
            GameObject obj = Instantiate(this.ObjectToPool);
            obj.SetActive(false);

            this.PooledObejcts.Add(obj);
        }
    }

    public GameObject GetNextFreeObject()
    {
        // Get the next object to return.
        GameObject obj = this.PooledObejcts[this.nextObject];

        // Increment the next object counter.
        this.nextObject++;
        if (this.nextObject >= this.PooledObejcts.Count)
            this.nextObject = 0;

        // Return the object instance.
        return obj;
    }
}
