#include "tool.h"

std::vector<std::string> Tool::split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	int p = str.size() - 1;
	while (std::isspace(str[p]))
		p--;
	str = str.substr(0, p + 1);
	str += pattern;//扩展字符串以方便操作
	int size = str.size();

	for (int i = 0; i<size; i++)
	{
		pos = str.find(pattern, i);
		if (pos<size)
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}
