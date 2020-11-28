#include "genScripts.h"


bool genScripts::genLogicEvent(std::string filePath, const std::string& genPath, const std::string& logicEventTemp)
{
	std::ifstream file{ filePath };
	if (!file) {
		std::cerr << filePath << "��ʧ��" << std::endl;
		return false;
	}
	std::istreambuf_iterator<char> h(file), t;
	std::string file_str(h, t);
	file.close();

	//�Ƴ����ַ�
	std::regex Space{ R"(\s+)" };
	file_str = std::regex_replace(file_str, Space, "");
	//�滻�������
	std::regex NameTag{ R"(<!--\#\$(\S+?)\#\$-->)" };
	while (true) {
		std::sregex_iterator i(file_str.begin(), file_str.end(), NameTag);
		if (i != std::sregex_iterator{}) {
			file_str.replace(i->position(), i->length(), "<#$>" + std::string{ (*i)[1] } +"</#$>");
		}
		else
			break;
	}
	//�Ƴ�ע��
	std::regex Comment{ R"(<!--[\s\S]*?-->)" };
	file_str = std::regex_replace(file_str, Comment, "");

	std::regex ARGTYPE{ R"(%%ARGTYPE%%)" };
	std::regex ARGNAME{ R"(%%ARGNAME%%)" };
	//��ʼ����
	std::smatch matches;
	std::regex Main_pat{ R"(<(.*?)>(.*?)</\1>)" };
	//�Ƴ�<root></root>
	std::regex_match(file_str, matches, Main_pat);
	file_str = matches[2];
	for (std::sregex_iterator r(file_str.begin(), file_str.end(), Main_pat); r != std::sregex_iterator{}; ++r) {
		if ((*r)[1] == "Volatile") {
			;
		}
		else if ((*r)[1] == "Properties") {
			;
		}
		else if ((*r)[1] == "Components") {
			;
		}
		else if ((*r)[1] == "Interfaces") {
			;
		}
		else if ((*r)[1] == "BaseMethods") {
			if ((*r)[2] != "") {
				//�Ƴ�<BaseMethods></BaseMethods>��ʼ��������
				std::string methods_str = (*r)[2];
				for (std::sregex_iterator m(methods_str.begin(), methods_str.end(), Main_pat); m != std::sregex_iterator{}; ++m) {
					if (std::regex_search(std::string{ (*m)[2] }, std::regex{ R"(<Exposed/>)" })) {
						//����ģ��
						std::string temp_method = std::regex_replace(logicEventTemp, std::regex{ R"(%%METHODNAME%%)" }, std::string{ (*m)[1] });
						std::regex_search(temp_method, matches, std::regex{ R"(//TAG-ARG([\d\D]*)//ARG-TAG)" });
						std::string temp_arg = matches[1];

						std::vector<std::string> args;
						//��������
						std::string args_str{ (*m)[2] };
						for (std::sregex_iterator a(args_str.begin(), args_str.end(), Main_pat); a != std::sregex_iterator{}; ++a) {
							if ((*a)[1] == "Arg") {
								args.push_back(std::regex_replace(temp_arg, ARGTYPE, std::string{ (*a)[2] }));
							}
							else if ((*a)[1] == "#$") {
								args[args.size() - 1] = std::regex_replace(args.back(), ARGNAME, std::string{ (*a)[2] });
							}
							else {
								std::cerr << (*m)[1] << "::" << (*a)[1] << " >>>δ��ʶ��ñ�ǩ,�����Զ��䴦��." << std::endl;
							}
						}
						//�滻UNICODEΪFString,δ����������Ϊarg[i]
						std::string buf;
						for (int i = 0; i != args.size(); ++i) {
							buf += std::regex_replace(std::regex_replace(args[i], std::regex{ R"(UNICODE)" }, "FString"), ARGNAME, "arg" + std::to_string(i + 1));
						}
						mtx.lock();
						std::fstream logicEventH{ genPath + "\\LogicEvent.h", std::ios_base::app };
						if (!logicEventH) {
							std::cerr << genPath + "\\LogicEvent.h��ʧ��,����" + std::string{ (*m)[1] } +"δ��д��." << std::endl;
						}
						else {
							logicEventH << std::regex_replace(temp_method, std::regex{ R"(//TAG-ARG([\d\D]*)//ARG-TAG)" }, buf);
							logicEventH.close();
						}
						mtx.unlock();
					}
				}
			}
		}
		else if ((*r)[1] == "CellMethods") {
			if ((*r)[2] != "") {
				//�Ƴ�<BaseMethods></BaseMethods>��ʼ��������
				std::string methods_str = (*r)[2];
				for (std::sregex_iterator m(methods_str.begin(), methods_str.end(), Main_pat); m != std::sregex_iterator{}; ++m) {
					if (std::regex_search(std::string{ (*m)[2] }, std::regex{ R"(<Exposed/>)" })) {
						//����ģ��
						std::string temp_method = std::regex_replace(logicEventTemp, std::regex{ R"(%%METHODNAME%%)" }, std::string{ (*m)[1] });
						std::regex_search(temp_method, matches, std::regex{ R"(//TAG-ARG([\d\D]*)//ARG-TAG)" });
						std::string temp_arg = matches[1];

						std::vector<std::string> args;
						//��������
						std::string args_str{ (*m)[2] };
						for (std::sregex_iterator a(args_str.begin(), args_str.end(), Main_pat); a != std::sregex_iterator{}; ++a) {
							if ((*a)[1] == "Arg") {
								args.push_back(std::regex_replace(temp_arg, ARGTYPE, std::string{ (*a)[2] }));
							}
							else if ((*a)[1] == "#$") {
								args[args.size() - 1] = std::regex_replace(args.back(), ARGNAME, std::string{ (*a)[2] });
							}
							else {
								std::cerr << (*m)[1] << "::" << (*a)[1] << " >>>δ��ʶ��ñ�ǩ,�����Զ��䴦��." << std::endl;
							}
						}
						//�滻UNICODEΪFString,δ����������Ϊarg[i]
						std::string buf;
						for (int i = 0; i != args.size(); ++i) {
							buf += std::regex_replace(std::regex_replace(args[i], std::regex{ R"(UNICODE)" }, "FString"), ARGNAME, "arg" + std::to_string(i + 1));
						}
						mtx.lock();
						std::fstream logicEventH{ genPath + "\\LogicEvent.h", std::ios_base::app };
						if (!logicEventH) {
							std::cerr << genPath + "\\LogicEvent.h��ʧ��,����" + std::string{ (*m)[1] } +"δ��д��." << std::endl;
						}
						else {
							logicEventH << std::regex_replace(temp_method, std::regex{ R"(//TAG-ARG([\d\D]*)//ARG-TAG)" }, buf);
							logicEventH.close();
						}
						mtx.unlock();
					}
				}
			}
		}
		else if ((*r)[1] == "ClientMethods") {
			if ((*r)[2] != "") {
				//�Ƴ�<BaseMethods></BaseMethods>��ʼ��������
				std::string methods_str{ (*r)[2] };
				for (std::sregex_iterator m(methods_str.begin(), methods_str.end(), Main_pat); m != std::sregex_iterator{}; ++m) {
					//����ģ��
					std::string temp_method = std::regex_replace(logicEventTemp, std::regex{ R"(%%METHODNAME%%)" }, std::string{ (*m)[1] });
					std::regex_search(temp_method, matches, std::regex{ R"(//TAG-ARG([\d\D]*)//ARG-TAG)" });
					std::string temp_arg = matches[1];

					std::vector<std::string> args;
					//��������
					std::string args_str{ (*m)[2] };
					for (std::sregex_iterator a(args_str.begin(), args_str.end(), Main_pat); a != std::sregex_iterator{}; ++a) {
						if ((*a)[1] == "Arg") {
							args.push_back(std::regex_replace(temp_arg, ARGTYPE, std::string{ (*a)[2] }));
						}
						else if ((*a)[1] == "#$") {
							args[args.size() - 1] = std::regex_replace(args.back(), ARGNAME, std::string{ (*a)[2] });
						}
						else {
							std::cerr << (*m)[1] << "::" << (*a)[1] << " >>>δ��ʶ��ñ�ǩ,�����Զ��䴦��." << std::endl;
						}
					}
					//�滻UNICODEΪFString,δ����������Ϊarg[i]
					std::string buf;
					for (int i = 0; i != args.size(); ++i) {
						buf += std::regex_replace(std::regex_replace(args[i], std::regex{ R"(UNICODE)" }, "FString"), ARGNAME, "arg" + std::to_string(i + 1));
					}

					mtx.lock();
					std::fstream logicEventH{ genPath + "\\LogicEvent.h", std::ios_base::app };
					if (!logicEventH) {
						std::cerr << genPath + "\\LogicEvent.h��ʧ��,����" + std::string{ (*m)[1] } +"δ��д��." << std::endl;
					}
					else {
						logicEventH << std::regex_replace(temp_method, std::regex{ R"(//TAG-ARG([\d\D]*)//ARG-TAG)" }, buf);
						logicEventH.close();
					}
					mtx.unlock();
				}
			}
		}
		else {
			std::cerr << (*r)[1] << "--�޷�ʶ��ñ�ǩ,�����Զ��䴦��" << std::endl;
		}
	}
	return true;
}

