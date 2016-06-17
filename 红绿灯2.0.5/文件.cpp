#include <iostream>
#include <io.h>
#include <string>
#include<vector>
#include <windows.h>
#include <tchar.h>

using namespace std;

void getJustCurrentDir(string path, vector<string>& files)  
{  
	//文件句柄  
	long   hFile   =   0;  
	//文件信息  
	struct _finddata_t fileinfo;  
	string p;  
	if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1)  
	{  
		do  
		{    
			if((fileinfo.attrib &  _A_SUBDIR))  
			{  
				if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)  
				{
					files.push_back(path + "\\" + fileinfo.name);
					//files.push_back(p.assign(path).append("\\").append(fileinfo.name) );
				}

			}   
		}while(_findnext(hFile, &fileinfo)  == 0);  
		_findclose(hFile); 
	} 
}

void getJustCurrentFile( string path, vector<string>& files)  
{  
	//文件句柄  
	long   hFile   =   0;  
	//文件信息  
	struct _finddata_t fileinfo;  
	string p;  
	if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1)  
	{  
		do  
		{   
			if((fileinfo.attrib &  _A_SUBDIR))  
			{  
				;
			}  
			else  
			{  
				files.push_back(path + "\\" + fileinfo.name);
				//files.push_back(p.assign(path).append("\\").append(fileinfo.name) );  
			}   
		}while(_findnext(hFile, &fileinfo)  == 0);  
		_findclose(hFile); 
	} 
}

void myExec(bool isDay, const char * imgPath){
	SHELLEXECUTEINFO ShExecInfo = {0}; 
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO); 
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS; 
	ShExecInfo.hwnd = NULL; 
	ShExecInfo.lpVerb = _T("open"); 
	if(isDay)
		ShExecInfo.lpFile = _T("DayProcess.exe"); 
	else
		ShExecInfo.lpFile = _T("NightProcess.exe"); 
	ShExecInfo.lpParameters = _T(imgPath); 
	ShExecInfo.lpDirectory = NULL; 
	ShExecInfo.nShow = SW_HIDE; 
	ShExecInfo.hInstApp = NULL; 
	ShellExecuteEx(&ShExecInfo);
	DWORD dwExitCode;
	GetExitCodeProcess(ShExecInfo.hProcess,&dwExitCode);
	while (dwExitCode == STILL_ACTIVE)
	{ 
		Sleep((DWORD)20);
		GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
	}

	CloseHandle(ShExecInfo.hProcess);

}
int main1(int argc, char** argv){
	vector<string> paths;
	
	string dirPath = "E:\\桌面\\白天黑夜成功版2";
	if (argc == 2){
		dirPath = argv[1];
	}else{
		dirPath = "E:\\桌面\\白天黑夜成功版2";
	}

	getJustCurrentDir(dirPath, paths);
	for(int i = 0; i < paths.size(); i++){

		vector<string> files;
		if(paths[i] == dirPath + "\\" + "day"){
		
			getJustCurrentFile(dirPath + "\\" + "day", files);
			for(int j = 0; j < files.size(); j++){
				cout << files[j] << endl;
				myExec(true, files[j].c_str());
			}
		}else{
			
			getJustCurrentFile(dirPath + "\\" + "night", files);
			for(int j = 0; j < files.size(); j++){
				cout << files[j] << endl;
				myExec(false, files[j].c_str());
			}
		}

	}
	
	return 0;
}