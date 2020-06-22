// 
// ¤Ç¤Ì¤Á...
// 

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <string.h>
#include <time.h>
#include <fstream>
#include <sstream>
#include <stack>
#include "m_cio.h"

namespace fs = std::filesystem;

bool isverbose = 0;

void printPathes(fs::path pth, std::vector<std::string> ignores) {
	fs::directory_iterator itr(pth/* / "a"*/);
	while (itr != fs::end(itr)) {
		const fs::directory_entry& entry = *itr;
		bool valid = 1;
		for (auto pth : ignores) {
			if (entry.path().string() == pth)
				valid = 0;
		}

		if (valid) {
			std::cout << entry.path().string() << std::endl;
			if (entry.is_directory()) {
				printPathes(entry, ignores);
			}
		}
		itr++;
	}
	return;
}

void copyDir(fs::path src, fs::path dest, std::vector<std::string> ignores, std::string nowpath = "") {
	fs::directory_iterator itr(src);
	while (itr != fs::end(itr)) {
		const fs::directory_entry& entry = *itr;
		bool valid = 1;
		for (auto pth : ignores) {
			if (entry.path().string() == pth)
				valid = 0;
		}
		if (valid) {
			if (isverbose) {
				std::cout << entry.path().string() << " : " << (entry.is_directory() ? "directory" : "file") << std::endl;
			}
			if (entry.is_directory()) {
				fs::create_directory(dest / nowpath / entry.path().filename());
				copyDir(entry.path(), dest, ignores, nowpath + entry.path().filename().string() + "\\");
			}
			else {
				fs::copy_file(entry, dest / nowpath / entry.path().filename().string());
			}
		}
		itr++;
	}
	return;
}

void fileonlyRollback(fs::path src, fs::path dest, std::string nowpath = "") {
	//std::cout << nowpath << std::endl;
	fs::directory_iterator itr(src / nowpath);
	while (itr != fs::end(itr)) {
		const fs::directory_entry& entry = *itr;
		if (!fs::exists(dest / nowpath / entry.path().filename().string())) {
			throw std::exception("directoty structures are different");
			return;
		}
		if (entry.is_directory()) {
			fileonlyRollback(src, dest, nowpath + entry.path().filename().string() + '\\');
			std::cout << entry.path().string();
			scss(" : directory\n");
		}
		else {
			fs::remove(dest / nowpath / entry.path().filename().string());
			fs::copy_file(src / nowpath / entry.path().filename().string(), dest / nowpath / entry.path().filename().string());
			std::cout << entry.path().string();
			scss(" : copy success\n");
		}
		itr++;
	}
	return;
}

int main(int argc, char **argv) {
	if (argc == 1) {
		setcolor(10, 0);
		std::cout << R"(
                                  _                   _                       
                                 | |                 | |                      
  ___   _   _  _   _   __ _  ___ | |__    __ _   ___ | | __ _   _  _ __   ___ 
 / _ \ | | | || | | | / _` ||___|| '_ \  / _` | / __|| |/ /| | | || '_ \ / __|
| (_) || |_| || |_| || (_| |     | |_) || (_| || (__ |   < | |_| || |_) |\__ \
 \___/  \__,_| \__, | \__,_|     |_.__/  \__,_| \___||_|\_\ \__,_|| .__/ |___/
                __/ |                                             | |         
               |___/                                              |_|         
::Backup - Rollback tool.
License: MIT License

[add] <directory> (ignore:)
[backup]
[play] <time> (verbose)
[rollback] <when> (restruct|fileonly)

)";
		setcolor(7, 0);
	}
	else if (argc > 1) {
		std::vector<std::string> ignores;
		if (argc >= 3 $$ !strcmp(argv[1], "add")) {
			if (argc == 4 && !strcmp(argv[3], "ignore:")) {
				std::cout << "\nType paths to Ignore(!!! to stop, \\ required. (ex: \\config))" << std::endl;
				bool t = 1;
				while (t) {
					std::cout << fs::current_path().string() << ' ';
					std::string buff;
					std::cin >> buff;
					if (buff == "!!!") {
						t = false;
						break;
					}
					else {
						buff = '.' + buff;
						// fprintf(ignore, "%s\n", fs::absolute(buff.c_str()).string());
						ignores.push_back(fs::absolute(buff.c_str()).string());
					}
				}
			}
			
			std::cout << std::endl;
			setcolor(10, 0);
			printPathes(fs::current_path(), ignores);
			setcolor(7, 0);

			FILE* ignore;
			FILE* ouyafile;
			ignore = fopen(".ouyaignore", "w");
			ouyafile = fopen(".ouyafile", "w");

			std::cout << "will add to" << std::endl;
			fprintf(ouyafile, fs::absolute(argv[2]).string().c_str());
			setcolor(10, 0);
			std::cout << fs::absolute(argv[2]).string() << std::endl;
			fs::path toBackup(fs::absolute(argv[2]).string());
			if (fs::exists(fs::absolute(argv[2]).string())) {
				std::string buff;
				std::cout << std::endl << argv[2] << " directory already exists.\nClear it?[y/n]\n> ";
				std::cin >> buff;
				if (buff == "y") {
					fs::remove_all(toBackup);
					fs::create_directory(toBackup);
				}
				else {
					setcolor(4, 0);
					std::cout << "ouya needs clear directory.\n";
					setcolor(7, 0);
					return -1;
				}
			}
			else {
				fs::create_directory(toBackup);
			}

			setcolor(7, 0);
			for (auto q : ignores) {
				fprintf(ignore, "%s\n", q.c_str());
			}

			fclose(ouyafile);
			fclose(ignore);
		}
		else if ((argc == 3 || argc == 4) $$ !strcmp(argv[1], "play")) {
			// std::cout << fs::current_path().string() << std::endl << "added" << std::endl;
			FILE* ignore;
			FILE* attr;

			if ((ignore = fopen(".ouyaignore", "r")) == NULL) {
				err("file .ouyaignore required. \"ouya add backups\" to create .ouyaignore\n");
				return -1;
			}
			if ((attr = fopen(".ouyafile", "r")) == NULL) {
				err("file .ouyafile required. \"ouya add backups\" to create .ouyafile\n");
				return -1;
			}


			int delay = atoi(argv[2]);
			delay *= 1000;
			
			char pbuff[200];
			fscanf(attr, "%s", pbuff);
			
			fs::path $dirr(pbuff);
			std::vector<std::string> ignores;

			while (fscanf(ignore, "%s", pbuff) != EOF) {
				ignores.push_back(pbuff);
				std::cout << "Path to ignore : " << pbuff << std::endl;
			}
			
			if (!fs::exists($dirr)) {
				err("\nNo directory to paste.\n");
				return -1;
			}

			if (argc == 4) {
				if (!strcmp(argv[3], "verbose")) {
					isverbose = true;
				}
			}
			
			char _c_buff[80];
			time_t _rawtime;
			tm* _timeinfo;
			time(&_rawtime);
			_timeinfo = localtime(&_rawtime);
			strftime(_c_buff, 80, "[%c]", _timeinfo);
			std::string toMessage("ouya backups started at ");
			toMessage += _c_buff;
			toMessage += "\n\n";
			scss(toMessage);
			
			fclose(attr);
			fclose(ignore);
			while (1) {
				char c_buff[80];
				time_t rawtime;
				tm* timeinfo;
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				strftime(c_buff, 80, "backup_%a_%b_%d_%H%M%S", timeinfo);
				std::cout << c_buff << " Backing up...\n";
				fs::path backupDir($dirr / c_buff);
				fs::create_directory(backupDir);
				try {
					copyDir(fs::current_path(), backupDir, ignores);
				}
				catch (const std::exception& errn) {
					err("\nError during copying directory\nError code : ");
					std::cout << errn.what() << std::endl;
					
					return -1;
				}
				scss("done : directory backed up at\n" + backupDir.string() + "\n\n");
				Sleep(delay);
			}
		}
		else if (argc == 2 $$ !strcmp(argv[1], "backup")) {
			FILE* ignore;
			FILE* attr;

			if ((ignore = fopen(".ouyaignore", "r")) == NULL) {
				err("file .ouyaignore required. \"ouya add backups\" to create .ouyaignore\n");
				return -1;
			}
			if ((attr = fopen(".ouyafile", "r")) == NULL) {
				err("file .ouyafile required. \"ouya add backups\" to create .ouyafile\n");
				return -1;
			}

			char pbuff[200];
			fscanf(attr, "%s", pbuff);

			fs::path $dirr(pbuff);
			std::vector<std::string> ignores;

			while (fscanf(ignore, "%s", pbuff) != EOF) {
				ignores.push_back(pbuff);
				std::cout << "Path to ignore : " << pbuff << std::endl;
			}

			if (!fs::exists($dirr)) {
				err("\nNo directory to paste.\n");
				return -1;
			}

			if (argc == 4) {
				if (!strcmp(argv[3], "verbose")) {
					isverbose = true;
				}
			}

			fclose(attr);
			fclose(ignore);
			char c_buff[80];
			time_t rawtime;
			tm* timeinfo;
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(c_buff, 80, "backup_%a_%b_%d_%H%M%S", timeinfo);
			std::cout << c_buff << " Backing up...\n";
			fs::path backupDir($dirr / c_buff);
			fs::create_directory(backupDir);
			try {
				copyDir(fs::current_path(), backupDir, ignores);
			}
			catch (const std::exception& errn) {
				err("\nError during copying directory\nError code : ");
				std::cout << errn.what() << std::endl;

				return -1;
			}
			scss("done : directory backed up at\n" + backupDir.string() + "\n\n");
		}
		else if (argc == 4 $$ !strcmp(argv[1], "rollback")) {
			if (!strcmp(argv[3], "restruct")) {
				FILE* ignore;
				FILE* attr;
				if ((ignore = fopen(".ouyaignore", "r")) == NULL) {
					err("file .ouyaignore required. \"ouya add backups\" to create .ouyaignore\n");
					return -1;
				}
				if ((attr = fopen(".ouyafile", "r")) == NULL) {
					err("file .ouyafile required. \"ouya add backups\" to create .ouyafile\n");
					return -1;
				}
				char pbuff[200];
				fscanf(attr, "%s", pbuff);

				fs::path $dirr(pbuff);
				$dirr = ($dirr / (std::string("backup_") + argv[2]));
				std::vector<std::string> ignores;
				if (!fs::exists($dirr)) {
					err(std::string("backup backup_") + argv[2] + " not founed.");
					return -1;
				}
				fclose(attr);
				fclose(ignore);

				std::cout << $dirr.string() << std::endl;
				std::cout << fs::current_path().string() << std::endl;
				scss("restruct rollback has been started...\n");
				try {
					std::string tmpath = fs::current_path().string();
					// scss("move to tmpPath Done.");
					for (auto ddir : fs::directory_iterator(fs::current_path())) {
						std::cout << ddir.path().string() << std::endl;
						fs::remove_all(ddir);
					}
					// fs::remove_all(fs::current_path());
					// scss("clear the directory Done.");
					fs::create_directory(tmpath);
					fs::copy($dirr, fs::current_path(), fs::copy_options::recursive);
					// scss("done.\n");
				}
				catch (const std::exception& errn) {
					err("\nError during copying directory\nError code : ");
					std::cout << errn.what() << std::endl;

					return -1;
				}
			}
			else if (!strcmp(argv[3], "fileonly")) {
				FILE* ignore;
				FILE* attr;
				if ((ignore = fopen(".ouyaignore", "r")) == NULL) {
					err("file .ouyaignore required. \"ouya add backups\" to create .ouyaignore\n");
					return -1;
				}
				if ((attr = fopen(".ouyafile", "r")) == NULL) {
					err("file .ouyafile required. \"ouya add backups\" to create .ouyafile\n");
					return -1;
				}
				char pbuff[200];
				fscanf(attr, "%s", pbuff);

				fs::path $dirr(pbuff);
				$dirr = ($dirr / (std::string("backup_") + argv[2]));
				std::vector<std::string> ignores;
				if (!fs::exists($dirr)) {
					err(std::string("backup backup_") + argv[2] + " not founed.");
					return -1;
				}
				fclose(attr);
				fclose(ignore);

				scss("file-only backup has been started...");

				try {
					// fs::copy($dirr, fs::current_path(), fs::copy_options::recursive);
					fileonlyRollback($dirr, fs::current_path());
				}
				catch (const std::exception& errn) {
					err("\nError during copying directory\nError code : ");
					std::cout << errn.what() << std::endl;

					return -1;
				}
				std::cout << std::endl;
			}
		}
		else if (argc == 2 $$ !strcmp(argv[1], "batch")) {
			std::string buff;
			std::stack<char> stk;
			while (1) {
				std::cout << std::endl << fs::current_path().string();
				std::cout << "\n@ ~ $ ";
				std::getline(std::cin, buff);
				while (!stk.empty()) {
					stk.pop();
				}
				FILE* bat;
				bat = fopen("ouya_batch.bat", "w");
				fprintf(bat, "@echo off\n");
				for (size_t i = 0; i < buff.length(); i++) {
					if (buff.at(i) == ')') {
						if (stk.top() == '(') {
							stk.pop();
						}
					}
					else if (buff.at(i) == '(') {
						stk.push('(');
					}
				}
				fprintf(bat, "%s\n", buff.c_str());
				while (!stk.empty()) {
					std::cout << "  ... ";
					std::getline(std::cin, buff);
					fprintf(bat, "%s\n", buff.c_str());
					for (size_t i = 0; i < buff.length(); i++) {
						if (buff.at(i) == ')') {
							if (stk.top() == '(') {
								stk.pop();
							}
						}
						else if (buff.at(i) == '(') {
							stk.push('(');
						}
					}
				}
				fclose(bat);
				system(".\\ouya_batch.bat");
				std::cout << std::endl;
			}
		}
	}
	return 0;
}