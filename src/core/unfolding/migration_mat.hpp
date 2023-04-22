#pragma once

#include "bin.hpp"
#include "load_data.hpp"
#include <format>

inline Mat CalculateMigrationMat( Bins& bins )
{
	size_t mat_size = (size_t)std::pow( bins.mBins.size(), bins.mSize.size() );
	auto mat = CreateSqrMat( mat_size );
	
	size_t bins_size = 0;
	for( auto& bin : bins.mBins )
		bins_size += bin.Size();

	for( size_t i = 0; i < bins.mBins.size(); i++ )
	{
		auto& bin = bins.mBins[i];
		for( auto [exp, sim] : bin )
		{
			size_t sim_idx = FromMultidimentionalIdx( bins.GetBinByValue( sim ).mIdx, bins.mSize );
			mat[i][sim_idx]++;
		}

		for( size_t j = 0; j < mat_size; j++ )
		{
			auto value = mat[i][j] / Float( bins_size );
			mat[i][j] = value > 0.00001f ? value : 0.0f;
		}
	}
	return mat;
}