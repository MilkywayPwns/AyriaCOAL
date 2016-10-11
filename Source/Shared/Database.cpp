/*
	Initial author: (https://github.com/)MilkywayPwns for Ayria.se
	License: GPL 2.0
	Started: 2016-10-11
	Notes:
		Database functions
*/

#include "../STDInclude.h"
#include "../Utility/Strings/Variadicstring.h"
#include "Database.h"

// Pointer to database
sqlite3 *Database::DatabasePtr;

// SQL Insert function
bool Database::Insert(std::string table, std::map < std::string, QueryValue > values)
{
	// Check if database connection is open
	if (!Database::DatabasePtr)
	{
		VAPrint("Database error: Tried to query the database, but the database connection has been closed.\n");
		return false;
	}
	
	// Temponary variables
	std::string valstr;
	std::string colstr;

	// Convert from array into strings
	for (auto it : values)
	{
		// Add columns and values to the query, replace unallowed tokens with its html code
		colstr += Database::EscapeString(it.first) + ",";
		valstr += "'" + Database::EscapeString(it.second.ToString()) + "',";
	}

	// Remove the last ,
	colstr[colstr.size() - 1] = '\0';
	valstr[valstr.size() - 1] = '\0';

	// Create SQL query string
	std::string SQL = "INSERT INTO " + Database::EscapeString(table) + " (" + colstr + ") " + 
	"VALUES " + valstr;

	// Execute SQL query
	char *error;
	int result = sqlite3_exec(Database::DatabasePtr, SQL.c_str(), nullptr, nullptr, &error);

	// Check if query succeeded
	if (result != SQLITE_OK && error)
		VAPrint("Database error: %s", error);

	// Return result
	return (result == SQLITE_OK);
}

// SQL Select function
std::vector< Database::QueryResult > Database::Select(std::string table, std::map < std::string, QueryValue > where_stmt, std::vector < std::string > column_stmt)
{
	// Return result
	std::vector < Database::QueryResult > result;

	// Check if database connection is open
	if (!Database::DatabasePtr)
	{
		VAPrint("Database error: Tried to query the database, but the database connection has been closed.\n");
		return result;
	}

	// Build the SELECT data
	std::string data_select = "*";

	// Update selected columns, if specified
	if (column_stmt.size() > 0)
	{
		// Empty query
		data_select = "";

		// Add column names to select query
		for (auto col : column_stmt)
			data_select += Database::EscapeString(col) + ",";

		// 0 terminate string early
		data_select[data_select.size() - 1] = '\0';
	}

	// Build the WHERE data
	std::string data_where = "";

	// Update WHERE data, if specified
	if (where_stmt.size() > 0)
	{
		// Loop through columns
		bool firstEntry = true;
		for (auto col : where_stmt)
		{
			// Add column condition into query
			data_where += ((firstEntry) ? "WHERE " : "AND ") + Database::EscapeString(col.first) + "='" + Database::EscapeString(col.second.ToString()) + "' ";
			
			// Make sure WHERE changes into AND after first entry has been parsed
			firstEntry = false;	
		}
	}

	// Build SQL query
	std::string SQL = "SELECT " + data_select + " " + 
		"FROM " + Database::EscapeString(table) + " " + data_where;

	// Initiate SQL query
	sqlite3_stmt *statement = nullptr;
	const char *pzTail = nullptr;
	int rc = sqlite3_prepare_v2(Database::DatabasePtr, SQL.c_str(), -1, &statement, &pzTail);

	// Check if query succeeded
	if (rc == SQLITE_OK)
	{
		// Loop through the data
		while (sqlite3_step(statement))			// While there is a next row available within our interest...
		{
			// Class with our data
			QueryResult row;

			// Get the number of columns in the table
			int columns = sqlite3_column_count(statement);

			// Loop through columns, add them to our array
			for (int i = 0; i < columns; i++)
			{
				// Get column name
				std::string colname = std::string(sqlite3_column_name(statement, i));

				// Get value for column
				switch (int type = sqlite3_column_type(statement, i))
				{
					case SQLITE_TEXT:
						row.append(colname, QueryValue(UnescapeString((const char*)sqlite3_column_text(statement, i))));
						break;
					case SQLITE_INTEGER:
						row.append(colname, QueryValue(sqlite3_column_int64(statement, i)));
						break;
					// TODO: Add support for more types
					default:
						VAPrint("Database error: Unhandled data type %i in function %s\n", type, __FUNCTION__);
				}
			}

			// Add data to return result
			result.push_back(row);
		}
	}
	else
	{
		// Something went wrong, log to console
		VAPrint("Database error: Something went wrong while trying to obtain data from the database, errorcode is %i (0x%X).\n", rc, rc);
		return result;
	}

	// Free memory
	sqlite3_free(statement);

	// Return array of results
	return result;
}

// EscapeString, replaces specific characters with its html code
std::string Database::EscapeString(std::string str)
{
	std::string _out = "";

	// loop through characters in the input string
	for (auto chr : str)
	{
		if (chr == '\'')
			_out += "&apos;";
		else if (chr == '\"')
			_out += "&quot;";
		else
			_out += chr;
	}

	// return escaped string
	return _out;
}
// Opposite of EscapeString
std::string Database::UnescapeString(std::string str)
{
	std::string _out = "";

	// loop through characters in the input string
	for (size_t idx = 0; idx < str.size(); idx++)
	{
		if (str.substr(idx, 6) == "&apos;")				// single quote
			_out += "\'";
		else if (str.substr(idx, 6) == "&quot;")		// double quote
			_out += "\"";
		else											// any other character
			_out += str[idx];
	}

	// return escaped string
	return _out;
}

// Load/Save the database
bool Database::Load(std::string filepath)
{
	// Open the database connection, returns true if succeeded
	return (sqlite3_open(filepath.c_str(), &Database::DatabasePtr) == SQLITE_OK);
}
void Database::Save()
{
	// Close the database connection
	sqlite3_close(Database::DatabasePtr);
}
