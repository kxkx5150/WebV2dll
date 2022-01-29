// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <combaseapi.h>
#include <commctrl.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
#include <vector>
#include <wil/com.h>
#include <wrl.h>
#include "WebV2dll.h"
#include "WebView2.h"
#pragma comment(lib, "comctl32.lib")

#define WM_WEBV_USER (WM_APP + 0)
#define WM_JSMSG (WM_APP + 1)

const TCHAR* strClassName = TEXT("CREATE_WEBVIEW2");
using namespace Microsoft::WRL;
struct windowobj {
    HWND hwnd = nullptr;
    wil::com_ptr<ICoreWebView2> webviewWindow = nullptr;
    wil::com_ptr<ICoreWebView2Controller> webviewController = nullptr;
    wil::com_ptr<ICoreWebView2_2> webView2 = nullptr;
    std::wstring startup_script = L"";
    int createid = -1;
};

HINSTANCE hInst;
int g_randomid = 0;
std::wstring g_startup_script = L"";
std::vector<windowobj> m_windowobjs;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
void create_randomid()
{
    g_randomid = 0;
}
int get_current_windowobj_idx(HWND hwnd)
{
    for (int i = 0; i < m_windowobjs.size(); i++) {
        if (m_windowobjs[i].hwnd == hwnd) {
            return i;
        }
    }
    return -1;
}
void load_url(HWND hWnd, const TCHAR* url)
{
    int idx = get_current_windowobj_idx(hWnd);
    m_windowobjs[idx].webviewWindow->Navigate(url);
}
void resize_webview(HWND hWnd)
{
    int idx = get_current_windowobj_idx(hWnd);
    if (idx == -1)
        return;
    if (m_windowobjs[idx].webviewController != nullptr) {
        RECT bounds;
        GetClientRect(m_windowobjs[idx].hwnd, &bounds);
        m_windowobjs[idx].webviewController->put_Bounds(bounds);
    };
}
void set_copydata(HWND hWnd, std::wstring s, int wmmsg)
{
    DWORD bytes = (s.size()+10) * sizeof(wchar_t);
    TCHAR* buffer = new TCHAR[s.length() + 1];
    wcscpy_s(buffer, s.length() + 1, s.c_str());
    COPYDATASTRUCT data_to_send = { 0 };
    data_to_send.dwData = g_randomid;
    data_to_send.cbData = bytes;
    data_to_send.lpData = buffer;
    SendMessage(hWnd, wmmsg, 0, (LPARAM)&data_to_send);
    delete[] buffer;
}
void set_jsmsg_to_copydata(HWND hWnd, std::wstring s)
{
    set_copydata(hWnd, s, WM_JSMSG);
}
HWND get_main_hwnd(int createid)
{
    for (int i = 0; i < m_windowobjs.size(); i++) {
        if (m_windowobjs[i].createid == createid) {
            return m_windowobjs[i].hwnd;
        }
    }
    return nullptr;
}
void close_window(HWND hWnd)
{
    PostMessage(hWnd, WM_CLOSE, 0, 0);
}
void reload_page(HWND hWnd)
{
    int idx = get_current_windowobj_idx(hWnd);
    m_windowobjs[idx].webviewWindow->Reload();
}
void set_startup_script(HWND hWnd, const TCHAR* script)
{
    int idx = get_current_windowobj_idx(hWnd);
    m_windowobjs[idx].startup_script = script;
}
void set_global_startup_script(const TCHAR* script)
{
    g_startup_script = script;
}
void exec_js(HWND hWnd, const TCHAR* script)
{
    int idx = get_current_windowobj_idx(hWnd);

    m_windowobjs[idx].webviewWindow->ExecuteScript(script,
        Callback<ICoreWebView2ExecuteScriptCompletedHandler>([](HRESULT error, PCWSTR result) -> HRESULT {
            return S_OK;
        }).Get());
}
void send_json(HWND hWnd, const TCHAR* jsonstr)
{
    int idx = get_current_windowobj_idx(hWnd);
    m_windowobjs[idx].webviewWindow->PostWebMessageAsJson(jsonstr);
}
void close_window_event(HWND hWnd)
{
    int idx = get_current_windowobj_idx(hWnd);
    m_windowobjs.erase(m_windowobjs.begin() + idx);
    if (m_windowobjs.size() < 1) {
        PostQuitMessage(0);
    }
}
void CHECK_FAILURE(HRESULT hr)
{
    if (FAILED(hr)) {
        std::wstring message;
        message = std::wstring(L"Something went wrong.");
        MessageBoxW(nullptr, message.c_str(), nullptr, MB_OK);
    }
}
void webview_events(HWND hWnd)
{
    int idx = get_current_windowobj_idx(hWnd);
    EventRegistrationToken token;
    m_windowobjs[idx].webviewWindow->add_WebMessageReceived(
        Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [hWnd](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                wil::unique_cotaskmem_string source;
                CHECK_FAILURE(args->get_Source(&source));
                wil::unique_cotaskmem_string webMessageAsJson;
                CHECK_FAILURE(args->get_WebMessageAsJson(&webMessageAsJson));
                Sleep(200);
                set_jsmsg_to_copydata(hWnd, webMessageAsJson.get());
                return S_OK;
            })
            .Get(),
        &token);

    std::wstring script = g_startup_script;
    if (4 < m_windowobjs[idx].startup_script.length())
        script = m_windowobjs[idx].startup_script;

    m_windowobjs[idx].webviewWindow->AddScriptToExecuteOnDocumentCreated(script.c_str(),
        Callback<ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>(
            [](HRESULT error, PCWSTR id) -> HRESULT {
                return S_OK;
            })
            .Get());
    EventRegistrationToken m_DOMContentLoadedToken;
    wil::com_ptr<ICoreWebView2_2> webView2;
    m_windowobjs[idx].webviewWindow->QueryInterface(IID_PPV_ARGS(&webView2));
    m_windowobjs[idx].webView2 = webView2;
    webView2->add_DOMContentLoaded(
        Callback<ICoreWebView2DOMContentLoadedEventHandler>(
            [hWnd](ICoreWebView2* sender, ICoreWebView2DOMContentLoadedEventArgs* args) -> HRESULT {
                std::wstring msgstr = L"{\"msg\"\:\"DOMContentLoaded\"}";
                set_jsmsg_to_copydata(hWnd, msgstr);
                return S_OK;
            })
            .Get(),
        &m_DOMContentLoadedToken);
}
void init_webview2(HWND hWnd, std::wstring url2str)
{
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hWnd, url2str](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                env->CreateCoreWebView2Controller(hWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [hWnd, url2str](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            wil::com_ptr<ICoreWebView2> webviewWindow;
                            if (controller != nullptr) {
                                auto webviewController = controller;
                                webviewController->get_CoreWebView2(&webviewWindow);
                                int idx = get_current_windowobj_idx(hWnd);
                                m_windowobjs[idx].webviewController = controller;
                                m_windowobjs[idx].webviewWindow = webviewWindow;
                            }
                            ICoreWebView2Settings* Settings;
                            webviewWindow->get_Settings(&Settings);
                            Settings->put_IsScriptEnabled(TRUE);
                            Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                            Settings->put_IsWebMessageEnabled(TRUE);

                            resize_webview(hWnd);
                            webview_events(hWnd);
                            load_url(hWnd, url2str.c_str());
                            return S_OK;
                        })
                        .Get());
                return S_OK;
            })
            .Get());
}
bool create_webview2(HWND hWnd, int createid, const TCHAR* url)
{
    if (hWnd == NULL)
        return false;

    windowobj wobj;
    wobj.hwnd = hWnd;
    wobj.webviewWindow = nullptr;
    wobj.webviewController = nullptr;
    wobj.webView2 = nullptr;
    wobj.startup_script = L"";
    wobj.createid = createid;
    m_windowobjs.push_back(wobj);

    std::wstring url2str = url;
    init_webview2(hWnd, url2str);
    return true;
}

