#include "genScripts.h"


bool genScripts::genEntityCpp(std::string filePath, const std::string& genPath, const std::string& entityCppTemp)
{
	std::ifstream file{ filePath };
	if (!file) {
		std::cerr << filePath << "�޷���,����ʧ��..." << std::endl;
		return false;
	}
	std::istreambuf_iterator<char> h(file), t;
	std::string file_str(h, t);
	file.close();

	//�Ƴ��ļ���.def��׺
	filePath = std::regex_replace(filePath, std::regex{ "/" }, "\\");
	size_t pathlen = filePath.find_last_of("\\") + 1;
	size_t namelen = filePath.find_last_of(".") - pathlen;
	std::string fileName{ filePath.substr(pathlen , namelen) };
	std::string cppPath{ genPath + "\\" + fileName + ".cpp" };

	std::smatch matches;
	//����ǩ������
	std::regex reg_fdecl{ R"(([_[:alpha:]][_[:alnum:]]*)\s*([_[:alpha:]][_:[:alnum:]]*)\(\s*([\d\D]*?)\s*\))" };
	//��¼�Ѿ����ڵķ���
	std::map<std::string, std::vector<std::string>> HADMETHODS;
	//�¼�����
	std::regex reg_edecl{ R"(KBENGINE_REGISTER_EVENT(_OVERRIDE_FUNC)?\(\s*(\w*))" };
	//��¼��ע����¼�
	std::vector<std::string> HADEVENT;


	//���Ҫ�����ļ������ڶ����ʼ��
	if (_access(cppPath.c_str(), 0) == -1) {
		std::string temp_ = std::regex_replace(entityCppTemp, std::regex{ R"(%%CLASSNAME%%)" }, fileName);
		temp_ = std::regex_replace(temp_, std::regex{ R"(//TAG-REGEVENT([\d\D]*)//REGEVENT-TAG)" }, "//TAG-REGEVENT\n//REGEVENT-TAG");
		temp_ = std::regex_replace(temp_, std::regex{ R"(//TAG-BASE([\d\D]*)//BASE-TAG)" }, "//TAG-BASE\n//BASE-TAG");
		temp_ = std::regex_replace(temp_, std::regex{ R"(//TAG-CELL([\d\D]*)//CELL-TAG)" }, "//TAG-CELL\n//CELL-TAG");
		temp_ = std::regex_replace(temp_, std::regex{ R"(//TAG-CLIENT([\d\D]*)//CLIENT-TAG)" }, "//TAG-CLIENT\n//CLIENT-TAG");
		std::ofstream{ cppPath } << temp_;
	}
	//�����Ҫ�����ļ��Ѵ�����������
	else {
		std::ifstream cppfile{ cppPath };
		if (!cppfile) {
			std::cerr << cppPath << "�޷���,����ʧ��..." << std::endl;
			return false;
		}
		else {
			bool check_reg{ false }, check_base{ false }, check_cell{ false }, check_client{ false }, check_dataup{ false };
			for (std::string line; getline(cppfile, line); ) {
				//����������
				if (line.size() > 0 && line.substr(line.size() - 1, 1) == "\\") {
					std::string s;
					getline(cppfile, s);
					line = line.substr(0, line.size() - 1) + s;
				}
				if (line == "//TAG-REGEVENT") {
					for (getline(cppfile, line); line != "//REGEVENT-TAG"; getline(cppfile, line)) {
						if (std::regex_search(line, reg_edecl)) {
							HADEVENT.push_back(line);
						}
					}
					check_reg = true;
				}
				else if (line == "//TAG-BASE") {
					for (getline(cppfile, line); line != "//BASE-TAG"; getline(cppfile, line)) {
						//������ǩ������map
						if (std::regex_search(line, reg_fdecl)) {
							HADMETHODS["BASE"].push_back(line);
						}
					}
					check_base = true;
				}
				else if (line == "//TAG-CELL") {
					for (getline(cppfile, line); line != "//CELL-TAG"; getline(cppfile, line)) {
						//������ǩ������map
						if (std::regex_search(line, matches, reg_fdecl)) {
							HADMETHODS["BASE"].push_back(line);
						}
					}
					check_cell = true;
				}
				else if (line == "//TAG-CLIENT") {
					for (getline(cppfile, line); line != "//CLIENT-TAG"; getline(cppfile, line)) {
						//������ǩ������map
						if (std::regex_search(line, reg_fdecl)) {
							HADMETHODS["BASE"].push_back(line);
						}
					}
					check_client = true;
				}
			}
			if (!check_reg || !check_base || !check_cell || !check_client) {
				std::cerr << cppPath << "��ȱ��TAG��ǩ,����ʧ��..." << std::endl;
				return false;
			}
			cppfile.close();
		}
	}

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

	//������Ϣ��
	std::string CLASSNAME = fileName;
	std::map<std::string, std::vector<std::string>> METHODS;
	std::map<std::string, std::vector<std::string>> ARGS;
	std::map<std::string, std::string> ITFMETHODS;
	std::map<std::string, std::string> COMMETHODS;
	//��ʼ����
	std::regex Main_pat{ R"(<(.*?)>(.*?)</\1>)" };
	{
		//�Ƴ�<root></root>
		std::regex_match(file_str, matches, Main_pat);
		file_str = matches[2];
		for (std::sregex_iterator r(file_str.begin(), file_str.end(), Main_pat); r != std::sregex_iterator{}; ++r) {
			if ((*r)[1] == "Interfaces") {
				if ((*r)[2] != "") {
					//������ʱ�ļ�Ŀ¼
					std::string tempPath{ genPath + "\\temp" + fileName };
					//������ʱ�ļ������ļ���
					if (_access(tempPath.c_str(), 0) == -1)
						_mkdir(tempPath.c_str());

					//�Ƴ�<Interfaces></Interfaces>
					std::string itf_str{ (*r)[2] };
					for (std::sregex_iterator i(itf_str.begin(), itf_str.end(), Main_pat); i != std::sregex_iterator{}; ++i) {
						std::string itfName = (*i)[2];
						//�ӿڼ̳нӿ�ʱ·��������interfaces
						std::string itfSourPath;
						size_t ll = filePath.find_last_of("\\");
						size_t ilong = sizeof("interfaces") - 1;
						if (filePath.size() > ilong && filePath.substr(ll - ilong, ilong) == "interfaces")
							itfSourPath = filePath.substr(0, ll) + "\\" + itfName + ".def";
						else
							itfSourPath = filePath.substr(0, filePath.find_last_of("\\")) + "\\interfaces\\" + itfName + ".def";
						std::string itfGenPath = tempPath + "\\" + itfName + ".cpp";
						if (genEntityCpp(itfSourPath, tempPath, entityCppTemp)) {
							std::ifstream itfcppfile{ itfGenPath };
							if (!itfcppfile) {
								std::cerr << itfGenPath << "�޷���,����ʧ��..." << std::endl;
								return false;
							}
							else {
								//����ǰ�ļ���û�еĽӿں�����������
								for (std::string line; getline(itfcppfile, line);) {
									if (line == "//TAG-BASE") {
										for (getline(itfcppfile, line); line != "//BASE-TAG"; getline(itfcppfile, line)) {
											while (std::regex_search(line, reg_fdecl) && IsExistMethod(HADMETHODS["BASE"], line, reg_fdecl)) {
												do
												{
													getline(itfcppfile, line);
												} while (line != "//BASE-TAG" && !std::regex_search(line, reg_fdecl));
												if (line == "//BASE-TAG")
													break;
											}
											if (line == "//BASE-TAG")
												break;
											ITFMETHODS["BASE"] += line + "\n";
										}
									}
									else if (line == "//TAG-CELL") {
										for (getline(itfcppfile, line); line != "//CELL-TAG"; getline(itfcppfile, line)) {
											while (std::regex_search(line, reg_fdecl) && IsExistMethod(HADMETHODS["CELL"], line, reg_fdecl)) {
												do
												{
													getline(itfcppfile, line);
												} while (line != "//CELL-TAG" && !std::regex_search(line, reg_fdecl));
												if (line == "//CELL-TAG")
													break;
											}
											if (line == "//CELL-TAG")
												break;
											ITFMETHODS["CELL"] += line + "\n";
										}
									}
									else if (line == "//TAG-CLIENT") {
										for (getline(itfcppfile, line); line != "//CLIENT-TAG"; getline(itfcppfile, line)) {
											while (std::regex_search(line, reg_fdecl) && IsExistMethod(HADMETHODS["CLIENT"], line, reg_fdecl)) {
												do
												{
													getline(itfcppfile, line);
												} while (line != "//CLIENT-TAG" && !std::regex_search(line, reg_fdecl));
												if (line == "//CLIENT-TAG")
													break;
											}
											if (line == "//CLIENT-TAG")
												break;
											ITFMETHODS["CLIENT"] += line + "\n";
										}
									}
									else {
										;
									}
								}
								itfcppfile.close();
							}
						}
						//ɾ����ʱ�ļ�;
						remove(itfGenPath.c_str());
					}
					_rmdir(tempPath.c_str());
				}
			}
			else if ((*r)[1] == "Components") {
				if ((*r)[2] != "") {
					//������ʱ�ļ�Ŀ¼
					std::string tempPath{ genPath + "\\temp" + fileName };
					//������ʱ�ļ������ļ���
					if (_access(tempPath.c_str(), 0) == -1)
						_mkdir(tempPath.c_str());

					//�Ƴ�<Components></Components>
					std::string com_str{ (*r)[2] };
					for (std::sregex_iterator i(com_str.begin(), com_str.end(), Main_pat); i != std::sregex_iterator{}; ++i) {
						std::string comName = (*i)[1];
						std::string comSourPath = filePath.substr(0, filePath.find_last_of("\\")) + "\\components\\" + comName + ".def";
						std::string comGenPath = tempPath + "\\" + comName + ".cpp";
						if (genEntityCpp(comSourPath, tempPath, entityCppTemp)) {
							std::ifstream comcppfile{ comGenPath };
							if (!comcppfile) {
								std::cerr << comGenPath << "�޷���,����ʧ��..." << std::endl;
								return false;
							}
							else {
								//����ǰ�ļ���û�е����������������
								for (std::string line; getline(comcppfile, line);) {
									if (line == "//TAG-BASE") {
										for (getline(comcppfile, line); line != "//BASE-TAG"; getline(comcppfile, line)) {
											while (std::regex_search(line, reg_fdecl) && IsExistMethod(HADMETHODS["BASE"], line, reg_fdecl)) {
												do
												{
													getline(comcppfile, line);
												} while (line != "//BASE-TAG" && !std::regex_search(line, reg_fdecl));
												if (line == "//BASE-TAG")
													break;
											}
											if (line == "//BASE-TAG")
												break;
											COMMETHODS["BASE"] += line + "\n";
										}
									}
									else if (line == "//TAG-CELL") {
										for (getline(comcppfile, line); line != "//CELL-TAG"; getline(comcppfile, line)) {
											while (std::regex_search(line, reg_fdecl) && IsExistMethod(HADMETHODS["CELL"], line, reg_fdecl)) {
												do
												{
													getline(comcppfile, line);
												} while (line != "//CELL-TAG" && !std::regex_search(line, reg_fdecl));
												if (line == "//CELL-TAG")
													break;
											}
											if (line == "//CELL-TAG")
												break;
											COMMETHODS["CELL"] += line + "\n";
										}
									}
									else if (line == "//TAG-CLIENT") {
										for (getline(comcppfile, line); line != "//CLIENT-TAG"; getline(comcppfile, line)) {
											while (std::regex_search(line, reg_fdecl) && IsExistMethod(HADMETHODS["CLIENT"], line, reg_fdecl)) {
												do
												{
													getline(comcppfile, line);
												} while (line != "//CLIENT-TAG" && !std::regex_search(line, reg_fdecl));
												if (line == "//CLIENT-TAG")
													break;
											}
											if (line == "//CLIENT-TAG")
												break;
											COMMETHODS["CLIENT"] += line + "\n";
										}
									}
									else {
										;
									}
								}
								comcppfile.close();
							}
						}
						//ɾ����ʱ�ļ�;
						remove(comGenPath.c_str());
					}
					_rmdir(tempPath.c_str());
				}
			}
			else if ((*r)[1] == "BaseMethods") {
				if ((*r)[2] != "") {
					//�Ƴ�<BaseMethods></BaseMethods>
					std::string meth_str{ (*r)[2] };
					for (std::sregex_iterator m(meth_str.begin(), meth_str.end(), Main_pat); m != std::sregex_iterator{}; ++m) {
						if (std::regex_search(std::string{ (*m)[2] }, std::regex{ R"(<Exposed/>)" })) {
							METHODS["BASE"].push_back((*m)[1]);
							//�Ƴ�<������></������>
							std::string args_str{ (*m)[2] };
							for (std::sregex_iterator a(args_str.begin(), args_str.end(), Main_pat); a != std::sregex_iterator{}; ++a) {
								if ((*a)[1] == "Arg") {
									ARGS[(*m)[1]].push_back((*a)[2]);
								}
								else if ((*a)[1] == "#$") {
									ARGS[(*m)[1]].push_back("#$" + std::string{ (*a)[2] });
								}
								else {
									std::cerr << fileName << "::" << (*r)[1] << "::" << (*m)[1] << "::" << (*a)[1] << "--�޷�ʶ��ñ�ǩ,�����Զ��䴦��" << std::endl;
								}
							}
						}
					}
				}
			}
			else if ((*r)[1] == "CellMethods") {
				if ((*r)[2] != "") {
					//�Ƴ�<CellMethods></CellMethods>
					std::string meth_str{ (*r)[2] };
					for (std::sregex_iterator m(meth_str.begin(), meth_str.end(), Main_pat); m != std::sregex_iterator{}; ++m) {
						if (std::regex_search(std::string{ (*m)[2] }, std::regex{ R"(<Exposed/>)" })) {
							METHODS["CELL"].push_back((*m)[1]);
							//�Ƴ�<������></������>
							std::string args_str{ (*m)[2] };
							for (std::sregex_iterator a(args_str.begin(), args_str.end(), Main_pat); a != std::sregex_iterator{}; ++a) {
								if ((*a)[1] == "Arg") {
									ARGS[(*m)[1]].push_back((*a)[2]);
								}
								else if ((*a)[1] == "#$") {
									ARGS[(*m)[1]].push_back("#$" + std::string{ (*a)[2] });
								}
								else {
									std::cerr << fileName << "::" << (*r)[1] << "::" << (*m)[1] << "::" << (*a)[1] << "--�޷�ʶ��ñ�ǩ,�����Զ��䴦��" << std::endl;
								}
							}
						}
					}
				}
			}
			else if ((*r)[1] == "ClientMethods") {
				if ((*r)[2] != "") {
					//�Ƴ�<ClientMethods></ClientMethods>
					std::string meth_str{ (*r)[2] };
					for (std::sregex_iterator m(meth_str.begin(), meth_str.end(), Main_pat); m != std::sregex_iterator{}; ++m) {
						METHODS["CLIENT"].push_back((*m)[1]);
						//�Ƴ�<������></������>
						std::string args_str{ (*m)[2] };
						for (std::sregex_iterator a(args_str.begin(), args_str.end(), Main_pat); a != std::sregex_iterator{}; ++a) {
							if ((*a)[1] == "Arg") {
								ARGS[(*m)[1]].push_back((*a)[2]);
							}
							else if ((*a)[1] == "#$") {
								ARGS[(*m)[1]].push_back("#$" + std::string{ (*a)[2] });
							}
							else {
								std::cerr << fileName << "::" << (*r)[1] << "::" << (*m)[1] << "::" << (*a)[1] << "--�޷�ʶ��ñ�ǩ,�����Զ��䴦��" << std::endl;
							}
						}
					}
				}
			}
			else {
				std::cerr << fileName << "::" << (*r)[1] << "--�޷�ʶ��ñ�ǩ,�����Զ��䴦��" << std::endl;
			}
		}
	}
	//����ģ��
	std::string temp_ = std::regex_replace(entityCppTemp, std::regex{ R"(%%CLASSNAME%%)" }, fileName);
	std::regex_search(temp_, matches, std::regex{ R"(//TAG-REGEVENT\n([\d\D]*?)//REGEVENT-TAG)" });
	std::string temp_event = matches[1];
	std::regex_search(temp_, matches, std::regex{ R"(//TAG-BASE\n([\d\D]*?)//BASE-TAG)" });
	std::string temp_base = matches[1];
	std::regex_search(temp_, matches, std::regex{ R"(//TAG-CELL\n([\d\D]*?)//CELL-TAG)" });
	std::string temp_cell = matches[1];
	std::regex_search(temp_, matches, std::regex{ R"(//TAG-CLIENT\n([\d\D]*?)//CLIENT-TAG)" });
	std::string temp_client = matches[1];
	std::regex_search(temp_, matches, std::regex{ R"(//TAG-DATAUP\n([\d\D]*?)//DATAUP-TAG)" });
	std::string temp_dataup = matches[1];
	//�����ļ�
	std::ifstream entityHin{ cppPath };
	if (!entityHin) {
		std::cerr << cppPath << "�޷���,����ʧ��..." << std::endl;
		return false;
	}
	else {
		std::ofstream entityHout{ cppPath + ".new" };
		for (std::string line; getline(entityHin, line); entityHout << line << "\n") {
			if (line == "//REGEVENT-TAG") {
				for (auto& base : METHODS["BASE"]) {
					//����¼���ע������������
					bool have{ false };
					for (auto& he : HADEVENT) {
						if (he.find(base)) {
							have = true;
						}
					}
					if (have)
						continue;

					std::string Ebuf = std::regex_replace(temp_event, std::regex{ R"(%%CALLMETHOD%%)" }, base);

					if (ARGS[base].size() != 0) {
						std::vector<std::string> args;
						argX(ARGS[base], args);
						std::string tempbuf{ "data." + args[1] };
						for (size_t i = 3; i < args.size(); i += 2)
							tempbuf += ", data." + args[i];
						Ebuf = std::regex_replace(Ebuf, std::regex{ R"(%%DATALIST%%)" }, tempbuf);
					}
					else
						Ebuf = std::regex_replace(Ebuf, std::regex{ R"(%%DATALIST%%)" }, "");

					entityHout << Ebuf;
				}
				for (auto& cell : METHODS["CELL"]) {
					//����¼���ע������������
					bool have{ false };
					for (auto& he : HADEVENT) {
						if (he.find(cell)) {
							have = true;
						}
					}
					if (have)
						continue;
					//��������д���ļ�
					std::string Ebuf = std::regex_replace(temp_event, std::regex{ R"(%%CALLMETHOD%%)" }, cell);

					if (ARGS[cell].size() != 0) {
						std::vector<std::string> args;
						argX(ARGS[cell], args);
						std::string tempbuf{ "data." + args[1] };
						for (size_t i = 3; i < args.size(); i += 2)
							tempbuf += ", data." + args[i];
						Ebuf = std::regex_replace(Ebuf, std::regex{ R"(%%DATALIST%%)" }, tempbuf);
					}
					else
						Ebuf = std::regex_replace(Ebuf, std::regex{ R"(%%DATALIST%%)" }, "");

					entityHout << Ebuf;
				}
			}
			else if (line == "//BASE-TAG") {
				entityHout << ITFMETHODS["BASE"];
				entityHout << COMMETHODS["BASE"];
				for (auto& base : METHODS["BASE"]) {
					//��������Ѷ�������������
					bool have = IsExistMethod(HADMETHODS["BASE"], base, ARGS[base], reg_fdecl);
					if (have)
						continue;
					//��������д���ļ�
					std::string Bbuf = std::regex_replace(temp_base, std::regex{ R"(%%METHODNAME%%)" }, base);

					if (ARGS[base].size() != 0) {
						std::vector<std::string> args;
						argX(ARGS[base], args);
						//���ɲ�������
						std::string tempbuf = args[0];
						for (size_t i = 1; i < args.size(); ++i) {
							if (i % 2 == 0)
								tempbuf += ", ";
							tempbuf += " " + args[i];
						}
						Bbuf = std::regex_replace(Bbuf, std::regex{ R"(%%ARGDECL%%)" }, tempbuf);
						//���ɸ�ʽ������
						//Bbuf = std::regex_replace(Bbuf, std::regex{ R"(%%ARGFORMAT%%)" },"#");
						//���ɲ����б�
						tempbuf = args[1];
						for (size_t i = 3; i < args.size(); ++i)
							tempbuf += ", " + args[i];
						Bbuf = std::regex_replace(Bbuf, std::regex{ R"(%%ARGLIST%%)" }, tempbuf);
					}
					else {
						Bbuf = std::regex_replace(Bbuf, std::regex{ R"(%%ARGDECL%%)" }, "");
						Bbuf = std::regex_replace(Bbuf, std::regex{ R"(%%ARGFORMAT%%)" }, "");
						Bbuf = std::regex_replace(Bbuf, std::regex{ R"(%%ARGLIST%%)" }, "");
					}

					entityHout << Bbuf;
				}
			}
			else if (line == "//CELL-TAG") {
				entityHout << ITFMETHODS["CELL"];
				entityHout << COMMETHODS["CELL"];
				for (auto& cell : METHODS["CELL"]) {
					//��������Ѷ�������������
					bool have = IsExistMethod(HADMETHODS["CELL"], cell, ARGS[cell], reg_fdecl);
					if (have)
						continue;
					//��������д���ļ�
					std::string Cbuf = std::regex_replace(temp_cell, std::regex{ R"(%%METHODNAME%%)" }, cell);

					if (ARGS[cell].size() != 0) {
						std::vector<std::string> args;
						argX(ARGS[cell], args);
						//���ɲ�������
						std::string tempbuf = args[0];
						for (size_t i = 1; i < args.size(); ++i) {
							if (i % 2 == 0)
								tempbuf += ", ";
							tempbuf += " " + args[i];
						}
						Cbuf = std::regex_replace(Cbuf, std::regex{ R"(%%ARGDECL%%)" }, tempbuf);
						//���ɸ�ʽ������
						//Bbuf = std::regex_replace(Bbuf, std::regex{ R"(%%ARGFORMAT%%)" },"#");
						//���ɲ����б�
						tempbuf = args[1];
						for (size_t i = 3; i < args.size(); ++i)
							tempbuf += ", " + args[i];
						Cbuf = std::regex_replace(Cbuf, std::regex{ R"(%%ARGLIST)" }, tempbuf);
					}
					else {
						Cbuf = std::regex_replace(Cbuf, std::regex{ R"(%%ARGDECL%%)" }, "");
						Cbuf = std::regex_replace(Cbuf, std::regex{ R"(%%ARGFORMAT%%)" }, "");
						Cbuf = std::regex_replace(Cbuf, std::regex{ R"(%%ARGLIST%%)" }, "");
					}

					entityHout << Cbuf;
				}
			}
			else if (line == "//CLIENT-TAG") {
				entityHout << ITFMETHODS["CLIENT"];
				entityHout << COMMETHODS["CLIENT"];
				for (auto& client : METHODS["CLIENT"]) {
					//��������Ѷ�������������
					bool have = IsExistMethod(HADMETHODS["CLIENT"], client, ARGS[client], reg_fdecl);
					if (have)
						continue;
					//��������д���ļ�
					std::string Cbuf = std::regex_replace(temp_client, std::regex{ R"(%%METHODNAME%%)" }, client);

					if (ARGS[client].size() != 0) {
						std::vector<std::string> args;
						argX(ARGS[client], args);
						//���ɲ�������
						std::string tempbuf = args[0];
						for (size_t i = 1; i < args.size(); ++i) {
							if (i % 2 == 0)
								tempbuf += ", ";
							tempbuf += " " + args[i];
						}
						Cbuf = std::regex_replace(Cbuf, std::regex{ R"(%%ARGDECL%%)" }, tempbuf);
						//����dataup
						tempbuf = "";
						for (size_t i = 1; i < args.size(); i += 2) {
							tempbuf += std::regex_replace(temp_dataup, std::regex{ R"(%%ARGNAME%%)" }, args[i]);
						}
						Cbuf = std::regex_replace(Cbuf, std::regex{ R"(//TAG-DATAUP[\d\D]*?//DATAUP-TAG)" }, tempbuf);
					}
					else {
						Cbuf = std::regex_replace(Cbuf, std::regex{ R"(%%ARGDECL%%)" }, "");
						Cbuf = std::regex_replace(Cbuf, std::regex{ R"(//TAG-DATAUP[\d\D]*?//DATAUP-TAG)" }, "");
					}

					entityHout << Cbuf;
				}
			}
		}
		entityHin.close();
		entityHout.close();
		if (remove(cppPath.c_str()) == 0) {
			rename(std::string{ cppPath + ".new" }.c_str(), cppPath.c_str());
		}
		else {
			std::cerr << cppPath << "�޷�ɾ��,������Ϊ" << filePath << ".new�����ļ�" << std::endl;
		}
	}
	return true;
}

