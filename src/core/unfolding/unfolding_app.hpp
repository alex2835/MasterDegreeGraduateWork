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

	int mMaxDims;
	std::span<sfVec> mTrainingSim;
	std::span<sfVec> mTrainingExp;
	std::span<sfVec> mTestingSim;
	std::span<sfVec> mTestingExp;

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
		bool mUpdateErrorAxises = false;

		BinningProjections1D mProjections1D;
		BinningProjections2D mProjections2D;
		std::vector<Float> mMigrationRaw;
	};
	UIData mUIData;

public:
	using Application::Application;

	void Init() override;
	void Update() override;
	void Draw() override;
	void DrawTopBar();

private:
	void LoadData( const std::string& filename );
	void UpdateUIData();
	void TestWithoutUI();
};