
#include "bin.hpp"
#include <ranges>
#include <algorithm>
#include <stdexcept>


void FixedSizeBinning( Bins& /*bins*/, size_t bins_count )
{
	if( bins_count < 1 )
		std::runtime_error( "Invalid binning size" );

}

Vec ShiftDimTransform( const Vec& vec, size_t shift_dim, size_t max_dim )
{
	Vec v( vec.size() );
	for( size_t i = 0; i < vec.size(); i++ )
		v[( i + shift_dim ) % max_dim] = vec[i] + ( 1000 * max_dim );
	return v;
}

std::vector<Bins> SplitRowsIntoBins( const InputData& data, BinningType type, Int bins_count )
{
	std::vector<Bins> splited_rows;

	const auto& exp_row = data.mExp;
	const auto& sim_row = data.mSim;
	Bins bins{ sim_row, exp_row };

	for( size_t i = 0; i < data.mExp.Size(); i++ )
		bins.mData.push_back( std::make_pair( exp_row.mData[i], sim_row.mData[i] ) );

	auto dims = data.mExp.Dim();
	for( size_t dim = 0; dim < dims; dim++ )
	{
		Bins dim_bins = bins;
		dim_bins.mDecompDim = dim;

		std::ranges::sort( dim_bins.mData,
						   [=]( const std::pair<Vec, Vec>& f, const std::pair<Vec, Vec>& s )
		{
			return ShiftDimTransform( f.first, dim, dims ) < ShiftDimTransform( s.first, dim, dims );
		} );

		// Split into bins
		switch( type )
		{
		case BinningType::FixedSize:
			FixedSizeBinning( bins, bins_count );
			break;
		default:
			throw std::runtime_error( "Invalid binning type" );
			break;
		}
		splited_rows.push_back( std::move( dim_bins ) );
	}
	return splited_rows;
}