#pragma once

#include "static_vector.hpp"
#include "bin.hpp"
#include "migration_mat.hpp"
#include <sstream>


inline dfVec CalculateHistogram( Bins& bins, std::span<sfVec> data, size_t dim_shift )
{
	dfVec hist;
	hist.setlength( bins.mBins.size() );
	for( int i = 0; i < hist.length(); i++ )
		hist[i] = 0;

	for( const auto& vec : data )
	{
		if( !bins.HitBinRange( vec ) )
			continue;
		auto md_idx = bins.GetBinByValue( ShiftDimTransform( vec, bins.Dims(), dim_shift ) ).mIdx;
		auto idx = FromMultidimentionalIdx( md_idx, bins.mSize );
		hist[idx]++;
	}
	return hist;
}

inline dfVec CalculateProbabilities( const dfVec& hist )
{
	dfVec probabilities;
	probabilities.setlength( hist.length() );

	Float size = 0;
	for( int i = 0; i < hist.length(); i++ )
		size += (Float)hist[i];

	for( int i = 0; i < hist.length(); i++ )
		probabilities[i] = hist[i] / size;
	return probabilities;
}

enum class NeighborsMatType
{
	Binary,
	NonbinaryStatistic,
	NonbinaryMassCenters
};

inline bool IsNaigbors( const Bin& first, const Bin& second )
{
	size_t sum = 0;
	for( size_t i = 0; i < first.mIdx.size(); i++ )
		sum += abs( first.mIdx[i] - second.mIdx[i] );
	return sum == 1;
}

inline dfMat CalculateBinaryNeighborsMat( const Bins& bins )
{
	auto size = bins.OneDimSize();
	auto mat = CreateSqrMat( size );
	for( size_t i = 0; i < size; i++ )
	{
		int sum = 0;
		for( size_t j = 0; j < size; j++ )
		{
			int value = IsNaigbors( bins[i], bins[j] );
			mat[i][j] = -value;
			sum += value;
		}
		mat[i][i] = sum;
	}
	return mat;
}

inline Float NeighborsStatProximity( const Bin& first, const Bin& second )
{
	Float proximity = 0;
	for( const auto& pair : first )
		proximity += second.ValueInBin( pair.second );
	return proximity;
}

inline Float NeighborsMassCenterProximity( const Bin& first, const Bin& second )
{
	sfVec f_mean( first.Dims() );
	sfVec s_mean( second.Dims() );
	for( const auto& pair : first )
		f_mean = f_mean + pair.first;
	f_mean = f_mean / ( (int)first.Size() + 1 );
	for( const auto& pair : second )
		s_mean = s_mean + pair.first;
	s_mean = s_mean / ( (int)second.Size() + 1 );
	
	Float proximity = 0;
	for( size_t i = 0; i < f_mean.size(); i++ )
		proximity += std::pow( f_mean[i] - s_mean[i], 2 );

	proximity = std::sqrt( proximity ) + 0.001;
	return 1.0 / proximity;
}

inline dfMat CalculateNotBinaryNeighborsMat( const Bins& bins, NeighborsMatType type )
{
	auto size = bins.OneDimSize();
	auto mat = CreateSqrMat( size );
	for( size_t i = 0; i < size; i++ )
	{
		Float sum = 0; 
		for( size_t j = 0; j < size; j++ )
		{
			if( i == j )
				continue;

			Float value = 0;
			switch( type )
			{
			case NeighborsMatType::NonbinaryStatistic:
				value = NeighborsStatProximity( bins[i], bins[j] );
				break;
			case NeighborsMatType::NonbinaryMassCenters:
				value = NeighborsMassCenterProximity( bins[i], bins[j] );
				break;
			}
			mat[i][j] = -value;
			sum += value;
		}
		mat[i][i] = sum;
		for( size_t j = 0; j < size; j++ )
			mat[i][j] /= ( sum ? sum : 1 );
	}
	return mat;
}

// C or K mat
inline dfMat CalculateNeighborsMat( const Bins& bins, NeighborsMatType type )
{
	switch( type )
	{
	case NeighborsMatType::Binary:
		return CalculateBinaryNeighborsMat( bins );
	case NeighborsMatType::NonbinaryStatistic:
	case NeighborsMatType::NonbinaryMassCenters:
		return CalculateNotBinaryNeighborsMat( bins, type );
	}
	throw std::runtime_error( "Invaid Meighbors type" );
}

inline dfMat FluctuateMat( dfMat mat )
{
	for( int i = 0; i < mat.rows(); i++ )
		mat[i][i] += 0.001;
	return mat;
}

inline dfMat ExtendSystemMat( const dfMat& mat, Float alpha )
{
	dfMat res;
	res.setlength( mat.rows() * 2, mat.cols() );
	for( int i = 0; i < res.rows(); i++ )
		for( int j = 0; j < res.cols(); j++ )
			res[i][j] = 0;
	auto axE = CreateSqrIdentityMat( mat.rows(), alpha );
	alglib::rmatrixcopy( mat.rows(), mat.cols(), mat, 0, 0, res, 0, 0 );
	alglib::rmatrixcopy( axE.rows(), axE.cols(), axE, 0, 0, res, mat.rows(), 0 );
	return res;
}

inline Float FindMaxSingularDiffValue( const dfVec& vec )
{
	Float max_dif = 0;
	Float max = 0;
	for( int i = 0; i < vec.length() - 1; i++ )
	{
		if( vec[i] - vec[i + 1] > max_dif )
		{
			max_dif = vec[i] - vec[i + 1];
			max = vec[i];
		}
	}
	return max;
}

// U s Vt
inline std::tuple<dfMat, dfVec, dfMat> SVD( const dfMat A )
{
	dfMat U;
	dfVec s;
	dfMat Vt;
	alglib::rmatrixsvd( A, A.rows(), A.cols(), 2, 2, 2, s, U, Vt );
	return { U, s, Vt };
}

inline dfVec SolveSystem( const dfMat& A,
						  const Bins& bins,
						  dfVec m,
						  NeighborsMatType nighbors_type,
						  double alpha,
						  bool debug )
{
	std::stringstream out;
	//auto& out = std::cout;

	out << "m\n" << m << "\n\n";
	out << "A\n" << A << "\n\n";

	auto C = CalculateNeighborsMat( bins, nighbors_type );
	auto Cf = FluctuateMat( C );
	out << "C\n" << C << "\n\n";
	auto Ci = MatInverse( Cf );

	auto AxCi = MatMul( A, Ci );
	out << "AxCi\n" << AxCi << "\n\n";

	auto eAxCi = ExtendSystemMat( AxCi, std::sqrt( alpha ) );
	out << "hAxCi\n" << eAxCi << "\n\n";

	auto eAxCiC = MatMul( eAxCi, Cf );
	out << "eAxCiC\n" << eAxCiC << "\n\n";

	
	auto [U, s, Vt] = SVD( eAxCiC );
	out << "U\n" << U << "\n\n";
	out << "s\n" << s << "\n\n";
	out << "Vt\n" << Vt << "\n\n";

	out << "alpha \n" << alpha << "\n\n";

	auto Ut = MatTranpose( U );
	out << "Ut\n" << Ut << "\n\n";

	dfVec m_ex;
	m_ex.setlength( Ut.rows() );
	for( int i = 0; i < Ut.rows(); i++ )
		m_ex[i] = i < m.length() ? m[i] : 0;

	auto d = MatVecMul( Ut, m_ex );
	out << "d\n" << d << "\n\n";

	dfVec z;
	z.setlength( s.length() );
	for( int i = 0; i < s.length(); i++ )
		z[i] = ( d[i] / s[i] ) * ( std::pow( s[i], 2 ) / ( std::pow( s[i], 2 ) + alpha ) );
	out << "z\n" << z << "\n\n";

	auto V = MatTranpose( Vt );
	auto tau = MatVecMul( V, z );

	out << "tau\n" << tau << "\n\n";

	if( debug )
		std::cout << out.str() << std::endl;

	return tau;
}