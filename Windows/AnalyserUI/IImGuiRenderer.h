#pragma once

#include "pch.h"

class IImGuiRenderer
{
public:
	virtual ~IImGuiRenderer() {}

	// Init and start thread
	virtual void Start() = 0;
	virtual void Draw() = 0;
};