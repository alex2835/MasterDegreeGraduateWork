#pragma once

#include "bin.hpp"
#include "load_data.hpp"
#include <format>

inline dfMat CalculateMigrationMat( Bins& bins )
{
	size_t mat_size = bins.mBins.size();
	auto mat = CreateSqrMat( mat_size );

	for( size_t i = 0; i < mat_size; i++ )
	{
		auto& bin = bins.mBins[i];
		for( auto [sim, exp] : bin )
		{
			size_t exp_idx = FromMultidimentionalIdx( bins.GetBinByValue( exp ).mIdx, bins.mSize );
			mat[exp_idx][i]++;
		}
	}
	for( size_t j = 0; j < mat_size; j++ )
	{
		double amount = 0;
		for( size_t i = 0; i < mat_size; i++ )
			amount += mat[i][j];
		for( size_t i = 0; i < mat_size; i++ )
			mat[i][j] /= amount ? amount : 1.0;
	}
	return mat;
}