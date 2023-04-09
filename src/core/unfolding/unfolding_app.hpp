#pragma once

#include "load_data.hpp"
#include "bin.hpp"

#include <imgui.h>
#include <implot.h>

class UnfoldingApp
{
	InputData mInputData;
	std::vector<Bins> mSpitedRows;
	Mat mMigrationMat;
public:
	void Init();
	void Update();
	void Draw();
};
