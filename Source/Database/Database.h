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

#define TYPE_TO_STRING(__in_type) \
		QueryValue(__in_type __in) \
			: m_val(std::to_string(__in_type(__in))) \
		{ \
		} \
		QueryValue operator=(__in_type __in) \
		{ \
			m_val = std::to_string(__in_type(__in)); \
		}

		// define operators / init funcs

		// 8 bits
		TYPE_TO_STRING(int8_t)
		TYPE_TO_STRING(uint8_t)
		// 16 bits
		TYPE_TO_STRING(int16_t)
		TYPE_TO_STRING(uint16_t)
		// 32 bits
		TYPE_TO_STRING(int32_t)
		TYPE_TO_STRING(uint32_t)
		// 64 bits
		TYPE_TO_STRING(int64_t)
		TYPE_TO_STRING(uint64_t)

		// Float point
		TYPE_TO_STRING(double)
		TYPE_TO_STRING(float)

		std::string Value()
		{
			return m_val;
		}
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

