#include "genScripts.h"

//将目录下所有匹配pattrn的文件路径放入files.
void getFiles(std::string path, std::string pattern, std::vector<std::string>& files)
{
	//文件句柄  
	intptr_t hFile = 0;   //win10
	//文件信息  
	struct _finddata_t fileinfo;
	if ((hFile = _findfirst(std::string{ path + "\\" + pattern }.c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib &  _A_SUBDIR)) {
				//				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				//					getFiles(std::string{path}.append("\\").append(fileinfo.name), files);
			}
			else {
				files.push_back(path + "\\" + fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

//解析配置文件
bool parse(std::string curPath)
{
	//打开配置文件
	std::ifstream conf(curPath + "\\genScripts.bat");
	if (!conf) {
		std::cerr << curPath << "下无法找到genScripts.bat文件" << std::endl;
		return false;
	}

	std::regex logicEventBegin{ R"(goto\s*LOGIC_EVENT_TEMP)" };
	std::regex logicEventEnd{ R"(:\s*LOGIC_EVENT_TEMP)" };
	std::regex entityHeadBegin{ R"(goto\s*ENTITY_HEAD_TEMP)" };
	std::regex entityHeadEnd{ R"(:\s*ENTITY_HEAD_TEMP)" };
	std::regex entityCppBegin{ R"(goto\s*ENTITY_CPP_TEMP)" };
	std::regex entityCppEnd{ R"(:\s*ENTITY_CPP_TEMP)" };
	std::regex comment{ R"(:.*)" };

	std::string entityPath;
	std::string logicEventTemp;
	std::string entityHeadTemp;
	std::string entityCppTemp;
	//读取LOGIC_EVENT,ENTITY_HEAD,ENTITY_CPP模板到字符串.
	for (std::string line; getline(conf, line);)
	{
		if (line == "") {
			continue;
		}
		else if (std::regex_match(line, logicEventBegin)) {
			std::string LEstr;
			for (getline(conf, line); !std::regex_match(line, logicEventEnd); getline(conf, line)) {
				LEstr += line + "\n";
			}
			logicEventTemp = LEstr;
		}
		else if (std::regex_match(line, entityHeadBegin)) {
			std::string EHstr;
			for (getline(conf, line); !std::regex_match(line, entityHeadEnd); getline(conf, line)) {
				EHstr += line + "\n";
			}
			entityHeadTemp = EHstr;
		}
		else if (std::regex_match(line, entityCppBegin)) {
			std::string ECstr;
			for (getline(conf, line); !std::regex_match(line, entityCppEnd); getline(conf, line)) {
				ECstr += line + "\n";
			}
			entityCppTemp = ECstr;
		}
		else {
			std::smatch matches;
			if (std::regex_match(line, matches, std::regex{ R"(::ENTITY_DEFS_PATH\s*=\s*\"(\S+)\"\s*)" }))
				entityPath = matches[1];
		}
	}
	conf.close();

	//由于所有文件都要向LogicEvent.h写入,生成前先将其初始化.
	int hpos = logicEventTemp.find("//TAG-METHOD");
	if (_access(std::string{ curPath }.append("\\LogicEvent.h").c_str(), 0) == -1) {
		std::fstream{ curPath + "\\LogicEvent.h", std::ios_base::out } << logicEventTemp.substr(0, hpos);
	}
	else {
		std::cout << curPath << "\\LogicEvnet.h已存在.\n:输入Y覆盖其内容,否则退出" << std::endl;
		char s;
		std::cin >> s;
		if (s == 'y' || s == 'Y')
			std::fstream{ curPath + "\\LogicEvent.h", std::ios_base::out } << logicEventTemp.substr(0, hpos);
		else
			return false;
	}
	logicEventTemp = logicEventTemp.substr(hpos + sizeof("//TAG-METHOD"), logicEventTemp.length());

	//获取目录文件名
	std::vector<std::string> files;
	getFiles(entityPath, "*.def", files);
	for (auto& f : files) {
		genScripts::genEntityH(f, curPath, entityHeadTemp);
		genScripts::genEntityCpp(f, curPath, entityCppTemp);
	}
	getFiles(entityPath + "\\components", "*.def", files);
	getFiles(entityPath + "\\interfaces", "*.def", files);
	for (auto& f : files) {
		genScripts::genLogicEvent(f, curPath, logicEventTemp);
	}
	return true;
}

int main(int argc, char* argv[])
{
	if (argc == 2 && parse(argv[1]))
		std::cout << "SUCCEED." << std::endl;
	else if (argc != 2)
		std::cout << "ERROR:需要提供genScripts.bat所在文件夹路径" << std::endl;
	else
		std::cout << "FAILED." << std::endl;

	system("pause");
	return 0;
}