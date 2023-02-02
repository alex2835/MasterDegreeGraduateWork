#pragma once

#include "LoadData.hpp"
#include "Utils.hpp"

struct Bins
{
	const Row& mRowRef;
	Float mBinStep;
	std::vector<Int> mBins;
	std::vector<Float> mPropobilities;
};

std::vector<Bins> SplitRowsIntoBins( const InputData& data, Int bin_size );