#pragma once

#include "load_data.hpp"
#include "utils.hpp"

#include <set>
#include <algorithm>
#include <iostream>
#include <format>

size_t FromMultidimentionalIdx( siVec idx, siVec md_size );

enum class BinningType
{
	Static,
	Dynamic,
	Hybrid
};

struct Bin
{
	// multidimentional index
	siVec mIdx;
	sfVec mBegin;
	sfVec mEnd;
	// sim, exp
	std::vector<std::pair<sfVec, sfVec>> mData;
	
	sfVec Begin()
	{
		return mBegin;
	}
	sfVec End()
	{
		return mEnd;
	}
	size_t Size()
	{
		return mData.size();
	}
	auto begin()
	{
		return mData.begin();
	}
	auto end()
	{
		return mData.end();
	}
};

struct Bins
{
private:
	// begin, end
	std::vector<std::vector<std::pair<Float, Float>>> mCache;
public:
	std::vector<Bin> mBins;
	siVec mSize;


	auto begin()
	{
		return mBins.begin();
	}

	auto end()
	{
		return mBins.end();
	}

	size_t Dims()
	{
		return mSize.size();
	}

	size_t OneDimSize()
	{
		size_t res = 1;
		for( auto dim_size : mSize )
			res *= dim_size;
		return res;
	}

	void PutBin( Bin&& bin )
	{
		mBins.push_back( std::move( bin ) );
	}

	void PutInBin( std::pair<sfVec, sfVec> sim_exp )
	{
		auto& bin = GetBinByValue( sim_exp.first );
		bin.mData.push_back( sim_exp );
	}

	Bin& operator[]( size_t idx )
	{
		if( idx > mBins.size() )
			throw std::runtime_error( std::format( "operator[]: Out of bins bound {}", idx ) );
		return mBins[idx];
	}

	Bin& GetBinByValue( sfVec value )
	{
		if( mCache.empty() )
			CalculateCache();

		siVec idx;
		for( size_t dim = 0; dim < Dims(); dim++ )
		{
			auto ptr = std::ranges::lower_bound( mCache[dim], std::pair( value[dim], value[dim] ),
				[]( std::pair<Float, Float> bin,
					std::pair<Float, Float> value )
			{
				return bin.first <= value.first;
			} );

			if( ptr == mCache[dim].end() )
			{
				auto last_id = mCache[dim].size() - 1;
				if( value[dim] <= mCache[dim][last_id].second )
					idx.push_back( last_id );
				else
					throw std::runtime_error( std::format( "GetBinByvalue: Out of bins bound {}", value ) );
			}
			else
			{
				auto dist = std::distance( mCache[dim].begin(), ptr ) - 1;
				idx.push_back( std::max( (int)dist, 0 ) );
			}
		}
		return mBins[FromMultidimentionalIdx( idx, mSize )];
	}

private:
	void CalculateCache()
	{
		for( size_t dim = 0; dim < Dims(); dim++ )
			mCache.push_back( std::vector<std::pair<Float,Float>>( mSize[dim] ) );
		for( auto& bin : mBins )
			for( size_t dim = 0; dim < Dims(); dim++ )
				mCache[dim][bin.mIdx[dim]] = { bin.mBegin[dim] , bin.mEnd[dim] };
	}
};

Bins CalculateBins( std::span<sfVec> sim,
					std::span<sfVec> exp,
					size_t dims,
					size_t dims_shift,
					BinningType type,
					Int bins_count );


inline size_t MultiDimPow( siVec md_size, size_t dim )
{
	size_t res = 1;
	for( size_t i = 0; i < dim; i++ )
		res *= md_size[i];
	return res;
}

inline siVec ToMultidimentionalIdx( size_t flat_idx,
									size_t dims,
									size_t dim_size )
{
	siVec res( dims );
	size_t i = 0;
	while( flat_idx != 0 )
	{
		res[i++] = flat_idx % dim_size;
		flat_idx /= dim_size;
	}
	return res;
}

inline size_t FromMultidimentionalIdx( siVec idx, siVec md_size )
{
	size_t res = 0;
	for( size_t i = 0; i < idx.size(); i++ )
		res += idx[i] * MultiDimPow( md_size, i );
	return res;
}


struct BinningProjection1D
{
	std::vector<Float> bin_xs;
	std::vector<Float> sim_ys;
	std::vector<Float> exp_ys;
};
using BinningProjections1D = std::vector<BinningProjection1D>;

inline BinningProjections1D Caclucate1DBinningProjections( Bins& bins )
{
	BinningProjections1D projections;
	for( size_t dim = 0; dim < bins.Dims(); dim++ )
	{
		projections.push_back( { std::vector<Float>( bins.mSize[dim] ),
	  						     std::vector<Float>( bins.mSize[dim] ),
							     std::vector<Float>( bins.mSize[dim] ) } );
	}
	for( auto& bin : bins )
	{
		for( size_t dim = 0; dim < bins.Dims(); dim++ )
		{
			projections[dim].bin_xs[bin.mIdx[dim]] = ( bin.mEnd[dim] + bin.mBegin[dim] ) / 2;
			projections[dim].sim_ys[bin.mIdx[dim]] += bin.Size();
			for( auto& sim_exp : bin )
			{
				auto md_idx = bins.GetBinByValue( sim_exp.second ).mIdx;
				projections[dim].exp_ys[md_idx[dim]]++;
			}
		}
	}
	return projections;
}


struct BinningProjection2D
{
	int x_size;
	int y_size;
	int second_dim;
	std::vector<int> hmap;
};
using BinningProjections2D = std::vector<BinningProjection2D>;

inline BinningProjections2D Caclucate2DBinningProjections( Bins& bins )
{
	BinningProjections2D projections;
	for( size_t dim = 0; dim < bins.Dims(); dim++ )
	{
		BinningProjection2D projection;

		projection.second_dim = int( ( dim + 1 ) % bins.Dims() );
		projection.x_size = (int)bins.mSize[dim];
		projection.y_size = (int)bins.mSize[projection.second_dim];

		projection.hmap.resize( projection.x_size * projection.y_size );
		for( auto& bin : bins )
			projection.hmap[bin.mIdx[dim] * projection.x_size + bin.mIdx[projection.second_dim]] += (int)bin.Size();

		projections.push_back( std::move( projection ) );
	}
	return projections;
}

