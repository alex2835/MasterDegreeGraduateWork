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
	Dynamic
};

struct Bin
{
	siVec mIdx;
	sfVec mBegin;
	sfVec mEnd;
	// sim, exp
	std::set<std::pair<sfVec, sfVec>> mData;
	

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
	std::vector<Bin> mBins;
	std::vector<std::vector<std::pair<Float, Float>>> mCache;
	siVec mSize;

	auto begin()
	{
		return mBins.begin();
	}

	auto end()
	{
		return mBins.end();
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
		bin.mData.emplace( sim_exp );
		//std::cout << std::format( "bins:{} {}  value: {}\n", bin.mBegin, bin.mEnd, exp_sim.first );
	}

	Bin& operator[]( size_t idx )
	{
		if( idx > mBins.size() )
			throw std::runtime_error( std::format( "operator[]: Out of bins bound {}", idx ) );
		return mBins[idx];
	}

	Bin& GetBinByValue( sfVec value )
	{
		//// Linear
		//siVec lin_id;
		//size_t flat_lin_id = 0;
		//for( auto& bin : mBins )
		//{
		//	//std::cout << std::format( "bins:{} {}  value: {} {}\n", bin.mBegin, bin.mEnd, value, 
		//	//						  bin.mBegin.AllEqualOrLess( value ) && bin.mEnd.AllEqualOrGreater( value ) );

		//	if( bin.mBegin.AllEqualOrLess( value ) &&
		//		bin.mEnd.AllEqualOrGreater( value ) )  
		//	{
		//		//return bin;
		//		lin_id = bin.mIdx;
		//		break;
		//	}
		//	flat_lin_id++;
		//}
		////throw std::runtime_error( std::format( "GetBinByvalue: Out of bins bound {}", value ) );

		// Binary
		if( mCache.empty() )
			CalculateCache();

		siVec idx;
		for( size_t dim = 0; dim < mSize.size(); dim++ )
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
		for( size_t dim = 0; dim < mSize.size(); dim++ )
			mCache.push_back( std::vector<std::pair<Float,Float>>( mSize[dim] ) );
		for( auto& bin : mBins )
			for( size_t dim = 0; dim < mSize.size(); dim++ )
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


struct BinningProjections
{
	std::vector<std::vector<Float>> bin_xs;
	std::vector<std::vector<Float>> sim_ys;
	std::vector<std::vector<Float>> exp_ys;
};

inline BinningProjections Caclucate1DBinningProjections( Bins& bins, size_t dims )
{
	BinningProjections projections;
	for( size_t dim = 0; dim < dims; dim++ )
	{
		projections.bin_xs.push_back( std::vector<Float>( bins.mSize[dim] ) );
		projections.sim_ys.push_back( std::vector<Float>( bins.mSize[dim] ) );
		projections.exp_ys.push_back( std::vector<Float>( bins.mSize[dim] ) );
	}
	for( auto& bin : bins )
	{
		for( size_t dim = 0; dim < dims; dim++ )
		{
			projections.bin_xs[dim][bin.mIdx[dim]] = ( bin.mEnd[dim] + bin.mBegin[dim] ) / 2;
			projections.sim_ys[dim][bin.mIdx[dim]] += bin.Size();
			for( auto& sim_exp : bin )
			{
				auto md_idx = bins.GetBinByValue( sim_exp.second ).mIdx;
				projections.exp_ys[dim][md_idx[dim]]++;
			}
		}
	}
	//for( auto y : projections.exp_ys[1] )
	//	std::cout << y << ", ";
	//std::cout << std::endl;

	return projections;
}
