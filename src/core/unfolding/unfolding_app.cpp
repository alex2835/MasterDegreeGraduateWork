
#include "unfolding_app.hpp"
#include "utils.hpp"
#include "bin.hpp"
#include "migration_mat.hpp"
#include "system_solver.hpp"
#include "imgui.h"
#include "ImGuiFileDialog.h"
#include <format>
#include <filesystem>
#include <random>


inline std::vector<Float> GetMatRawData( const dfMat& m )
{
	std::vector<Float> raw( m.rows() * m.cols() );
	for( int i = 0; i < m.rows(); i++ )
		for( int j = 0; j < m.cols(); j++ )
			raw[i * m.rows() + j] = (Float)m[i][j];
	return raw;
}

void UnfoldingApp::UpdateUIData()
{
	mUIData.mProjections1D = Caclucate1DBinningProjections( mBins );
	mUIData.mProjections2D = Caclucate2DBinningProjections( mBins );
	mUIData.mMigrationRaw = GetMatRawData( mMigrationMat );
	mUIData.mSimTestHist = CalculateHistogram( mBins, mTestingSim, mUIData.mDimShift );
	mUIData.mExpTestHist = CalculateHistogram( mBins, mTestingExp, mUIData.mDimShift );
	mUIData.mSolution = SolveSystem( mMigrationMat, 
									 mBins,
									 mUIData.mSimTestHist,
									 mUIData.mNeighborsMatType,
									 mUIData.mAlpha + mUIData.mAlphaLow / 1000000,
									 mUIData.mDebugOuput );
}

void UnfoldingApp::LoadData( const std::string& filename )
{
	mInputData = InputData();

	mInputData = ::LoadData( { filename } );
	mMaxDims = (int)mInputData.mCols.size() / 2;
	mUIData.mDims = mMaxDims;
	mUIData.mDimShift = 0;

	size_t parts = 2;
	auto splited_sim = SplitData( ToSpan( mInputData.mSim ), parts );
	auto splited_exp = SplitData( ToSpan( mInputData.mExp ), parts );
	mTrainingSim = splited_sim[0];
	mTrainingExp = splited_exp[0];
	mTestingSim = splited_sim[1];
	mTestingExp = splited_exp[1];
}

void UnfoldingApp::LoadDataGaus( Float M, Float D )
{
	mInputData = InputData();

	std::random_device rd{};
	std::mt19937 gen{ rd() };
	std::normal_distribution<> d{ M, D };
	std::normal_distribution<> smear{ -3.5, 0.5 };

	Column sim( "sim" );
	Column exp( "exp" );
	mInputData.mSim.mNames.push_back( "sim" );
	mInputData.mExp.mNames.push_back( "exp" );

	for( size_t i = 0; i < 1000000; i++ )
	{
		auto exp_value = d( gen );
		auto sim_value = 0.5 * exp_value + smear( gen );
		sim.mData.push_back( sim_value );
		exp.mData.push_back( exp_value );
		mInputData.mSim.mData.push_back( sfVec{ sim_value } );
		mInputData.mExp.mData.push_back( sfVec{ exp_value } );
	}
	mInputData.mCols.push_back( std::move( sim ) );
	mInputData.mCols.push_back( std::move( exp ) );

	size_t parts = 2;
	auto splited_sim = SplitData( ToSpan( mInputData.mSim ), parts );
	auto splited_exp = SplitData( ToSpan( mInputData.mExp ), parts );
	mTrainingSim = splited_sim[0];
	mTrainingExp = splited_exp[0];
	mTestingSim = splited_sim[1];
	mTestingExp = splited_exp[1];

	mMaxDims = 1;
	mUIData.mDims = mMaxDims;
	mUIData.mDimShift = 0;
}


void UnfoldingApp::TestWithoutUI()
{
	LoadData( "res/sim_p_6.txt" );
	mMaxDims = (int)mInputData.mExp.Size();
	mUIData.mBinningType = BinningType::Static;
	mUIData.mBinsNum = 4;
	mUIData.mDims = 1;
	mUIData.mDimShift = 0;
	
	mBins = CalculateBins( mTrainingSim,
						   mTrainingExp,
						   mUIData.mDims,
						   mUIData.mDimShift,
						   mUIData.mBinningType,
						   mUIData.mBinsNum );
	mMigrationMat = CalculateMigrationMat( mBins );
	auto m = CalculateHistogram( mBins, mTrainingSim, mUIData.mDimShift );
	auto solution = SolveSystem( mMigrationMat, mBins, m, NeighborsMatType::NonbinaryStatistic, 0.1f, true );
}


void UnfoldingApp::Init()
{
	auto test_witnout_ui = false;
	if( test_witnout_ui )
	{
		TestWithoutUI();
		stop();
	}
	else
	{ 
		LoadData( "res/sim_p_6.txt" );
		//LoadDataGaus( 5, 2.5 );
		mMaxDims = (int)mInputData.mCols.size() / 2;
		mUIData.mBinningType = BinningType::Static;
		mUIData.mNeighborsMatType = NeighborsMatType::Binary;
		mUIData.mBinsNum = BIN_SIZE;
		mUIData.mDims = 1;
		mUIData.mDimShift = 0;
	}
}

void UnfoldingApp::Update()
{
	// Data loading
	if( !mUIData.mFilePath.empty() )
	{
		try {
			LoadData( mUIData.mFilePath );
			mUIData.mRebinning = true;
		}
		catch( const std::exception& e ) {
			std::cerr << e.what() << std::endl;
		}
		mUIData.mFilePath.clear();
	}

	// Binning
	if( mUIData.mRebinning )
	{
		// free space
		mBins = Bins();
		mMigrationMat = dfMat();

		// calculate
		mBins = CalculateBins( mTrainingSim,
							   mTrainingExp,
							   mUIData.mDims,
							   mUIData.mDimShift,
							   mUIData.mBinningType,
							   mUIData.mBinsNum );
		mUIData.mRebinning = false;
		mUIData.mUpdateBinningAxises = true;
		mUIData.mUpdateErrorAxises = true;
		mMigrationMat = CalculateMigrationMat( mBins );
		UpdateUIData();
	}
}

void UnfoldingApp::DrawTopBar()
{
	if( ImGui::BeginMainMenuBar() )
	{
		if( ImGui::BeginMenu( "File" ) )
		{
			if( ImGui::MenuItem( "Open" ) )
			{
				ImGuiFileDialog::Instance()->OpenDialog( "ChooseFileDlgKey", "Choose File", ".txt", "." );
				mUIData.mRebinning = true;
			}
			if( ImGui::MenuItem( "Exit" ) )
				stop();
			ImGui::EndMenu();
		}
		if( ImGui::BeginMenu( "Examples" ) )
		{
			if( ImGui::MenuItem( "Gaus" ) )
			{
				LoadDataGaus( 5, 2 );
				mUIData.mRebinning = true;
			}
			ImGui::EndMenu();
		}
		if( ImGui::BeginMenu( "Source code" ) )
		{
			char url[128] = "https://github.com/alex2835/MasterDegreeGraduateWork.git";
			ImGui::InputText( "", url, 128, ImGuiInputTextFlags_ReadOnly );
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	// Display file dialog
	if( ImGuiFileDialog::Instance()->Display( "ChooseFileDlgKey" ) )
	{
		if( ImGuiFileDialog::Instance()->IsOk() )
			mUIData.mFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
		ImGuiFileDialog::Instance()->Close();
	}
}

void UnfoldingApp::Draw()
{
	DrawTopBar();

	ImGui::Begin( "Controll panel" );
	{
		if( ImGui::SliderInt( "Dims", &mUIData.mDims, 1, mMaxDims ) )
			mUIData.mRebinning = true;

		if( ImGui::SliderInt( "DimShift", &mUIData.mDimShift, 0, mMaxDims - 1 ) )
			mUIData.mRebinning = true;

		if( ImGui::SliderInt( "Bins", &mUIData.mBinsNum, MIN_BIN_SIZE, MAX_BIN_SIZE ) )
			mUIData.mRebinning = true;
		if( ImGui::Combo( "Binning type", (int*)&mUIData.mBinningType, "static\0dynamic\0dynamic median\0hybrid", 4 ) )
			mUIData.mRebinning = true;

		if( ImGui::Combo( "Neighbors mat type", (int*)&mUIData.mNeighborsMatType, "binary\0nonbinary stat\0mass center", 3 ) )
			mUIData.mRebinning = true;
		
		if( ImGui::SliderFloat( "Alpha", &mUIData.mAlpha, 0.0f, 0.05f ) )
			mUIData.mRebinning = true;

		if( ImGui::SliderFloat( "AlphaLow", &mUIData.mAlphaLow, 0, 1000 ) )
			mUIData.mRebinning = true;

		if( ImGui::Checkbox( "Debug output", &mUIData.mDebugOuput ) )
			mUIData.mRebinning = true;

		ImGui::Checkbox( "Migration mat values", &mUIData.mMibrationMatValues );

		if( ImGui::Button( "rebuild", ImVec2( 100, 40 ) ) )
			mUIData.mRebinning = true;
	}
	ImGui::End();


	ImGui::StyleColorsLight();

	// Histogram
	ImGui::Begin( "Histogram" );
	{
		if( mInputData.mCols.empty() )
			ImGui::Text( "Input data not selected" );
		else
		{
			ImGui::Text( "Data distribution projection" );
			
			auto shift = mUIData.mDimShift * 2;
			const auto& cols = mInputData.mCols;
			for( size_t i = shift; i < mUIData.mDims * 2 + shift; i+=2 )
			{
				auto dim = i / 2;
				auto exp_id = i % cols.size();
				auto sim_id =  ( i + 1 ) % cols.size();

				if( ImPlot::BeginPlot( std::format( "##Histogram{}", dim ).c_str() ) )
				{
					auto size = cols[dim].mData.size();

					ImPlot::SetupAxes( NULL, NULL, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit );
					ImPlot::SetNextFillStyle( IMPLOT_AUTO_COL, 0.5f );
					ImPlot::SetNextFillStyle( ImVec4{ 0.7f, 0.4f, 0.1f, 0.9f }, 0.4f );
					auto exp_name = std::format( "{} dim:{}", cols[exp_id].mName, dim);
					ImPlot::PlotHistogram( exp_name.c_str(), cols[exp_id].mData.data(), (int)size, ImPlotBin_Scott );

					ImPlot::SetNextFillStyle( ImVec4{ 0.3f, 0.4f, 0.7f, 0.9f }, 0.4f );
					auto sim_name = std::format( "{} dim:{}", cols[sim_id].mName, dim );
					ImPlot::PlotHistogram( sim_name.c_str(), cols[sim_id].mData.data(), (int)size, ImPlotBin_Scott );
					ImPlot::EndPlot();
				}
			}
		}
	}
	ImGui::End();


	// Binning
	if( mInputData.mCols.empty() )
		ImGui::Text( "Input data not selected" );
	else
	{
		ImGui::Begin( "Binning" );
		// 1D projection
		auto& projections1d = mUIData.mProjections1D;
		for( size_t dim = 0; dim < mBins.Dims(); dim++ )
		{
			if( mUIData.mUpdateBinningAxises )
				ImPlot::SetNextAxesToFit();

			if( ImPlot::BeginPlot( std::format( "##Binning{}", dim ).c_str() ) )
			{
				ImPlot::SetNextMarkerStyle( ImPlotMarker_Circle );
				ImPlot::SetNextFillStyle( ImVec4{ 0.7f, 0.4f, 0.1f, 0.9f }, 0.4f );

				auto& projection1d = projections1d[dim];
				ImPlot::PlotStems( std::format( "1D Projection dim: {}", dim ).c_str(),
								   projection1d.bin_xs.data(),
								   projection1d.sim_ys.data(),
								   (int)projection1d.bin_xs.size() );
				ImPlot::EndPlot();
			}
		}
		mUIData.mUpdateBinningAxises = false;

		// 2D projection
		ImPlot::PushColormap( mUIData.mColorMap );
		auto& projections2d = mUIData.mProjections2D;
		if( mBins.Dims() > 1 )
		{
			for( size_t dim = 0; dim < mBins.Dims(); dim++ )
			{
				if( ImPlot::BeginPlot( std::format( "##BinningHeat{}", dim ).c_str() ) )
				{
					auto& projection2d = projections2d[dim];
					ImPlot::PlotHeatmap( std::format( "2D Projection dims: {} {}", dim, projection2d.second_dim ).c_str(),
										 projection2d.hmap.data(),
										 projection2d.x_size,
										 projection2d.y_size,
										 0.0,
										 0.0,
										 mUIData.mMibrationMatValues ? "%d" : NULL );
					ImPlot::EndPlot();
				}
			}
		}
		ImPlot::PopColormap();
		ImGui::End();
	}


	// Migration mat
	ImGui::Begin( "Migration" );
	{
		ImPlot::PushColormap( mUIData.mColorMap );

		if( ImPlot::ColormapButton( ImPlot::GetColormapName( mUIData.mColorMap ), ImVec2( -1, 0 ), mUIData.mColorMap ) )
			mUIData.mColorMap = ( mUIData.mColorMap + 1 ) % ImPlot::GetColormapCount();

		ImPlot::ColormapScale( "Color range", 0, 1, ImVec2( 0, -1 ) );
		ImGui::SameLine();

		if( ImPlot::BeginPlot( "##Heatmap", ImVec2( -1, -1 ), ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText ) )
		{
			ImPlot::PlotHeatmap( "Migration mat",
								 mUIData.mMigrationRaw.data(),
								 (int)mMigrationMat.rows(),
								 (int)mMigrationMat.cols(),
								 0.0,
								 1.0,
								 mUIData.mMibrationMatValues ? "%.3f" : NULL,
								 ImPlotPoint( 0, 0 ),
								 ImPlotPoint( 1, 1 ) );
			ImPlot::EndPlot();
		}
		ImPlot::PopColormap();
	}
	ImGui::End();


	// Errors
	ImGui::Begin( "Error" );
	{
		if( mUIData.mUpdateErrorAxises )
			ImPlot::SetNextAxesToFit();

		if( ImPlot::BeginPlot( "##OriginalError" ) )
		{
			auto& sim_hist = mUIData.mSimTestHist;
			auto& exp_hist = mUIData.mExpTestHist;

			std::vector<Float> xs;
			for( int i = 0; i < sim_hist.length(); i++ )
				xs.push_back( i );

			ImPlot::SetNextFillStyle( ImVec4{ 0.7f, 0.4f, 0.1f, 0.9f }, 0.4f );
			ImPlot::PlotBars( "sim hist", xs.data(), sim_hist.getcontent(), (int)sim_hist.length(), 0.4 );
			
			ImPlot::SetNextFillStyle( ImVec4{ 0.3f, 0.4f, 0.7f, 0.9f }, 0.4f );
			ImPlot::PlotBars( "exp hist", xs.data(), exp_hist.getcontent(), (int)exp_hist.length(), 0.4 );

			Float total_error = 0;
			std::vector<Float> ys;
			std::vector<Float> errors;
			for( int i = 0; i < sim_hist.length(); i++ )
			{
				Float e = std::pow( ( sim_hist[i] - exp_hist[i] ), 2 );
				ys.push_back( ( sim_hist[i] + exp_hist[i] ) / 2 );
				errors.push_back( e );
				total_error += e;
			}
			total_error /= (Float)xs.size();
			ImPlot::PlotErrorBars( "Sqr error", xs.data(), ys.data(), errors.data(), (int)xs.size() );
			ImPlot::EndPlot();

			ImGui::Text( "MSE %0.3f", total_error );
		}

		if( mUIData.mUpdateErrorAxises )
			ImPlot::SetNextAxesToFit();

		if( ImPlot::BeginPlot( "##SolutionError" ) )
		{
			auto& sim_hist = mUIData.mSimTestHist;
			auto& exp_hist = mUIData.mExpTestHist;
			auto& solution = mUIData.mSolution;
			
			std::vector<Float> xs;
			for( int i = 0; i < solution.length(); i++ )
				xs.push_back( i );

			ImPlot::SetNextFillStyle( ImVec4{ 0.7f, 0.4f, 0.1f, 0.9f }, 0.4f );
			ImPlot::PlotBars( "sim hist", xs.data(), sim_hist.getcontent(), (int)sim_hist.length(), 0.4 );

			ImPlot::SetNextFillStyle( ImVec4{ 0.3f, 0.4f, 0.7f, 0.9f }, 0.4f );
			ImPlot::PlotBars( "exp hist", xs.data(), exp_hist.getcontent(), (int)exp_hist.length(), 0.4 );

			ImPlot::SetNextFillStyle( ImVec4{ 0.3f, 0.7f, 0.5f, 0.9f }, 0.2f );
			ImPlot::PlotBars( "solution", xs.data(), solution.getcontent(), (int)solution.length(), 0.4 );

			Float total_error = 0;
			std::vector<Float> ys;
			std::vector<Float> errors;
			for( int i = 0; i < solution.length(); i++ )
			{
				Float e = std::pow( ( solution[i] - exp_hist[i] ), 2 );
				ys.push_back( ( solution[i] + exp_hist[i] ) / 2 );
				errors.push_back( e );
				total_error += e;
			}
			total_error /= (Float)xs.size();
			ImPlot::PlotErrorBars( "Sqr error", xs.data(), ys.data(), errors.data(), (int)xs.size() );
			ImPlot::EndPlot();
			ImGui::Text( "MSE %0.3f", total_error );
		}
		mUIData.mUpdateErrorAxises = false;
	}
	ImGui::End();


	// Singular values
	ImGui::Begin( "Singular valus" );
	{
		auto [U, s, Vt] = SVD( mMigrationMat );
		dfVec log;
		log.setlength( s.length() );
		std::vector<Float> xs;

		for( int i = 0; i < s.length(); i++ )
		{
			log[i] = std::log( s[i] );
			xs.push_back( i );
		}

		if( ImPlot::BeginPlot( "##Alpha" ) )
		{
			ImPlot::PlotBars( "alpha", xs.data(), s.getcontent(), (int)s.length(), 0.4 );
			ImPlot::EndPlot();
		}

		if( ImPlot::BeginPlot( "##Log" ) )
		{
			ImPlot::PlotBars( "Log", xs.data(), log.getcontent(), (int)s.length(), 0.4 );
			ImPlot::EndPlot();
		}
	}
	ImGui::End();

	//ImPlot::ShowDemoWindow();
}

