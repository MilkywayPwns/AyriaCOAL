/*
	Initial author: (https://github.com/)MilkywayPwns for Ayria.se
	License: GPL 2.0
	Started: 2016-10-11
	Notes:
		Database functions
*/

// SQLite header
#include "../../Thirdparty/sqlite/sqlite3.h"
#include <map>

// Pointer to database

namespace Database
{
	// Database pointer
	extern sqlite3 *DatabasePtr;

	// Classes
	class QueryValue
	{
	protected:
		std::string m_val;

	public:
		QueryValue()
			: m_val("")
		{
		}
		QueryValue(std::string str)
			: m_val(str)
		{
		}
		QueryValue operator=(std::string str)
		{
			m_val = str;
		}
		std::string ToString()
		{
			return m_val;
		}

#define GENERATE_TYPEFUNCS(__in_type) \
		QueryValue(__in_type __in) \
			: m_val(std::to_string(__in_type(__in))) \
		{ \
		} \
		QueryValue operator=(__in_type __in) \
		{ \
			m_val = std::to_string(__in_type(__in)); \
		}

#define GENERATE_TYPEFUNCS_ADV(__in_type, __in_to_name, __in_to_func) \
		GENERATE_TYPEFUNCS(__in_type) \
		__in_type __in_to_name() \
		{ \
			return __in_to_func(m_val); \
		}

		// define operators / init funcs

		// 8 bits
		GENERATE_TYPEFUNCS(int8_t)
		GENERATE_TYPEFUNCS(uint8_t)
		// 16 bits
		GENERATE_TYPEFUNCS(int16_t)
		GENERATE_TYPEFUNCS(uint16_t)
		// 32 bits
		GENERATE_TYPEFUNCS_ADV(int32_t, ToInt32, std::stoi)
		GENERATE_TYPEFUNCS_ADV(uint32_t, ToUint32, std::stoul)
		// 64 bits
		GENERATE_TYPEFUNCS_ADV(int64_t, ToInt64, std::stoll)
		GENERATE_TYPEFUNCS_ADV(uint64_t, ToUint64, std::stoull)
		// Float point
		GENERATE_TYPEFUNCS_ADV(double, ToDouble, std::stod)
		GENERATE_TYPEFUNCS_ADV(float, ToFloat, std::stof)
	};
	class QueryResult
	{
	protected:
		std::map < std::string, QueryValue > result;

	public:
		QueryResult()
		{

		}

		// Get
		std::map < std::string, QueryValue > GetRaw()
		{
			return result;
		}
		size_t size()
		{
			return result.size();
		}

		// Set
		void append(std::string column, QueryValue val)
		{
			result[column] = val;
		}

		// Operators
		QueryValue operator [](std::string col)
		{
			return result[col];
		}
		void operator=(QueryResult &other)
		{
			this->result = other.GetRaw();
		}
	};

	// Query functions
	bool Insert(std::string table, std::map < std::string, QueryValue > values);
	std::vector < QueryResult > Select(std::string table, std::map < std::string, QueryValue > where_stmt = {}, std::vector < std::string > column_stmt = {});

	// Escapes characters, prevents SQLi
	std::string EscapeString(std::string str);
	std::string UnescapeString(std::string str);

	// Database loading/saving
	void Save();
	bool Load(std::string filepath);
}

