#pragma once

#include "linalg.h"
#include "static_vector.hpp"

#include <type_traits>
#include <charconv>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <array>
#include <stdexcept>
#include <format>
#include <span>
#include <cstdlib>

// ============== Consts ============== 

constexpr size_t BIN_SIZE = 10;
constexpr size_t MAX_BIN_SIZE = 200;
constexpr size_t MIN_BIN_SIZE = 2;
constexpr size_t MAX_VEC_SIZE = 3;

// ============== Types ============== 

using dfMat = alglib::real_2d_array;
using dfVec = alglib::real_1d_array;
using sfVec = Vector<Float, MAX_VEC_SIZE>;
using siVec = Vector<int64_t, MAX_VEC_SIZE>;


// ============== Stringfication ============== 

template <typename T, size_t S>
struct std::formatter<Vector<T, S>> : std::formatter<std::string>
{
	auto format( const Vector<T, S>& vec, format_context& ctx )
	{
		std::string s = "[";
		for( size_t i = 0; i < vec.size(); i++ )
			s += std::to_string( vec[i] ) + ", ";
		s += "]";
		return formatter<string>::format( s, ctx );
	}
};

template <>
struct std::formatter<dfVec> : std::formatter<std::string>
{
	auto format( const dfVec& vec, format_context& ctx )
	{
		std::string s = "[";
		for( int i = 0; i < vec.length(); i++ )
			s += std::to_string( vec[i] ) + ", ";
		s += "]";
		return formatter<string>::format( s, ctx );
	}
};

inline std::ostream& operator<<( std::ostream& stream, const dfVec& vec )
{
	return stream << std::format( "{}", vec);
}

inline std::ostream& operator<<( std::ostream& stream, const dfMat& mat )
{
	for( int i = 0; i < mat.rows(); i++ )
	{
		stream << "[ ";
		for( int j = 0; j < mat.cols(); j++ )
			stream << std::setw( 5 ) <<
					  std::setfill( ' ' ) <<
					  mat[i][j] << ", ";
		stream << "]\n";
	}
	return stream;
}

template <typename T, size_t S>
std::ostream& operator<<( std::ostream& stream, const Vector<T, S>& vec )
{
	return stream << std::format( "{}", vec );
}



// ============== Data ============== 

template <typename Vec>
auto ToSpan( Vec& vec )
{
	return std::span( vec.begin(), vec.end() );
}

template <typename T>
std::vector<std::span<T>> SplitData( const std::span<T> vec, size_t parts )
{
	std::vector<std::span<T>> res;
	auto part_size = size_t( std::ceil( (Float)vec.size() / parts ) );
	for( size_t i = 0; i < parts; i++ )
		res.push_back( vec.subspan( i * part_size, part_size ) );
	return res;
}

// ============== Mat/Vec ============== 

inline dfMat CreateSqrMat( size_t size )
{
	dfMat mat;
	mat.setlength( size, size );
	for( size_t i = 0; i < size; i++ )
		for( size_t j = 0; j < size; j++ )
			mat[i][j] = 0;
	return mat;
}

inline dfMat CreateSqrIdentityMat( size_t size, Float value = 1.0 )
{
	auto mat = CreateSqrMat( size );
	for( int i = 0; i < mat.rows(); i++ )
		mat[i][i] = value;
	return mat;
}

inline dfVec MatVecMul( const dfMat& mat, const dfVec& vec )
{
	if( mat.rows() != vec.length() )
	{
		std::cout << "rows " << mat.rows() << " cols " << mat.cols() << " vecl " << vec.length() << "\n";
		throw std::runtime_error( "MatVec mul failed: invalid mat vec sizes" );
	}
	dfVec res;
	res.setlength( mat.rows() );
	alglib::rmatrixmv( mat.rows(), mat.cols(), mat, 0, 0, 0, vec, 0, res, 0 );
	return res;
}

inline dfMat MatMul( const dfMat& mat1, const dfMat& mat2 )
{
	if( mat1.cols() != mat2.rows() )
		throw std::runtime_error( std::format( "MatMul: Invalid mat sizes: mat1: {} {}, mat2: {} {}",
								  mat1.rows(), mat1.cols(), mat2.rows(), mat2.cols() ) );

	dfMat res;
	res.setlength( mat1.rows(), mat2.cols() );
	alglib::rmatrixgemm( mat1.rows(), mat2.cols(), mat1.cols(), 1, mat1, 0, 0, 0, mat2, 0, 0, 0, 0, res, 0, 0 );
	return res;
}

inline dfMat MatMulColMajor( const dfMat& mat1, const dfMat& mat2 )
{
	dfMat res;
	res.setlength( mat1.rows(), mat2.cols() );
	for( int i = 0; i < res.rows(); i++ )
		for( int j = 0; j < res.cols(); j++ )
			res[i][j] = 0;

	for( int i = 0; i < mat1.rows(); i++ )
	{
		for( int j = 0; j < mat2.cols(); j++ )
		{
			res[j][i] = 0;
			for( int k = 0; k < mat1.cols(); k++ )
				res[j][i] += mat1[k][i] * mat2[j][k];
		}
	}
	return res;
}

inline dfVec MatVecMulColMajor( const dfMat& mat, const dfVec& vec )
{
	dfVec res;
	res.setlength( vec.length() );
	for( int i = 0; i < vec.length(); i++ )
		res[i] = 0;

	for( int i = 0; i < mat.rows(); i++ )
		for( int j = 0; j < mat.cols(); j++ )
			res[i] += mat[j][i] * vec[j];

	return res;
}

inline dfMat MatTranpose( const dfMat& mat )
{
	dfMat res;
	res.setlength( mat.cols(), mat.rows() );
	alglib::rmatrixtranspose( mat.rows(), mat.cols(), mat, 0, 0, res, 0, 0 );
	return res;
}

inline dfMat MatInverse( dfMat mat )
{
	alglib::ae_int_t info;
	alglib::matinvreport rep;
	alglib::rmatrixinverse( mat, info, rep );
	if( info == -3 )
		throw std::runtime_error( 
				std::format( "Matrix inversion failed with report r1: {} rinf:L {}",
				rep.r1, rep.rinf ) );
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

