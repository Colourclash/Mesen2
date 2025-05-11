#pragma once

class IImGuiDraw
{
public:
	virtual ~IImGuiDraw() {}
	virtual void Draw() = 0;
};