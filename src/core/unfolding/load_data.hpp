#pragma once

#include "utils.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <string>
#include <iostream>
#include <string_view>
using namespace std::literals;

struct Column
{
	std::string mName;
	std::vector<Float> mData;
};

struct Rows
{
	std::vector<std::string> mNames;
	std::vector<sfVec> mData;

	const sfVec& operator[]( size_t idx ) const
	{
		return mData[idx];
	}
	auto begin()
	{
		return mData.begin();
	}
	auto end()
	{
		return mData.end();
	}
	size_t Dim() const
	{
		if( mData.size() == 0 )
			throw std::runtime_error( "Can't determine dimension for empty Rows" );
		return mData.front().size();
	}
	size_t Size() const
	{
		return mData.size();
	}
};

struct InputData
{
	std::vector<Column> mCols;
	Rows mSim;
	Rows mExp;
};

InputData LoadData( std::vector<std::string> files );