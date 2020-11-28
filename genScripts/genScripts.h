#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <mutex>
#include <map>
#include <thread>
#include <direct.h>
#include <filesystem>
#include <io.h>


namespace genScripts
{

	//∑¿÷πÕ¨ ±–¥»ÎLogicEventH
	static std::mutex mtx;

	inline void argX(std::vector<std::string>& in, std::vector<std::string>& out) {
		size_t count = 1;
		for (auto& i : in) {
			if (i.substr(0, 2) != "#$") {
				if (count % 2 == 0) {
					out.push_back("arg" + std::to_string(count / 2));
					++count;
				}
				if (i == "UNICODE")
					out.push_back("FString");
				else
					out.push_back(i);
			}
			else
				out.push_back(i.substr(2, i.size() - 2));

			++count;
		}
		if (count % 2 == 0)
			out.push_back("arg" + std::to_string(count / 2));
	}

	inline bool IsExistMethod(std::vector<std::string>& HADMETHODS, std::string& fname, std::vector<std::string>& args, std::regex& reg_fdecl) {
		std::smatch matches;
		std::vector<std::string> argdecl;
		bool have{ false };
		argX(args, argdecl);
		for (auto& hm : HADMETHODS) {
			std::regex_search(hm, matches, reg_fdecl);
			if (matches[2] == fname) {
				std::string args_str = matches[3];

				std::regex comma{ R"(\s*,\s*)" };
				std::vector<std::string> argTypes(std::sregex_token_iterator(args_str.begin(), args_str.end(), comma, -1), std::sregex_token_iterator{});
				if (argTypes.size() == args.size() / 2) {
					for (size_t i = 0; i < argTypes.size(); ++i) {
						if (argTypes[i].substr(0, argTypes[i].find_first_of(" \t\n")) == args[i * 2]) {
							if (i == argTypes.size() - 1)
								have = true;
						}
						else
							break;
					}
				}
			}
		}
		return have;
	}

	inline bool IsExistMethod(std::vector<std::string>& HADMETHODS, std::string& fdecl, std::regex& reg_fdecl) {
		std::smatch matches1;
		std::smatch matches2;
		bool have{ false };
		for (auto& hm : HADMETHODS) {
			std::regex_search(hm, matches1, reg_fdecl);
			std::regex_search(fdecl, matches2, reg_fdecl);
			if (matches1[2] == matches2[2]) {
				std::string args_str1 = matches1[3];
				std::string args_str2 = matches2[3];
				std::regex comma{ R"(\s*,\s*)" };
				std::vector<std::string> argTypes1(std::sregex_token_iterator(args_str1.begin(), args_str1.end(), comma, -1), std::sregex_token_iterator{});
				std::vector<std::string> argTypes2(std::sregex_token_iterator(args_str2.begin(), args_str2.end(), comma, -1), std::sregex_token_iterator{});
				if (argTypes1.size() == argTypes2.size()) {
					for (size_t i = 0; i < argTypes1.size(); ++i) {
						if (argTypes1[i].substr(0, argTypes1[i].find_first_of(" \t\n")) == \
							argTypes2[i].substr(0, argTypes2[i].find_first_of(" \t\n"))) {
							if (i == argTypes1.size() - 1)
								have = true;
						}
						else
							break;
					}
				}
			}
		}
		return have;
	}

	bool genLogicEvent(std::string filePath, const std::string& genPath, const std::string& logicEventTemp);
	bool genEntityH(std::string filePath, const std::string& genPath, const std::string& entityHeadTemp);
	bool genEntityCpp(std::string filePath, const std::string& genPath, const std::string& entityCppTemp);

}