#ifndef UNICODE
#define UNICODE
#endif 


// headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // win32
#include <d3d11.h> // D3D 11
#include <dxgi.h> // graphics devices and swap chain
#include <assert.h> // for assert()
#include <stdint.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <vector>
#include <WICTextureLoader.h>
#include "Vertex.h"
#include <chrono>
#include <dxgidebug.h>
#include <cmath>

// libraries
#pragma comment( lib, "user32" ) // win32 stuff
#pragma comment( lib, "gdi32" ) // graphics device interface
#pragma comment( lib, "d3d11" ) // Direct3D 11
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler (need this later)
#pragma comment( lib, "DirectXTK.lib")
#pragma comment(lib, "dxguid.lib")


namespace dx = DirectX;
using Clock = std::chrono::steady_clock;

static const int windowWidth = 640;
static const int windowHeight = 480;


float rotationAngle(float angle);


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND InitWindow(HINSTANCE hInstance);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	float angle = 0.0f;

	// One full rotation every 12 seconds.
	const float angularSpeed = dx::XM_2PI / 12.0f;
	float deltaTime = 0;

	HRESULT hr = CoInitialize(nullptr);
	assert(SUCCEEDED(hr));

	// Direct3D device and swap chain.
	ID3D11Device *device_Ptr = nullptr; //Returns the address of a pointer to an ID3D11Device object that represents the device created. If this parameter is NULL, no ID3D11Device will be returned'.
	ID3D11DeviceContext *device_Context_Ptr = nullptr; //Returns the address of a pointer to an ID3D11DeviceContext object that represents the device context. If this parameter is NULL, no ID3D11DeviceContext will be returned.
	IDXGISwapChain *swap_Chain_Ptr = nullptr; //Returns the address of a pointer to the IDXGISwapChain object that represents the swap chain used for rendering.
	

	ID3D11RasterizerState* rasterizer_State_Ptr;


	// Render target view for the back buffer of the swap chain.
	ID3D11RenderTargetView *render_Target_View_Ptr = nullptr; //Address of a pointer to an ID3D11RenderTargetView. Set this parameter to NULL to validate the other input parameters (the method will return S_FALSE if the other input parameters pass validation).
	
	ID3D11Texture2D* framebuffer;


	MSG msg = {};
	bool shouldClose = false;
	HWND hwnd = InitWindow(hInstance);

	DXGI_SWAP_CHAIN_DESC swap_Chain_Descr = { 0 }; //Describes a swap chain.
	swap_Chain_Descr.BufferDesc.RefreshRate.Numerator = 0;
	swap_Chain_Descr.BufferDesc.RefreshRate.Denominator = 1;
	swap_Chain_Descr.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //32-bit unsigned-normalized-integer format that supports 8 bits for each color channel and 8-bit alpha.
	swap_Chain_Descr.SampleDesc.Count = 1;
	swap_Chain_Descr.SampleDesc.Quality = 0;
	swap_Chain_Descr.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_Chain_Descr.BufferCount = 1;
	swap_Chain_Descr.OutputWindow = hwnd;
	swap_Chain_Descr.Windowed = true;

	//-----------------------------------------------------

	D3D_FEATURE_LEVEL feature_Level;
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED; // Use this flag if your application will only call methods of Direct3D 11 interfaces from a single thread
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&swap_Chain_Descr,
		&swap_Chain_Ptr,
		&device_Ptr,
		&feature_Level,
		&device_Context_Ptr);
	assert(S_OK == hr && swap_Chain_Ptr && device_Ptr && device_Context_Ptr);

	//---------------------------------------

	hr = swap_Chain_Ptr->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&framebuffer);
	assert(SUCCEEDED(hr));

	hr = device_Ptr->CreateRenderTargetView(framebuffer, 0, &render_Target_View_Ptr);
	assert(SUCCEEDED(hr));
	framebuffer->Release();

	//----------------------------------

	flags = D3DCOMPILE_ENABLE_STRICTNESS; // Forces strict compile, which might not allow for legacy syntax.
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif
	ID3DBlob *vs_Blob_Ptr = nullptr, *ps_Blob_Ptr = nullptr, *error_Blob = nullptr;

	// COMPILE VERTEX SHADER
	hr = D3DCompileFromFile(
		L"shaders.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VS_Main",
		"vs_5_0",
		flags,
		0,
		&vs_Blob_Ptr,
		&error_Blob);

	if (FAILED(hr)) {
		if (error_Blob) {
			OutputDebugStringA((char*)error_Blob->GetBufferPointer());
			error_Blob->Release();
		}
		if (vs_Blob_Ptr) { vs_Blob_Ptr->Release(); }
		assert(false);
	}

	// COMPILE PIXEL SHADER
	hr = D3DCompileFromFile(
		L"shaders.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PS_Main",
		"ps_5_0",
		flags,
		0,
		&ps_Blob_Ptr,
		&error_Blob);
	if (FAILED(hr)) {
		if (error_Blob) {
			OutputDebugStringA((char*)error_Blob->GetBufferPointer());
			error_Blob->Release();
		}
		if (ps_Blob_Ptr) { ps_Blob_Ptr->Release(); }
		assert(false);
	}

	//--------------------------------------------------------

	ID3D11VertexShader *vertex_Shader_Ptr = nullptr;
	ID3D11PixelShader *pixel_Shader_Ptr = nullptr;

	hr = device_Ptr->CreateVertexShader(
		vs_Blob_Ptr->GetBufferPointer(),
		vs_Blob_Ptr->GetBufferSize(),
		nullptr,
		&vertex_Shader_Ptr);
	assert(SUCCEEDED(hr));

	hr = device_Ptr->CreatePixelShader(
		ps_Blob_Ptr->GetBufferPointer(),
		ps_Blob_Ptr->GetBufferSize(),
		nullptr,
		&pixel_Shader_Ptr);
	assert(SUCCEEDED(hr));

	ID3D11InputLayout *input_Layout_Ptr = nullptr;
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		//{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXTURE",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},


	};

	hr = device_Ptr->CreateInputLayout(
		inputElementDesc,
		ARRAYSIZE(inputElementDesc),
		vs_Blob_Ptr->GetBufferPointer(),
		vs_Blob_Ptr->GetBufferSize(),
		&input_Layout_Ptr);
	assert(SUCCEEDED(hr));

	//--------------------------------------------------------

	ID3D11SamplerState *sampler_State_Ptr; //is used to specify all of the parameters that are used when a shader program samples a texture resource.

	D3D11_SAMPLER_DESC sampler_State_Desc = {};
	sampler_State_Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_State_Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; //Tile the texture at every (u,v) integer junction. For example, for u values between 0 and 3, the texture is repeated three times.
	sampler_State_Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_State_Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_State_Desc.ComparisonFunc = D3D11_COMPARISON_NEVER;// A function that compares sampled data against existing sampled data.  //Never pass the comparison.
	sampler_State_Desc.MinLOD = 0;
	sampler_State_Desc.MaxLOD = D3D11_FLOAT32_MAX; // no limit

	hr = device_Ptr->CreateSamplerState(&sampler_State_Desc, &sampler_State_Ptr);
	assert(SUCCEEDED(hr));


	ID3D11ShaderResourceView *myTexture_Ptr; //A shader-resource-view interface specifies the subresources a shader can access during rendering

	hr = dx::CreateWICTextureFromFile(device_Ptr, L"../Texture/bth.jpg", nullptr, &myTexture_Ptr);


	assert(SUCCEEDED(hr));


	//--------------------------------------------------------
	
	Vertex vertex_Data_Array[] =
	{
		{-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,0.0f,-1.0f},// point at bottom-left
		{-0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f,0.0f,-1.0f},// point at top-left
		{0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f,-1.0f},// point at top-right
	    {0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f,0.0f,-1.0f},// point at bottom-right
	};

	//bind to pipline
	UINT vertex_Stride = sizeof(Vertex); // spacing of the elements in the array
	UINT vertex_Offset = 0;
	UINT vertex_Count = (UINT)std::size(vertex_Data_Array);

	//--------------------------------------------------------

	ID3D11Buffer *vertex_Buffer_Ptr = nullptr;
	{ /*** load mesh data into vertex buffer **/
		D3D11_BUFFER_DESC vertex_Buff_Descr = {};
		vertex_Buff_Descr.ByteWidth = sizeof(vertex_Data_Array);
		vertex_Buff_Descr.Usage = D3D11_USAGE_IMMUTABLE; // helps the driver optimisation, based on how often the buffer values will change
		vertex_Buff_Descr.BindFlags = D3D11_BIND_VERTEX_BUFFER; //binding a buffer of type to the BindFlags

		D3D11_SUBRESOURCE_DATA sr_Data = { 0 }; // points to our array of floats
		sr_Data.pSysMem = vertex_Data_Array;
		HRESULT hr = device_Ptr->CreateBuffer(&vertex_Buff_Descr, &sr_Data, &vertex_Buffer_Ptr);
		assert(SUCCEEDED(hr));//gives us a pointer to a vertex buffer.
	}
	 

	//--------------------------------------------------------
	const unsigned int indices_Data_Array[] =
	{
		0,1,2,
		0,2,3,
	};


	ID3D11Buffer *index_Buffer_Ptr;
	
	D3D11_BUFFER_DESC index_Buffer_Descr = {};
	index_Buffer_Descr.ByteWidth = sizeof(indices_Data_Array);
	index_Buffer_Descr.Usage = D3D11_USAGE_DEFAULT;
	index_Buffer_Descr.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_Buffer_Descr.StructureByteStride = sizeof(unsigned int);

	D3D11_SUBRESOURCE_DATA sri_Data = {}; //is used to provide the initial data to be loaded into a resource
	sri_Data.pSysMem = indices_Data_Array;
	hr = device_Ptr->CreateBuffer(&index_Buffer_Descr, &sri_Data, &index_Buffer_Ptr);
	assert(SUCCEEDED(hr));
	
	//bind index buffer
	UINT indices_Count = (UINT)std::size(indices_Data_Array);
	device_Context_Ptr->IASetIndexBuffer(index_Buffer_Ptr, DXGI_FORMAT_R32_UINT, 0);

	//--------------------------------------------------------


	//--------------------------------------------------------

	dx::XMMATRIX world = dx::XMMatrixIdentity();

	dx::XMVECTOR cameraPosition = dx::XMVectorSet(0.0f,0.0f,-2.0f,0.0f);
	dx::XMVECTOR camTarget = dx::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f); // center
	dx::XMVECTOR camUp = dx::XMVectorSet(0.0f, 4.0f, 0.0f, 0.0f); // y up

	dx::XMMATRIX viewMatrix = dx::XMMatrixLookAtLH(cameraPosition, camTarget, camUp);

	float fieldOfViewD = 90.0f;
	float fieldOfView = (fieldOfViewD / 360.0f)*dx::XM_2PI;

	float aspectRatio = (float)windowWidth / (float)windowHeight;
	float nearZ = 0.1f;
	float farZ = 1000.0f;

	dx::XMMATRIX projectionMatrix = dx::XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, nearZ, farZ);

	dx::XMMATRIX worldViewProjection = world * viewMatrix * projectionMatrix;


	struct constantBufferCam
	{
		dx::XMMATRIX WVP;
		dx::XMMATRIX world;
		dx::XMMATRIX view;
		dx::XMMATRIX projection;

	};

	constantBufferCam cBufferCam;

	cBufferCam.WVP = worldViewProjection;
	cBufferCam.world = world;
	cBufferCam.view = viewMatrix;
	cBufferCam.projection = projectionMatrix;



	//--------------------------------------------------------

	ID3D11Buffer *constant_Buffer_Ptr_Cam = nullptr;
	{
		D3D11_BUFFER_DESC constant_Buff_Descr_Cam = {};
		constant_Buff_Descr_Cam.ByteWidth = sizeof(constantBufferCam);
		constant_Buff_Descr_Cam.Usage = D3D11_USAGE_DEFAULT;
		constant_Buff_Descr_Cam.BindFlags = D3D11_BIND_CONSTANT_BUFFER; //binding a buffer of type to the BindFlags

		D3D11_SUBRESOURCE_DATA csd_Data_Cam = { 0 }; // points to our array of floats
		csd_Data_Cam.pSysMem = &cBufferCam;
		HRESULT hr = device_Ptr->CreateBuffer(&constant_Buff_Descr_Cam, &csd_Data_Cam, &constant_Buffer_Ptr_Cam);
		assert(SUCCEEDED(hr));//gives us a pointer to a vertex buffer.
	}

	//--------------------------------------------------------
	//bind consant buffer
	device_Context_Ptr->VSSetConstantBuffers(1, 1, &constant_Buffer_Ptr_Cam);


	 
	//--------------------------------------------------------
	struct light
	{
		light() {};

		dx::XMFLOAT3 position;
		float pad1;
		dx::XMFLOAT3 ambientColor;
		float pad2;
		dx::XMFLOAT4 diffuse;
		dx::XMFLOAT3 direction;
		float pad3;
		dx::XMFLOAT3 attenuation;
		float pad4;
		dx::XMFLOAT3 cameraPosition;
		float pad5;
		dx::XMFLOAT3 specularColor;
		float pad6;


	};

	light lightObj;

	lightObj.position = dx::XMFLOAT3(0.0f, 1.0f, -2.0f);
	lightObj.ambientColor = dx::XMFLOAT3(0.1f, 0.1f, 0.1f);
	lightObj.diffuse = dx::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lightObj.direction = dx::XMFLOAT3(0.25f, 0.5f, -1.0f);
	lightObj.attenuation = dx::XMFLOAT3(0.6f, 0.3f, 0.1f);
	lightObj.cameraPosition = dx::XMFLOAT3(0.0f, 0.0f, -1.0f);
	lightObj.specularColor = dx::XMFLOAT3(1.0f, 1.0f, 1.0f);

	dx::XMStoreFloat3(&lightObj.cameraPosition, cameraPosition);


	//--------------------------------------------------------

	ID3D11Buffer *constant_Buffer_Ptr_Light = nullptr;
	{
		D3D11_BUFFER_DESC constant_Buff_Descr_Light = {};
		constant_Buff_Descr_Light.ByteWidth = sizeof(light);
		constant_Buff_Descr_Light.Usage = D3D11_USAGE_DEFAULT; // helps the driver optimisation, based on how often the buffer values will change
		constant_Buff_Descr_Light.BindFlags = D3D11_BIND_CONSTANT_BUFFER; //binding a buffer of type to the BindFlags

		D3D11_SUBRESOURCE_DATA csd_Data_Light = { 0 }; // points to our array of floats
		csd_Data_Light.pSysMem = &lightObj;
		HRESULT hr = device_Ptr->CreateBuffer(&constant_Buff_Descr_Light, &csd_Data_Light, &constant_Buffer_Ptr_Light);
		assert(SUCCEEDED(hr));//gives us a pointer to a vertex buffer.
	}

	//--------------------------------------------------------
	//bind consant buffer
	device_Context_Ptr->VSSetConstantBuffers(2, 1, &constant_Buffer_Ptr_Light);
	device_Context_Ptr->PSSetConstantBuffers(2, 1, &constant_Buffer_Ptr_Light);

	//--------------------------------------------------------

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthClipEnable = TRUE;

	hr = device_Ptr->CreateRasterizerState(
		&rasterizerDesc,
		&rasterizer_State_Ptr
	);

	assert(SUCCEEDED(hr));

	ShowWindow(hwnd, nCmdShow);

	auto previousTime = Clock::now();

	while (!shouldClose)
	{


		/**** handle user input and other window events ****/
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			break;
		}
		/*** TODO: RENDER A FRAME HERE WITH DIRECT3D ***/
		/* clear the back buffer to cornflower blue for the new frame */
		float backGround_Color[4] = {
			0x64 / 255.0f,0x95 / 255.0f,0xED / 255.0f,1.0f
		};
		device_Context_Ptr->ClearRenderTargetView(render_Target_View_Ptr, backGround_Color);


		// Update elapsed time.
		auto currentTime = Clock::now();
		deltaTime = std::chrono::duration<float>(currentTime - previousTime).count();
		previousTime = currentTime;

		angle = std::fmod(angle + angularSpeed * deltaTime,	dx::XM_2PI);

		// Rotate the quad around its own center.
		dx::XMMATRIX localRotation = dx::XMMatrixRotationY(angle);

		// Move the quad away from the origin.
		dx::XMMATRIX translation = dx::XMMatrixTranslation(1.0f, 0.0f, 0.0f);

		// Rotate the offset around the origin to create an orbit.
		dx::XMMATRIX orbitRotation = dx::XMMatrixRotationY(angle);

		cBufferCam.world = localRotation * translation * orbitRotation;
		cBufferCam.WVP = cBufferCam.world *	cBufferCam.view * cBufferCam.projection;

		device_Context_Ptr->UpdateSubresource(constant_Buffer_Ptr_Cam, 0, nullptr, &cBufferCam, 0, 0);

		// ClearDepthStencilView().

		//--------------------------------------------------------
		/**** Rasteriser state - set viewport area *****/
		RECT winRect;
		GetClientRect(hwnd, &winRect);

		D3D11_VIEWPORT viewPort;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;
		viewPort.Width = (FLOAT)(winRect.right - winRect.left);
		viewPort.Height = (FLOAT)(winRect.bottom - winRect.top);
		viewPort.MinDepth = 0;
		viewPort.MaxDepth = 1;

		device_Context_Ptr->RSSetViewports(1, &viewPort);


		//--------------------------------------------------------
		/**** Output Merger *****/
		device_Context_Ptr->OMSetRenderTargets(1, &render_Target_View_Ptr, nullptr);

		//--------------------------------------------------------
		/***** Input Assembler (map how the vertex shader inputs should be read from vertex buffer) ******/
		device_Context_Ptr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		device_Context_Ptr->IASetInputLayout(input_Layout_Ptr);
		device_Context_Ptr->IASetVertexBuffers(0, 1, &vertex_Buffer_Ptr, &vertex_Stride, &vertex_Offset);

		//--------------------------------------------------------
		/*** set vertex shader to use and pixel shader to use, and constant buffers for each ***/
		device_Context_Ptr->VSSetShader(vertex_Shader_Ptr, nullptr, 0);
		device_Context_Ptr->PSSetShader(pixel_Shader_Ptr, nullptr, 0);

		device_Context_Ptr->PSSetShaderResources(0, 1, &myTexture_Ptr);
		device_Context_Ptr->PSSetSamplers(0, 1, &sampler_State_Ptr);


		//--------------------------------------------------------
		/*** draw the vertex buffer with the shaders ****/
		device_Context_Ptr->RSSetState(rasterizer_State_Ptr);
		device_Context_Ptr->DrawIndexed(indices_Count,0 ,0);


		//--------------------------------------------------------
		/**** swap the back and front buffers (show the frame we just drew) ****/
		swap_Chain_Ptr->Present(1, 0);

	}

	// Unbind resources held by the device context.
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	device_Context_Ptr->OMSetRenderTargets(1, nullViews, nullptr);

	ID3D11ShaderResourceView* nullSRVs[] = { nullptr };
	device_Context_Ptr->PSSetShaderResources(0, 1, nullSRVs);

	ID3D11SamplerState* nullSamplers[] = { nullptr };
	device_Context_Ptr->PSSetSamplers(0, 1, nullSamplers);

	device_Context_Ptr->VSSetShader(nullptr, nullptr, 0);
	device_Context_Ptr->PSSetShader(nullptr, nullptr, 0);

	ID3D11Buffer* nullBuffers[] = { nullptr };
	device_Context_Ptr->IASetVertexBuffers(
		0, 1, nullBuffers,
		&vertex_Stride, &vertex_Offset);

	device_Context_Ptr->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer* nullConstantBuffers[] = { nullptr };
	device_Context_Ptr->VSSetConstantBuffers(1, 1, nullConstantBuffers);
	device_Context_Ptr->VSSetConstantBuffers(2, 1, nullConstantBuffers);
	device_Context_Ptr->PSSetConstantBuffers(2, 1, nullConstantBuffers);

	// Release buffers.
	if (vertex_Buffer_Ptr)
	{
		vertex_Buffer_Ptr->Release();
		vertex_Buffer_Ptr = nullptr;
	}

	if (index_Buffer_Ptr)
	{
		index_Buffer_Ptr->Release();
		index_Buffer_Ptr = nullptr;
	}
	if (constant_Buffer_Ptr_Cam)
	{
		constant_Buffer_Ptr_Cam->Release();
		constant_Buffer_Ptr_Cam = nullptr;
	}
	if (constant_Buffer_Ptr_Light)
	{
		constant_Buffer_Ptr_Light->Release();
		constant_Buffer_Ptr_Light = nullptr;
	}
	// Release shaders and input layout.
	if (vertex_Shader_Ptr)
	{
		vertex_Shader_Ptr->Release();
		vertex_Shader_Ptr = nullptr;
	}
	if (pixel_Shader_Ptr) 
	{
		pixel_Shader_Ptr->Release();
		pixel_Shader_Ptr = nullptr;
	}
	if (input_Layout_Ptr) 
	{
		input_Layout_Ptr->Release();
		input_Layout_Ptr = nullptr;
	}
	// Release texture and sampler.
	if (myTexture_Ptr)
	{
		myTexture_Ptr->Release();
		myTexture_Ptr = nullptr;
	}
	if (sampler_State_Ptr)
	{
		sampler_State_Ptr->Release();
		sampler_State_Ptr = nullptr;
	}

	// Release shader compilation blobs.
	if (vs_Blob_Ptr)
	{
		vs_Blob_Ptr->Release();
		vs_Blob_Ptr = nullptr;
	}
	if (ps_Blob_Ptr)
	{
		ps_Blob_Ptr->Release();
		ps_Blob_Ptr = nullptr;
	}
	if (error_Blob)
	{
		error_Blob->Release();
		error_Blob = nullptr;
	}
	// Release render target.
	if (render_Target_View_Ptr)
	{
		render_Target_View_Ptr->Release();
		render_Target_View_Ptr = nullptr;
	}
	// Release swap chain, context and device.
	if (swap_Chain_Ptr)
	{
		swap_Chain_Ptr->Release();
		swap_Chain_Ptr = nullptr;
	}
	if (rasterizer_State_Ptr)
	{
		rasterizer_State_Ptr->Release();
		rasterizer_State_Ptr = nullptr;
	}
	if (device_Context_Ptr)
	{
		device_Context_Ptr->Release();
		device_Context_Ptr = nullptr;
	}
	if (device_Ptr)
	{
		device_Ptr->Release();
		device_Ptr = nullptr;
	}

	CoUninitialize();

	return static_cast<int>(msg.wParam);
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX); // size in ram
	wcex.style = CS_HREDRAW | CS_VREDRAW; // window style
	wcex.lpfnWndProc = WndProc; // WndProc
	wcex.hInstance = hInstance; // Handle to the instance
	wcex.lpszClassName = L"BasicWindow"; // class name
	if (!RegisterClassEx(&wcex))
	{
		return nullptr;
	}

	RECT rc = { 0,0,windowWidth,windowHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindow
	(
		L"BasicWindow", // class name
		L"Hello Triangle", // window title
		WS_OVERLAPPEDWINDOW, //style
		CW_USEDEFAULT, // window position x
		CW_USEDEFAULT, // window position y
		rc.right - rc.left, // window width
		rc.bottom - rc.top, // window height
		nullptr, // no parent window
		nullptr, // no menu
		hInstance, // handle to instance
		nullptr // no params passed
	);

	return handle;

}


float rotationAngle(float inAngle)
{
	
	float angle = (inAngle / 360.0f)*dx::XM_2PI;
	return angle;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//HWND hwnd. The windows hangle
	//UINT message. The messages code
	//WPARAM wParam. Extra message info
	//LPARAM lParam. Extra message info

	switch (message)
	{

	case WM_LBUTTONUP:
	{
		int xPos = LOWORD(lParam);
		int yPos = HIWORD(lParam);
		MessageBox(nullptr, L"You Clicked the left mouse button", L"Important Notice", MB_OK);
	} break;

	case WM_RBUTTONUP:
	{
		int xPos = LOWORD(lParam);
		int yPos = HIWORD(lParam);
		MessageBox(nullptr, L"You Clicked the right mouse button", L"Important Notice", MB_OK);
	} break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
	} break;

	case WM_KEYDOWN:
	{
		if (wParam == 'A')
		{
			MessageBox(nullptr, L"You Pressed the A button", L"Important Notice", MB_OK);
		}
		else if (wParam == 'D')
		{
			MessageBox(nullptr, L"You Pressed the D button", L"Important Notice", MB_OK);
		}
		else if (wParam == 'W')
		{
			MessageBox(nullptr, L"You Pressed the W button", L"Important Notice", MB_OK);

		}
		else if (wParam == 'S')
		{
			MessageBox(nullptr, L"You Pressed the S button", L"Important Notice", MB_OK);

		}
		else
		{
			MessageBox(nullptr, L"You Pressed a button", L"Important Notice", MB_OK);
		}

	}break;

	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}
