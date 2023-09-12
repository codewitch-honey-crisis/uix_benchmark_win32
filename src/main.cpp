// WIN32 boilerplate
#include <windows.h>

#include <commctrl.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <wchar.h>
// enable patched d2d1.h interfaces
#define WIDL_EXPLICIT_AGGREGATE_RETURNS
// have to use the patched version
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <app.hpp>



// Our application entry point.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr)) {
        {
           app the_app;

            if (SUCCEEDED(the_app.initialize())) {
               the_app.run();
            }
        }
        CoUninitialize();
        return 0;
    }
    return -1;
}