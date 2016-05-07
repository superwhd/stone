#pragma once

#include "Token.hpp"
#include "String.hpp"
#include "Reader.hpp"

namespace stone {
	using namespace ds;
	typedef char char_type;
	class Lexer {
	public:
		Lexer(String<char_type> *string) {
			m_reader = new StringReader<char_type>(string);
			m_head = 0;
			m_lineNumber = 1;
		}

		Token *read() {
			if (!fillTokens(1))
				return nullptr;
			return m_tokens[m_head++];
		}

		Token *peek(int step) {
			if (!fillTokens(step))
				return nullptr;
			return m_tokens[m_head + step];
		}

		~Lexer() {
			delete m_reader;
		}
	private:
		static bool isInCharSet(char_type ch, char_type charset[]) {
			for (uint i = 0; charset[i]; i++)
				if (ch == charset[i])
					return true;
			return false;
		}
		static bool isBlankChar(char_type ch) {
			static char_type blankCharSet[] = " \t";
			return isInCharSet(ch, blankCharSet);
		}
		bool isSymbol(char_type ch) {
			static char_type symbolBeginCharSet[] = ";,.|&^!+-*/=<>[](){}";
			return isInCharSet(ch, symbolBeginCharSet);
		}
		bool couldBeLonger(char_type ch) {
			static char_type couldBeLongerCharSet[] = "><=+-&|";
			return isInCharSet(ch, couldBeLongerCharSet);
		}
		bool isNum(char_type ch) {
			return '0' <= ch && ch <= '9';
		}
		bool isLetter(char_type ch) {
			return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
		}
		bool fillAToken(Reader<char_type> *reader) {
			char_type ch;
			while (!reader->isEnd()) {
				ch = reader->read();
				if (!isBlankChar(ch))
					break;
				reader->next();
			}
			if (reader->isEnd())
				return false;
			String<char_type> string;
			if ('_' == ch || isLetter(ch)) { // IdToken
				while (!reader->isEnd()) {
					ch = reader->read();
					if (isLetter(ch) || '_' == ch || isNum(ch))
						string.append(ch);
					else
						break;
					reader->next();
				}
				m_tokens.append(new IdToken(string, m_lineNumber));
				return true;
			} else if (isNum(ch)) { // NumToken
				int visDot = 0;
				while (!reader->isEnd()) {
					ch = reader->read();
					if (isNum(ch) || ('.' == ch)) {
						string.append(ch);
						visDot += ('.' == ch);
					} else break;
					reader->next();
				}
				if (visDot <= 1)
					m_tokens.append(new NumToken(string, m_lineNumber));
				else
					m_tokens.append(new ErrToken(string, m_lineNumber));
			} else if (isSymbol(ch)) { // SymToken
				string.append(ch);
				reader->next();
				char_type ch2 = reader->read(); // does not support << and >> currently
				if ((ch2 == '=' && isInCharSet(ch, "+-*/&|^!><")) ||
					(ch == ch2 && isInCharSet(ch, "+-&|="))) {
					string.append(ch2);
					reader->next();
				}
				m_tokens.append(new SymToken(string, m_lineNumber));
			} else { //ErrToken
				m_tokens.append(new ErrToken(string, m_lineNumber));
				reader->next();
			}
			return true;
		}
		bool fillTokens(uint atLeast = 1) {
			if (m_tokens.size() - m_head >= atLeast)
				return true;
			while (!m_reader->isEnd() && m_tokens.size() - m_head < atLeast) {
				String<char_type> line = m_reader->readLine();
				Reader<char_type> *lineReader = new StringReader<char_type>(&line);
				while (!lineReader->isEnd()) {
					fillAToken(lineReader); 
				}
				m_lineNumber++;
			}
			return m_tokens.size() - m_head >= atLeast;
		}

		Reader<char_type> *m_reader;
		Array<Token*> m_tokens;
		uint m_head;
		uint m_lineNumber;
	};
}