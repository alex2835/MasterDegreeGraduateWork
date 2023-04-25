#pragma once

#include "load_data.hpp"
#include "bin.hpp"

#include <imgui.h>
#include <implot.h>

class UnfoldingApp
{
	InputData mInputData;
	Bins mBins;
	dfVec mHistogram;
	dfVec mProbabilities;
	dfMat mMigrationMat;

	int mMaxDims;

	struct UIData
	{
		int mBinsNum;
		int mDims;
		BinningType mBinningType;

		std::string mFilePath;
		bool mStop = false;
		bool mRebinning = true;
	};
	UIData mUIData;

public:
	void Init();
	void Update();
	void Draw();
	void DrawTopBar();
	bool Stop();
};
