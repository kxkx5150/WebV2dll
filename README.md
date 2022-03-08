# WebView2dll for VC++ App

<br><br>

Windows10
Webview2 Runtime (required)  
https://developer.microsoft.com/en-us/microsoft-edge/webview2/#download-section  

<br>

Windows 11  
Webview2 Runtime installed by default
<br><br><br>

Webview2  
https://docs.microsoft.com/en-us/microsoft-edge/webview2/  

<br><br><br>

WebV2dll.h  
WebV2dll.lib  
WebV2dll.dll  
WebView2Loader.dll  

<br><br><br>

<pre>

#include "WebV2dll.h"  
#pragma comment(lib, "WebV2dll.lib")  
#define WM_JSMSG (WM_APP + 1)  

</pre>

<br>

<pre>

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);


   create_webview2(hWnd, 0, L"https://www.google.com/");
   //create_webview2(hWnd, 0, L"C:\\html\\index.html");

</pre>

<br>

<pre>

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    ...





    // add
    case WM_JSMSG: {
        COPYDATASTRUCT* copy_data_structure = { 0 };
        copy_data_structure = (COPYDATASTRUCT*)lParam;
        LPCWSTR arguments = (LPCWSTR)copy_data_structure->lpData;
        OutputDebugString(arguments);
    } break;

    case WM_SIZE: {
        resize_webview(hWnd);
    } break;





}

</pre>


