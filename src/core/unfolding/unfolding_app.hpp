#pragma once

#include "load_data.hpp"
#include "bin.hpp"

#include <imgui.h>
#include <implot.h>

class UnfoldingApp
{
	InputData mInputData;
	Bins mBins;
	Mat mMigrationMat;
public:
	void Init();
	void Update();
	void Draw();
};
