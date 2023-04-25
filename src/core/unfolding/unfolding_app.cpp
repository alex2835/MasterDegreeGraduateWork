
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
			raw[i * m.rows() + j] = m[i][j];
	return raw;
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

dfVec CalculateHistogram( Bins& bins )
{
	dfVec hist;
	hist.setlength( bins.mBins.size() );

	size_t i = 0;
	for( auto& bin : bins.mBins )
		hist[i++] = (Float)bin.Size();
	return hist;
}

dfVec CalculateProbabilities( Bins& bins )
{
	dfVec probabilities;
	probabilities.setlength( bins.mBins.size() );

	size_t size = 0;
	for( auto& bin : bins.mBins )
		size += bin.Size();

	size_t i = 0;
	for( auto& bin : bins.mBins )
		probabilities[i++] = (Float)bin.Size() / size;
	return probabilities;
}


void UnfoldingApp::Init()
{
	mInputData = LoadData( { "res/sim_p_6.txt" } );
	mMaxDims = (int)mInputData.mCols.size() / 2;
	mUIData.mBinningType = BinningType::Static;
	mUIData.mBinsNum = BIN_SIZE;
	mUIData.mDims = mMaxDims;

	//size_t parts = 2;
	//auto splited_sim = SplitData( ToSpan( mInputData.mSim ), parts );
	//auto splited_exp = SplitData( ToSpan( mInputData.mExp ), parts );

	//
	//mHistogram = CalculateHistogram( mBins );
	//mProbabilities = CalculateProbabilities( mBins );
	//mMigrationMat = CalculateMigrationMat( mBins );
	//auto[ u,s,vt ] = SVD( mMigrationMat );

	//std::cout << "probabilities: ";
	//for( int i = 0; i < mProbabilities.length(); i++ )
	//	std::cout << mProbabilities[i] << " ";

	//std::cout << "\nu\n" << u.tostring( 3 ) << std::endl;
	//std::cout << "s\n" << s.tostring( 3 ) << std::endl;
	//std::cout << "Vt\n" << vt.tostring( 3 ) << std::endl;
}

bool UnfoldingApp::Stop()
{
	return mUIData.mStop;
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
		}
		catch( const std::exception& e ) {
			std::cerr << e.what() << std::endl;
		}
		mUIData.mFilePath.clear();
	}

	// Binning
	if( mUIData.mRebinning )
	{
		mBins = CalculateBins( ToSpan( mInputData.mExp ), 
							   ToSpan( mInputData.mSim ),
							   mUIData.mDims,
							   mUIData.mBinningType,
							   mUIData.mBinsNum );
		mUIData.mRebinning = false;
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
				mUIData.mStop = true;
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

	// Histogram
	ImGui::Begin( "Histogram" );
	{
		if( mInputData.mCols.empty() )
			ImGui::Text( "Input data not selected" );
		else
		{
			ImGui::SliderInt( "Dims", &mUIData.mDims, 1, mMaxDims );
			ImGui::Text( "Data distribution projection" );
			
			const auto& cols = mInputData.mCols;
			for( size_t i = 0; i < cols.size() && i < mUIData.mDims * 2; i+=2 )
			{
				auto dim = i / 2;

				if( ImPlot::BeginPlot( std::format( "##Histogram{}", dim ).c_str() ) )
				{
					auto size = cols[dim].mData.size();

					ImPlot::SetupAxes( NULL, NULL, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit );
					ImPlot::SetNextFillStyle( IMPLOT_AUTO_COL, 0.5f );
					ImPlot::SetNextFillStyle( ImVec4{ 0.7f, 0.4f, 0.1f, 0.9f }, 0.4f );
					auto exp_name = std::format( "Exp dim{}", dim );
					ImPlot::PlotHistogram( exp_name.c_str(), cols[i].mData.data(), (int)size, ImPlotBin_Scott );

					ImPlot::SetNextFillStyle( ImVec4{ 0.3f, 0.4f, 0.7f, 0.9f }, 0.4f );
					auto sim_name = std::format( "Sim dim{}", dim );
					ImPlot::PlotHistogram( sim_name.c_str(), cols[i+1].mData.data(), (int)size, ImPlotBin_Scott );
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

		if( ImGui::SliderInt( "Bins", &mUIData.mBinsNum, 0, MAX_BIN_SIZE ) )
			mUIData.mRebinning = true;
		if( ImGui::Combo( "Binning type", (int*)&mUIData.mBinningType, "static\0dynamic", 2 ) )
			mUIData.mRebinning = true;

		// 1D projection
		auto projections = CaclucateBinningProjections( mBins, mUIData.mDims );
		for( size_t dim = 0; dim < mUIData.mDims; dim++ )
		{
			if( ImPlot::BeginPlot( std::format( "##Binning{}", dim ).c_str() ) )
			{
				ImPlot::SetNextMarkerStyle( ImPlotMarker_Circle );
				ImPlot::SetNextFillStyle( ImVec4{ 0.3f, 0.4f, 0.7f, 0.9f }, 0.4f );

				ImPlot::PlotStems( std::format( "1D Projection dim: {}", dim ).c_str(),
								   projections.exp_xs[dim].data(),
								   projections.exp_ys[dim].data(),
								   (int)projections.exp_xs[dim].size() );
				ImPlot::EndPlot();
			}
		}

		// 2D projection
		for( size_t dim = 0; dim < mUIData.mDims; dim++ )
		{
			auto second_dim = ( dim + 1 ) % mUIData.mDims;
			auto x_size = mBins.mSize[dim];
			auto y_size = mBins.mSize[second_dim];
			std::vector<Float> hmap( x_size * y_size );

			for( auto& bin : mBins )
				hmap[bin.mIdx[dim] * x_size + bin.mIdx[second_dim]] += (Float)bin.Size();

			if( ImPlot::BeginPlot( std::format( "##BinningHeat{}", dim ).c_str() ) )
			{
				ImPlot::PlotHeatmap( std::format( "2D Projection dims: {} {}", dim, second_dim ).c_str(),
									 hmap.data(),
									 (int)x_size,
									 (int)y_size );
				ImPlot::EndPlot();
			}

			//if( mUIData.mDims == 2 )
			//{
			//	std::vector<Float> xs;
			//	std::vector<Float> ys;
			//	for( auto& bin : mBins )
			//	{
			//		for( auto& pair : bin )
			//		{
			//			xs.push_back( pair.first[0] );
			//			ys.push_back( pair.first[1] );
			//		}
			//	}
			//	if( ImPlot::BeginPlot( "##BinningPoints" ) )
			//	{
			//		ImPlot::PlotScatter( "sss", xs.data(), ys.data(), (int)xs.size() );
			//		ImPlot::EndPlot();
			//	}
			//}
		}
		ImGui::End();
	}


	//ImGui::Begin( "Comp" );
	//if( ImPlot::BeginPlot( "##StemPlots" ) )
	//{
	//	ImPlot::SetupAxisLimits( ImAxis_X1, 0, 1.0f );
	//	ImPlot::SetupAxisLimits( ImAxis_Y1, 0, 0.1 );
	//	{
	//		auto& bins = mSpitedRows[0];
	//		std::vector<float> xs;
	//		for( Float i = 0; i < bins.mBins.size(); i += 0.1f )
	//			xs.push_back( i );

	//		ImPlot::SetNextMarkerStyle( ImPlotMarker_Diamond );
	//		//ImPlot::PlotStems( bins.mRowRef.mName.c_str(), xs.data(), bins.mBins.data(), (Int)bins.mBins.size() );
	//		ImPlot::PlotStems( bins.mRowRef.mName.c_str(), xs.data(), bins.mPropobilities.data(), (Int)bins.mPropobilities.size() );
	//	}
	//	{
	//		auto& bins = mSpitedRows[1];
	//		std::vector<float> xs;
	//		for( Float i = 0; i < bins.mBins.size(); i += 0.1f )
	//			xs.push_back( i );

	//		ImPlot::SetNextMarkerStyle( ImPlotMarker_Circle );
	//		//ImPlot::PlotStems( bins.mRowRef.mName.c_str(), xs.data(), bins.mBins.data(), (Int)bins.mBins.size() );
	//		ImPlot::PlotStems( bins.mRowRef.mName.c_str(), xs.data(), bins.mPropobilities.data(), (Int)bins.mPropobilities.size() );
	//	}
	//	ImPlot::EndPlot();
	//}
	//ImGui::End();
		

	ImGui::Begin( "Migration" );
	static ImPlotColormap map = ImPlotColormap_Viridis;
	ImGui::SameLine();
	ImGui::LabelText( "##Colormap Index", "%s", "Change Colormap" );
	ImGui::SetNextItemWidth( 300 );
	ImPlot::PushColormap( map );

	if( ImPlot::BeginPlot( "##Heatmap1", ImVec2( -1, -1 ), ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText ) )
	{
		//ImPlot::SetupAxes( NULL, NULL, axes_flags, axes_flags );
		//ImPlot::SetupAxisTicks( ImAxis_X1, 0 + 1.0 / 14.0, 1 - 1.0 / 14.0 );
		//ImPlot::SetupAxisTicks( ImAxis_Y1, 1 - 1.0 / 14.0, 0 + 1.0 / 14.0 );
		ImPlot::PlotHeatmap( "heat", GetMatData(mMigrationMat).data(), (int)mMigrationMat.rows(), (int)mMigrationMat.cols(), 0.0, 0.5, "%g", ImPlotPoint(0, 0), ImPlotPoint(1, 1));
		ImPlot::EndPlot();
	}
	ImGui::End();

	ImPlot::ShowDemoWindow();
}

