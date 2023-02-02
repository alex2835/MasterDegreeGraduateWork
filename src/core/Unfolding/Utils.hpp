#pragma once

#include "Matrix.hpp"

#include <type_traits>
#include <charconv>
#include <string>
#include <stdexcept>
#include <vector>

// ============== Types ============== 

typedef float Float;
typedef int Int;

typedef Matrix<Float> Mat;
typedef std::vector<Float> Vec;


// ============== Consts ============== 

constexpr Int BIN_SIZE = 10;


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

