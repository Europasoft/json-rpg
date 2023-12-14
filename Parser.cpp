#include "Parser.h"
#include <limits.h>
#include <iostream>

namespace JSON 
{
	JSONFile load(str_view text)
	{
		return JSONFile();
	}

	JSONFile loadFromFile(str_view filePath)
	{
		return JSONFile();
	}

	void testLexer(str_view filePath)
	{
		std::ifstream fs;
		size_t fileSize;
		if (!openFile(filePath, fs, fileSize)) { return; }
		str_t file;
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
	Result lex(str_view text, std::vector<Token>& tokens)
	{
		static_assert(CHAR_BIT == 8);
		Token token;
		bool inString = false;
		bool inNumber = false;
		bool inLiteral = false;

		for (size_t i = 0; i < text.length(); i++)
		{
			const auto c = ctu8(text[i]);
			auto numBytes = numBytesChar(c);
			if (!numBytes) { return Result::Error_InvalidEncoding; }

			if (!inString) // structural
			{
				if (inNumber && !isNumerical(c))
				{
					tokens.push_back(token);
					token.reset();
					inNumber = false;
				}
				if (isNumerical(c))
				{
					if (inNumber) { token.data.push_back(c); } // digit or symbol (found next digit)
					else { token.data.push_back(c); token.type = TokenType::Number; inNumber = true; } // start of number
				} 
				else if (isWhitespaceChar(c)) { continue; }
				else if (isStructuralChar(c)) { tokens.push_back(Token(TokenType::Structural, str_t(1, c))); }
				else if (c == STR_DELIM) { token.type = TokenType::String; inString = true; }

				// TODO: check for all possible literals
				else if (isliteralBooleanStr(i, text)) { tokens.push_back(Token(TokenType::Boolean, literalBooleanValue(c))); }
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
	

	bool isNumerical(char_t c) 
	{ 
		return isDigitValid(digitFromChar(c)) || // base 10 digits
			ctu8(c) == 69 || ctu8(c) == 101 || // E, e
			ctu8(c) == 43 || ctu8(c) == 45 || ctu8(c) == 46; // +, -, .
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

	bool isliteralBooleanStr(size_t i, str_view text)
	{
		const int64_t len = text.length() - i;
		if (len < 4) { return false; }
		str_view s = text.substr(i, len == 4 ? 4 : 5);
		return s == "true" || s == "false";
	}

	bool isLiteralNullStr(size_t i, str_view text) 
	{
		if (text.length() - i < 4) { return false; }
		return ctu8(text[i]) == 0x6E && ctu8(text[i+1]) == 0x75 && ctu8(text[i+2]) == 0x6C && ctu8(text[i+3]) == 0x6C;
	}


	bool getFileSize(str_view filePath, size_t& fileSizeOut)
	{
		if (!std::filesystem::exists(filePath)) { return false; }
		fileSizeOut = static_cast<size_t>(std::filesystem::file_size(filePath));
		return true;
	}

	bool openFile(str_view filePath, std::ifstream& fileStreamOut, size_t& fileSizeOut)
	{
		if (!getFileSize(filePath, fileSizeOut)) { return false; }
		fileStreamOut.open(filePath, std::ios_base::binary);
		return static_cast<bool>(fileStreamOut);
	}

	str_view tokenTypeToString(TokenType t) 
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