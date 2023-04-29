#pragma once

#include "core/Application.hpp"
#include "load_data.hpp"
#include "bin.hpp"

#include <imgui.h>
#include <implot.h>

using App::Application;

class UnfoldingApp : public Application
{
	InputData mInputData;
	Bins mBins;
	Bins mBinsProjection;
	dfMat mMigrationMat;

	dfVec mHistogram;
	dfVec mProbabilities;
	int mMaxDims;

	struct UIData
	{
		int mBinsNum;
		int mDims;
		int mDimShift;
		BinningType mBinningType;

		std::string mFilePath;
		bool mMibrationMatValues = false;

		bool mRebinning = true;
		bool mUpdateBinningAxises = false;
	};
	UIData mUIData;

public:
	using Application::Application;

	void Init() override;
	void Update() override;
	void Draw() override;
	void DrawTopBar();

	void TestWithoutUI();
};
