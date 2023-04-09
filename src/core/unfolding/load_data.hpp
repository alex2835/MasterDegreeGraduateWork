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
	Vec mData;
};

struct Rows
{
	std::vector<std::string> mNames;
	std::vector<Vec> mData;

	const Vec& operator[]( size_t idx ) const
	{
		return mData[idx];
	}
	auto begin() const
	{
		return mData.cbegin();
	}
	auto end() const
	{
		return mData.cend();
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
	Rows mExp;
	Rows mSim;
};

InputData LoadData( std::vector<std::string> files );