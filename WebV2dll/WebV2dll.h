#pragma once
#include <Windows.h>

#ifdef WEBV2DLL_EXPORTS
#define __DLL_PORT extern "C" __declspec(dllexport)
#else
#define __DLL_PORT extern "C" __declspec(dllimport)
#endif

__DLL_PORT void create_randomid();
__DLL_PORT bool create_webview2(HWND hWnd, int createid, const TCHAR* url);

__DLL_PORT HWND get_main_hwnd(int createid);
__DLL_PORT void close_window(HWND hWnd);
__DLL_PORT void resize_webview(HWND hWnd);
__DLL_PORT void load_url(HWND hWnd, const TCHAR* url);
__DLL_PORT void reload_page(HWND hWnd);

__DLL_PORT void set_startup_script(HWND hWnd, const TCHAR* script);
__DLL_PORT void set_global_startup_script(const TCHAR* script);
__DLL_PORT void exec_js(HWND hWnd, const TCHAR* script);
__DLL_PORT void send_json(HWND hWnd, const TCHAR* jsonstr);
