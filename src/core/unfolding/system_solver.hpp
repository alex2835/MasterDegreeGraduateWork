#pragma once

#include "static_vector.hpp"
#include "bin.hpp"
#include "migration_mat.hpp"
#include <sstream>


dfVec CalculateHistogram( Bins& bins, std::span<sfVec> data, size_t dim_shift )
{
	dfVec hist;
	hist.setlength( bins.mBins.size() );
	for( int i = 0; i < hist.length(); i++ )
		hist[i] = 0;

	for( const auto& vec : data )
	{
		auto md_idx = bins.GetBinByValue( ShiftDimTransform( vec, bins.Dims(), dim_shift ) ).mIdx;
		auto idx = FromMultidimentionalIdx( md_idx, bins.mSize );
		hist[idx]++;
	}
	return hist;
}

dfVec CalculateProbabilities( const dfVec& hist )
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


dfMat CalculateLinearNeighborsMat( const Bins& bins )
{
	auto size = bins.OneDimSize();
	auto mat = CreateSqrMat( size );
	for( size_t i = 1; i < size - 1; i++ )
	{
		mat[i][i - 1] = -1;
		mat[i][i] = 2;
		mat[i][i + 1] = -1;
	}
	mat[0][0] = 1;
	mat[0][1] = -1;
	mat[size - 1][size - 2] = -1;
	mat[size - 1][size - 1] = 1;
	return mat;
}


enum class NeighborsType
{
	C,
	KBinary,
	KNotBinary
};

// C or K mat
dfMat CalculateNeighborsMat( const Bins& bins, NeighborsType type )
{
	switch( type )
	{
	case NeighborsType::C:
		return CalculateLinearNeighborsMat( bins );
	case NeighborsType::KBinary:
	case NeighborsType::KNotBinary:
		return dfMat();
	}
	throw std::runtime_error( "Invaid Meighbors type" );
}

dfMat FluctuateMat( dfMat mat )
{
	for( int i = 0; i < mat.rows(); i++ )
		mat[i][i] += 0.0001;
	return mat;
}

dfMat ExtendSystemMat( const dfMat& mat, Float alpha )
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

Float FindMaxSingularDiffValue( const dfVec& vec )
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
std::tuple<dfMat, dfVec, dfMat> SVD( const dfMat A )
{
	dfMat U;
	dfVec s;
	dfMat Vt;
	alglib::rmatrixsvd( A, A.rows(), A.cols(), 2, 2, 2, s, U, Vt );
	return { U, s, Vt };
}

dfVec SolveSystem( const dfMat& A,
				   const Bins& bins,
				   dfVec m,
				   double alpha,
				   bool debug )
{
	std::stringstream out;
	//auto& out = std::cout;

	out << "m\n" << m << "\n\n";
	out << "A\n" << A << std::endl;

	auto C = CalculateNeighborsMat( bins, NeighborsType::C );
	auto Cf = FluctuateMat( C );
	auto Ci = MatInverse( Cf );

	auto AxCi = MatMul( A, Ci );
	out << "AxCi\n" << AxCi << std::endl;

	auto eAxCi = ExtendSystemMat( AxCi, std::sqrt( alpha ) );
	out << "hAxCi\n" << eAxCi << "\n\n";

	auto eAxCiC = MatMul( eAxCi, Cf );
	out << "eAxCiC\n" << eAxCiC << "\n\n";

	
	auto [U, s, Vt] = SVD( eAxCiC );
	out << "U\n" << U << "\n\n";
	out << "s\n" << s << "\n\n";
	out << "Vt\n" << Vt << "\n\n";

	std::cout << "alpha \n" << alpha << "\n\n";

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
	auto CixV = V;
	auto tau = MatVecMul( CixV, z );

	out << "tau\n" << tau << "\n\n";

	if( debug )
		std::cout << out.str() << std::endl;

	return tau;
}