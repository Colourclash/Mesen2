#pragma once

#include "pch.h"

class IImGuiDraw
{
public:
	virtual ~IImGuiDraw() {}
	virtual void Draw() = 0;
};