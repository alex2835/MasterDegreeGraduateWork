
#include "UnfoldingApp.hpp"
#include "Utils.hpp"


void UnfoldingApp::Init()
{
	mInputData = LoadData( { "res/sim_p_2.txt" } );
	mSpitedRows = SplitRowsIntoBins( mInputData, BIN_SIZE );
}

void UnfoldingApp::Update()
{
}

void UnfoldingApp::Draw()
{
	ImGui::Begin( "Histogram" );
	for( auto rows : { &mInputData.mExp, &mInputData.mSim } )
	{
		if( ImPlot::BeginPlot( ( "##Histograms" + rows->front().mName ).c_str() ) )
		{
			ImPlot::SetupAxes( NULL, NULL, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit );
			ImPlot::SetNextFillStyle( IMPLOT_AUTO_COL, 0.5f );

			for( auto row : *rows )
				ImPlot::PlotHistogram( row.mName.c_str(), row.mData.data(), static_cast<int>( 1000 ),
					200, 2.0, ImPlotRange( row.mMin, 10 ) );
			ImPlot::EndPlot();
		}
	}
	ImGui::End();


	ImGui::Begin( "Bars" );
	for( const auto& bins : mSpitedRows )
	{
		//auto& bins = mSpitedRows.front();

		std::vector<float> xs;
		for( Float i = 0; i < bins.mBins.size(); i += 0.1f )
			xs.push_back( i );

		//static double xs[51], ys1[51], ys2[51];
		//for( int i = 0; i < 51; ++i ) {
		//	xs[i] = i * 0.02;
		//	ys1[i] = 1.0 + 0.5 * sin( 25 * xs[i] ) * cos( 2 * xs[i] );
		//	ys2[i] = 0.5 + 0.25 * sin( 10 * xs[i] ) * sin( xs[i] );
		//}

		if( ImPlot::BeginPlot( ( "##StemPlots" + bins.mRowRef.mName ).c_str() ) )
		{
			ImPlot::SetupAxisLimits( ImAxis_X1, 0, (Float)bins.mBins.size() / 20 );
			ImPlot::SetupAxisLimits( ImAxis_Y1, 0, 0.1 );
			ImPlot::SetNextMarkerStyle( ImPlotMarker_Circle );

			//ImPlot::PlotStems( bins.mRowRef.mName.c_str(), xs.data(), bins.mBins.data(), (Int)bins.mBins.size() );
			ImPlot::PlotStems( bins.mRowRef.mName.c_str(), xs.data(), bins.mPropobilities.data(), (Int)bins.mPropobilities.size() );

			//ImPlot::PlotStems( "Stems 1", xs, ys1, 51 );
		   //ImPlot::SetNextMarkerStyle( ImPlotMarker_Circle );
		   //ImPlot::PlotStems( "Stems 2", xs, ys2, 51 );
			ImPlot::EndPlot();
		}
	}
	ImGui::End();

	ImPlot::ShowDemoWindow();
}

