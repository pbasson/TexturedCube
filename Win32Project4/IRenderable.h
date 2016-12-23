#pragma once

class DirectXWrapper;

class IRenderable
{
public:
	virtual void Initialise(DirectXWrapper *wrapper) = 0;
	virtual void Render(DirectXWrapper *wrapper) = 0;
};