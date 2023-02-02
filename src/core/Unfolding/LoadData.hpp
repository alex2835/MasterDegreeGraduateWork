#pragma once

#include "Utils.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <string>
#include <iostream>
#include <string_view>
using namespace std::literals;

struct Row
{
	std::string mName;
	std::vector<Float> mData;
	float mMin = std::numeric_limits<Float>::max();
	float mMax = std::numeric_limits<Float>::min();

	void Insert( Float value );
};

struct InputData
{
	std::vector<Row> mExp;
	std::vector<Row> mSim;
};

InputData LoadData( std::vector<std::string> files );