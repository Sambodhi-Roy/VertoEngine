#pragma once
#include <windows.h>
#include <string>
#include <thread>
#include <chrono>  
#include <atomic>

enum COLOUR
{
    FG_BLACK = 0x0000,
    FG_WHITE = 0x000F
};

enum PIXEL_TYPE
{
    PIXEL_SOLID = 0x2588
};

class VertoEngine
{
public:
    VertoEngine()
    {
        m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);

        m_rectWindow = { 0, 0, 1, 1 };
    }

    int ScreenWidth() { return m_nScreenWidth; }
    int ScreenHeight() { return m_nScreenHeight; }

    // Construct console
    bool ConstructConsole(int width, int height, int fontw, int fonth)
    {
        m_nScreenWidth = width;
        m_nScreenHeight = height;

        // Create screen buffer
        m_bufScreen = new CHAR_INFO[m_nScreenWidth * m_nScreenHeight];
        memset(m_bufScreen, 0, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);

        // Set console window size
        m_rectWindow = { 0, 0, 1, 1 };
        SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow);

        COORD coord = { (short)m_nScreenWidth, (short)m_nScreenHeight };
        if (!SetConsoleScreenBufferSize(m_hConsole, coord))
            return false;

        if (!SetConsoleActiveScreenBuffer(m_hConsole))
            return false;

        m_rectWindow = { 0, 0, (short)(m_nScreenWidth - 1), (short)(m_nScreenHeight - 1) };
        if (!SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow))
            return false;

        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        cfi.nFont = 0;
        cfi.dwFontSize.X = fontw;
        cfi.dwFontSize.Y = fonth;
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;
        wcscpy_s(cfi.FaceName, L"Consolas");
        SetCurrentConsoleFontEx(m_hConsole, false, &cfi);

        return true;
    }

    // Start engine
    void Start()
    {
        m_bAtomActive = true;
        std::thread t = std::thread(&VertoEngine::GameThread, this);
        t.join();
    }

protected:
    // To be overridden
    virtual bool OnUserCreate() = 0;
    virtual bool OnUserUpdate(float fElapsedTime) = 0;

    void Fill(int x1, int y1, int x2, int y2, short c, short col)
    {
        for (int x = x1; x < x2; x++)
            for (int y = y1; y < y2; y++)
                Draw(x, y, c, col);
    }

    void Draw(int x, int y, short c = PIXEL_SOLID, short col = FG_WHITE)
    {
        if (x >= 0 && x < m_nScreenWidth && y >= 0 && y < m_nScreenHeight)
        {
            m_bufScreen[y * m_nScreenWidth + x].Char.UnicodeChar = c;
            m_bufScreen[y * m_nScreenWidth + x].Attributes = col;
        }
    }

    void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, short c = PIXEL_SOLID, short col = FG_WHITE)
    {
        DrawLine(x1, y1, x2, y2, c, col);
        DrawLine(x2, y2, x3, y3, c, col);
        DrawLine(x3, y3, x1, y1, c, col);
    }

    void DrawLine(int x1, int y1, int x2, int y2, short c = PIXEL_SOLID, short col = FG_WHITE)
    {
        int dx = abs(x2 - x1), dy = -abs(y2 - y1);
        int sx = x1 < x2 ? 1 : -1;
        int sy = y1 < y2 ? 1 : -1;
        int err = dx + dy, e2;

        while (true)
        {
            Draw(x1, y1, c, col);
            if (x1 == x2 && y1 == y2) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x1 += sx; }
            if (e2 <= dx) { err += dx; y1 += sy; }
        }
    }

protected:
    int m_nScreenWidth;
    int m_nScreenHeight;
    CHAR_INFO* m_bufScreen;
    std::wstring m_sAppName;

    HANDLE m_hConsole;
    HANDLE m_hConsoleIn;
    SMALL_RECT m_rectWindow;

    std::atomic<bool> m_bAtomActive;

private:
    void GameThread()
    {
        if (!OnUserCreate()) return;

        auto tp1 = std::chrono::system_clock::now();
        auto tp2 = std::chrono::system_clock::now();

        while (m_bAtomActive)
        {
            tp2 = std::chrono::system_clock::now();
            std::chrono::duration<float> elapsedTime = tp2 - tp1;
            tp1 = tp2;
            float fElapsedTime = elapsedTime.count();

            if (!OnUserUpdate(fElapsedTime))
                m_bAtomActive = false;

            WriteConsoleOutput(m_hConsole, m_bufScreen, { (short)m_nScreenWidth, (short)m_nScreenHeight }, { 0,0 }, &m_rectWindow);
        }
    }
};
