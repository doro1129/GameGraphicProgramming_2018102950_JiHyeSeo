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
  Class:    MiddleCube

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
class MiddleCube : public BaseCube
{
public:
    MiddleCube() = default;
    MiddleCube(const MiddleCube& other) = delete;
    MiddleCube(MiddleCube&& other) = delete;
    MiddleCube& operator=(const MiddleCube& other) = delete;
    MiddleCube& operator=(MiddleCube&& other) = delete;
    ~MiddleCube() = default;

    virtual void Update(_In_ FLOAT deltaTime) override;
};