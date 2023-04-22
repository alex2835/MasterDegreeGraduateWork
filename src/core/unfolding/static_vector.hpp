#pragma once

#include <array>
#include <stdexcept>
#include <format>
#include <ranges>
#include <algorithm>

typedef double Float;
typedef int Int;

template <typename T, size_t MaxSize>
class Vector
{
	std::array<T, MaxSize> mData{ 0 };
	size_t mSize{ 0 };

public:
	Vector() = default;
	explicit Vector( size_t size, T value = T() )
		: mSize( size )
	{
		std::ranges::fill( mData, value );
	}

	void push_back( const T& f )
	{
		if( mSize >= mData.size() )
			throw std::runtime_error( "Vector push overflow, increase max size" );
		mData[mSize++] = f;
	}

	T& operator[]( size_t idx )
	{
		if( idx >= mSize )
			throw std::runtime_error( std::format( "Out of vector's bound size: {} idx: {}", mSize, idx ) );
		return mData[idx];
	}

	T operator[]( size_t idx ) const
	{
		if( idx >= mSize )
			throw std::runtime_error( std::format( "Out of vector's bound size: {} idx: {}", mSize, idx ) );
		return mData[idx];
	}

	size_t size() const
	{
		return mSize;
	}
	auto begin()
	{
		return mData.begin();
	}
	auto end()
	{
		return mData.end();
	}

	Vector<T, MaxSize>& operator= ( const Vector<T, MaxSize>& other )
	{
		mSize = other.mSize;
		mData = other.mData;
		return *this;
	}

	bool operator== ( const Vector<T, MaxSize>& other ) const noexcept
	{
		return mSize == other.mSize &&
			   mData == other.mData;
	}

	bool operator< ( const Vector<T, MaxSize>& other ) const noexcept
	{
		return mSize <= other.mSize &&
			mData < other.mData;
	}
	bool operator<= ( const Vector<T, MaxSize>& other ) const noexcept
	{
		return mSize <= other.mSize &&
			   mData <= other.mData;
	}
	bool operator>= ( const Vector<T, MaxSize>& other ) const noexcept
	{
		return !operator<( other );
	}
	bool operator> ( const Vector<T, MaxSize>& other ) const noexcept
	{
		return !operator<=( other );
	}

	Vector<T, MaxSize> operator+( const Vector<T, MaxSize>& other )
	{
		Vector<T, MaxSize> res( mSize );
		for( size_t i = 0; i < mSize; i++ )
			res[i] = mData[i] + other[i];
		return res;
	}
	Vector<T, MaxSize> operator+( const T& value )
	{
		Vector<T, MaxSize> res( mSize );
		for( size_t i = 0; i < mSize; i++ )
			res[i] = mData[i] + value;
		return res;
	}

	Vector<T, MaxSize> operator*( const Vector<T, MaxSize>& other )
	{
		Vector<T, MaxSize> res( mSize );
		for( size_t i = 0; i < mSize; i++ )
			res[i] = mData[i] * other[i];
		return res;
	}
	Vector<T, MaxSize> operator-( const Vector<T, MaxSize>& other )
	{
		Vector<T, MaxSize> res( mSize );
		for( size_t i = 0; i < mSize; i++ )
			res[i] = mData[i] - other[i];
		return res;
	}
	Vector<T, MaxSize> operator/( const Vector<T, MaxSize>& other )
	{
		Vector<T, MaxSize> res( mSize );
		for( size_t i = 0; i < mSize; i++ )
			res[i] = mData[i] / other[i];
		return res;
	}
	Vector<T, MaxSize> operator/( T value )
	{
		Vector<T, MaxSize> res( mSize );
		for( size_t i = 0; i < mSize; i++ )
			res[i] = mData[i] / value;
		return res;
	}

	template <typename C>
	Vector<C, MaxSize> cast()
	{
		Vector<C, MaxSize> res( mSize );
		for( size_t i = 0; i < mSize; i++ )
			res[i] = static_cast<C>( mData[i] );
		return res;

	}
};


template <typename T, size_t S>
struct std::formatter<Vector<T, S>> : std::formatter<std::string>
{
	auto format( Vector<T, S> vec, format_context& ctx )
	{
		std::string s = "[";
		for( size_t i = 0; i < vec.size(); i++ )
			s += std::to_string( vec[i] ) + ", ";
		s += "]";
		return formatter<string>::format( s, ctx );
	}
};
