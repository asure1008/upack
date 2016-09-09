#ifndef _BPACK_H_
#define _BPACK_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <time.h>
#include <process.h> 
#include "Lock.h"

using namespace std;

typedef struct{
	void* pthis;
	int index;
	char* path;
}ThreadArgList;


class UPack
{
public:
	UPack(string maxBytesPart);
	~UPack();
	void init();
	void PrintProgress(int isPack);
	void PrintPackInfo(int isPack);
	bool WritePart(ofstream &fout, char *data, __int64 len);
	bool ReadPart(ifstream &in, char *data, __int64 len);
	void GetList(string packPath, vector<string>& iteams);
	void PrintList(vector<string>& iteams);
	bool GetIndex(__int64 &index);

	static unsigned int __stdcall ThreadFileWrite(void* packPath);

	//write bpack
	bool Pack(string sourcePath, const string& packPath);
	//write bpack
	bool Unpack(const string& packPath, const string& destPath);
	bool UnpackThread(const string& packPath, const string& destPath, int treadNum);
	bool WriteFiles(ofstream &fout, vector<string> folders, vector<string> files);

	//Gets the current directory folders
	void GetFoldersCurrent(string path, vector<string>& folders);
	//Gets the current directory files
	void GetFilesCurrent(string path, vector<string>& files);
	bool CreatDir(const string& path);

private:

	char _version[sizeof(__int64)];
	time_t  _createDate;
	//拆分多个包，单个包最大字节数
	__int64 _maxBytesPart;
	int _numPart;
	__int64 _iteamFlag;
	string _packName;
	int _threadNum;

	string _rootDir;
	string _packDir;
	//文件和文件夹个数统计
	//__int64 _countFile;
	//文件字节数统计
	__int64 _packBytes;
	//索引表头指针始位置
	__int64 _fileIndex;

	
	__int64 _countItems, _countFiles, _countFolder;

	//for unpack use
	__int64 _numItems, _numFiles, _numFolder;  
	__int64 _unpackBytes;

	vector<__int64> _vecFileIndex;
	__int64 _Index;

	clock_t _startTime;
	Mutex _Lock;
};



#endif