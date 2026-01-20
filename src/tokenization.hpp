#pragma once
#include <optional>
#include <string>
#include <stdexcept>
#include <unordered_map>

enum class TokenType
{
  exit,
  int_lit,
  semi,
  ident,
  cnst,
  assign,
  plus,
  mod,
  mul,
  sub,
  div,
  open_paren,
  close_paren,
  open_curly,
  close_curly,
  eq,
  lt,
  gt,
  lte,
  gte,
  neq,
  if_,
  print,
  else_,
  elif,
  int_,
  char_,
  char_lit,
  bool_,
  bool_lit,
  true_,
  false_,
  let,
  and_,
  or_,
  not_,

};

struct Token
{
  TokenType type;
  std::optional<std::string> val;
  Token(TokenType t, std::optional<std::string> v = std::nullopt)
      : type(t), val(std::move(v)) {}
};

class Tokeniser
{
public:
  explicit Tokeniser(std::string contents) : src(std::move(contents))
  {
  }

  std::vector<Token> tokenise()
  {
    std::vector<Token> tokens;
    std::string buf = "";

    // Map for single-character tokens
    const std::unordered_map<char, TokenType> singleCharTokens = {
        {';', TokenType::semi},
        {'=', TokenType::assign},
        {'+', TokenType::plus},
        {'*', TokenType::mul},
        {'-', TokenType::sub},
        {'/', TokenType::div},
        {'<', TokenType::lt},
        {'>', TokenType::gt},
        {'%', TokenType::mod},
        {'(', TokenType::open_paren},
        {')', TokenType::close_paren},
        {'{', TokenType::open_curly},
        {'}', TokenType::close_curly},
        {'!', TokenType::not_}};

    const std::unordered_map<std::string, TokenType> doubleCharTokens = {
        {"==", TokenType::eq},
        {"!=", TokenType::neq},
        {"<=", TokenType::lte},
        {">=", TokenType::gte},
        {"&&", TokenType::and_},
        {"||", TokenType::or_}};

    // Map for keywords
    const std::unordered_map<std::string, TokenType> keywords = {
        {"exit", TokenType::exit},
        {"const", TokenType::cnst},
        {"print", TokenType::print},
        {"if", TokenType::if_},
        {"else", TokenType::else_},
        {"elif", TokenType::elif},
        {"int", TokenType::int_},
        {"char", TokenType::char_},
        {"bool", TokenType::bool_},
        {"true", TokenType::true_},
        {"false", TokenType::false_},
        {"let", TokenType::let}};

    while (peek().has_value())
    {
      char c = peek().value();

      if (std::isalpha(c))
      {
        buf.push_back(consume());
        while (peek().has_value() && std::isalnum(peek().value()))
        {
          buf.push_back(consume());
        }

        auto it = keywords.find(buf);
        if (it != keywords.end())
        {
          // Handle boolean literals
          if (it->second == TokenType::true_ || it->second == TokenType::false_)
          {
            tokens.emplace_back(TokenType::bool_lit, buf);
          }
          else
          {
            tokens.emplace_back(it->second);
          }
        }
        else
        {
          tokens.emplace_back(TokenType::ident, buf);
        }
        buf.clear();
      }
      else if (std::isdigit(c))
      {
        buf.push_back(consume());
        while (peek().has_value() && std::isdigit(peek().value()))
        {
          buf.push_back(consume());
        }
        tokens.emplace_back(TokenType::int_lit, buf);
        buf.clear();
      }
      else if (std::isspace(c))
      {
        consume();
      }
      else if (c == '\'')
      {
        consume(); // consume opening '

        if (!peek().has_value())
        {
          std::cerr << "Unexpected end of input after '\''\n";
          std::exit(EXIT_FAILURE);
        }

        char charValue;
        char nextChar = consume();

        if (nextChar == '\\') // escaped character
        {
          if (!peek().has_value())
          {
            std::cerr << "Unexpected end of input after escape character\n";
            std::exit(EXIT_FAILURE);
          }

          char escapeChar = consume();
          switch (escapeChar)
          {
          case 'n':
            charValue = '\n';
            break;
          case 't':
            charValue = '\t';
            break;
          case '\\':
            charValue = '\\';
            break;
          case '\'':
            charValue = '\'';
            break;
          case '0':
            charValue = '\0';
            break;
          default:
            std::cerr << "Unknown escape sequence \\" << escapeChar << "\n";
            std::exit(EXIT_FAILURE);
          }
        }
        else
        {
          if (nextChar == '\n')
          {
            std::cerr << "Error: newline in character literal\n";
            std::exit(EXIT_FAILURE);
          }
          charValue = nextChar; // normal character
        }

        // closing '
        if (!peek().has_value() || peek().value() != '\'')
        {
          std::cerr << "Expected closing single quote for char literal\n";
          std::exit(EXIT_FAILURE);
        }

        consume(); // consume closing '
        buf = charValue;
        tokens.emplace_back(TokenType::char_lit, buf);
        buf.clear();
        continue;
      }
      else if (c == '/')
      {
        if (peek(1).has_value() && peek(1).value() == '/')
        {
          while (peek().has_value() && peek().value() != '\n')
          {
            consume();
          }
        }
        else if (peek(1).has_value() && peek(1).value() == '*')
        {
          consume();
          consume();
          while (peek().has_value() && peek().value() != '*' && peek(1).has_value() && peek(1).value() != '/')
          {
            consume();
          }
          consume();
          consume();
        }
      }
      else
      {
        // Check for two-character operators first
        if (peek(1).has_value())
        {
          std::string twoCharOp = {peek().value(), peek(1).value()};
          auto it2 = doubleCharTokens.find(twoCharOp);
          if (it2 != doubleCharTokens.end())
          {
            tokens.emplace_back(it2->second);
            consume();
            consume();
            continue;
          }
        }

        // Then fallback to single-char tokens
        auto it = singleCharTokens.find(c);
        if (it != singleCharTokens.end())
        {
          tokens.emplace_back(it->second);
          consume();
        }
        else
        {
          std::cerr << "Wrong input: unknown character '" << c << "'\n";
          std::exit(EXIT_FAILURE);
        }
      }
    }

    return tokens;
  }

  std::optional<char> peek(int offset = 0)
  {
    if (index + offset >= src.size())
    {
      return {};
    }
    else
    {
      return src[index + offset];
    }
  }

  char consume()
  {
    return src[index++];
  }

  const std::string src;
  size_t index = 0;
};