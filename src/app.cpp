#include <math.h>

#include <app.hpp>

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif
app::app() : m_hwnd(nullptr),
             m_pDirect2dFactory(nullptr),
             m_pRenderTarget(nullptr),
             m_bitmap(nullptr),
             m_frame_buffer(nullptr) {
}
app::~app() {
    if (m_pRenderTarget != NULL) {
        m_pRenderTarget->Release();
    }
    if (m_pDirect2dFactory != NULL) {
        m_pDirect2dFactory->Release();
    }
    if (m_bitmap != nullptr) {
        m_bitmap->Release();
    }
    if (m_frame_buffer != NULL) {
        free(m_frame_buffer);
    }
}
HRESULT app::initialize() {
    HRESULT hr;

    // Initialize device-indpendent resources, such
    // as the Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    if (SUCCEEDED(hr)) {
        // Register the window class.
        WNDCLASSEX wcex = {sizeof(WNDCLASSEX)};
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = app::WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(LONG_PTR);
        wcex.hInstance = HINST_THISCOMPONENT;
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName = NULL;
        wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
        wcex.lpszClassName = L"UIXDemo";

        RegisterClassEx(&wcex);

        // Because the CreateWindow function takes its size in pixels,
        // obtain the system DPI and use it to scale the window size.
        FLOAT dpiX, dpiY;

        // The factory returns the current system DPI. This is also the value it will use
        // to create its own windows.
        m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);

        // Create the window.
        m_hwnd = CreateWindow(
            L"UIXDemo",
            L"UIX Demo",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this);
        hr = m_hwnd ? S_OK : E_FAIL;
    }
    if (SUCCEEDED(hr)) {
        printf("got here\r\n");
        m_frame_buffer = (uint8_t *)malloc(frame_buffer_type::sizeof_buffer({640, 480}));
        if (m_frame_buffer == NULL) {
            hr = E_OUTOFMEMORY;
            if (SUCCEEDED(hr)) {
                m_screen.buffer1(m_frame_buffer);
                m_screen.dimensions({640, 480});
            }
        }
    }

    if (SUCCEEDED(hr)) {
        ShowWindow(m_hwnd, SW_SHOWNORMAL);
        UpdateWindow(m_hwnd);
    }

    return hr;
}
LRESULT CALLBACK app::WndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam) {
    LRESULT result = 0;

    if (message == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

        app *pDemoApp = (app *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hWnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pDemoApp));

        result = 1;
    } else {
        bool wasHandled = false;

        app *pDemoApp = reinterpret_cast<app *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hWnd,
                GWLP_USERDATA)));

        if (pDemoApp) {
            switch (message) {
                case WM_SIZE: {
                    UINT width = LOWORD(lParam);
                    UINT height = HIWORD(lParam);
                    // pDemoApp->OnResize(width, height);
                }
                    result = 0;
                    wasHandled = true;
                    break;

                case WM_DISPLAYCHANGE: {
                    InvalidateRect(hWnd, NULL, FALSE);
                }
                    result = 0;
                    wasHandled = true;
                    break;

                case WM_PAINT: {
                    if (hWnd == pDemoApp->m_hwnd) {
                        pDemoApp->render();
                        ValidateRect(hWnd, NULL);
                    }
                }
                    result = 0;
                    wasHandled = true;
                    break;

                case WM_DESTROY: {
                    PostQuitMessage(0);
                }
                    result = 1;
                    wasHandled = true;
                    break;
            }
        }

        if (!wasHandled) {
            result = DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    return result;
}
// Process and dispatch messages
void app::run() {
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}
void app::render() {
    HRESULT hr = create_device_resources();
    if (SUCCEEDED(hr)) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        m_screen.update();
        D2D1_RECT_U ru;
        ru.top = 0;
        ru.left = 0;
        ru.right = m_screen.bounds().x2;
        ru.bottom = m_screen.bounds().y2;
        m_bitmap->CopyFromMemory(&ru, m_frame_buffer, 640 * 4);
        D2D1_RECT_F rf;
        rf.top = 0;
        rf.left = 0;
        rf.bottom = rc.bottom - rc.top;
        rf.right = rc.right - rc.left;
        D2D1_RECT_F srf;
        srf.top = 0;
        srf.left = 0;
        srf.bottom = ru.bottom;
        srf.right = ru.right;
        m_pRenderTarget->DrawBitmap(m_bitmap, rf, 1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, srf);
    }
}
HRESULT app::create_device_resources() {
    // Initialize device-dependent resources.
    HRESULT hr = S_OK;

    if (m_pRenderTarget == NULL) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top);
        D2D1_RENDER_TARGET_PROPERTIES rprops = D2D1::RenderTargetProperties();
        // Create a Direct2D render target.
        hr = m_pDirect2dFactory->CreateHwndRenderTarget(
            rprops,
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget);

        if (SUCCEEDED(hr)) {
            D2D1_BITMAP_PROPERTIES props;
            props.dpiX = rprops.dpiX;
            props.dpiY = rprops.dpiY;
            props.pixelFormat = rprops.pixelFormat;
            size.width = 639;
            size.height = 479;
            hr = m_pRenderTarget->CreateBitmap(size, props, &m_bitmap);
            // ID2D1RenderTarget* prt;
            //  linker choked on IID_ID2D1RenderTarget and I'm not sure what to link to, so this works instead
            // static const GUID render_target_iid = {0x2cd90694, 0x12e2, 0x11dc, {0x9f, 0xed, 0x00, 0x11, 0x43, 0xa0, 0x55, 0xf9}};
            // m_pRenderTarget->QueryInterface(render_target_iid,(void**)&prt);
        } else {
            // do nothing
        }
    }
    return hr;
}
void app::destroy_device_resources() {
    if (m_bitmap != nullptr) {
        m_bitmap->Release();
        m_bitmap = NULL;
    }
    if (m_pRenderTarget != NULL) {
        m_pRenderTarget->Release();
        m_pRenderTarget = NULL;
    }
}
