#include "stdafx.h"
#include "Activity1.h"
#include "WindowsWrapper.h"
#include "DirectXWrapper.h"
#include "Mesh.h"
// Include Standard, Src Header, Win Wrapper, DirectXWrapper, Mesh


#define MOUSE_SCALING ((float)(2.0*M_PI/1000.0))	//Define Mouse Scaling
#define MOVEMENT_SPEED (0.5f/10.0f)			//Define Movement Speed


void CheckMovementKeys(Vector3 &movement)		//CheckMovementKeys Fuction
{
	// zero the movement vector
	movement.x = 0;
	movement.y = 0;
	movement.z = 0;

	// we use a Win32 API function GetKeyboardState which tells us the status of all the keys on the keyboard
	BYTE keys[256];
	if (!GetKeyboardState(keys))
		return;

	if ((keys[VK_UP] & 0x80) || (keys[0x57] & 0x80)) // up key or 'w' are pressed - move forward, forwards is Z for the camera
	{
		movement.z += MOVEMENT_SPEED;
	}
	if ((keys[VK_DOWN] & 0x80) || (keys[0x53] & 0x80)) // down key or 's' are pressed
	{
		movement.z -= MOVEMENT_SPEED;
	}
	if ((keys[VK_LEFT] & 0x80) || (keys[0x41] & 0x80)) // left key or 'a'
	{
		movement.x -= MOVEMENT_SPEED;
	}
	if ((keys[VK_RIGHT] & 0x80) || (keys[0x44] & 0x80)) // right key or 'd'
	{
		movement.x += MOVEMENT_SPEED;
	}
}


//Main Entry
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR    lpCmdLine, int nCmdShow)
{
	//Start with WindowsWrapper & DirectXWrapper pointer creation
	WindowsWrapper *ourWindow = new WindowsWrapper(1280, 720, L"Activity 1", L"WindowsWrapperClass");
	DirectXWrapper *ourDX = DirectXWrapper::GetDirectXWrapper(ourWindow->GetHwnd(), GetModuleHandle(NULL));
	
	//Create Mesh
	Mesh *mesh = new Mesh();
	ourDX->AddRenderableObject(mesh);
	Mesh *mesh1 = new Mesh();
	ourDX->AddRenderableObject(mesh1);
	mesh1->SetPosition(1, 4, 5);

	//Enable Depth
	ourDX->EnableZBuffer(true);
	
	//Camera Start Position
	Vector3 camPos = Vector3(5.0f, 3.0f, -3.0f);
	//Main Loop
	do
	{	
		//Mouse variable 
		POINT p; 
		GetCursorPos(&p);
		float yaw = (float)p.x*MOUSE_SCALING; //Yaw view - Rotation on Pitch
		float pitch = (float)p.y*MOUSE_SCALING - (float)M_PI / 2; //	
		
		if (pitch > (float)M_PI / 2)
			pitch = (float)M_PI / 2;

		Vector3 movement;
		CheckMovementKeys(movement);

		ourDX->SetFirstPersonCamera(camPos, yaw, pitch, 0, movement);
	
		ourWindow->Tick();
		ourDX->Render();
	
		Sleep(10);
	} while (!ourWindow->QuitRequested()); 
	
	return 0;
}
