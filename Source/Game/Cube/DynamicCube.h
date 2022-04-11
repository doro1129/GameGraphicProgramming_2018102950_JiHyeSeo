/*+===================================================================
  File:      BASECUBE.H

  Summary:   Base cube header file contains declarations of BaseCube
             class used for the lab samples of Game Graphics
             Programming course.

  Classes: Cube

  � 2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "Common.h"

#include "Renderer/Renderable.h"
#include "Cube/BaseCube.h"

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Class:    DynamicCube

  Summary:  Base class for renderable 3d cube object

  Methods:  Initialize
              Initializes a basic cube
            Update
              Pure virtual function that updates the cube every frame
            YourCube
              Constructor.
            ~YourCube
              Destructor.
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
class DynamicCube : public BaseCube
{
public:
    DynamicCube();
    DynamicCube(const DynamicCube& other) = delete;
    DynamicCube(DynamicCube&& other) = delete;
    DynamicCube& operator=(const DynamicCube& other) = delete;
    DynamicCube& operator=(DynamicCube&& other) = delete;
    ~DynamicCube() = default;

    virtual void Update(_In_ FLOAT deltaTime) override;

private:
    FLOAT m_count;
    FLOAT m_count2;
    BOOL m_sizeup;
};