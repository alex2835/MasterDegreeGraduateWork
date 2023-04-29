
#include "unfolding_app.hpp"
#include "utils.hpp"
#include "bin.hpp"
#include "migration_mat.hpp"
#include "system_solver.hpp"
#include "imgui.h"
#include "ImGuiFileDialog.h"
#include <format>
#include <filesystem>

void UnfoldingApp::UpdateUIData()
{
	mUIData.mProjections1D = Caclucate1DBinningProjections( mBins, mBins.mSize.size() );
	mUIData.mProjections2D = Caclucate2DBinningProjections( mBins, mBins.mSize.size() );
}

void UnfoldingApp::TestWithoutUI()
{
	mInputData = LoadData( { "res/sim_p_6.txt" } );
	mMaxDims = (int)mInputData.mCols.size() / 2;
	mUIData.mBinningType = BinningType::Static;
	mUIData.mBinsNum = BIN_SIZE;
	mUIData.mDims = 2;
	mUIData.mDimShift = 0;

	size_t parts = 2;
	auto splited_exp = SplitData( ToSpan( mInputData.mExp ), parts );
	auto splited_sim = SplitData( ToSpan( mInputData.mSim ), parts );

	mBins = CalculateBins( splited_exp[0],
						   splited_exp[0],
						   mUIData.mDims,
						   mUIData.mDimShift,
						   mUIData.mBinningType,
						   mUIData.mBinsNum );
	mMigrationMat = CalculateMigrationMat( mBins );
	auto[ u,s,vt ] = SVD( mMigrationMat );

	mHistogram = CalculateSimHistogram( mBins );
	mProbabilities = CalculateProbabilities( mHistogram );

	std::cout << "probabilities: ";
	for( int i = 0; i < mProbabilities.length(); i++ )
		std::cout << mProbabilities[i] << " ";

	std::cout << "\nU\n" << u.tostring( 3 ) << std::endl;
	std::cout << "S\n" << s.tostring( 3 ) << std::endl;
	std::cout << "Vt\n" << vt.tostring( 3 ) << std::endl;
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
		mInputData = LoadData( { "res/sim_p_6.txt" } );
		mMaxDims = (int)mInputData.mCols.size() / 2;
		mUIData.mBinningType = BinningType::Static;
		mUIData.mBinsNum = BIN_SIZE;
		mUIData.mDims = mMaxDims;
		mUIData.mDimShift = 0;
	}
}

void UnfoldingApp::Update()
{
	// Data loading
	if( !mUIData.mFilePath.empty() )
	{
		try {
			mInputData = LoadData( { mUIData.mFilePath } );
			mMaxDims = (int)mInputData.mCols.size() / 2;
			mUIData.mDims = mMaxDims;
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
		mBins = CalculateBins( ToSpan( mInputData.mExp ), 
							   ToSpan( mInputData.mSim ),
							   mUIData.mDims,
							   mUIData.mDimShift,
							   mUIData.mBinningType,
							   mUIData.mBinsNum );
		mUIData.mRebinning = false;
		mUIData.mUpdateBinningAxises = true;
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
				ImGuiFileDialog::Instance()->OpenDialog( "ChooseFileDlgKey", "Choose File", ".txt", ".");
			if( ImGui::MenuItem( "Exit" ) )
				stop();
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
		if( ImGui::Combo( "Binning type", (int*)&mUIData.mBinningType, "static\0dynamic", 2 ) )
			mUIData.mRebinning = true;

		ImGui::Checkbox( "Migration mat values", &mUIData.mMibrationMatValues );
	}
	ImGui::End();


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
		for( size_t dim = 0; dim < mBins.mSize.size(); dim++ )
		{
			if( mUIData.mUpdateBinningAxises )
				ImPlot::SetNextAxesToFit();

			if( ImPlot::BeginPlot( std::format( "##Binning{}", dim ).c_str() ) )
			{
				ImPlot::SetNextMarkerStyle( ImPlotMarker_Circle );
				ImPlot::SetNextFillStyle( ImVec4{ 0.3f, 0.4f, 0.7f, 0.9f }, 0.4f );

				auto& projection1d = projections1d[dim];
				ImPlot::PlotStems( std::format( "1D Projection dim: {}", dim ).c_str(),
								   projection1d.bin_xs.data(),
								   projection1d.sim_ys.data(),
								   (int)projection1d.bin_xs.size() );
				ImPlot::EndPlot();
			}
		}
		// 2D projection
		auto& projections2d = mUIData.mProjections2D;
		if( mBins.mSize.size() > 1 )
		{
			for( size_t dim = 0; dim < mBins.mSize.size(); dim++ )
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
		mUIData.mUpdateBinningAxises = false;
		ImGui::End();
	}


	// Migration mat
	ImGui::Begin( "Migration" );
	static ImPlotColormap map = ImPlotColormap_Viridis;
	ImGui::SameLine();
	ImGui::LabelText( "##Colormap Index", "%s", "Change Colormap" );
	ImGui::SetNextItemWidth( 300 );
	ImPlot::PushColormap( map );

	if( ImPlot::BeginPlot( "##Heatmap1", ImVec2( -1, -1 ), ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText ) )
	{
		ImPlot::PlotHeatmap( "Migration mat",
							 GetMatData(mMigrationMat).data(),
							 (int)mMigrationMat.rows(),
							 (int)mMigrationMat.cols(),
							 0.0, 
							 1.0 / mMigrationMat.rows(),
							 mUIData.mMibrationMatValues? "%.3f" : NULL,
							 ImPlotPoint(0, 0),
							 ImPlotPoint(1, 1));
		ImPlot::EndPlot();
	}
	ImGui::End();

	ImPlot::ShowDemoWindow();
}

