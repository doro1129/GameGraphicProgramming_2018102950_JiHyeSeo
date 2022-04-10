#include "Cube/SideCube.h"

SideCube::SideCube()
{
    m_count = 0.0f;
}

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   SideCube::Update

  Summary:  Update the renderables each frame

  Args:     FLOAT deltaTime
              Time difference of a frame
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
void SideCube::Update(_In_ FLOAT deltaTime)
{
    m_count += deltaTime;

    //This cube is 30% smaller than the origin cube
    XMMATRIX mSpin = XMMatrixRotationZ(-m_count);
    XMMATRIX mOrbit = XMMatrixRotationY(-m_count * 2.0f);
    XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
    XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

    m_world = mScale * mSpin * mTranslate * mOrbit;
}