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
	void UpdateUIData();
	void TestWithoutUI();
};


inline std::vector<Float> GetMatData( const dfMat& m )
{
	std::vector<Float> raw( m.rows() * m.cols() );
	for( int i = 0; i < m.rows(); i++ )
		for( int j = 0; j < m.cols(); j++ )
			raw[i * m.rows() + j] = (Float)m[i][j];
	return raw;
}