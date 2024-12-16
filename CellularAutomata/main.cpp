#include <windows.h>
#include <d2d1.h>
#include <wingdi.h>
#pragma comment(lib,  "d2d1")
#include <random>
#include <iostream>
#include <string>

#include "basewin.h"

int CELL_SIZE = 3;
COLORREF CELL_COLOR1 = RGB((int)((double(12*16+12))/4), (int)((double(8*16+8))/4), (int)((double(9*16+9))/4));
COLORREF CELL_COLOR2 = RGB(12 * 16 + 12, 8 * 16 + 8, 9 * 16 + 9);
COLORREF COLORS[] = {RGB(60,60,60),RGB(0,170,30), RGB(110,110,110), RGB(0,200,0)};
int NUM_DIVS = 2;
bool CHANGE_MODE = false;

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}



struct CellList {

    CellList* end;
    CellList* start;
    int x;
    int y;

    CellList()
    {
        this->x = NULL;
        this->y = NULL;
        end = nullptr;
        start = nullptr;
    }

    void populate(CellList* start, int x, int y, CellList* end)
    {
        this->x = x;
        this->y = y;

        this->start = start;
        this->end = end;
        end->start = this;
        start->end = this;
    }

    void add(int x, int y)
    {
        CellList* poop = new CellList();
        poop->populate(start, x, y, this);
    }

    CellList* delink()
    {
        start->end = end;
        end->start = start;
        return start;
    }
};

class MainWindow : public BaseWindow<MainWindow>
{
    int num_divs = 4;//half_num_divs*2+1;


    HDC* cell_hdcs;

    int grid_width = 0, grid_height = 0;

    unsigned int*** cellss;

    CellList* cell_list;
    bool** in_cell_list;
    bool** cells_to_paint;
    int cell_list_length;


    HDC hdc;
    HDC hdc_mem;
    D2D1_SIZE_U size;
    HDC hdc_background;
    int half_num_divs = 1;

    void    CalculateLayout();
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();
    void    CreateCellHDCs();
    void    UpdateCells();
    void    PaintCells();

public:

    MainWindow()
    {
    }

    PCWSTR  ClassName() const { return L"Circle Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};
// Recalculate drawing layout when the size of the window changes.

COLORREF m_progress(float t) {


    // return COLORS[(int)std::round(t / NUM_DIVS)];

    //return 0;

    if (3 * t < 1) {
        float zero_to_1 = t * 3;
        return RGB(std::sqrt(1 - zero_to_1) * 255, std::sqrt(zero_to_1) * 255, 0);
    }
    else if (3 * t < 2) {
        float zero_to_1 = 3 * t - 1;
        return RGB(0, std::sqrt(1 - zero_to_1) * 255, std::sqrt(zero_to_1) * 255);

    }
    else if (t < 1)
    {
        float zero_to_1 = 3 * t - 2;

        return RGB(255 * std::sqrt(zero_to_1), 0, std::sqrt(1 - zero_to_1) * 255);
    }

    return RGB(0, 0, 0);
}

void MainWindow::CreateCellHDCs() {

    cell_hdcs = new HDC[num_divs];

    for (int i = 0; i < num_divs; i += 1) {

        HBITMAP h_bitmap = CreateCompatibleBitmap(hdc, CELL_SIZE, CELL_SIZE);
        cell_hdcs[i] = CreateCompatibleDC(hdc);
        COLORREF color = COLORS[i];//m_progress((float)i/num_divs);
        SelectObject(cell_hdcs[i], h_bitmap);

        for (int j = 0; j < CELL_SIZE;j += 1) {
            for (int k = 0;k < CELL_SIZE; k += 1) {
                SetPixel(cell_hdcs[i], j, k, color);

            }
        }
    }
}

int cell_stat = 0;

void MainWindow::PaintCells() {
    CellList* next = cell_list->end;

}

void MainWindow::UpdateCells() {
    unsigned int** cells = cellss[cell_stat];
    cell_stat = (cell_stat + 1)%2;
    unsigned int** new_cells = cellss[cell_stat];

    CellList* next = cell_list->end;
    CellList* new_cell_list = new CellList();
    new_cell_list->start = new_cell_list;
    new_cell_list->end = new_cell_list;

    int c = 0;

    while (next != cell_list) {
        int i = next->x;
        int j = next->y;

        int n = 0;

        int left = (i - 1 + grid_width) % grid_width;
        int right = (i + 1) % grid_width;
        int down = (j - 1 + grid_height) % grid_height;
        int up = (j + 1) % grid_height;

        /*n += (cells[left][up] - cells[i][j] + half_num_divs + num_divs) % num_divs - half_num_divs;
        n += (cells[left][j] - cells[i][j] + half_num_divs+ num_divs) % num_divs - half_num_divs;
        n += (cells[left][down] - cells[i][j] + half_num_divs+ num_divs) % num_divs - half_num_divs;
        n += (cells[i][down] - cells[i][j] + half_num_divs+ num_divs) % num_divs - half_num_divs;
        n += (cells[right][down] - cells[i][j] + half_num_divs+ num_divs) % num_divs - half_num_divs;
        n += (cells[right][j] - cells[i][j] + half_num_divs+ num_divs) % num_divs - half_num_divs;
        n += (cells[right][up] - cells[i][j] + half_num_divs+ num_divs) % num_divs - half_num_divs;
        n += (cells[i][up] - cells[i][j] + half_num_divs+ num_divs) % num_divs - half_num_divs;

        n = round(float(n) / float(8));

        new_cells[i][j] = (cells[i][j] + n + num_divs) % num_divs;*/

        int modulus = cells[i][j];

        n += cells[left][j];
        n += cells[left][down];
        n += cells[i][down];
        n += cells[right][down];
        n += cells[right][j];
        n += cells[right][up];
        n += cells[i][up];
        n += cells[left][up];

        if (n < 2)
        {
            new_cells[i][j] = 0;
        }

        else if (n == 3)
        {
            new_cells[i][j] = 1;
        }
        else if (n>3)
        {
            new_cells[i][j] = 0;
        }
        else
        {
            new_cells[i][j] = cells[i][j];
        }


        int dx = i * CELL_SIZE;
        int dy = j * CELL_SIZE;

        next = next->end;

        delete next->start;
        in_cell_list[i][j] = false;

        if (new_cells[i][j] != cells[i][j])
        {
            BitBlt(hdc_mem, dx, dy, dx + CELL_SIZE, dy + CELL_SIZE, cell_hdcs[new_cells[i][j] % num_divs], 0, 0, SRCCOPY);
            new_cell_list->add(i, j);
            c += 1;
        }

    }
    std::string s = std::to_string(c);
    std::wstring stemp = std::wstring(s.begin(), s.end());
    LPCWSTR sw = stemp.c_str();
    SetWindowText(m_hwnd, sw);

    cell_list->start = cell_list;
    cell_list->end = cell_list;

    next = new_cell_list->end;

    while (next != new_cell_list)
    {
        int i = next->x;
        int j = next->y;
        cells[i][j] = new_cells[i][j];

        int left = (i - 1 + grid_width) % grid_width;
        int right = (i + 1) % grid_width;
        int down = (j - 1 + grid_height) % grid_height;
        int up = (j + 1) % grid_height;


        if (!in_cell_list[left][up]) {
            cell_list->add(left, up);
            in_cell_list[left][up] = true;
        }
        if (!in_cell_list[i][up]) {
            cell_list->add(i, up);
            in_cell_list[i][up] = true;
        }
        if (!in_cell_list[right][up]) {
            cell_list->add(right, up);
            in_cell_list[right][up] = true;
        }
        if (!in_cell_list[right][j]) {
            cell_list->add(right, j);
            in_cell_list[right][j] = true;
        }
        if (!in_cell_list[right][down]) {
            cell_list->add(right, down);
            in_cell_list[right][down] = true;
        }
        if (!in_cell_list[i][down]) {
            cell_list->add(i, down);
            in_cell_list[i][down] = true;
        }
        if (!in_cell_list[left][down]) {
            cell_list->add(left, down);
            in_cell_list[left][down] = true;
        }
        if (!in_cell_list[left][j]) {
            cell_list->add(left, j);
            in_cell_list[left][j] = true;
        }
        if (!in_cell_list[i][j]) {
            cell_list->add(i, j);
            in_cell_list[i][j] = true;
        }

        next = next->end;
        delete next->start;
    }

    delete new_cell_list;
}

void MainWindow::CalculateLayout() {
    if (hdc != NULL)
    {
        DeleteDC(hdc_background);


        if (cell_hdcs != nullptr)
        {

            for (int i = 0; i < num_divs; i += 1) {
                DeleteDC(cell_hdcs[i]);
            }

        }

        if (hdc_mem != NULL)
        {

            DeleteDC(hdc_mem);

        }

        HBITMAP h_bitmap = CreateCompatibleBitmap(hdc, size.width, size.height);
        hdc_background = CreateCompatibleDC(hdc);
        SelectObject(hdc_background, h_bitmap);
        CreateCellHDCs();


        hdc_mem = CreateCompatibleDC(hdc);
        h_bitmap = CreateCompatibleBitmap(hdc, size.width, size.height);
        SelectObject(hdc_mem, h_bitmap);

        cell_stat = 0;

        unsigned int** cells;
        unsigned int** new_cells;

        if (cellss != nullptr) {
            cells = cellss[0];
            new_cells = cellss[1];

            for (int i = 0;i < grid_width;i += 1) {
                delete[] new_cells[i];
                delete[] cells[i];
            }

            delete[] cells;
            delete[] new_cells;
        }
        else {
            cellss = new unsigned int** [2];
        }

        if (in_cell_list != nullptr)
        {
            delete[] in_cell_list;
        }

        if (cells_to_paint != nullptr)
        {
            delete[] cells_to_paint;
        }




        grid_width = (int)(size.width / CELL_SIZE);
        grid_height = (int)(size.height / CELL_SIZE);

        if (cell_list != nullptr)
        {
            CellList* previous = cell_list->start;
            while (previous != cell_list)
            {
                CellList* to_delete = previous;
                previous = previous->start;
                delete to_delete;
            }
        }
        else {
            cell_list = new CellList();
            cell_list->end = cell_list;
            cell_list->start = cell_list;
        }

        cells = new unsigned int* [grid_width];
        new_cells = new unsigned int* [grid_width];
        in_cell_list = new bool* [grid_width];
        cells_to_paint = new bool* [grid_width];

        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(0, 1);

        for (int i = 0;i < grid_width;i += 1) {
            cells[i] = new unsigned int[grid_height] {};
            new_cells[i] = new unsigned int[grid_height] {};
            in_cell_list[i] = new bool[grid_height] {};
            for (int j = 0; j < grid_height;j += 1) {
                new_cells[i][j] = distribution(generator);
                cells[i][j] = new_cells[i][j];
                in_cell_list[i][j] = true;
                cell_list->add(i, j);
            }
        }

        cell_list_length = grid_width * grid_height;
        cellss[0] = cells;
        cellss[1] = new_cells;

        CellList* next = cell_list->end;

        while (next != cell_list)
        {
            int i = next->x;
            int j = next->y;

            int dx = i * CELL_SIZE;
            int dy = j * CELL_SIZE;

            BitBlt(hdc_mem, dx, dy, dx + CELL_SIZE, dy + CELL_SIZE, cell_hdcs[cells[i][j] % num_divs], 0, 0, SRCCOPY);

            next = next->end;
        }
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
    for (int i = 0; i < num_divs; i += 1) {
        DeleteDC(cell_hdcs[i]);
    }
    DeleteDC(hdc_mem);
}


void MainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    DeleteDC(hdc);
    hdc = BeginPaint(m_hwnd, &ps);
    HRESULT hr = CreateGraphicsResources();

    //BitBlt(hdc, 0, 0, size.width, size.height, hdc_mem, 0, 0, SRCCOPY);

    if (CHANGE_MODE)
    {
        BitBlt(hdc_mem, 0, 0, size.width, size.height, hdc_background, 0, 0, SRCCOPY);
    }

    UpdateCells();

    if (SUCCEEDED(hr))
    {


        //int temp = 0;

        /*for (int i = 0; i < num_divs; i += 1) {
            if (i - grid_width >= grid_width * temp) {
                temp += 1;
            }
            if (temp >= grid_height) {
                break;
            }
            int dx = i - temp * grid_width;
            int dy = temp;

            dx *= CELL_SIZE;
            dy *= CELL_SIZE;
            BitBlt(hdc_mems[0], dx, dy, dx + CELL_SIZE, dy + CELL_SIZE, cell_hdcs[i], 0, 0, SRCCOPY);
        }*/

        /*unsigned int** cells = cellss[cell_stat];

        CellList* next = cell_list->end;

        while (next != cell_list)
        {
            int i = next->x;
            int j = next->y;*/

            /*for (int i = 0; i < grid_width;i += 1)
            {
                for (int j = 0; j<grid_height; j+=1)
            {
                int dx = i * CELL_SIZE;
                int dy = j * CELL_SIZE;

                BitBlt(hdc_mem, dx, dy, dx + CELL_SIZE, dy + CELL_SIZE, cell_hdcs[cellss[cell_stat][i][j] % num_divs], 0, 0, SRCCOPY);
            }
            }*/

            /*for (int i = 0; i < grid_width; i += 1)
            {
                for (int j = 0; j < grid_height; j += 1)
                {
                    int dx = i * CELL_SIZE;
                    int dy = j * CELL_SIZE;

                    BitBlt(hdc_mem, dx, dy, dx + CELL_SIZE, dy + CELL_SIZE, cell_hdcs[cellss[cell_stat][i][j] % num_divs], 0, 0, SRCCOPY);
                }
            }*/
        BitBlt(hdc, 0, 0, size.width, size.height, hdc_mem, 0, 0, SRCCOPY);
    }

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
        InvalidateRect(m_hwnd, NULL, FALSE);
        return 0;

    case WM_CREATE:

        SetTimer(m_hwnd,              // handle to main window 
            NULL,             // timer identifier 
            0.1,                  // 10-second interval 
            (TIMERPROC)NULL);   // callback
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
