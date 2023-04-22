
#include "unfolding_app.hpp"
#include "utils.hpp"
#include "matrix.hpp"
#include "migration_mat.hpp"
#include "svd.hpp"

std::vector<Float> GetMatData( const Mat& m )
{
	std::vector<Float> raw( m.rows() * m.cols() );
	for( int i = 0; i < m.rows(); i++ )
		for( int j = 0; j < m.cols(); j++ )
			raw[i * m.rows() + j] = m[i][j];
	return raw;
}

// S U V
std::tuple<Mat, dfVec, Mat> SVD( const Mat& A )
{
	Mat U;
	dfVec S;
	Mat Vt;
	alglib::rmatrixsvd( A, A.rows(), A.cols(), 2, 2, 2, S, U, Vt);

	std::cout << "u\n" << U.tostring(2) << std::endl;
	std::cout << "s\n" << S.tostring(2) << std::endl;
	std::cout << "Vt\n" << Vt.tostring(2) << std::endl;

	return { U, S, Vt };
}


void UnfoldingApp::Init()
{
	mInputData = LoadData( { "res/sim_p_2.txt" } );
	mBins = SplitRowsIntoBins( mInputData, BinningType::FixedSize, BIN_SIZE );
	mMigrationMat = CalculateMigrationMat( mBins );

	auto[ s, u, v ] = SVD( mMigrationMat );
}

void UnfoldingApp::Update()
{
}

void UnfoldingApp::Draw()
{
	//ImGui::Begin( "Histogram" );
	////for( auto rows : { &mInputData.mExp, &mInputData.mSim } )
	//{
	//	//if( ImPlot::BeginPlot( ( "##Histograms" + rows->front().mName ).c_str() ) )
	//	if( ImPlot::BeginPlot( "##Histograms" ) )
	//	{
	//		ImPlot::SetupAxes( NULL, NULL, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit );
	//		ImPlot::SetNextFillStyle( IMPLOT_AUTO_COL, 0.5f );
	//		ImPlot::StyleColorsDark();

	//		for( auto rows : { &mInputData.mSim, &mInputData.mExp } )
	//		for( auto row : *rows )
	//			ImPlot::PlotHistogram( row.mName.c_str(), row.mData.data(), static_cast<int>( 1000 ),
	//				200, 2.0, ImPlotRange( row.mMin, 10 ) );
	//		ImPlot::EndPlot();
	//	}
	//}
	//ImGui::End();


	//ImGui::Begin( "Bars" );
	//for( const auto& bins : mSpitedRows )
	//{
	//	std::vector<float> xs;
	//	for( Float i = 0; i < bins.mBins.size(); i += 0.1f )
	//		xs.push_back( i );

	//	if( ImPlot::BeginPlot( ( "##StemPlots" + bins.mSimRef.mName ).c_str() ) )
	//	{
	//		ImPlot::SetupAxisLimits( ImAxis_X1, 0, (Float)bins.mBins.size() / 20 );
	//		ImPlot::SetupAxisLimits( ImAxis_Y1, 0, 0.1 );
	//		ImPlot::SetNextMarkerStyle( ImPlotMarker_Circle );

	//		//ImPlot::PlotStems( bins.mRowRef.mName.c_str(), xs.data(), bins.mBins.data(), (Int)bins.mBins.size() );
	//		ImPlot::PlotStems( bins.mSimRef.mName.c_str(), xs.data(), bins.mPropobilities.data(), (Int)bins.mPropobilities.size() );

	//		//ImPlot::PlotStems( "Stems 1", xs, ys1, 51 );
	//	   //ImPlot::SetNextMarkerStyle( ImPlotMarker_Circle );
	//	   //ImPlot::PlotStems( "Stems 2", xs, ys2, 51 );
	//		ImPlot::EndPlot();
	//	}
	//}
	//ImGui::End();


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

