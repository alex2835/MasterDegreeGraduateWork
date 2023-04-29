#pragma once

#include "bin.hpp"
#include "load_data.hpp"
#include <format>

inline dfMat CalculateMigrationMat( Bins& bins )
{
	size_t mat_size = bins.mBins.size();
	auto mat = CreateSqrMat( mat_size );
	
	size_t bins_size = 0;
	for( auto& bin : bins.mBins )
		bins_size += bin.Size();

	for( size_t i = 0; i < mat_size; i++ )
	{
		auto& bin = bins.mBins[i];
		for( auto [sim, exp] : bin )
		{
			size_t sim_idx = FromMultidimentionalIdx( bins.GetBinByValue( exp ).mIdx, bins.mSize );
			mat[i][sim_idx]++;
		}
		for( size_t j = 0; j < mat_size; j++ )
			mat[i][j] = mat[i][j] / Float( bins_size );
	}
	return mat;
}