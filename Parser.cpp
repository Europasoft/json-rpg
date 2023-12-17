#include "Parser.h"

#include <limits.h>
#include <iostream>
#include <format>
#include <cassert>
#include <iomanip>

namespace JSONTextUtils
{
	uint8_t ctu8(char_t c)
	{
		return static_cast<uint8_t>(c); // (signed) char to unsigned int
	}

	uint32_t numBytesChar(char_t c)
	{
		const auto uc = ctu8(c);
		if (uc <= 127) { return 1; } // U+0000 - U+007F (ASCII)
		else if ((uc & 0xE0) == 0xC0) { return 2; } // U+0080 - U+07FF
		else if ((uc & 0xF0) == 0xE0) { return 3; } // U+0800 - U+FFFF
		else if ((uc & 0xF8) == 0xF0) { return 4; } // U+10000 - U+10FFFF
		else { return 0; } // unknown encoding
	}

	bool isNumerical(char_t c)
	{
		return isDigitValid(digitFromChar(c)) || // base 10 digits
			ctu8(c) == 69 || ctu8(c) == 101 || // E, e
			ctu8(c) == 43 || ctu8(c) == 45 || ctu8(c) == 46; // +, -, .
	}

	bool isWhitespaceChar(char_t c)
	{
		const auto uc = ctu8(c);
		return uc == 0x20 || uc == 0x09 || uc == 0x0A || uc == 0x0D; // space, tab, newline, carriage return
	}

	bool isStructuralChar(char_t c)
	{
		return c == STC_SBR_L || c == STC_SBR_R || c == STC_CBR_L || c == STC_CBR_R ||
			c == STC_CL || c == STC_CM;
	}

	bool isUnicodeChar(char_t c)
	{
		return numBytesChar(c) > 1;
	}

	bool isControlChar(char_t c)
	{
		return ctu8(c) <= 31;
	}

	int32_t digitFromChar(char_t c)
	{
		return static_cast<int32_t>(ctu8(c)) - 48;
	}

	bool isDigitValid(int32_t d)
	{
		return d >= 0 && d <= 9;
	}

	bool isliteralBooleanStr(size_t i, str_view text)
	{
		const int64_t len = text.length() - i;
		if (len < 4) { return false; }
		str_view s = text.substr(i, len == 4 ? 4 : 5);
		return s == "true" || s == "false";
	}

	std::string literalBooleanValue(uint8_t c)
	{
		return c == 0x74 ? "1" : "0";
	}

	bool isLiteralNullStr(size_t i, str_view text)
	{
		if (text.length() - i < 4) { return false; }
		return ctu8(text[i]) == 0x6E && ctu8(text[i + 1]) == 0x75 && ctu8(text[i + 2]) == 0x6C && ctu8(text[i + 3]) == 0x6C;
	}

	uint32_t utf8to32be(str_view fullString, size_t& startIndexInOut)
	{
		const auto numBytes = numBytesChar(ctu8(fullString[startIndexInOut]));
		if (numBytes == 1)
		{
			return ctu8(fullString[startIndexInOut++]);
		}
		assert(numBytes > 0 && numBytes < 5 && "Invalid UTF-8 start byte");
		assert(startIndexInOut + numBytes <= fullString.size() && "Invalid or incomplete UTF-8 codepoint");

		const auto sf = 24 + numBytes;
		uint32_t value = ((ctu8(fullString[startIndexInOut]) << sf) >> sf);
		value = (value << (6 * (numBytes - 1)));
		for (uint32_t i = 1; i < numBytes; i++)
		{
			value |= (ctu8(fullString[startIndexInOut + i]) & 0x7F) << (6 * (numBytes - (i + 1)));
		}
		startIndexInOut += numBytes;
		return value;
	}

	uint32_t swapEndian32(uint32_t u)
	{
		return ((u & 0xFF) << 24) | ((u & 0xFF00) << 8) | ((u & 0xFF0000) >> 8) | ((u & 0xFF000000) >> 24);
	}

	std::u32string utf8to32str(str_view s, bool littleEndian)
	{
		std::u32string s32;
		size_t i = 0;
		if (!littleEndian)
		{
			while (i < s.size()) { s32 += utf8to32be(s, i); }
		}
		else
		{
			while (i < s.size()) { s32 += swapEndian32(utf8to32be(s, i)); }
		}
		return s32;
	}

	/*void test_utf8to32()
	{
		str_t vec("\xE0\xA4\xB9");
		std::cout << "\nUTF-8: ";
		for (auto c : vec)
		{
			std::cout << std::format("{:08b}", ctu8(c)) << ", ";
		}
		std::cout << "\n";
		size_t i = 0;
		while (i < vec.size())
		{
			uint32_t b = utf8to32be(vec, i);
			std::cout << "\nUTF-32BE: 0x" << std::hex << b << " (" << std::format("{:032b}", b) << ")";
		}
		std::cout << "\n";
		i = 0;
		while (i < vec.size())
		{
			uint32_t l = utf8to32le(vec, i);
			std::cout << "\nUTF-32LE: 0x" << std::hex << l << " (" << std::format("{:032b}", l) << ")";
		}
	}*/

	void test_utf8to32()
	{
		str_t s("\xE0\xA4\xB9");
		std::cout << "\nUTF-8: ";
		for (auto c : s)
		{
			std::cout << std::format("{:08b}", ctu8(c)) << ", ";
		}

		std::u32string s32be = utf8to32str(s);
		std::u32string s32le = utf8to32str(s, true);
		std::cout << "\nUTF-32BE:" << std::hex;
		for (auto c : s32be)
		{
			std::cout << " 0x" << static_cast<int32_t>(c);

		}
		std::cout << "\nUTF-32LE:";
		for (auto c : s32le)
		{
			std::cout << " 0x" << static_cast<int32_t>(c);
		}
	}
}

namespace JSON 
{
	JSONFile load(JSONTextUtils::str_view text)
	{
		return JSONFile();
	}

	JSONFile loadFromFile(JSONTextUtils::str_view filePath)
	{
		return JSONFile();
	}

	void testLexer(JSONTextUtils::str_view filePath)
	{
		std::ifstream fs;
		size_t fileSize;
		if (!openFile(filePath, fs, fileSize)) { return; }
		JSONTextUtils::str_t file;
		file.resize(fileSize);
		fs.read(&file[0], file.length());

		std::vector<Token> tokens;
		Result r = lex(file, tokens);
		for (auto& token : tokens) { std::cout << "  " << token.data; }
		std::cout << "\n\n";
		for (auto& token : tokens) { std::cout << "  " << token.data << "=" << tokenTypeToString(token.type); }
	}
}

namespace
{

	Result lex(JSONTextUtils::str_view text, std::vector<Token>& tokens)
	{
		static_assert(CHAR_BIT == 8);
		Token token;
		bool inString = false;
		bool inNumber = false;
		bool inLiteral = false;

		for (size_t i = 0; i < text.length(); i++)
		{
			const auto c = JSONTextUtils::ctu8(text[i]);
			auto numBytes = JSONTextUtils::numBytesChar(c);
			if (!numBytes) { return Result::Error_InvalidEncoding; }

			if (!inString) // structural
			{
				if (inNumber && !JSONTextUtils::isNumerical(c))
				{
					tokens.push_back(token);
					token.reset();
					inNumber = false;
				}
				if (JSONTextUtils::isNumerical(c))
				{
					if (inNumber) { token.data.push_back(c); } // digit or symbol (found next digit)
					else { token.data.push_back(c); token.type = TokenType::Number; inNumber = true; } // start of number
				} 
				else if (JSONTextUtils::isWhitespaceChar(c)) { continue; }
				else if (JSONTextUtils::isStructuralChar(c)) { tokens.push_back(Token(TokenType::Structural, str_t(1, c))); }
				else if (c == STR_DELIM) { token.type = TokenType::String; inString = true; }

				// TODO: check for all possible literals
				else if (JSONTextUtils::isliteralBooleanStr(i, text)) { tokens.push_back(Token(TokenType::Boolean, JSONTextUtils::literalBooleanValue(c))); }
				else if (numBytes > 1) { return Result::Error_IllegalTokenMultiByte; }
				else { return Result::Error_IllegalToken; }
			}
			else // string
			{
				if (numBytes == 1)
				{
					if (c != STR_DELIM) { token.data.push_back(c); } // string constituent ASCII
					else // end of string
					{
						tokens.push_back(token);
						token.reset();
						inString = false;
					}
				}
				else
				{
					// start of multi-byte UTF-8 codepoint
				}
			}
		}
		return Result::OK;
	}


	bool getFileSize(JSONTextUtils::str_view filePath, size_t& fileSizeOut)
	{
		if (!std::filesystem::exists(filePath)) { return false; }
		fileSizeOut = static_cast<size_t>(std::filesystem::file_size(filePath));
		return true;
	}

	bool openFile(JSONTextUtils::str_view filePath, std::ifstream& fileStreamOut, size_t& fileSizeOut)
	{
		if (!getFileSize(filePath, fileSizeOut)) { return false; }
		fileStreamOut.open(filePath, std::ios_base::binary);
		return static_cast<bool>(fileStreamOut);
	}

	JSONTextUtils::str_view tokenTypeToString(TokenType t)
	{
		switch (t)
		{
		case TokenType::Undefined: return str_view("Undefined");
		case TokenType::Structural: return str_view("Structural");
		case TokenType::String: return str_view("String");
		case TokenType::Number: return str_view("Number");
		case TokenType::Boolean: return str_view("Boolean");
		case TokenType::Null: return str_view("Null");
		default: return str_view("UnknownTokenType");
		}
	}
}



