#ifndef strfuncs_hpp
#define strfuncs_hpp

#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <iomanip>

template <typename T>
inline std::string num2str(T num)
{
  std::ostringstream ss;
  ss << num;
  return ss.str();
}

template <typename T>
inline std::string num2str(T num, int width)
{
  std::ostringstream ss;
  ss << std::setfill('0') << std::setw(width) << num;
  return ss.str();
}

template <typename T>
inline std::string num2hex(T num)
{
  std::ostringstream ss;
  ss << std::hex << num;
  return ss.str();
}

template <typename T>
inline T str2num(std::string const& str)
{
  std::istringstream ss(str);
  T result;
  return ss >> result ? result : 0;
}

inline void tokenize(std::string const& line, std::vector<std::string>& tokens)
{
  std::istringstream iss(line);
  std::copy(std::istream_iterator<std::string>(iss),
	    std::istream_iterator<std::string>(),
	    std::back_inserter<std::vector<std::string> >(tokens));
}

#endif // strfuncs_hpp
