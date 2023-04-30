#pragma once

#include "static_vector.hpp"
#include "bin.hpp"
#include "migration_mat.hpp"


dfVec CalculateHistogram( Bins& bins, std::span<sfVec> data )
{
	dfVec hist;
	hist.setlength( bins.mBins.size() );
	for( int i = 0; i < hist.length(); i++ )
		hist[i] = 0;

	for( const auto& vec : data )
	{
		auto md_idx = bins.GetBinByValue( vec ).mIdx;
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



enum class NeighborsType
{
	C,
	KBinary,
	KNotBinary
};

// C or K mat
dfMat CalculateNeighborsMat( Bins& /*bins*/, NeighborsType type )
{
	switch( type )
	{
	case NeighborsType::C:
	case NeighborsType::KBinary:
	case NeighborsType::KNotBinary:
		return dfMat();
	}
	throw std::runtime_error( "Invaid Meighbors type" );
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

dfVec SolveSystem( const dfMat& migration_mat, const dfVec& m )
{
	std::cout << "m\n" << m << "\n\n";

	auto [U, s, Vt] = SVD( migration_mat );
	std::cout << "U\n"  << U  << "\n\n";
	std::cout << "s\n"  << s  << "\n\n";
	std::cout << "Vt\n" << Vt << "\n\n";

	auto Ut = MatTranpose( U );
	auto d = MatVecMul( Ut, m );
	std::cout << "d\n" << d << "\n\n";

	dfVec z;
	z.setlength( d.length() );
	for( int i = 0; i < d.length(); i++ )
		z[i] = d[i] / s[i];

	std::cout << "z\n" << z << "\n\n";

	auto V = MatTranpose( Vt );
	auto tau = MatVecMul( V, z );

	std::cout << "tau\n" << tau << "\n\n";

	return tau;
}