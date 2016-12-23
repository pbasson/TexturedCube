#pragma once // idempotent pragma 

#include "IRenderable.h" // interface to renderable objects that we create

// This wrapper provides simplified access to initialising DirectX and other functionality
// There's some ugly details in here so you aren't expected to understand it yet, some comments are provided for the curious

class DirectXWrapper
{
public:
	// factory methods to create and delete the object - this isn't a singleton, however we have to control allocation on the heap for alignment reasons
	static DirectXWrapper* GetDirectXWrapper(HWND hwnd, HINSTANCE hinst);
	static void DeleteDirectXWrapper(DirectXWrapper *pWrapper);

	void Render(); 

	// public accessors
	bool					IsInitialised() { return initialised; }
	ID3D11Device*			GetDevice() { return pd3dDevice; } 
	ID3D11DeviceContext*	GetContext() { return pImmediateContext; }

	// add and remove IRenderable objects
	void AddRenderableObject(IRenderable *pObject);
	void RemoveRenderableObject(IRenderable *pObkect);

	// Set up an orbiting camera looking at this point with these euler angles and at this distance
	void SetOrbitCamera(Vector3 &lookat, float yaw, float pitch, float distance);

	// Set up a first person camera using the specified yaw, pitch, roll and position, finally move it locally by move
	void SetFirstPersonCamera(Vector3 &pos, float yaw, float pitch, float roll,Vector3 &movement);

	// Set the World Transform for a batch of geometry
	void SetWorldTransform(DirectX::XMMATRIX &worldMatrix);

	// Disable or enable the Z buffer
	void EnableZBuffer(bool enable);

private:
	DirectXWrapper(); // hide constructors as we need to use the factory method
	DirectXWrapper(HWND hwnd, HINSTANCE hinst);
	~DirectXWrapper();

	// some DirectX handles - note we're initialising these in the class definition, this is a feature of C++ 11 and is cleaner than using the constructor
	D3D_DRIVER_TYPE						driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL					featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*						pd3dDevice = nullptr;
	ID3D11Device1*						pd3dDevice1 = nullptr;
	ID3D11DeviceContext*				pImmediateContext = nullptr;
	ID3D11DeviceContext1*				pImmediateContext1 = nullptr;
	IDXGISwapChain*						pSwapChain = nullptr;
	IDXGISwapChain1*					pSwapChain1 = nullptr;
	ID3D11RenderTargetView*				pRenderTargetView = nullptr;
	ID3D11Buffer*						pConstantBuffer = nullptr;
	ID3D11Texture2D*                    pDepthStencil = nullptr;
	ID3D11DepthStencilView*             pDepthStencilView = nullptr;

	HWND ourWindowHandle;
	HINSTANCE ourInstance;

	// camera matrices - forcing 16 byte alignment
	__declspec(align(16))
	DirectX::XMMATRIX                viewMatrix;
	DirectX::XMMATRIX                projMatrix;
	DirectX::XMVECTOR				 viewDir;
	bool initialised;

	// an STD vector of IRenderable objects - the objects we render each frame
	// STD vectors are convenient resizable arrays, this saves having to worry about if the array is large enough
	vector<IRenderable*>	objectList;

	// a slowly increasing number we're going to use for animation
	float accumulator = 0;
};

