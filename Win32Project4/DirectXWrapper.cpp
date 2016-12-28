#include "stdafx.h"
#include "DirectXWrapper.h"

#include <new>

using namespace DirectX;

struct ConstantBuffer
{
	XMMATRIX mProjection;
	XMFLOAT3 mViewDir;
	float accumulator;
};

DirectXWrapper* DirectXWrapper::GetDirectXWrapper(HWND hwnd, HINSTANCE hinst)
{
	DirectXWrapper *ret = (DirectXWrapper*)_aligned_malloc(sizeof(DirectXWrapper), 16); 
	new (ret)DirectXWrapper(hwnd, hinst);
	return ret; 
}

void DirectXWrapper::DeleteDirectXWrapper(DirectXWrapper *pWrapper)
{
	pWrapper->~DirectXWrapper(); 
	_aligned_free(pWrapper);
}

// Create DirectX device and swap chains

DirectXWrapper::DirectXWrapper(HWND hwnd, HINSTANCE hinst) : ourWindowHandle(hwnd), ourInstance(hinst)
{
	initialised = false;

	CoInitialize(NULL); // needed by the WIC loader

	HRESULT hr = S_OK;

	// get the rectangular size of our window
	RECT rc;
	GetClientRect(ourWindowHandle, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	// determine some device creation flags
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// attempt to cover all bases
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,  // HW rasteriser
		D3D_DRIVER_TYPE_WARP,      // SW rasteriser
		D3D_DRIVER_TYPE_REFERENCE, // SW raseriser
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	// This loop enumerates attempts to create a device starting with HW - if it succeeds it breaks
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &pd3dDevice, &featureLevel, &pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &pd3dDevice, &featureLevel, &pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to Create a DirectX device"); // we print debug messages if anything goes wrong, at least then there's something in the debug log
		return; // couldn't start DirectX
	}

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	// QueryInterface is a feature of COM that gets us a pointer or reference to an Interface.
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to obtain DXGI factory");
		return;
	}

	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&pd3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&pImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(pd3dDevice, ourWindowHandle, &sd, nullptr, nullptr, &pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&pSwapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = ourWindowHandle;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(pd3dDevice, &sd, &pSwapChain);
	}

	dxgiFactory->Release();

	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create Swap Chain");
		return;
	}

	// Create a render target view - this allows us to bind the backbuffer of the swap chain as a render target - we render to the backbuffer
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to get back buffer");
		return;
	}

	hr = pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create render target vies");
		return;
	}

	pImmediateContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

	// Setup the viewport - the viewport sets up the clip space used in the final stages of projection
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pImmediateContext->RSSetViewports(1, &vp);

	// Create the constant buffer used in the vertex shader
	D3D11_BUFFER_DESC bd;
	memset(&bd, 0, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = pd3dDevice->CreateBuffer(&bd, nullptr, &pConstantBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create constant buffer");
		return;
	}

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = pd3dDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create depth stencil texture");
		return;
	}

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = pd3dDevice->CreateDepthStencilView(pDepthStencil, &descDSV, &pDepthStencilView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create depth stencil view");
		return;
	}

	initialised = true; // we succeeded in initialising DirectX
}

DirectXWrapper::~DirectXWrapper()
{
	if (pImmediateContext) pImmediateContext->ClearState();
	if (pConstantBuffer) pConstantBuffer->Release();
	if (pRenderTargetView) pRenderTargetView->Release();
	if (pSwapChain1) pSwapChain1->Release();
	if (pSwapChain) pSwapChain->Release();
	if (pImmediateContext1) pImmediateContext1->Release();
	if (pImmediateContext) pImmediateContext->Release();
	if (pd3dDevice1) pd3dDevice1->Release();
	if (pd3dDevice) pd3dDevice->Release();
	if (pDepthStencil) pDepthStencil->Release();
	if (pDepthStencilView) pDepthStencilView->Release();
}

#define ACCUMULATOR_INCREMENT (0.001f)

void DirectXWrapper::Render()
{
	if (!initialised)
		return; // we're not initialised

	accumulator += ACCUMULATOR_INCREMENT;
	if (accumulator > 1.0f)
		accumulator -= 1.0f;

	// clear the backbuffer
	pImmediateContext->ClearRenderTargetView(pRenderTargetView, Colors::DarkSlateGray);

	// Clear the depth buffer to 1.0 (max depth)
	pImmediateContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// get the rectangular size of our window
	RECT rc;
	GetClientRect(ourWindowHandle, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	// projection matrix - recalculate each frame in case window dimensions changed
	projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(90), width / (FLOAT)height, 0.01f, 100.0f);
	//projMatrix = XMMatrixOrthographicLH((float)8.889f, (float)5.0f, 0.01f, 100.0f);

	// iterate all the renderable objects and call their render function
	for (unsigned int i = 0; i < objectList.size(); i++)
	{
		objectList[i]->Render(this);
	}

	// Swap the foreground and background buffers and let DirectX do some housekeeping
	pSwapChain->Present(0, 0);
}

void DirectXWrapper::AddRenderableObject(IRenderable *pObject)
{
	objectList.push_back(pObject);
	// call the init
	pObject->Initialise(this);
}

void DirectXWrapper::RemoveRenderableObject(IRenderable *pObject)
{
	for (unsigned int i = 0; i < objectList.size(); i++)
	{
		if (objectList[i] == pObject)
		{
			objectList.erase(objectList.begin() + i);
			break;
		}
	}
}

void DirectXWrapper::SetOrbitCamera(Vector3 &lookat, float yaw, float pitch, float distance)
{
	// build a matrix starting with the rotation around the world origin
	viewMatrix = XMMatrixRotationRollPitchYaw(-pitch, -yaw, 0); 
	// apply the inverse of the translation as this is a view matrix, bear in mind we are now in post view rotation space so moving along -ve z moves us away from the 
	// rotated view transform in the direction of the camera.
	viewMatrix = viewMatrix * XMMatrixTranslation(-lookat.x, -lookat.y, -(lookat.z - distance));
}

// This function both needs to set the current position and direction of the camera - plus needs to apply first person camera space motion to it.
// What this means is that if we need to move the camera right in X, we need to do it in the direction the camera is facing, not in World X.

void DirectXWrapper::SetFirstPersonCamera(Vector3 &pos, float yaw, float pitch, float roll, Vector3 &movement)
{
	// create a matrix with the rotation in it
	XMMATRIX rot = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	// use the matrix to figure out where the coordinate axes should point - we have to use proper 4D homogeneous coordinates here so we convert 3D coordinates to homogeneous 4D
	XMVECTOR xAxis = XMLoadFloat3(&Vector3(1, 0, 0));
	XMVECTOR yAxis = XMLoadFloat3(&Vector3(0, 1, 0));
	XMVECTOR zAxis = XMLoadFloat3(&Vector3(0, 0, 1));
	// rotate our standard axes and see where they point
	xAxis = XMVector3Transform(xAxis, rot);
	yAxis = XMVector3Transform(yAxis, rot);
	zAxis = XMVector3Transform(zAxis, rot);
	// we need to move the camera by "movement" in camera space so let's do that first
	XMVECTOR pos4 = XMLoadFloat3(&pos); // load pos into a 4d homogeneous version, we don't care about w at this point but we need to do this in order to make the maths library happy
	pos4 += movement.x * xAxis; // move this much in x 
	pos4 += movement.y * yAxis; // etc
	pos4 += movement.z * zAxis;
	// now we calculate the view transform - we're going to use a DirectX helper function to do this
	XMVECTOR lookAt = pos4 + zAxis; // the lookat pos is our world space position plus the direction of the z axis
	viewMatrix = XMMatrixLookAtLH(pos4, lookAt, yAxis);
	// calculate and store the view dir
	viewDir = XMVector3Normalize(lookAt - pos4)*-1; // we need to invert it as the lighting calculation actually expects the inverse
	// finally we'll copy our updated pos4 back into the passed pos
	XMStoreFloat3(&pos, pos4);
}

void DirectXWrapper::SetWorldTransform(DirectX::XMMATRIX &worldMatrix)
{
	// the actual final matrix we send to the vertex shader
	XMMATRIX finalMatrix = worldMatrix * viewMatrix * projMatrix;

	// update constant buffer containing our matrix
	ConstantBuffer cb;
	cb.mProjection = XMMatrixTranspose(finalMatrix); // this transpose is because HLSL vertex shaders expect column major and DX works in row major
	XMStoreFloat3(&cb.mViewDir, viewDir);
	cb.accumulator = accumulator;

	// send it to the shaders
	pImmediateContext->UpdateSubresource(pConstantBuffer, 0, nullptr, &cb, 0, 0);
	pImmediateContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	pImmediateContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);
}

void DirectXWrapper::EnableZBuffer(bool enable)
{
	if (enable)
		pImmediateContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);
	else
		pImmediateContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);
}
