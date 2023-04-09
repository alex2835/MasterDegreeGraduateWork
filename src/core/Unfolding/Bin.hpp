#pragma once

#include "load_data.hpp"
#include "utils.hpp"


enum class BinningType
{
	FixedSize
};

struct Bin
{
	size_t mBegin;
	size_t mEnd;
};

struct Bins
{
	const Rows& mSimRows;
	const Rows& mExpCols;
	size_t mDecompDim;
	std::vector<std::pair<Vec, Vec>> mData;
	std::vector<Bin> mBins;
};

std::vector<Bins> SplitRowsIntoBins( const InputData& data, BinningType type, Int bins_count );