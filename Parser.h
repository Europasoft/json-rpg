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

namespace JSONTextUtils
{
	typedef std::string str_t;
	typedef std::string_view str_view;
	typedef char char_t;

	uint8_t ctu8(char_t c);

	uint32_t numBytesChar(char_t c);

	bool isNumerical(char_t c);
	bool isWhitespaceChar(char_t c);
	bool isStructuralChar(char_t c);

	bool isUnicodeChar(char_t c);
	bool isControlChar(char_t c);

	int32_t digitFromChar(char_t c);
	bool isDigitValid(int32_t d);

	bool isliteralBooleanStr(size_t i, str_view text);
	std::string literalBooleanValue(uint8_t c);
		
	// converts a single UTF-8 codepoint to UTF-32BE
	uint32_t utf8to32be(str_view fullString, size_t& startIndexInOut);
	// swaps the endianness of a 32-bit number (e.g. a UTF-32 codepoint)
	uint32_t swapEndian32(uint32_t u);
	// converts a whole string from UTF-8 to UTF-32
	std::u32string utf8to32str(str_view s, bool littleEndian = false);
	void test_utf8to32();

}

namespace JSON 
{
	enum class ObjectType { Undefined, Object, Array, String, Number, Boolean, Null };

	class Object
	{
	public:
		Object(ObjectType t) : type{ t } {};

		bool isNamed() const;
		bool isValue() const;
		bool isContainer() const;
		JSONTextUtils::str_view getValue() const;
		size_t size() const noexcept;

		std::unique_ptr<Object>& operator[](int i) { return subobjects[i]; }
		const std::unique_ptr<Object>& operator[](int i) const { return subobjects[i]; }

	private:
		ObjectType type = ObjectType::Undefined;
		std::vector<std::unique_ptr<Object>> subobjects;
		JSONTextUtils::str_t name, value;
	};

	Object load(JSONTextUtils::str_view text);
	Object loadFromFile(JSONTextUtils::str_view filePath);
	void testLexer(JSONTextUtils::str_view filePath);

}

namespace
{
	using str_view = JSONTextUtils::str_view;
	using str_t = JSONTextUtils::str_t;

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
		TokenType type; 
		str_t data;
		Token() = default;
		Token(TokenType t, str_view d) : type{ t }, data{ d } {};
		void reset() { data.clear(); type = TokenType::Undefined; }
	};

	Result lex(str_view text, std::vector<Token>& tokens);
	
	bool getFileSize(str_view filePath, size_t& fileSizeOut);
	bool openFile(str_view filePath, std::ifstream& fileStreamOut, size_t& fileSizeOut);

	str_view tokenTypeToString(TokenType t);


}




