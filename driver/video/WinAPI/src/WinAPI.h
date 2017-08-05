// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.
#ifndef WINAPI_H_INCLUDED
#define WINAPI_H_INCLUDED

#pragma once

#include "VideoDriver.h"
#define WIN32_LEAN_AND_MEAN
#include <string>
#include <windows.h>

/// Klasse für den WinAPI Videotreiber.
class VideoWinAPI : public VideoDriver
{
public:
    VideoWinAPI(VideoDriverLoaderInterface* CallBack);

    ~VideoWinAPI();

    /// Funktion zum Auslesen des Treibernamens.
    const char* GetName() const override;

    /// Treiberinitialisierungsfunktion.
    bool Initialize() override;

    /// Treiberaufräumfunktion.
    void CleanUp() override;

    /// Erstellt das Fenster mit entsprechenden Werten.
    bool CreateScreen(const std::string& title, unsigned short width, unsigned short height, const bool fullscreen) override;

    /// Erstellt oder verändert das Fenster mit entsprechenden Werten.
    bool ResizeScreen(unsigned short width, unsigned short height, const bool fullscreen) override;

    /// Schliesst das Fenster.
    void DestroyScreen() override;

    /// Wechselt die OpenGL-Puffer.
    bool SwapBuffers() override;

    /// Die Nachrichtenschleife.
    bool MessageLoop() override;

    /// Funktion zum Auslesen des TickCounts.
    unsigned long GetTickCount() const override;

    /// Funktion zum Holen einer Subfunktion.
    void* GetFunction(const char* function) const override;

    /// Listet verfügbare Videomodi auf
    void ListVideoModes(std::vector<VideoMode>& video_modes) const override;

    /// Funktion zum Setzen der Mauskoordinaten.
    void SetMousePos(int x, int y) override;

    /// Get state of the modifier keys
    KeyEvent GetModKeyState() const override;

    /// Gibt Pointer auf ein Fenster zurück (device-dependent!), HWND unter Windows
    void* GetMapPointer() const override;

private:
    /// Funktion zum Senden einer gedrückten Taste.
    void OnWMChar(unsigned c, bool disablepaste = false, LPARAM lParam = 0);
    void OnWMKeyDown(unsigned c, LPARAM lParam = 0);

    /// Funktion zum Pasten von Text aus dem Clipboard.
    void OnWMPaste();

    /// Callbackfunktion der WinAPI.
    static LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    bool mouse_l;    /// Status der Linken Maustaste.
    bool mouse_r;    /// Status der Rechten Maustaste.
    int mouse_z;     /// Scrolling position for mousewheel.
    DEVMODE dm_prev; /// Bildschirmmodus.
    HWND screen;     /// Fensterhandle.
    HDC screen_dc;   /// Zeichenkontext des Fensters.
    HGLRC screen_rc; /// OpenGL-Kontext des Fensters.
    bool isWindowResizable, isMinimized;
    std::wstring windowClassName;
};

#endif // !WINAPI_H_INCLUDED
