#pragma once
// WIN32 boilerplate
#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
// enable patched d2d1.h interfaces 
#define WIDL_EXPLICIT_AGGREGATE_RETURNS
// have to use the patched version
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <gfx_cpp14.hpp>
#include <uix.hpp>

class app {
public:
    using color_type = gfx::color<gfx::rgba_pixel<32>>;
    using screen_type = uix::screen<gfx::rgba_pixel<32>>;
    using frame_buffer_type = gfx::bitmap<typename screen_type::pixel_type>;
private:
    HWND m_hwnd;
    ID2D1Factory* m_pDirect2dFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1Bitmap* m_bitmap;
    screen_type m_screen;
    uint8_t* m_frame_buffer;
    void render();
    
    ::HRESULT create_device_resources();
    void destroy_device_resources();
public:    
    app();
    virtual ~app();
      // Register the window class and call methods for instantiating drawing resources
    ::HRESULT initialize();
    void run();
    static ::LRESULT CALLBACK WndProc(
                                        ::HWND hWnd,
                                        ::UINT message,
                                        ::WPARAM wParam,
                                        ::LPARAM lParam);
};