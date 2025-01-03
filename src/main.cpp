#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"DXGI.lib")
#pragma comment(lib,"DirectXTK.lib")
#pragma comment(lib,"D3DCompiler.lib")

#define UNICODE
#define _UNICODE
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <comdef.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <iostream>
#include <string>
#include <vector>

#include "core.h"

std::wstring stringToWide(std::string str) {
    std::wstring wide_string(str.begin(), str.end());
    return wide_string;
}

void Log(std::string message) {
    std::string errormessage = "Error: " + message;
    MessageBoxA(NULL, errormessage.c_str(), "Error", MB_ICONERROR);
}

void LogHR(HRESULT hr, std::string message) {
    _com_error error(hr);
    std::wstring errormessage = L"Error: " + stringToWide(message) + L"\n" + error.ErrorMessage();
    MessageBoxW(NULL, errormessage.c_str(), L"Error", MB_ICONERROR);
}

void LogHRW(HRESULT hr, std::wstring message) {
    _com_error error(hr);
    std::wstring errormessage = L"Error: " + message + L"\n" + error.ErrorMessage();
    MessageBoxW(NULL, errormessage.c_str(), L"Error", MB_ICONERROR);
}

HWND handle = NULL; //Handle to this window
HINSTANCE hInstance = NULL; //Handle to application instance
std::string window_title = "";
std::wstring window_title_wide = L""; //Wide string representation of window title
std::string window_class = "";
std::wstring window_class_wide = L""; //Wide string representation of window class name
int width = 800;
int height = 640;

Microsoft::WRL::ComPtr<ID3D11Device> device;
Microsoft::WRL::ComPtr<ID3D11DeviceContext> devicecontext;
Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rendertargetview;
Microsoft::WRL::ComPtr<ID3D11InputLayout> inputlayout;
Microsoft::WRL::ComPtr<ID3D11Buffer> vertexbuffer;

struct AdapterData {
    IDXGIAdapter *adapter;
    DXGI_ADAPTER_DESC desc;
};
std::vector<AdapterData> adapters;

HRESULT CompileShader(LPCWSTR srcFile,LPCSTR entryPoint,LPCSTR profile,ID3DBlob** blob ) {
    if ( !srcFile || !entryPoint || !profile || !blob ) {
       return E_INVALIDARG;
    }
    *blob = nullptr;
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif
    const D3D_SHADER_MACRO defines[] = {
        "EXAMPLE_DEFINE", "1",
        NULL, NULL
    };
    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile( srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, &shaderBlob, &errorBlob );
    if ( FAILED(hr) ) {
        if ( errorBlob ) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        if ( shaderBlob ) {
           shaderBlob->Release();
        }
        return hr;
    }    
    *blob = shaderBlob;
    return hr;
}

struct VertexShader {
    Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
    Microsoft::WRL::ComPtr<ID3D10Blob> shaderbuffer;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputlayout;
};
VertexShader vertexshader;

struct PixelShader {
    Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
    Microsoft::WRL::ComPtr<ID3D10Blob> shaderbuffer;
};
PixelShader pixelshader;

void vertexShaderInitialize(VertexShader *vertexshader, Microsoft::WRL::ComPtr<ID3D11Device> device, std::wstring shaderpath, D3D11_INPUT_ELEMENT_DESC *layoutdesc, u32 numelements) {
    HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), vertexshader->shaderbuffer.GetAddressOf());
    if (FAILED(hr)) {
        std::wstring errormessage = L"Failed to load vertex shader";
        errormessage += shaderpath;
        LogHRW(hr, errormessage);
        exit(-1);
    }
    hr = device.Get()->CreateVertexShader(vertexshader->shaderbuffer->GetBufferPointer(), vertexshader->shaderbuffer->GetBufferSize(), NULL, vertexshader->shader.GetAddressOf());
    if (FAILED(hr)) {
        std::wstring errormessage = L"Failed to create vertex shader";
        errormessage += shaderpath;
        LogHRW(hr, errormessage);
        exit(-1);
    }
    hr = device.Get()->CreateInputLayout(layoutdesc, numelements, vertexshader->shaderbuffer->GetBufferPointer(), vertexshader->shaderbuffer->GetBufferSize(), vertexshader->inputlayout.GetAddressOf());
    if (FAILED(hr)) {
        std::wstring errormessage = L"Failed to create vertex shader input layout";
        errormessage += shaderpath;
        LogHRW(hr, errormessage);
        exit(-1);
    }
}

void pixelShaderInitialize(PixelShader *pixelshader, Microsoft::WRL::ComPtr<ID3D11Device> device, std::wstring shaderpath) {
    HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), pixelshader->shaderbuffer.GetAddressOf());
    if (FAILED(hr)) {
        std::wstring errormessage = L"Failed to load pixel shader";
        errormessage += shaderpath;
        LogHRW(hr, errormessage);
        exit(-1);
    }
    hr = device.Get()->CreatePixelShader(pixelshader->shaderbuffer->GetBufferPointer(), pixelshader->shaderbuffer->GetBufferSize(), NULL, pixelshader->shader.GetAddressOf());
    if (FAILED(hr)) {
        std::wstring errormessage = L"Failed to create pixel shader";
        errormessage += shaderpath;
        LogHRW(hr, errormessage);
        exit(-1);
    }
}

void initDX() {
    // HRESULT hr = D3D11CreateDeviceAndSwapChain(IDXGIAdapter *pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL *pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc, IDXGISwapChain **ppSwapChain, ID3D11Device **ppDevice, D3D_FEATURE_LEVEL *pFeatureLevel, ID3D11DeviceContext **ppImmediateContext)
    // if (adapters.size() > 0) {
    //     return adapters;
    // }
    Microsoft::WRL::ComPtr<IDXGIFactory> factory;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)factory.GetAddressOf());
    if(FAILED(hr)) {
        LogHR(hr, "Failed to create DXGIFactory to enumerate adapters.");
        exit(-1);
    }
    IDXGIAdapter *adapter;
    u32 index = 0;
    printf("enumerating adapters: \n");
    while(SUCCEEDED(factory->EnumAdapters(index, &adapter))) {
        AdapterData adapterdata = {.adapter = adapter};
        HRESULT hr = adapter->GetDesc(&adapterdata.desc);
        if (FAILED(hr)) {
            LogHR(hr, "Failed to get adapter description.");
            exit(-1);
        }
        printf("%ls\n", adapterdata.desc.Description);
        printf("%llu\n", adapterdata.desc.DedicatedVideoMemory);
        puts("");
        adapters.push_back(adapterdata);
        index += 1;
    }
    adapter = NULL;
    assert(adapters.size() > 0);
    u64 maxmemory = 0;
    for (i32 i = 0; i < adapters.size(); i++) {
        if (adapters[i].desc.DedicatedVideoMemory > maxmemory) {
            maxmemory = adapters[i].desc.DedicatedVideoMemory;
        }
    }
    printf("largest DedicatedVideoMemory = %llu\n", maxmemory);
    AdapterData chosenadapter = {};
    for (i32 i = 0; i < adapters.size(); i++) {
        if(adapters[i].desc.DedicatedVideoMemory == maxmemory) {
            chosenadapter = adapters[i];
        }
    }
    printf("chosenadapter = %p\n", chosenadapter.adapter);
    printf("chosenadapter description = %ls\n", chosenadapter.desc.Description);
    DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
    swapchaindesc.BufferDesc.Width = width;
    swapchaindesc.BufferDesc.Height = height;
    swapchaindesc.BufferDesc.RefreshRate.Numerator = 60;
    swapchaindesc.BufferDesc.RefreshRate.Denominator = 1;
    swapchaindesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchaindesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapchaindesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    swapchaindesc.SampleDesc.Count = 1;
    swapchaindesc.SampleDesc.Quality = 0;

    swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchaindesc.BufferCount = 1;
    swapchaindesc.OutputWindow = handle;
    swapchaindesc.Windowed = TRUE;
    swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapchaindesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    hr = D3D11CreateDeviceAndSwapChain(chosenadapter.adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, NULL, NULL, 0, D3D11_SDK_VERSION, &swapchaindesc, swapchain.GetAddressOf(), device.GetAddressOf(), NULL, devicecontext.GetAddressOf());
    if (FAILED(hr)) {
        LogHR(hr, "Failed to create Device & Swap Chain.");
        exit(-1);
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
    hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(backbuffer.GetAddressOf()));
    if (FAILED(hr)) {
        LogHR(hr, "Failed to get buffer from swap chain.");
        exit(-1);
    }

    hr = device->CreateRenderTargetView(backbuffer.Get(), NULL, rendertargetview.GetAddressOf());
    if (FAILED(hr)) {
        LogHR(hr, "Failed to create render target view from device.");
        exit(-1);
    }

    devicecontext->OMSetRenderTargets(1, rendertargetview.GetAddressOf(), NULL);

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = width;
    viewport.Height = height;

    devicecontext->RSSetViewports(1, &viewport);
}

void initShaders() {
    HRESULT hr;
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    u32 numelements = sizeofarray(layout);
    // ID3DBlob *vsBlob = nullptr;
    // hr = CompileShader(L"shaders/vertexshader.hlsl", "main", "vs_4_0_level_9_1", &vsBlob);
    // if (FAILED(hr)) {
    //     LogHR(hr, "Failed to compile shader.");
    //     exit(-1);
    // }
    vertexShaderInitialize(&vertexshader, device, L"shaders/vertexshader.cso", layout, numelements);
    hr = device->CreateInputLayout(layout, numelements, vertexshader.shaderbuffer->GetBufferPointer(), vertexshader.shaderbuffer->GetBufferSize(), inputlayout.GetAddressOf());
    if (FAILED(hr)) {
        LogHR(hr, "Failed to create input layout.");
        exit(-1);
    }
    pixelShaderInitialize(&pixelshader, device, L"shaders/pixelshader.cso");
}

struct Vertex {
    f32 x; f32 y; f32 z;
    f32 r; f32 g; f32 b;
};

void initScene() {
    Vertex verts[] = {
        {.5f, -.5f, 0.f, 1.f, 0.f, 0.f},
        {-.5f, -.5f, 0.f, 0.f, 1.f, 0.f},
        {0.f, .5f, 0.f, 0.f, 0.f, 1.f},
    };
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.Usage = D3D11_USAGE_DEFAULT;
    vertexbufferdesc.ByteWidth = sizeof(Vertex) * sizeofarray(verts);
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexbufferdesc.CPUAccessFlags = 0;
    vertexbufferdesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexbufferdata = {};
    vertexbufferdata.pSysMem = verts;

    HRESULT hr = device->CreateBuffer(&vertexbufferdesc, &vertexbufferdata, vertexbuffer.GetAddressOf());
    if (FAILED(hr)) {
        LogHR(hr, "Failed to create vertex buffer");
        exit(-1);
    }
}

void RenderFrame() {
    f32 bgcolor[] = {.1f, .1f, .1f, 1.f};
    bool vsync = true;
    devicecontext->ClearRenderTargetView(rendertargetview.Get(), bgcolor);
    //
    devicecontext->IASetInputLayout(vertexshader.inputlayout.Get());
    devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    devicecontext->VSSetShader(vertexshader.shader.Get(), NULL, 0);
    devicecontext->PSSetShader(pixelshader.shader.Get(), NULL, 0);

    u32 stride = sizeof(Vertex);
    u32 offset = 0;
    devicecontext->IASetVertexBuffers(0, 1, vertexbuffer.GetAddressOf(), &stride, &offset);

    devicecontext->Draw(3, 0);
    //
    swapchain->Present(vsync, NULL);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    // LogHR(E_INVALIDARG, "procgen");
	window_title = "dx11";
	window_title_wide = stringToWide(window_title);
	window_class = "windowclass";
	window_class_wide = stringToWide(window_class); //wide string representation of class string (used for registering class and creating window)

	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; 
	wc.lpfnWndProc = WindowProc; 
	wc.cbClsExtra = 0; 
	wc.cbWndExtra = 0; 
	wc.hInstance = hInstance; 
	wc.hIcon = NULL;   
	wc.hIconSm = NULL; 
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); 
	wc.hbrBackground = NULL; 
	wc.lpszMenuName = NULL; 
	wc.lpszClassName = window_class_wide.c_str(); 
	wc.cbSize = sizeof(WNDCLASSEX); 
	RegisterClassEx(&wc); 

	handle = CreateWindowEx(0, //Extended Windows style - we are using the default. For other options, see: https://msdn.microsoft.com/en-us/library/windows/desktop/ff700543(v=vs.85).aspx
		window_class_wide.c_str(), //Window class name
		window_title_wide.c_str(), //Window Title
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, //Windows style - See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms632600(v=vs.85).aspx
		0, //Window X Position
		0, //Window Y Position
		width, //Window Width
		height, //Window Height
		NULL, //Handle to parent of this window. Since this is the first window, it has no parent window.
		NULL, //Handle to menu or child window identifier. Can be set to NULL and use menu in WindowClassEx if a menu is desired to be used.
		hInstance, //Handle to the instance of module to be used with this window
		nullptr); //Param to create window

	if (handle == NULL) {
		LogHR(GetLastError(), "CreateWindowEX Failed for window: " + window_title);
	}

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(handle, SW_SHOW);
	SetForegroundWindow(handle);
	SetFocus(handle);

    initDX();
    initShaders();
    initScene();

    while(true) {
        // Handle the windows messages.
        MSG msg;
        ZeroMemory(&msg, sizeof(MSG)); // Initialize the message structure.

        while (PeekMessage(&msg, handle, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }

        RenderFrame();

        // Check if the window was closed
        if (msg.message == WM_NULL) {
            if (!IsWindow(handle)) {
                handle = NULL; //Message processing loop takes care of destroying this window
                UnregisterClass(window_class_wide.c_str(), hInstance);
                return false;
            }
        }
    }

    // destroy window
	if (handle != NULL) {
		UnregisterClass(window_class_wide.c_str(), hInstance);
		DestroyWindow(handle);
	}

	return 0;
}
