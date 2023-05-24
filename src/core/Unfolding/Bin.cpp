
#include "bin.hpp"
#include <ranges>
#include <algorithm>
#include <stdexcept>

void PrintBins( const Bins& bins )
{
	for( const Bin& bin : bins )
		std::cout << "\nbegin: " << bin.mBegin
		<< " end" << bin.mEnd
		<< " end" << bin.mIdx
		<< " size " << bin.Size();
	std::cout << std::endl;
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

Bins StaticBinning( const std::span<sfVec> sim,
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

	Bins bins;
	bins.mSize = siVec( dims, bins_count );

	for( size_t i = 0; i < std::pow( bins_count, dims ); i++ )
	{
		siVec multi_dim_idx = ToMultidimentionalIdx( i, dims, bins_count );
		siVec next = multi_dim_idx + 1;
		bins.PutBin( Bin{ multi_dim_idx,
						  min + step * multi_dim_idx.cast<Float>(),
						  min + step * next.cast<Float>() } );
	}
	bins.mBins.front().mBegin = min;
	bins.mBins.back().mEnd = max;

	for( size_t i = 0; i < exp.size(); i++ )
		bins.PutInBin( ShiftDimTransform( { exp[i], sim[i] }, dims, dims_shift ) );

	return  bins;
}

template <typename F>
void DynamicBinning( Bins& bins, 
					 size_t iterations,
					 F find_bin_center )
{
	while( iterations-- )
	{
		std::vector<std::vector<int>> projections;
		for( size_t dim = 0; dim < bins.Dims(); dim++ )
			projections.push_back( std::vector<int>( bins.mSize[dim] ) );

		for( const auto& bin : bins )
			for( size_t dim = 0; dim < bins.Dims(); dim++ )
				projections[dim][bin.mIdx[dim]] += (int)bin.Size();


		for( int dim = 0; dim < bins.Dims(); dim++ )
		{
			const auto& projection = projections[dim];

			auto max_iter = std::ranges::max_element( projection );
			int max_bin = (int)std::distance( projection.begin(), max_iter );
			
			Float bin_center = find_bin_center( bins, dim, max_bin );
			for( int i = 0; i < bins.mBins.size(); i++ )
			{
				auto& bin = bins[i];

				if( bin.mIdx[dim] > max_bin )
					bin.mIdx[dim]++;

				if( bin.mIdx[dim] == max_bin )
				{
					Bin first;
					first.mIdx = bin.mIdx;
					first.mBegin = bin.mBegin;
					first.mEnd = bin.mEnd;
					first.mEnd[dim] = bin_center;

					Bin second;
					second.mIdx = bin.mIdx;
					second.mIdx[dim]++;
					second.mBegin = bin.mBegin;
					second.mEnd = bin.mEnd;
					second.mBegin[dim] = bin_center;

					for( auto& sim_exp : bin )
					{
						if( sim_exp.first[dim] < bin_center )
							first.mData.push_back( sim_exp );
						else
							second.mData.push_back( sim_exp );
					}

					//std::cout << i << std::endl;
					bins.mBins.erase( bins.mBins.begin() + i );
					//PrintBins( bins );
					bins.mBins.emplace( bins.mBins.begin() + i, std::move( second ) );
					//PrintBins( bins );
					bins.mBins.emplace( bins.mBins.begin() + i, std::move( first ) );
					//PrintBins( bins );
					i++;
				}
			}
			bins.mSize[dim]++;
		}
	}
	//PrintBins( bins );
	std::ranges::sort( bins.mBins, []( const Bin& f, const Bin& s )
	{
		return f.mIdx < s.mIdx;
	} );
	//PrintBins( bins );
	bins.mCache.clear();
}


Float FindCenterBinDefault( const Bins& bins, int dim, int max_bin )
{
	for( const auto& bin : bins )
		if( bin.mIdx[dim] == max_bin )
			return Float( bin.mBegin[dim] + bin.mEnd[dim] ) / 2;
	throw std::runtime_error( 
		std::format( "find center failed with max_dim: {} max_bin: {}", dim, max_bin ) );
}

Float FindCenterBinMedian( const Bins& bins, int dim, int max_bin )
{
	std::vector<Float> points;
	for( const auto& bin : bins )
	{
		if( bin.mIdx[dim] == max_bin )
		{
			for( auto& pair : bin )
				points.push_back( pair.first[dim] );
		}
	}
	std::ranges::sort( points );
	auto median = points[points.size() / 2];
	return median;
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

	switch( type )
	{
	case BinningType::Static:
		return StaticBinning( sim, exp, dims, dims_shift, bins_count );
	case BinningType::Dynamic:
	{
		auto bins = StaticBinning( sim, exp, dims, dims_shift, 1 );
		DynamicBinning( bins, bins_count - 1, FindCenterBinDefault );
		return bins;
	}
	case BinningType::DynamicMedian:
	{
		auto bins = StaticBinning( sim, exp, dims, dims_shift, 1 );
		DynamicBinning( bins, bins_count - 1, FindCenterBinMedian );
		return bins;
	}
	case BinningType::Hybrid:
	{
		auto static_bins  = std::max( 2, bins_count / 3 );
		auto dynamic_bins = bins_count - static_bins;
		auto bins = StaticBinning( sim, exp, dims, dims_shift, static_bins );
		DynamicBinning( bins, dynamic_bins, FindCenterBinDefault );
		return bins;
	}
	}
	throw std::runtime_error( "Invalid binning type" );
}
