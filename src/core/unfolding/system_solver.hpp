#pragma once

#include "static_vector.hpp"
#include "bin.hpp"
#include "migration_mat.hpp"


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

//sfVec SolveSystem( dfMat& migration_mat, Bins& bins )
//{
//
//}