#pragma once

#include "core/Application.hpp"
#include "load_data.hpp"
#include "system_solver.hpp"
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
		NeighborsMatType mNeighborsMatType;
		bool mDebugOuput = false;
		float mAlpha = 0.001f;
		float mAlphaLow = 0.0001f;

		std::string mFilePath;
		bool mMibrationMatValues = false;

		bool mRebinning = true;
		bool mUpdateBinningAxises = false;
		bool mUpdateErrorAxises = false;

		BinningProjections1D mProjections1D;
		BinningProjections2D mProjections2D;
		std::vector<Float> mMigrationRaw;
		dfVec mSimTestHist;
		dfVec mExpTestHist;
		dfVec mSolution;
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
	void LoadDataGaus( Float M, Float D );
	void UpdateUIData();
	void TestWithoutUI();
};