
#include "bin.hpp"
#include <ranges>
#include <algorithm>
#include <stdexcept>

std::pair<sfVec, sfVec> GetMinMax( const std::span<sfVec> exp,
								   const std::span<sfVec> sim,
								   size_t dims )
{
	sfVec min( dims, std::numeric_limits<Float>::max() );
	sfVec max( dims, -std::numeric_limits<Float>::max() );
	for( size_t i = 0; i < exp.size(); i++ )
	{
		auto s = sim[i];
		auto e = exp[i];
		for( size_t j = 0; j < dims; j++ )
		{
			min[j] = std::min( std::min( e[j], s[j] ), min[j] );
			max[j] = std::max( std::max( e[j], s[j] ), max[j] );
		}
	}
	return { min, max };
}

Bins FixedSizeBinning( const std::span<sfVec> exp,
					   const std::span<sfVec> sim,
					   size_t dims,
					   size_t bins_count )
{
	if( bins_count < 1 )
		std::runtime_error( "Invalid binning size" );

	auto [min, max] = GetMinMax( exp, sim, dims );
	sfVec step = ( max - min ) / (Float)bins_count;

	//std::cout << "min" << std::format("{}", min) << std::endl;
	//std::cout << "max" << std::format("{}", max) << std::endl;

	Bins bins;
	bins.mSize = siVec( dims, bins_count );				

	for( size_t i = 0; i < std::pow( bins_count, dims ); i++ )
	{
		siVec multi_dim_idx = ToMultidimentionalIdx( i, dims, bins_count );
		siVec next = multi_dim_idx + 1;

		//std::cout << std::format( "{}", multi_dim_idx ) << std::endl;
		//std::cout << std::format( "{}", next ) << std::endl;

		bins.PutBin( Bin{ multi_dim_idx,
						  min + step * multi_dim_idx.cast<Float>(),
						  min + step * next.cast<Float>() } );
	}
	bins.mBins.front().mBegin = min;
	bins.mBins.back().mEnd = max;

	for( size_t i = 0; i < exp.size(); i++ )
		bins.PutInBin( std::make_pair( exp[i], sim[i] ) );
	
	return  bins;
}

Bins CalculateBins( const std::span<sfVec> exp, 
					const std::span<sfVec> sim,
					size_t dims,
					BinningType type,
					Int bins_count )
{
	if( exp.size() == 0 || sim.size() == 0 )
		throw std::runtime_error( "Input data are empty" );

	// Split into bins
	switch( type )
	{
	case BinningType::Static:
		return FixedSizeBinning( exp, sim, dims, bins_count );
		break;
	case BinningType::Dynamic:
		std::cout << "Dynamic binning not implemented yet" << std::endl;
		return FixedSizeBinning( exp, sim, dims, bins_count );
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
