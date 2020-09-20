#pragma once

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace Strutil {

	template <typename T>
	static std::string to_string_precision(const T a_value, const int precision = 6, const int width = 6) {
		std::ostringstream out;
		std::ios_base::fmtflags oldFlags = out.flags();
		out.precision(precision);
		out.setf(std::ios_base::fixed);

		out << std::setw(width) << a_value;

		out.flags(oldFlags);
		return out.str();
	}

	/**
	 * @brief Converts any datatype into std::string.
	 *        Datatype must support << operator.
	 * @tparam T
	 * @param value - will be converted into std::string.
	 * @return Converted value as std::string.
	 */
	template<typename T>
	static inline std::string to_string(T value) {
		std::stringstream ss;
		ss << value;

		return ss.str();
	}

	/**
	 * @brief Converts std::string into any datatype.
	 *        Datatype must support << operator.
	 * @tparam T
	 * @param str - std::string that will be converted into datatype T.
	 * @return Variable of datatype T.
	 */
	template<typename T>
	static inline T parse_string(const std::string & str) {
		T result;
		std::istringstream(str) >> result;

		return result;
	}

	/**
	 * @brief Converts std::string to lower case.
	 * @param str - std::string that needs to be converted.
	 * @return Lower case input std::string.
	 */
	static inline std::string to_lower(const std::string & str) {
		auto result = str;
		std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) -> unsigned char {
			return std::tolower(c);
		});

		return result;
	}

	/**
	 * @brief Converts std::string to upper case.
	 * @param str - std::string that needs to be converted.
	 * @return Upper case input std::string.
	 */
	static inline std::string to_upper(const std::string & str) {
		auto result = str;
		std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) -> unsigned char {
			return std::toupper(c);
		});

		return result;
	}

	/**
	 * @brief Compares two std::strings ignoring their case (lower/upper).
	 * @param str1 - std::string to compare
	 * @param str2 - std::string to compare
	 * @return True if str1 and str2 are equal, false otherwise.
	 */
	static inline bool compare_ignore_case(const std::string & str1, const std::string & str2) {
		return to_lower(str1) == to_lower(str2);
	}

	/**
	 * @brief Trims (in-place) white spaces from the left side of std::string.
	 *        Taken from: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring.
	 * @param str - input std::string to remove white spaces from.
	 */
	static inline void trim_left(std::string & str) {
		str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) { return !std::isspace(ch); }));
	}

	/**
	 * @brief Trims (in-place) white spaces from the right side of std::string.
	 *        Taken from: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring.
	 * @param str - input std::string to remove white spaces from.
	 */
	static inline void trim_right(std::string & str) {
		str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) { return !std::isspace(ch); }).base(), str.end());
	}

	/**
	 * @brief Trims (in-place) white spaces from the both sides of std::string.
	 *        Taken from: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring.
	 * @param str - input std::string to remove white spaces from.
	 */
	static inline void trim(std::string & str) {
		trim_left(str);
		trim_right(str);
	}

	/**
	 * @brief Trims white spaces from the left side of std::string.
	 *        Taken from: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring.
	 * @param str - input std::string to remove white spaces from.
	 * @return Copy of input str with trimmed white spaces.
	 */
	static inline std::string trim_left_copy(std::string str) {
		trim_left(str);
		return str;
	}

	/**
	  * @brief Trims white spaces from the right side of std::string.
	  *        Taken from: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring.
	  * @param str - input std::string to remove white spaces from.
	  * @return Copy of input str with trimmed white spaces.
	  */
	static inline std::string trim_right_copy(std::string str) {
		trim_right(str);
		return str;
	}

	/**
	  * @brief Trims white spaces from the both sides of std::string.
	  *        Taken from: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring.
	  * @param str - input std::string to remove white spaces from.
	  * @return Copy of input str with trimmed white spaces.
	  */
	static inline std::string trim_copy(std::string str) {
		trim(str);
		return str;
	}

	/**
	 * @brief Replaces (in-place) the first occurance of target with replacement.
	 *        Taken from: http://stackoverflow.com/questions/3418231/c-replace-part-of-a-string-with-another-string.
	 * @param str - input std::string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replecament - substring that will replace target.
	 * @return True if replacement was successfull, false otherwise.
	 */
	static inline bool replace_first(std::string & str, const std::string & target, const std::string & replecament) {
		size_t start_pos = str.find(target);
		if (start_pos == std::string::npos) {
			return false;
		}

		str.replace(start_pos, target.length(), replecament);
		return true;
	}

	/**
	 * @brief Replaces (in-place) last occurance of target with replacement.
	 *        Taken from: http://stackoverflow.com/questions/3418231/c-replace-part-of-a-string-with-another-string.
	 * @param str - input std::string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replecament - substring that will replace target.
	 * @return True if replacement was successfull, false otherwise.
	 */
	static inline bool replace_last(std::string & str, const std::string & target, const std::string & replecament) {
		size_t start_pos = str.rfind(target);
		if (start_pos == std::string::npos) {
			return false;
		}

		str.replace(start_pos, target.length(), replecament);
		return true;
	}

	/**
	 * @brief Replaces (in-place) all occurances of target with replacement.
	 *        Taken from: http://stackoverflow.com/questions/3418231/c-replace-part-of-a-string-with-another-string.
	 * @param str - input std::string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replecament - substring that will replace target.
	 * @return True if replacement was successfull, false otherwise.
	 */
	static inline bool replace_all(std::string & str, const std::string & target, const std::string & replecament) {
		if (target.empty()) {
			return false;
		}

		size_t start_pos = 0;
		const bool found_substring = str.find(target, start_pos) != std::string::npos;

		while ((start_pos = str.find(target, start_pos)) != std::string::npos) {
			str.replace(start_pos, target.length(), replecament);
			start_pos += replecament.length();
		}

		return found_substring;
	}

	/**
	 * @brief Checks if std::string str ends with specified suffix.
	 * @param str - input std::string that will be checked.
	 * @param suffix - searched suffix in str.
	 * @return True if suffix was found, false otherwise.
	 */
	static inline bool ends_with(const std::string & str, const std::string & suffix) {
		const auto pos = str.rfind(suffix);

		return (pos != std::string::npos) && (pos == (str.length() - suffix.length()));
	}

	/**
	 * @brief Checks if std::string str starts with specified prefix.
	 * @param str - input std::string that will be checked.
	 * @param prefix - searched prefix in str.
	 * @return True if prefix was found, false otherwise.
	 */
	static inline bool starts_with(const std::string & str, const std::string & prefix) {
		return str.find(prefix) == 0;
	}

	/**
	 * @brief Splits input std::string str according to input delim.
	 * @param str - std::string that will be splitted.
	 * @param delim - the delimiter.
	 * @return std::vector<std::string> that contains all splitted tokens.
	 */
	static inline std::vector<std::string> split(const std::string & str, const char delim) {
		std::vector<std::string> tokens;
		std::stringstream ss(str);

		std::string token;
		while (std::getline(ss, token, delim)) {
			tokens.push_back(token);
		}

		return tokens;
	}

	/**
	 * @brief Splits input std::string str according to input std::string delim.
	 *        Taken from: https://stackoverflow.com/a/46931770/1892346.
	 * @param str - std::string that will be splitted.
	 * @param delim - the delimiter.
	 * @return std::vector<std::string> that contains all splitted tokens.
	 */
	static inline std::vector<std::string> split(const std::string & str, const std::string & delim) {
		size_t pos_start = 0, pos_end, delim_len = delim.length();
		std::string token;
		std::vector<std::string> tokens;

		while ((pos_end = str.find(delim, pos_start)) != std::string::npos) {
			token = str.substr(pos_start, pos_end - pos_start);
			pos_start = pos_end + delim_len;
			tokens.push_back(token);
		}

		tokens.push_back(str.substr(pos_start));
		return tokens;
	}

	/**
	 * @brief Joins all elements of std::vector tokens of arbitrary datatypes
	 *        into one std::string with delimiter delim.
	 * @tparam T - arbitrary datatype.
	 * @param tokens - vector of tokens.
	 * @param delim - the delimiter.
	 * @return std::string with joined elements of vector tokens with delimiter delim.
	 */
	template<typename T>
	static inline std::string join(const std::vector<T> & tokens, const std::string & delim) {
		std::ostringstream result;
		for (auto it = tokens.begin(); it != tokens.end(); ++it) {
			if (it != tokens.begin()) {
				result << delim;
			}

			result << *it;
		}

		return result.str();
	}

	/**
	 * @brief Creates new std::string with repeated n times substring str.
	 * @param str - substring that needs to be repeated.
	 * @param n - number of iterations.
	 * @return std::string with repeated substring str.
	 */
	static inline std::string repeat(const std::string & str, unsigned n) {
		std::string result;

		for (unsigned i = 0; i < n; ++i) {
			result += str;
		}

		return result;
	}

	/**
	 * @brief Creates new std::string with repeated n times char c.
	 * @param c - char that needs to be repeated.
	 * @param n - number of iterations.
	 * @return std::string with repeated char c.
	 */
	static inline std::string repeat(char c, unsigned n) {
		return std::string(n, c);
	}

	/**
	 * @brief Checks if input std::string str contains specified substring.
	 * @param str - std::string to be checked.
	 * @param substring - searched substring.
	 * @return True if substring was found in str, false otherwise.
	 */
	static inline bool contains(const std::string & str, const std::string & substring) {
		return str.find(substring) != std::string::npos;
	}

	/**
	 * @brief Checks if input std::string str matches specified reular expression regex.
	 * @param str - std::string to be checked.
	 * @param regex - the std::regex regular expression.
	 * @return True if regex matches str, false otherwise.
	 */
	static inline bool matches(const std::string & str, const std::regex & regex) {
		return std::regex_match(str, regex);
	}
}
