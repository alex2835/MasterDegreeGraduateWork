
#include "load_data.hpp"
#include <format>
#include <ranges>
#include <algorithm>

InputData LoadData( std::vector<std::string> files )
{
	InputData data;
	std::vector<Column> colums;

	// Get cols
	for( const auto& path : files )
	{
		std::ifstream file( path );
		std::string column_names;
		std::getline( file, column_names );

		std::vector<Column> file_columns;
		for( auto name : Split( column_names, ","sv ) )
			file_columns.emplace_back( Column{ std::string( Trim( name ) ) } );

		for( std::string line; std::getline( file, line ); )
		{
			int i = 0;
			for( auto value : Split( line, ","sv ) )
				file_columns[i++].mData.push_back( ParseFloat( std::string( Trim( value ) ) ) );
		}
		colums.insert( colums.end(), std::move_iterator( file_columns.begin() ),
								     std::move_iterator( file_columns.end() ) );
	}

	// Convert into rows
	std::vector<Column*> exp_cols;
	std::vector<Column*> sim_cols;
	for( auto& col : colums )
	{
		if( col.mName.ends_with( "exp" ) )
			exp_cols.push_back( &col );
		else
			sim_cols.push_back( &col );
	}
	if( sim_cols.size() != exp_cols.size() )
		throw std::runtime_error( std::format( "Invalid columns amount sim: {} != exp: {}",
								  sim_cols.size(), exp_cols.size() ) );
	
	auto sort_func = []( const Column* col1, const Column* col2 )
	{
		return col1->mName < col2->mName;
	};
	std::ranges::sort( exp_cols, sort_func );
	std::ranges::sort( sim_cols, sort_func );

	auto fill_rows = []( std::vector<Column*> cols ) -> Rows
	{
		Rows rows;
		for( const auto* col : cols )
			rows.mNames.push_back( col->mName );
		for( size_t i = 0; i < cols.front()->mData.size(); i++ )
		{
			sfVec row;
			for( const auto* col : cols )
				row.push_back( col->mData[i] );
			rows.mData.push_back( std::move( row ) );
		}
		return rows;
	};
	data.mExp = fill_rows( exp_cols );
	data.mSim = fill_rows( sim_cols );
	return data;
}