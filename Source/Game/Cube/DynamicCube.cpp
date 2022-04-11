#include "Cube/DynamicCube.h"

DynamicCube::DynamicCube()
    : m_count(0.0f),
    m_count2(1.0f),
    m_sizeup(false)
{ }

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   DynamicCube::Update

  Summary:  Update the renderables each frame

  Args:     FLOAT deltaTime
              Time difference of a frame
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
void DynamicCube::Update(_In_ FLOAT deltaTime)
{
    m_count += deltaTime;

    if (m_sizeup)
    {
        m_count2 -= deltaTime;
        if (m_count2 <= 1.0f)
        {
            m_count *= -1;
            m_sizeup = false;
        }
    }
    else
    {
        m_count2 += deltaTime;
        if (m_count2 >= 5.0f)
        {
            m_count2 *= -1;
            m_sizeup = true;
        }
    }
    
    XMMATRIX mSpin = XMMatrixRotationY(-m_count);
    XMMATRIX mOrbit = XMMatrixRotationZ(-m_count * 2.0f);
    XMMATRIX mTranslate = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
    XMMATRIX mScale = XMMatrixScaling(0.1f * m_count2, 0.1f * m_count2, 0.1f * m_count2);

    m_world = mScale * mSpin * mOrbit * mTranslate;
}