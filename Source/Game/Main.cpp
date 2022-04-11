/*+===================================================================
  File:      MAIN.CPP

  Summary:   This application demonstrates creating a Direct3D 11 device

  Origin:    http://msdn.microsoft.com/en-us/library/windows/apps/ff729718.aspx

  Originally created by Microsoft Corporation under MIT License
  � 2022 Kyung Hee University
===================================================================+*/

#include "Common.h"

#include <memory>

#include "Game/Game.h"
#include "Cube/MiddleCube.h"
#include "Cube/SideCube.h"
#include "Cube/DynamicCube.h"

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: wWinMain

  Summary:  Entry point to the program. Initializes everything and
            goes into a message processing loop. Idle time is used to
            render the scene.

  Args:     HINSTANCE hInstance
              Handle to an instance.
            HINSTANCE hPrevInstance
              Has no meaning.
            LPWSTR lpCmdLine
              Contains the command-line arguments as a Unicode
              string
            INT nCmdShow
              Flag that says whether the main application window
              will be minimized, maximized, or shown normally

  Returns:  INT
              Status code.
-----------------------------------------------------------------F-F*/
INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{

#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"Game Graphics Programming Lab 04: 3D Spaces and Transformations");

    std::shared_ptr<library::VertexShader> vertexShader = std::make_shared<library::VertexShader>(L"Shaders/Shaders.fxh", "VS", "vs_5_0");
    if (FAILED(game->GetRenderer()->AddVertexShader(L"MainShader", vertexShader)))
    {
        return 0;
    }
    
    std::shared_ptr<library::PixelShader> pixelShader = std::make_shared<library::PixelShader>(L"Shaders/Shaders.fxh", "PS", "ps_5_0");
    if (FAILED(game->GetRenderer()->AddPixelShader(L"MainShader", pixelShader)))
    {
        return 0;
    }

    //Add cubes and set their shaders
    std::shared_ptr<MiddleCube> middleCube = std::make_shared<MiddleCube>();
    if (FAILED(game->GetRenderer()->AddRenderable(L"MiddleCube", middleCube)))
    {
        return 0;
    }

    if (FAILED(game->GetRenderer()->SetVertexShaderOfRenderable(L"MiddleCube", L"MainShader")))
    {
        return 0;
    }

    if (FAILED(game->GetRenderer()->SetPixelShaderOfRenderable(L"MiddleCube", L"MainShader")))
    {
        return 0;
    }

    std::shared_ptr<SideCube> sideCube = std::make_shared<SideCube>();
    if (FAILED(game->GetRenderer()->AddRenderable(L"SideCube", sideCube)))
    {
        return 0;
    }

    if (FAILED(game->GetRenderer()->SetVertexShaderOfRenderable(L"SideCube", L"MainShader")))
    {
        return 0;
    }

    if (FAILED(game->GetRenderer()->SetPixelShaderOfRenderable(L"SideCube", L"MainShader")))
    {
        return 0;
    }

    std::shared_ptr<DynamicCube> dynamicCube = std::make_shared<DynamicCube>();
    if (FAILED(game->GetRenderer()->AddRenderable(L"DynamicCube", dynamicCube)))
    {
        return 0;
    }

    if (FAILED(game->GetRenderer()->SetVertexShaderOfRenderable(L"DynamicCube", L"MainShader")))
    {
        return 0;
    }

    if (FAILED(game->GetRenderer()->SetPixelShaderOfRenderable(L"DynamicCube", L"MainShader")))
    {
        return 0;
    }

    if (FAILED(game->Initialize(hInstance, nCmdShow)))
    {
        return 0;
    }

    return game->Run();
}