
#include "LoadData.hpp"

void Row::Insert( Float value )
{
	mData.push_back( value );
	if( value < mMin )
		mMin = value;
	if( value > mMax )
		mMax = value;
}

InputData LoadData( std::vector<std::string> files )
{
	InputData data;
	for( const auto& path : files )
	{
		std::ifstream file( path );
		std::string row_names;
		std::getline( file, row_names );

		std::vector<Row*> file_rows;
		for( auto name : Split( row_names, ","sv ) )
		{
			if( name.ends_with( "exp"sv ) )
				file_rows.push_back( &data.mExp.emplace_back( Row{ std::string( Trim( name ) ) } ) );
			else if( name.ends_with( "sim"sv ) )
				file_rows.push_back( &data.mSim.emplace_back( Row{ std::string( Trim( name ) ) } ) );
			else
				throw std::runtime_error( "Invalid row type: "s + std::string( name ) );
		}
		for( std::string line; std::getline( file, line ); )
		{
			int i = 0;
			for( auto value : Split( line, ","sv ) )
				file_rows[i++]->Insert( ParseFloat( std::string( Trim( value ) ) ) );
		}
	}
	if( data.mExp.size() != data.mSim.size() )
		throw std::runtime_error( "Invalid row amount: " + std::to_string( data.mExp.size() ) + 
								                  " != " + std::to_string( data.mSim.size() ) );
	return data;
}