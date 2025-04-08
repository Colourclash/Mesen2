#pragma once

#include "pch.h"

class IAnalyserUI
{
	public:
		virtual ~IAnalyserUI() {}
		virtual void Draw() = 0;
};