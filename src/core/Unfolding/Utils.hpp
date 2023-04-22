#pragma once

#include "linalg.h"
#include "static_vector.hpp"

#include <type_traits>
#include <charconv>
#include <string>
#include <stdexcept>
#include <vector>
#include <array>
#include <stdexcept>
#include <format>

// ============== Consts ============== 

constexpr size_t BIN_SIZE = 10;
constexpr size_t MAX_VEC_SIZE = 3;

// ============== Types ============== 

using Mat = alglib::real_2d_array;
using dfVec = alglib::real_1d_array;
using sfVec = Vector<Float, MAX_VEC_SIZE>;
using siVec = Vector<int64_t, MAX_VEC_SIZE>;


inline Mat CreateSqrMat( size_t size )
{
	Mat mat;
	mat.setlength( size, size );
	for( size_t i = 0; i < size; i++ )
		for( size_t j = 0; j < size; j++ )
			mat[i][j] = 0;
	return mat;
}

// ============== Strings ============== 

inline Float ParseFloat( const std::string& str )
{
	if constexpr( std::is_same_v<Float, float> )
		return std::stof( str );
	else if( std::is_same_v<Float, double> )
		return std::stod( str );
	else
		throw std::runtime_error( "Invalid float type" );
}

template <typename StrType, typename DelimType>
std::vector<std::string_view> Split( const StrType& str, const DelimType& delim )
{
	std::vector<std::string_view> res;
	size_t start = 0;
	size_t end = str.find( delim );
	while( end != std::string::npos )
	{
		res.push_back( std::string_view( &str [start], end - start ) );
		start = end + delim.size();
		end = str.find( delim, start );
	}
	res.push_back( std::string_view( &str [start], str.size() - start ) );
	return res;
}

inline std::string_view Trim( std::string_view in )
{
	auto left = in.begin();
	for( ;; ++left )
	{
		if( left == in.end() )
			return std::string_view();
		if( !isspace( *left ) )
			break;
	}
	auto right = in.end() - 1;
	for( ; right > left && isspace( *right ); --right );
	return std::string_view( left, ++right );
}

