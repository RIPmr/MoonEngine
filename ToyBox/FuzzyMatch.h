#pragma once
#include <cstdint>
#include <ctype.h>
#include <cstring>
#include <cstdio>

namespace MOON {
	class MatchTool {
	public:
		inline static bool fuzzy_match_simple(char const *pattern, char const *str) {
			while (*pattern != '\0' && *str != '\0') {
				if (tolower(*pattern) == tolower(*str))
					++pattern;
				++str;
			}
			return *pattern == '\0' ? true : false;
		}

		inline static bool fuzzy_match(char const *pattern, char const *str, int &outScore) {
			uint8_t matches[256];
			return fuzzy_match(pattern, str, outScore, matches, sizeof(matches));
		}

		inline static bool fuzzy_match(char const *pattern, char const *str, int &outScore, uint8_t *matches, int maxMatches) {
			int recursionCount = 0;
			int recursionLimit = 10;
			return fuzzy_match_recursive(pattern, str, outScore, str, nullptr, matches, maxMatches, 0, recursionCount, recursionLimit);
		}

		inline static bool fuzzy_match_recursive(const char *pattern, const char *str, int &outScore,
			const char *strBegin, uint8_t const *srcMatches, uint8_t *matches, int maxMatches,
			int nextMatch, int & recursionCount, int recursionLimit) {
			++recursionCount;
			if (recursionCount >= recursionLimit)
				return false;

			if (*pattern == '\0' || *str == '\0')
				return false;

			bool recursiveMatch = false;
			uint8_t bestRecursiveMatches[256];
			int bestRecursiveScore = 0;
			bool first_match = true;

			while (*pattern != '\0' && *str != '\0') {
				if (tolower(*pattern) == tolower(*str)) {
					if (nextMatch >= maxMatches)
						return false;

					if (first_match && srcMatches) {
						memcpy(matches, srcMatches, nextMatch);
						first_match = false;
					}

					uint8_t recursiveMatches[256];
					int recursiveScore;

					if (fuzzy_match_recursive(pattern, str + 1, recursiveScore, strBegin, matches, recursiveMatches, sizeof(recursiveMatches), nextMatch, recursionCount, recursionLimit)) {
						if (!recursiveMatch || recursiveScore > bestRecursiveScore) {
							memcpy(bestRecursiveMatches, recursiveMatches, 256);
							bestRecursiveScore = recursiveScore;
						}
						recursiveMatch = true;
					}

					matches[nextMatch++] = (uint8_t)(str - strBegin);
					++pattern;
				}
				++str;
			}

			bool matched = *pattern == '\0' ? true : false;

			if (matched) {
				const int sequential_bonus = 15;
				const int separator_bonus = 30;
				const int camel_bonus = 30;
				const int first_letter_bonus = 15;
				const int leading_letter_penalty = -5;
				const int max_leading_letter_penalty = -15;
				const int unmatched_letter_penalty = -1;

				while (*str != '\0')
					++str;

				outScore = 100;

				int penalty = leading_letter_penalty * matches[0];
				if (penalty < max_leading_letter_penalty)
					penalty = max_leading_letter_penalty;

				outScore += penalty;

				int unmatched = (int)(str - strBegin) - nextMatch;
				outScore += unmatched_letter_penalty * unmatched;

				for (int i = 0; i < nextMatch; ++i) {
					uint8_t currIdx = matches[i];
					if (i > 0) {
						uint8_t prevIdx = matches[i - 1];
						if (currIdx == (prevIdx + 1))
							outScore += sequential_bonus;
					}

					if (currIdx > 0) {
						char neighbor = strBegin[currIdx - 1];
						char curr = strBegin[currIdx];
						if (::islower(neighbor) && ::isupper(curr))
							outScore += camel_bonus;
						bool neighborSeparator = neighbor == '_' || neighbor == ' ';
						if (neighborSeparator)
							outScore += separator_bonus;
					} else {
						outScore += first_letter_bonus;
					}
				}

			}

			if (recursiveMatch && (!matched || bestRecursiveScore > outScore)) {
				memcpy(matches, bestRecursiveMatches, maxMatches);
				outScore = bestRecursiveScore;
				return true;
			} else if (matched) {
				return true;
			} else {
				return false;
			}
		}

	};
}

/*
关于匹配打分

打分时候得考虑到什么因素？首先，下面是可能设计的打分点：
匹配的字母
未匹配的字母
连续匹配的字母
接近开始
分隔符后的字母（空格符包括空格，下划线）
写字母后面的大写字母（又名CamelCase，驼峰命名法）

这部分很简单。匹配的字母加分。不匹配字母减分。匹配接近开始加分。匹配短语中间的第一个字母加分。
在驼峰命名案例中匹配大写字母加分。
当然具体怎么加分，加多少分？目前我还不知道正确的答案。权重取决于你的预期数据集。
文件路径与文件名不同。文件扩展名则可以忽略的。单个单词关心连续的匹配，但不包括分隔符或骆驼大小写。
目前我们定义了一个各项指标的权衡。它对很多不同的数据集都有很好的效果。

分数从0开始：
匹配的字母：+0分
不匹配的字母：-1点
连续匹配加分：+5分
分隔符匹配加分：+10分
驼峰匹配加分：+10分
不匹配的大写字母：-3分（最大-9）

需要指出的是打分值没有啥实在的意义，只作为一个相对比较的参考。
[得分范围也没有限定到0-100，它大概是-50-50之间]

由于不匹配的字母减分，较长的单词具有可能会得到比较低的最低分值。
由于匹配加分，更长的搜索模式可能更可能得到最高分。

分隔符和驼峰加分比较大。连续的匹配加分比较有意义。
如果你不匹配前三个字母，会减分。如果在开始附近匹配会加分。中间和结束之间的匹配没有区别。
完全匹配没有明确的加分机制。不匹配的字母有会减分。所以更短的字符串和更近的匹配会得分更大。
大概的加分情况就是这样。对于单个搜索模式，结果可以按分数排序。
*/