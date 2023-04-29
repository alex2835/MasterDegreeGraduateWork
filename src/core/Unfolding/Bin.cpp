
#include "bin.hpp"
#include <ranges>
#include <algorithm>
#include <stdexcept>

std::pair<sfVec, sfVec> ShiftDimTransform( const std::pair<sfVec, sfVec>& pair,
										   size_t dims,
										   size_t shift_dims )
{
	sfVec f( dims );
	sfVec s( dims );
	for( size_t i = 0; i < dims; i++ )
	{
		f[i] = pair.first[( i + shift_dims ) % pair.first.size()];
		s[i] = pair.second[( i + shift_dims ) % pair.second.size()];
	}
	return { f, s };
}

std::pair<sfVec, sfVec> GetMinMax( const std::span<sfVec> exp,
								   const std::span<sfVec> sim )
{
	auto dims = exp.front().size();
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

Bins FixedSizeBinning( const std::span<sfVec> sim, 
					   const std::span<sfVec> exp,
					   size_t dims,
					   size_t dims_shift,
					   size_t bins_count )
{
	if( bins_count < 1 )
		std::runtime_error( "Invalid binning size" );

	auto [min, max] = ShiftDimTransform( GetMinMax( sim, exp ), dims, dims_shift );
	sfVec step( dims );
	for( size_t dim = 0; dim < dims; dim++ )
		step[dim] = ( max[dim] - min[dim] ) / (Float)bins_count;

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
		bins.PutInBin( ShiftDimTransform( { sim[i], exp[i] }, dims, dims_shift ) );
	
	return  bins;
}

Bins CalculateBins( const std::span<sfVec> sim, 
					const std::span<sfVec> exp,
					size_t dims,
					size_t dims_shift,
					BinningType type,
					Int bins_count )
{
	if( sim.size() == 0 || exp.size() == 0 )
		throw std::runtime_error( "Input data are empty" );

	// Split into bins
	switch( type )
	{
	case BinningType::Static:
		return FixedSizeBinning( sim, exp, dims, dims_shift, bins_count );
		break;
	case BinningType::Dynamic:
		std::cout << "Dynamic binning not implemented yet" << std::endl;
		return FixedSizeBinning( sim, exp, dims, dims_shift, bins_count );
		break;
	}
	throw std::runtime_error( "Invalid binning type" );
}
