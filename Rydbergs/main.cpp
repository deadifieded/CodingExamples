#include "ParticleThing.h"

#include <windows.h>
#include <d2d1.h>
#include <wingdi.h>
#pragma comment(lib,  "d2d1")
#include <random>
#include <iostream>
#include <string>
#include <chrono>

#include "basewin.h"

COLORREF BACKGROUND_COLOR = RGB(255, 255, 255);

ParticleBoxThing pbt(1000);

const UINT ID1 = 1, ID2 = 2;

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

class MainWindow : public BaseWindow<MainWindow>
{
    HDC hdc;
    HDC hdc_mem;
    D2D1_SIZE_U size;
    HDC hdc_background;
    double last_paint;
    double current_time;

    void    CalculateLayout();
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();
    void    Update();

public:

    MainWindow()
    {
    }

    PCWSTR  ClassName() const { return L"Circle Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

void MainWindow::Update() {
    pbt.update();
}

void MainWindow::CalculateLayout() {
    if (hdc != NULL)
    {
        DeleteDC(hdc_background);
        

        if (hdc_mem != NULL)
        {

            DeleteDC(hdc_mem);

        }

        HBITMAP h_bitmap = CreateCompatibleBitmap(hdc, size.width, size.height);
        hdc_background = CreateCompatibleDC(hdc);
        SelectObject(hdc_background, h_bitmap);


        for (int i = 0; i < size.width; i += 1)
        {
            for (int j = 0; j < size.width; j += 1)
            {
                SetPixel(hdc_background, i, j, BACKGROUND_COLOR);
            }
        }


        hdc_mem = CreateCompatibleDC(hdc);


        h_bitmap = CreateCompatibleBitmap(hdc, size.width, size.height);
        SelectObject(hdc_mem, h_bitmap);

        pbt.createGraphicsResources(size.width, size.height, hdc_mem);
    }
}

HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (hdc_mem == NULL)
    {
        CalculateLayout();
    }
    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    DeleteDC(hdc);
    DeleteDC(hdc_background);
    DeleteDC(hdc_mem);
}


void MainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    DeleteDC(hdc);
    hdc = BeginPaint(m_hwnd, &ps);
    HRESULT hr = CreateGraphicsResources();

    BitBlt(hdc, 0, 0, size.width, size.height, hdc_mem, 0, 0, SRCCOPY);
    
    BitBlt(hdc_mem, 0, 0, size.width, size.height, hdc_background, 0, 0, SRCCOPY);

    pbt.paint(hdc_mem);


    BitBlt(hdc, 0, 0, size.width, size.height, hdc_mem, 0, 0, SRCCOPY);

    EndPaint(m_hwnd, &ps);
}

void MainWindow::Resize()
{
    if (m_hwnd != NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        size = D2D1::SizeU(rc.right, rc.bottom);

        CalculateLayout();
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    MainWindow win;



    if (!win.Create(L"Circle", WS_POPUP | WS_VISIBLE))
    {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}



LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_TIMER:

        switch (wParam)
        {
        case ID1:
            Update();
            Update();
            Update();
            Update();
            Update();
            Update();
            Update();
            Update();
            Update();
            Update();
        case ID2:
            InvalidateRect(m_hwnd, NULL, NULL);
        }

        
        return 0;

    case WM_CREATE:

        SetTimer(m_hwnd,              // handle to main window 
            ID1,             // timer identifier 
            10,                  // 10-second interval 
            (TIMERPROC)NULL);   // callback

        SetTimer(m_hwnd,              // handle to main window 
            ID2,             // timer identifier 
            20,                  // 10-second interval 
            NULL);
        return 0;

    case WM_DESTROY:
        DiscardGraphicsResources();
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        OnPaint();
        return 0;



    case WM_SIZE:
        Resize();
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}
