
#include "bin.hpp"
#include <ranges>
#include <algorithm>
#include <stdexcept>

std::pair<sfVec, sfVec> GetMinMax( const Rows& exp, const Rows& sim )
{
	sfVec min( exp[0].size() );
	sfVec max( exp[0].size() );
	for( size_t i = 0; i < sim.Size(); i++ )
	{
		auto s = sim[i];
		auto e = exp[i];
		for( size_t j = 0; j < s.size(); j++ )
		{
			min[j] = std::min( std::min( min[j], s[j] ), e[j] );
			max[j] = std::max( std::max( max[j], s[j] ), e[j] );
		}
	}
	return { min, max };
}

Bins FixedSizeBinning( const Rows& exp, const Rows& sim, size_t bins_count )
{
	if( bins_count < 1 )
		std::runtime_error( "Invalid binning size" );

	auto [min, max] = GetMinMax( exp, sim );
	sfVec step = ( max - min ) / (Float)bins_count;
	size_t dims = max.size();
	
	Bins bins;
	bins.mSize = siVec( dims, bins_count );

	for( size_t i = 0; i <= std::pow( bins_count, dims ) - dims; i++ )
	{
		if( i > 0 && i % bins_count == 0 )
			continue;

		siVec multi_dim_idx = ToMultidimentionalIdx( i, dims, bins_count );
		siVec next = multi_dim_idx + 1;

		bins.PutBin( Bin{ multi_dim_idx,
						  step * multi_dim_idx.cast<Float>(),
						  step * next.cast<Float>() } );
	}
	
	for( size_t i = 0; i < exp.Size(); i++ )
		bins.PutInBin( std::make_pair( exp[i], sim[i] ) );
	
	return  bins;
}

Bins SplitRowsIntoBins( const InputData& data, BinningType type, Int bins_count )
{
	if( data.mExp.Size() == 0 || data.mSim.Size() == 0 )
		throw std::runtime_error( "Input data are empty" );

	// Split into bins
	switch( type )
	{
	case BinningType::FixedSize:
		return FixedSizeBinning( data.mExp, data.mSim, bins_count );
		break;
	}
	throw std::runtime_error( "Invalid binning type" );
}



//std::pair<sfVec, sfVec> ShiftDimTransform( const std::pair<sfVec, sfVec>& pair,
//										   size_t shift_dim,
//										   size_t max_dim )
//{
//	sfVec f( pair.first.size() );
//	sfVec s( pair.first.size() );
//
//	for( size_t i = 0; i < f.size(); i++ )
//	{
//		f[( i + shift_dim ) % max_dim] = pair.first[i];
//		s[( i + shift_dim ) % max_dim] = pair.second[i];
//	}
//	return { f, s };
//}
