#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

#define STC_SBR_L '['
#define STC_SBR_R ']'
#define STC_CBR_L '{'
#define STC_CBR_R '}'
#define STC_CL ':'
#define STC_CM ','
#define STR_DELIM '"'

namespace JSON 
{
	typedef std::string str_t;
	typedef std::string_view str_view;
	typedef char char_t;

	class JSONFile {};

	JSONFile load(str_view text);
	JSONFile loadFromFile(str_view filePath);
	void testLexer(str_view filePath);

}

namespace
{
	using str_t = JSON::str_t;
	using str_view = JSON::str_view;
	using char_t = JSON::char_t;
	
	uint8_t ctu8(char_t c) { return static_cast<uint8_t>(c); } // (signed) char to unsigned int

	enum class Result
	{
		OK = 0,
		Error_InvalidEncoding = 1, // unsupported character encoding
		Error_IllegalToken = 2, // disallowed ASCII in structural text
		Error_IllegalTokenMultiByte = 3, // multi-byte character in structural text
		Error_IncompleteUnicodeInString = 4, // incomplete or corrupted multi-byte codepoint
	};


	enum class TokenType { Undefined, Structural, String, Number, Boolean, Null };
	struct Token 
	{ 
		TokenType type; str_t data;
		Token() = default;
		Token(TokenType t, str_view d) : type{ t }, data{ d } {};
		void reset() { data.clear(); type = TokenType::Undefined; }
	};

	Result lex(str_view text, std::vector<Token>& tokens);

	uint32_t numBytesChar(char_t c);
	bool isNumerical(char_t c);
	bool isWhitespaceChar(char_t c);
	bool isStructuralChar(char_t c);


	bool isUnicodeChar(char_t c) { return numBytesChar(c) > 1; }
	bool isControlChar(char_t c) { return ctu8(c) <= 31; }

	
	int32_t digitFromChar(char_t c) { return static_cast<int32_t>(ctu8(c)) - 48; }
	bool isDigitValid(int32_t d) { return d >= 0 && d <= 9; }

	bool isliteralBooleanStr(size_t i, str_view text);
	str_t literalBooleanValue(uint8_t c) { return c == 0x74 ? "1" : "0"; }
	

	bool getFileSize(str_view filePath, size_t& fileSizeOut);
	bool openFile(str_view filePath, std::ifstream& fileStreamOut, size_t& fileSizeOut);

	str_view tokenTypeToString(TokenType t);
}

