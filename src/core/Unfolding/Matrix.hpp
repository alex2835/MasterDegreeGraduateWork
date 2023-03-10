#pragma once 

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <istream>
#include <ostream>
#include <string>
#include <initializer_list>

template <class T>
class Matrix
{
private:
	template <typename TE> friend Matrix<TE> operator*( const Matrix<TE>& a, const Matrix<TE>& b );
	template <typename TE> friend Matrix<TE> operator*( const TE& a, const Matrix<TE>& b );
	template <typename TE> friend Matrix<TE> operator*( const Matrix<TE>& a, const TE& b );
	template <typename TE> friend Matrix<TE> operator+( const Matrix<TE>& a, const Matrix<TE>& b );
	template <typename TE> friend Matrix<TE> operator-( const Matrix<TE>& a, const Matrix<TE>& b );
	template <typename TE> friend bool operator==( const Matrix<TE>& a, const Matrix<TE>& b );
	template <typename TE> friend std::ostream& operator<<( std::ostream& os, const Matrix<TE>& x );
	template <typename TE> friend std::istream& operator>>( std::istream& is, Matrix<TE>& x );


	std::vector<std::vector<T>> mat;
	size_t N;
	size_t M;


public:
	Matrix<T>() : N( 0 ), M( 0 ) {}

	Matrix<T>( size_t N, size_t M, T init = 0 )
	{
		this->N = N;
		this->M = M;

		mat.resize( N );
		for( size_t i = 0; i < N; i++ )
		{
			mat[i].resize( M );
			fill( mat[i].begin(), mat[i].end(), init );
		}

	}

	Matrix<T>( const Matrix& m )
	{
		mat = m.mat;
		N = m.N;
		M = m.M;
	}

	Matrix<T>( const std::vector<std::vector<T>>& m )
	{
		size_t c = 0;
		if( m.size() > 0 )
		{
			c = m[0].size();
			for( size_t i = 1; i < m.size(); i++ )
				if( m[i].size() != c )
					throw std::logic_error( "Not a matrix" );
		}
		mat = m;
		N = m.size();
		M = c;
	}

	Matrix<T>( std::initializer_list<std::initializer_list<T>> list )
	{
		N = list.size();
		mat.resize( N );

		M = 0;
		if( N > 0 )
			M = list.begin()->size();

		size_t i = 0;
		for( const auto& r : list )
		{
			mat[i].resize( M );
			size_t j = 0;
			for( const auto& val : r )
			{
				mat[i][j] = val;;
				j++;
			}
			if( M != j )
				std::logic_error( "Not a matrix" );

			i++;

		}
	}

	Matrix<T>& operator=( const std::vector<std::vector<T>>& m );
	std::vector<T>& operator[]( size_t i ) {
		return mat[i];
	}
	std::vector<T> operator[]( size_t i ) const {
		return mat[i];
	}
	Matrix<T>& operator+=( const Matrix<T>& m );
	Matrix<T>& operator-=( const Matrix<T>& m );
	Matrix<T>& operator*=( const Matrix<T>& m );

	Matrix<T>& Fill( const T& val )
	{
		for( size_t i = 0; i < N; i++ )
			fill( mat[i].begin(), mat[i].end(), val );

		return *this;
	}

	Matrix<T>& AppendRow( std::vector<T> row )
	{
		if( N != row.size() )
			throw std::logic_error( "rows(A)!=row.size" );

		mat.push_back( std::move( row ) );
		N++;
		return *this;
	}

	Matrix<T>& AppendCol( std::vector<T> col )
	{
		if( M != col.size() )
			throw std::logic_error( "columns(A)!=col.size" );

		for( size_t i = 0; i < M; i++ )
			mat[i].push_back( col[i] );
		M++;
		return *this;
	}

	Matrix<T>& Clear()
	{
		mat.clear();
		N = M = 0;
		return *this;
	}

	std::vector<T> GetData()
	{
		std::vector<T> raw( N * M );
		for( size_t i = 0; i < N; i++ )
			for( size_t j = 0; j < M; j++ )
				raw[i*N+j] = mat[i][j];
		return raw;
	}

	size_t Rows() const {
		return N;
	}
	size_t Cols() const {
		return M;
	}

	Matrix<T> Coff( size_t i, size_t j ) const;
	T Det() const;
	Matrix<T> Inv() const;


	static T Det( const Matrix<T>& x );
	static Matrix<T> LinSolve( const Matrix& A, const Matrix& b ) {
		return A.Inv() * b;
	}

};


// ============= Implemetation =============

template <typename TE>
Matrix<TE> operator*( const Matrix<TE>& a, const Matrix<TE>& b )
{
	if( a.M != b.N )
		throw std::logic_error( "columns(A)!=rows(B)" );

	if( a.N < 1 )
		throw std::logic_error( "Empty matrices" );

	Matrix<TE> y( a.N, b.M );

	for( size_t i = 0; i < y.N; i++ )
	{
		for( size_t j = 0; j < y.M; j++ )
		{
			TE sum = 0;
			for( size_t k = 0; k < b.N; k++ )
				sum += a.mat[i][k] * b.mat[k][j];

			y.mat[i][j] = sum;

		}
	}

	return y;
}

template <typename TE>
Matrix<TE> operator*( const TE& a, const Matrix<TE>& b )
{
	Matrix<TE> y( b.N, b.M );
	for( size_t i = 0; i < b.N; i++ )
		for( size_t j = 0; j < b.M; j++ )
			y[i][j] = a * b[i][j];

	return y;
}

template <typename TE>
Matrix<TE> operator+( const Matrix<TE>& a, const Matrix<TE>& b )
{
	if( a.N != b.N || a.M != b.M )
		throw std::logic_error( "Operator+ requires two matrices of same size" );

	Matrix<TE> y( a.N, a.M );
	for( size_t i = 0; i < a.N; i++ )
		for( size_t j = 0; j < a.M; j++ )
			y.mat[i][j] = a.mat[i][j] + b.mat[i][j];

	return y;
}


template <typename TE>
Matrix<TE> operator-( const Matrix<TE>& a, const Matrix<TE>& b )
{
	if( a.N != b.N || a.M != b.M )
		throw std::logic_error( "Operator- requires two matrices of same size" );

	Matrix<TE> y( a.N, a.M );
	for( size_t i = 0; i < a.N; i++ )
		for( size_t j = 0; j < a.M; j++ )
			y.mat[i][j] = a.mat[i][j] - b.mat[i][j];

	return y;
}


template <typename TE>
Matrix<TE> operator*( const Matrix<TE>& a, const TE& b )
{
	return b * a;
}

template <typename TE>
bool operator==( const Matrix<TE>& a, const Matrix<TE>& b )
{
	if( a.N != b.N || a.M |= b.M )
		return false;

	for( size_t i = 0; i < a.N; i++ )
		if( a[i] != b[i] )
			return false;

	return true;
}

template <typename TE>
std::ostream& operator<<( std::ostream& os, const Matrix<TE>& x )
{
	os << '[' << std::endl;
	for( size_t i = 0; i < x.N - 1; i++ )
	{
		for( size_t j = 0; j < x.M - 1; j++ )
			os << x.mat[i][j] << ",\t";

		os << x.mat[i][x.M - 1] << " ;" << std::endl;
	}

	if( x.N > 0 )
	{
		for( size_t j = 0; j < x.M - 1; j++ )
			os << x.mat[x.N - 1][j] << ",\t";

		os << x.mat[x.N - 1][x.M - 1] << std::endl;
	}

	os << "]" << std::endl;

	return os;
}

template <typename TE>
std::istream& operator>>( std::istream& is, Matrix<TE>& x )
{
	size_t m;
	x.Clear();
	char c;
	TE v;

	is >> c;
	c = is.peek();

	if( c != ']' )
	{
		do
		{
			x.AppendRow();
			m = 0;
			c = 0;
			while( c != ';' && c != ']' )
			{
				is >> v >> c;
				m++;
				if( m > x.M && x.N > 1 )
					throw std::logic_error( "Input is not a mtrix" );

				if( m > x.M )
					x.AppendCol();

				x.mat[x.N - 1][m - 1] = v;

			}
		} while( c != ']' );

		if( m != x.M )
			throw std::logic_error( "Input is not a matrix" );
	}

	return is;

}


// CLASS METHODS
template <class T>
Matrix<T>& Matrix<T>::operator=( const std::vector<std::vector<T>>& m )
{
	size_t c = 0;
	if( m.size() > 0 )
	{
		c = m[0].size();
		for( size_t i = 1; i < m.size(); i++ )
			if( m[i].size() != c )
				throw std::logic_error( "Not a matrix" );
	}
	mat = m.mat;
	N = m.size();
	M = c;

	return *this;
}

template <class T>
Matrix<T>& Matrix<T>::operator+=( const Matrix<T>& m )
{
	if( N != m.N || M != m.M )
		throw std::logic_error( "Operator+= requires two matrices of same size" );

	for( size_t i = 0; i < N; i++ )
		for( size_t j = 0; j < M; j++ )
			mat[i][j] += m.mat[i][j];

	return *this;
}

template <class T>
Matrix<T>& Matrix<T>::operator-=( const Matrix<T>& m )
{
	if( N != m.N || M != m.M )
		throw std::logic_error( "Operator= requires two matrices of same size" );

	for( size_t i = 0; i < N; i++ )
		for( size_t j = 0; j < M; j++ )
			mat[i][j] -= m.mat[i][j];

	return *this;
}

template <class T>
Matrix<T>& Matrix<T>::operator*=( const Matrix<T>& m )
{
	*this = *this * m;
	return *this;
}

template <class T>
Matrix<T> Matrix<T>::Coff( size_t i, size_t j ) const
{
	if( N == 0 )
		throw std::logic_error( "Coff: the matrix is empty" );

	Matrix<T> y( N - 1, M - 1 );

	size_t k_c = 0;
	for( size_t k_x = 0; k_x < N; k_x++ )
	{
		if( k_x == i )
			continue;

		size_t j_c = 0;
		for( size_t j_x = 0; j_x < N; j_x++ )
		{
			if( j_x == j )
				continue;

			y.mat[k_c][j_c] = mat[k_x][j_x];
			j_c++;
		}

		k_c++;
	}

	return y;

}

template <class T>
T Matrix<T>::Det( const Matrix<T>& x )
{
	if( x.N != x.M )
		throw std::logic_error( "Can't compute the determinant of a non square matrix" );

	if( x.N == 0 )
		throw std::logic_error( "Empty matrix" );

	if( x.N == 1 )
		return x.mat[0][0];

	T y = 0;
	signed char d = 1;

	for( size_t i = 0; i < x.N; i++ )
	{
		y += d * x.mat[i][0] * Det( x.Coff( i, 0 ) );
		d = -d;
	}

	return y;
}

template <class T>
T Matrix<T>::Det() const
{
	return Det( *this );
}

template <class T>
Matrix<T> Matrix<T>::Inv() const
{
	T det_x = Det();
	if( abs( det_x ) < std::numeric_limits<double>::epsilon() )
		throw std::logic_error( "Can't invert matrix  (determinant=0)" );

	Matrix<T> y( N, M );

	signed int d = 1;
	for( size_t i = 0; i < N; i++ )
	{
		for( size_t j = 0; j < N; j++ )
		{
			y.mat[j][i] = d * Det( Coff( i, j ) ) / det_x;
			d = -d;
		}
	}
	return y;

}
