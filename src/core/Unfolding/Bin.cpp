
#include "Bin.hpp"

std::vector<Bins> SplitRowsIntoBins( const InputData& data, Int bin_size )
{
	std::vector<Bins> splited_rows;
	for( auto rows : { &data.mExp, &data.mSim } )
	{
		for( const auto& row : *rows )
		{
			Bins row_bins{ row };
			row_bins.mBinStep = Float( row.mMax - row.mMin ) / bin_size;
			row_bins.mBins.resize( bin_size + 1 );
			for( auto value : row.mData )
				row_bins.mBins[std::min( Int( value / row_bins.mBinStep ), bin_size )]++;
			for( auto bin : row_bins.mBins )
				row_bins.mPropobilities.push_back( (Float)bin / row.mData.size() );
			splited_rows.push_back( row_bins );
		}
	}
	return splited_rows;
}