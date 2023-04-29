
#include "unfolding_app.hpp"
#include "utils.hpp"
#include "bin.hpp"
#include "migration_mat.hpp"
#include "imgui.h"
#include "ImGuiFileDialog.h"
#include <format>
#include <filesystem>

std::vector<Float> GetMatData( const dfMat& m )
{
	std::vector<Float> raw( m.rows() * m.cols() );
	for( int i = 0; i < m.rows(); i++ )
		for( int j = 0; j < m.cols(); j++ )
			raw[i * m.rows() + j] = (Float)m[i][j];
	return raw;
}

dfVec CalculateSimHistogram( Bins& bins )
{
	dfVec hist;
	hist.setlength( bins.mBins.size() );

	size_t i = 0;
	for( auto& bin : bins.mBins )
		hist[i++] = (Float)bin.Size();
	return hist;
}

dfVec CalculateExpHistogram( Bins& bins )
{
	dfVec hist;
	hist.setlength( bins.mBins.size() );

	for( auto& bin : bins.mBins )
	{
		for( const auto& sim_exp : bin )
		{
			auto md_idx = bins.GetBinByValue( sim_exp.second ).mIdx;
			auto idx = FromMultidimentionalIdx( md_idx, bins.mSize );
			hist[idx]++;
		}
	}
	return hist;
}

dfVec CalculateProbabilities( const dfVec& hist )
{
	dfVec probabilities;
	probabilities.setlength( hist.length() );

	Float size = 0;
	for( int i = 0; i < hist.length(); i++ )
		size += (Float)hist[i];
	
	for( int i = 0; i < hist.length(); i++ )
		probabilities[i] = hist[i] / size;
	return probabilities;
}

// U S Vt
std::tuple<dfMat, dfVec, dfMat> SVD( const dfMat A )
{
	dfMat U;
	dfVec S;
	dfMat Vt;
	alglib::rmatrixsvd( A, A.rows(), A.cols(), 2, 2, 2, S, U, Vt );
	return { U, S, Vt };
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
	//TestWithoutUI();

	mInputData = LoadData( { "res/sim_p_6.txt" } );
	mMaxDims = (int)mInputData.mCols.size() / 2;
	mUIData.mBinningType = BinningType::Static;
	mUIData.mBinsNum = BIN_SIZE;
	mUIData.mDims = mMaxDims;
	mUIData.mDimShift = 0;
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
		auto projections = Caclucate1DBinningProjections( mBins, mBins.mSize.size() );
		for( size_t dim = 0; dim < mBins.mSize.size(); dim++ )
		{
			if( mUIData.mUpdateBinningAxises )
				ImPlot::SetNextAxesToFit();

			if( ImPlot::BeginPlot( std::format( "##Binning{}", dim ).c_str() ) )
			{
				ImPlot::SetNextMarkerStyle( ImPlotMarker_Circle );
				ImPlot::SetNextFillStyle( ImVec4{ 0.3f, 0.4f, 0.7f, 0.9f }, 0.4f );

				ImPlot::PlotStems( std::format( "1D Projection dim: {}", dim ).c_str(),
								   projections.bin_xs[dim].data(),
								   projections.sim_ys[dim].data(),
								   (int)projections.bin_xs[dim].size() );
				ImPlot::EndPlot();
			}
		}
		// 2D projection
		if( mBins.mSize.size() > 1 )
		{
			for( size_t dim = 0; dim < mBins.mSize.size(); dim++ )
			{
				auto second_dim = ( dim + 1 ) % mBins.mSize.size();
				auto x_size = mBins.mSize[dim];
				auto y_size = mBins.mSize[second_dim];

				std::vector<int> hmap( x_size * y_size );
				for( auto& bin : mBins )
					hmap[bin.mIdx[dim] * x_size + bin.mIdx[second_dim]] += (int)bin.Size();

				if( ImPlot::BeginPlot( std::format( "##BinningHeat{}", dim ).c_str() ) )
				{
					ImPlot::PlotHeatmap( std::format( "2D Projection dims: {} {}", dim, second_dim ).c_str(),
										 hmap.data(),
										 (int)x_size,
										 (int)y_size,
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

