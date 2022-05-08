#include "Cube/CustomCube.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   CustomCube::Update

  Summary:  Updates the cube every frame

  Args:     FLOAT deltaTime
              Elapsed time

  Modifies: [m_world].
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
void CustomCube::Update(_In_ FLOAT deltaTime)
{
    static FLOAT s_totalTime = 0.0f;
    s_totalTime += deltaTime;

    m_world = XMMatrixRotationY(s_totalTime) * XMMatrixTranslation(3.0f, XMScalarSin(s_totalTime), 0.0f);
}