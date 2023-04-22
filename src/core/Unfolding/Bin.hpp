#pragma once

#include "load_data.hpp"
#include "utils.hpp"

#include <set>
#include <algorithm>


enum class BinningType
{
	FixedSize,
	Exponential
};

struct Bin
{
	siVec mIdx;
	sfVec mBegin;
	sfVec mEnd;
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
	siVec mSize;

	void PutBin( Bin&& bin )
	{
		mBins.push_back( std::move( bin ) );
	}

	void PutInBin( std::pair<sfVec, sfVec> exp_sim )
	{
		GetBinByValue( exp_sim.first ).mData.emplace( exp_sim );
	}

	Bin& operator[]( size_t idx )
	{
		if( idx > mBins.size() )
			throw std::runtime_error( std::format( "operator[]: Out of bins bound {}", idx ) );
		return mBins[idx];
	}

	Bin& GetBinByValue( sfVec value )
	{
		Bin bin;
		bin.mEnd = value;

		auto ptr = std::lower_bound( mBins.begin(), mBins.end(), bin,
		[]( const Bin& bin, const Bin& value )
		{
			return value.mEnd >= bin.mEnd;
		} );
		if( ptr == mBins.end() )
		{
			if( value <= mBins.back().mEnd )
				return mBins.back();
			throw std::runtime_error( std::format( "GetBinByvalue: Out of bins bound {}", value ) );
		}
		auto idx = std::distance( mBins.begin(), ptr );
		return mBins[idx];
	}

};

Bins SplitRowsIntoBins( const InputData& data, BinningType type, Int bins_count );

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

inline size_t MultiDimPow( siVec md_size, size_t dim )
{
	size_t res = 1;
	for( size_t i = 0; i < dim; i++ )
		res *= md_size[i];
	return res;
}

inline size_t FromMultidimentionalIdx( siVec idx, siVec md_size )
{
	size_t res = 0;
	for( size_t i = 0; i < idx.size(); i++ )
		res += idx[i] + ( i != 0 ? 
						  MultiDimPow( md_size, i ) :
						  0 );
	return res;
}