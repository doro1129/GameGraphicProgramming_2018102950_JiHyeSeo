#include "Window/MainWindow.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::Initialize

      Summary:  Initializes main window

      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                  Is a flag that says whether the main application window
                  will be minimized, maximized, or shown normally
                PCWSTR pszWindowName
                  The window name

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT MainWindow::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName)
    {
        static bool raw_input_initialized = false;
        if (raw_input_initialized == false)
        {
            RAWINPUTDEVICE rid = {
                .usUsagePage = 0x01, //Mouse
                .usUsage = 0x02,
                .dwFlags = 0,
                .hwndTarget = NULL
            };

            if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
            {
                return E_FAIL;
            }

            raw_input_initialized = true;
        }

        RECT rc = { 0, 0, 800, 600 };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        HRESULT hr = initialize(
            hInstance,
            nCmdShow,
            pszWindowName,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
            nullptr,
            nullptr);
        if (FAILED(hr))
        {
            return hr;
        }

        RECT clientRC;
        POINT p1, p2;

        GetClientRect(GetWindow(), &clientRC);
        
        p1 = {
            .x = clientRC.left,
            .y = clientRC.top
        };
        p2 = {
            .x = clientRC.right,
            .y = clientRC.bottom
        };

        ClientToScreen(GetWindow(), &p1);
        ClientToScreen(GetWindow(), &p2);

        clientRC = {
            .left = p1.x,
            .top = p1.y,
            .right = p2.x,
            .bottom = p2.y
        };

        ClipCursor(&clientRC);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetWindowClassName

      Summary:  Returns the name of the window class

      Returns:  PCWSTR
                  Name of the window class
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    PCWSTR MainWindow::GetWindowClassName() const
    {
        return L"Sample window Class";
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::HandleMessage

      Summary:  Handles the messages

      Args:     UINT uMessage
                  Message code
                WPARAM wParam
                  Additional data the pertains to the message
                LPARAM lParam
                  Additional data the pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    LRESULT MainWindow::HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;

        switch (uMsg)
        {
        case WM_PAINT:
        {
            hdc = BeginPaint(GetWindow(), &ps);
            EndPaint(GetWindow(), &ps);
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }

        case WM_INPUT:
        {
            UINT dataSize = 0;
            GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER)); //Need to populate data size first

            if (dataSize > 0)
            {
                std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(dataSize);
                if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawdata.get(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
                {
                    RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());
                    if (raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        m_mouseRelativeMovement.X = raw->data.mouse.lLastX;
                        m_mouseRelativeMovement.Y = raw->data.mouse.lLastY;
                    }
                }
            }

            return DefWindowProc(GetWindow(), uMsg, wParam, lParam); //Need to call DefWindowProc for WM_INPUT messages
        }

        case WM_KEYDOWN:
        {
            if (wParam == 0x57) //W key
            {
                m_directions.bFront = true;
            }
            if (wParam == 0x41) //A key
            {
                m_directions.bLeft = true;
            }
            if (wParam == 0x53) //S key
            {
                m_directions.bBack = true;
            }
            if (wParam == 0x44) //D key
            {
                m_directions.bRight = true;
            }
            if (wParam == VK_SHIFT) //Shift key
            {
                m_directions.bDown = true;
            }
            if (wParam == VK_SPACE) //Space key
            {
                m_directions.bUp = true;
            }
            break;
        }
            
        case WM_KEYUP:
        {
            if (wParam == 0x57) //W key
            {
                m_directions.bFront = false;
            }
            if (wParam == 0x41) //A key
            {
                m_directions.bLeft = false;
            }
            if (wParam == 0x53) //S key
            {
                m_directions.bBack = false;
            }
            if (wParam == 0x44) //D key
            {
                m_directions.bRight = false;
            }
            if (wParam == VK_SHIFT) //Shift key
            {
                m_directions.bDown = false;
            }
            if (wParam == VK_SPACE) //Space key
            {
                m_directions.bUp = false;
            }
            break;
        }

        default:
            return DefWindowProc(GetWindow(), uMsg, wParam, lParam);
        }

        return 0;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetDirections

      Summary:  Returns the keyboard direction input

      Returns:  const DirectionsInput&
                  Keyboard direction input
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const DirectionsInput& MainWindow::GetDirections() const
    {
        return m_directions;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetMouseRelativeMovement

      Summary:  Returns the mouse relative movement

      Returns:  const MouseRelativeMovement&
                  Mouse relative movement
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const MouseRelativeMovement& MainWindow::GetMouseRelativeMovement() const
    {
        return m_mouseRelativeMovement;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::ResetMouseMovement

      Summary:  Reset the mouse relative movement to zero
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void MainWindow::ResetMouseMovement()
    {
        m_mouseRelativeMovement.X = 0;
        m_mouseRelativeMovement.Y = 0;
    }
}
