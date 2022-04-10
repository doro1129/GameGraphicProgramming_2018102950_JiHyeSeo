#include "Cube/MiddleCube.h"

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MiddleCube::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void MiddleCube::Update(_In_ FLOAT deltaTime)
    {
        //Rotates around the origin
        RotateY(deltaTime);
    }