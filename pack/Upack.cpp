#include "UPack.h"

#include <io.h>
#include <direct.h>
#include <sstream>

//#include <opencv2/core/core.hpp>
//#include <opencv2/objdetect/objdetect.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>

#define HEADER_LENGTH 32
#define MAX_PATH 256
#define LOGGAP 1000
#define ITEAM_FLAG 123456789


UPack::UPack(string maxBytesPart)
{
	init();
	if (maxBytesPart != "-1")
	{
		_maxBytesPart = 0;
		for (int i = 0; i < maxBytesPart.length() - 1; i++){
			_maxBytesPart = _maxBytesPart * 10 + (maxBytesPart[i] - '0');
		}
		switch (maxBytesPart[maxBytesPart.length() - 1])
		{
		case 'K':
			_maxBytesPart *= 1024;
			break;
		case 'M':
			_maxBytesPart *= 1024 * 1024;
			break;
		case 'G':
			_maxBytesPart *= 1024 * 1024 * 1024;
			break;
		default:
			_maxBytesPart = -1;
			break;
		}
		if (_maxBytesPart == 0)
			_maxBytesPart = -1;
	}
}

UPack::~UPack()
{
}

void UPack::init()
{
	_maxBytesPart = -1;
	_numPart = 0;
	_threadNum = 1;
	_iteamFlag = ITEAM_FLAG;
	_Index = 0;

	_rootDir = "";
	_packDir = "";
	_packBytes = 0;
	_fileIndex = 0;
	_numItems = 0;
	_numFiles = 0;
	_numFolder = 0;

	// unpack
	_countItems = 0;
	_countFiles = 0;
	_countFolder = 0;
	_unpackBytes = 0;

	strcpy(_version, "1.1.0");
	_createDate = time(0);
	_startTime = clock();

	_vecFileIndex.clear();
}

void UPack::PrintProgress(int isPack)
{
	if (isPack){
		if (_countItems % LOGGAP == 0){
			double time = (double)(clock() - _startTime) / CLOCKS_PER_SEC;
			cout << "[INFO]  packedBytes:" << _packBytes / (1024 * 1024) << "MB  folders:" << _countFolder;
			cout << "   files:" << _countFiles << "   numItems:" << _countItems << "   time:" << time << "s" << endl;
		}
	}
	else{
		if (_countItems % LOGGAP == 0){
			double time = (double)(clock() - _startTime) / CLOCKS_PER_SEC;
			cout << "[INFO]  unpackedBytes:" << _unpackBytes / (1024 * 1024) << "MB  folders:" << _countFolder;
			cout << "   files:" << _countFiles << "   numItems:" << _countItems << "   time:" << time << "s" << endl;
		}
	}

}

void UPack::PrintPackInfo(int isPack)
{		
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y/%m/%d %X", localtime(&_createDate));
	if (isPack){
		double cost_time = (double)(clock() - _startTime) / CLOCKS_PER_SEC;
		printf("\n=================PackInfo========================\n");
		printf("createDate : %s\n", tmp);
		printf("cost time:  %0.2fs  %0.2fmin\n", cost_time, cost_time / 60.0); 
		printf("version: %s\n", _version);
		printf("numPart: %d\n", _numPart+1);
		printf("numFolders: %I64d\n", _countFolder);
		printf("numFiles:   %I64d\n", _countFiles);
		printf("numItems:   %I64d\n", _countItems);
		printf("packBytes:  %I64dB  %0.2fMB  %0.2fGB\n", _packBytes, (double)_packBytes / (1024 * 1024), (double)_packBytes / (1024 * 1024 * 1024));
		printf("=========================================\n");
	}
	else{
		double cost_time = (double)(clock() - _startTime) / CLOCKS_PER_SEC;
		printf("\n=================PackInfo========================\n");
		printf("createDate : %s\n", tmp);
		printf("cost time:  %0.2fs  %0.2fmin\n", cost_time, cost_time / 60.0);
		printf("version: %s\n", _version);
		printf("numPart: %d\n", _numPart+1);
		printf("numFolders: %I64d\n", _countFolder);
		printf("numFiles:   %I64d\n", _countFiles);
		printf("numItems:   %I64d\n", _countItems);
		printf("unpackBytes:%I64dB  %0.2fMB  %0.2fGB\n", _packBytes, (double)_packBytes / (1024 * 1024), (double)_packBytes / (1024 * 1024 * 1024));
		printf("=========================================\n");
	}
}

bool UPack::WritePart(ofstream &fout, char *data, __int64 len)
{
	if (_packBytes % _maxBytesPart + len <= _maxBytesPart || _maxBytesPart == -1){
		fout.write(data, len);
	}
	else
	{
		__int64 len_p = _maxBytesPart - _packBytes % _maxBytesPart;
		if (len_p > 0){
			fout.write(data, len_p);
			fout.close();
		}
		stringstream ss;
		string num_part;
		ss << ++_numPart;
		ss >> num_part;
		string packPath = _packDir + _packName + ".u" + num_part;

		fout.open(packPath, ios::binary);
		if (!fout){
			printf("[ERROR] can not create pack file: %s\n", packPath.c_str());
			return false;
		}
		fout.write(data + len_p, len - len_p);
	}
	_packBytes += len;
	return true;
}

bool UPack::ReadPart(ifstream &in, char *data, __int64 len)
{
	if (_unpackBytes % _maxBytesPart + len <= _maxBytesPart || _maxBytesPart == -1){
		in.read(data, len);
	}
	else
	{
		__int64 len_p = _maxBytesPart - _unpackBytes % _maxBytesPart;
		if (len_p > 0){
			in.read(data, len_p);
			in.close();
		}

		stringstream ss;
		string num_part;
		ss << ++_numPart;
		ss >> num_part;
		string packPath = _packDir + _packName + ".u" + num_part;

		in.open(packPath, ios::binary);
		if (!in)
		{
			cout << "[ERROR] read upk file failed." << packPath << endl;
			return false;
		}
		in.read(data+len_p, len - len_p);
	}
	_unpackBytes += len;
	return true;
}

bool UPack::WriteFiles(ofstream &fout, vector<string> folders, vector<string> files)
{
	char path[MAX_PATH];

	vector<string>::iterator it;
	for (it = folders.begin(); it != folders.end(); it++)
	{
		_vecFileIndex.push_back(_packBytes);
		WritePart(fout,(char*)&_iteamFlag, sizeof(__int64));
		vector<string> nextFiles, nextFolders;
		//获取该文件夹下的文件和文件夹list
		GetFilesCurrent(*it, nextFiles);
		GetFoldersCurrent(*it, nextFolders);
		
		__int64 file_info = nextFiles.size() + nextFolders.size();
		__int64 data_len = 0;
		WritePart(fout, (char*)&file_info, sizeof(__int64));
		//string tmp = (*it).substr(_rootDir.length(), (*it).length());
		//WritePart(fout, tmp.c_str(), 256);
	
		memcpy(path, (*it).c_str() + _rootDir.length(), (*it).length() - _rootDir.length()+1);
		WritePart(fout, path, MAX_PATH);

		WritePart(fout, (char*)&data_len, sizeof(__int64));
		//             |   int64      |   int64     |  char[256]   |    int64    |
		//_packBytes += sizeof(__int64) + sizeof(__int64)+MAX_PATH + sizeof(__int64); //280

		_countFolder++;
		_countItems++;//文件及文件夹个数统计（包括文件夹和文件）
		PrintProgress(1);

		WriteFiles(fout, nextFolders, nextFiles);
	}


	//写入文件数据
	// |  IteamFlag  |  file_info  |     path     |   data_len  |      data      |
	// |    int64    |   int64     |  char[256]   |    int64    | char[data_len] |
	//....
	for (it = files.begin(); it != files.end(); it++)
	{
		__int64 file_info = -1;
		__int64 data_len = 0;
		
		_vecFileIndex.push_back(_packBytes);

		//step1: write file path
		WritePart(fout, (char*)&_iteamFlag, sizeof(__int64));
		WritePart(fout, (char*)&file_info, sizeof(__int64));
		memcpy(path, (*it).c_str() + _rootDir.length(), (*it).length() - _rootDir.length() + 1);
		WritePart(fout, path, MAX_PATH);

		//step2: read file
		if (*it == _packDir + _packName)//压缩文件保存到子目录，防止重复压缩
		{
			continue;
		}	
		char dir[256];
		string source = "\\\\?\\" + *it;//转换为短路径
		int res = GetShortPathName(source.c_str(), dir, sizeof(dir));
		if (res == 0)
		{
			cout << "[error]The file is not exist :" << *it << endl;
			continue;
		}
		ifstream in(dir, ios::in | ios::binary | ios::ate);
		in.seekg(0, ios::end);
		data_len = in.tellg();
		in.seekg(0, ios::beg);

		char *buffer = new char[data_len];
		in.read(buffer, data_len);
		in.close();

		//step3: write file
		WritePart(fout, (char*)&data_len, sizeof(__int64));
		WritePart(fout, buffer, data_len);

		_countFiles++;//文件个数统计
		_countItems++;//文件及文件夹个数统计（包括文件夹和文件）
		//            |    int64    |   int64        |  char[256]   |    int64    | char[data_len] |
		//_packBytes += sizeof(__int64) + sizeof(__int64) + MAX_PATH + sizeof(__int64) + data_len; 
		delete[] buffer;

		PrintProgress(1);
	}

	return true;
}

bool UPack::Pack(string sourcePath, const string& packPath)
{
	printf("\n========================Do pack======================\n");
	
	if (_access(sourcePath.c_str(), 0) != 0)
	{
		cout << "[ERROR] The folder is not exist: " << sourcePath << endl;
		exit(0);
	}
	if (sourcePath[sourcePath.length() - 1] != '\\')
		sourcePath += "\\";
	_rootDir = sourcePath.substr(0, sourcePath.find_last_of("\\"));
	_rootDir = _rootDir.substr(0, _rootDir.find_last_of("\\")+1);

	_packName = packPath.substr(packPath.find_last_of("\\") + 1, packPath.length());
	_packDir = packPath.substr(0, packPath.find_last_of("\\") + 1);
	//====================写入文件头=======================//
	// |  headerLen  |  version  |  date  |  numPackFiles  |  maxBytesPart |  numIteam  |  numFolder  |  numfiles  |  packBytes  |  fileIndex  | ...... |
	// |    int64    |   int64   |  int64 |     int64      |     int64     |   int64    |   int64     |    int64   |    int64    |    int64    | ...... |
	__int64 header[HEADER_LENGTH];
	memset(header, 0, sizeof(__int64)*HEADER_LENGTH);

	header[0] = HEADER_LENGTH;
	header[2] = _createDate;
	header[3] = _numPart;
	header[4] = _maxBytesPart;


	bool ok = CreatDir( packPath.substr(0, packPath.find_last_of("\\")) );
	if (ok == false){
		printf("[ERROR] pack path error.  %s\n", packPath.c_str());
		return false;
	}

	ofstream fout(packPath, ios::binary);
	if (!fout){
		printf("[ERROR] can not create pack file: %s\n", packPath.c_str());
		return false;
	}

	fout.write((char*)header, HEADER_LENGTH * sizeof(__int64)); 
	//write version
	fout.seekp(sizeof(__int64), ios::beg);
	fout.write(_version, sizeof(__int64));
	fout.seekp(0, ios::end);
	
	_packBytes += HEADER_LENGTH * sizeof(__int64);

	//=====================写入文件数据=======================//	
	vector<string> folders, files;
	folders.push_back(sourcePath);

	//=====================递归写入文件数据===================//	
	WriteFiles(fout, folders, files);

	//=====================写入索引信息=======================//
	_fileIndex = _packBytes;
	_packBytes += _vecFileIndex.size()*sizeof(__int64);
	vector<__int64>::iterator it;
	for (it = _vecFileIndex.begin(); it != _vecFileIndex.end(); it++)
	{
		__int64 index = *it;
		if (index < 0)
			cout << index << endl;
		fout.write((char*)&index, sizeof(__int64));
	}

	//=====================更新文件头========================//
	fout.close();
	fstream out(packPath, ios::out | ios::in | ios::binary );
	if (!out){
		printf("[ERROR] can not create pack file: %s\n", packPath.c_str());
		return false;
	}
	out.seekp(3 * sizeof(__int64), ios::beg);
	out.write((char*)&_numPart, sizeof(__int64));
	out.seekp(5*sizeof(__int64), ios::beg);
	out.write((char*)&_countItems, sizeof(__int64));
	out.write((char*)&_countFolder, sizeof(__int64));
	out.write((char*)&_countFiles, sizeof(__int64));
	out.write((char*)&_packBytes, sizeof(__int64));
	out.write((char*)&_fileIndex, sizeof(__int64));

	//=====================打印Pack信息=======================//
	PrintPackInfo(1);
	fout.close();
}

bool UPack::Unpack(const string& packPath, const string& destPath)
{
	printf("\n========================Do Unpack======================\n");
	init();
	_rootDir = destPath + "\\";
	_packName = packPath.substr(packPath.find_last_of("\\") + 1, packPath.length());
	_packDir = packPath.substr(0, packPath.find_last_of("\\") + 1);

	//if (packPath.substr(packPath.length() - 4, packPath.length() - 1) == ".upk")
	//{
	//	cout << "[ERROR] 解压的文件非 .upk 文件" << endl;
	//	getchar();
	//	exit(0);
	//}

	ifstream in(packPath, ios::binary);
	if (!in)
	{
		cout << "[ERROR] read upk file failed." << packPath << endl;
		return false;
	}

	//=====================读取文件头=======================//
	__int64 header_length = 0;
	in.read((char*)(&header_length), sizeof(__int64));
	if (header_length != HEADER_LENGTH)
	{
		cout << "[ERROR]解压失败， 解压的文件非 .upk 文件" << endl;
		getchar();
		exit(0);
	}
	__int64* header = new __int64[header_length];
	in.read((char*)(header+1), sizeof(__int64)*(header_length-1));
	
	_createDate =	header[2];
	_numPart =		header[3];
	_maxBytesPart = header[4];
	_numItems =		header[5];
	_numFolder =	header[6];
	_numFiles =		header[7];
	_packBytes =	header[8];
	_fileIndex =	header[9];

	in.seekg(sizeof(__int64), ios::beg);
	ReadPart(in, (char*)&_version, sizeof(__int64));
	in.seekg(HEADER_LENGTH*sizeof(__int64), ios::beg);
	
	_unpackBytes  += HEADER_LENGTH * sizeof(__int64);

	delete[] header;

	//=====================读取数据=======================//
	// |  IteamFlag  |  file_info  |     path     |   data_len  |      data      |
	// |    int64    |   int64     |  char[256]   |    int64    | char[data_len] |
	for (__int64 i = 0; i < _numItems; i++)
	{
		__int64 file_info;
		__int64 data_len;
		char path[MAX_PATH];
		ReadPart( in, (char*)&_iteamFlag, sizeof(__int64));
		//in.seekg(sizeof(__int64));
		ReadPart( in, (char*)&file_info, sizeof(__int64));
		ReadPart( in, path, MAX_PATH);
		ReadPart( in, (char*)&data_len, sizeof(__int64));
		char *buffer = NULL;
		if (data_len>0)
			buffer = new char[data_len];

		//是文件夹则创建，是文件则写入
		if (file_info >= 0) // folder
		{
			CreatDir(_rootDir+string(path));
			//|    int64    |   int64     |  char[256]   |    int64    |
			//_unpackBytes += sizeof(__int64) + sizeof(__int64)+MAX_PATH + sizeof(__int64);
			_countFolder++;
			_countItems++;
		}
		else
		{
			ofstream fout(_rootDir+path, ios::binary);
			if (!fout){
				cout << "[ERROR] can not creat file: " << _rootDir + path << endl;
				continue;
			}
			ReadPart( in, buffer, data_len);
			fout.write(buffer, data_len);
			fout.close();
			// |    int64    |   int64     |  char[256]   |    int64    | char[data_len] |
			//_unpackBytes += sizeof(__int64) + sizeof(__int64)+MAX_PATH + sizeof(__int64)+data_len;
			_countFiles++;
			_countItems++;

			/*vector<char> data(data_len);
			for (int j = 0; j < data_len; j++)
			{
				data[j] = buffer[j];
			}
			cv::Mat img = cv::imdecode(cv::Mat(data), 1);
			cv::imshow("camera", img);
			cv::waitKey(0);*/
		}

		if (buffer!=NULL)
			delete[] buffer;

		PrintProgress(0);
	}
	in.close();

	PrintPackInfo(0);

	return true;
}

bool UPack::UnpackThread(const string& packPath, const string& destPath, int treadNum)
{
	printf("\n========================Do UnpackTread======================\n");
	init();
	_threadNum = treadNum;
	_rootDir = destPath + "\\";
	_packName = packPath.substr(packPath.find_last_of("\\") + 1, packPath.length());
	_packDir = packPath.substr(0, packPath.find_last_of("\\") + 1);
	ifstream in(packPath, ios::binary);
	if (!in)
	{
		cout << "[ERROR] read upk file failed." << packPath << endl;
		return false;
	}

	//=====================读取文件头=======================//
	__int64 header_length = 0;
	in.read((char*)(&header_length), sizeof(__int64));
	if (header_length != HEADER_LENGTH)
	{
		cout << "[ERROR]解压失败， 解压的文件非 .upk 文件" << endl;
		getchar();
		exit(0);
	}
	__int64* header = new __int64[header_length];
	in.read((char*)(header + 1), sizeof(__int64)*(header_length - 1));

	_createDate = header[2];
	_numPart = header[3];
	_maxBytesPart = header[4];
	_numItems = header[5];
	_numFolder = header[6];
	_numFiles = header[7];
	_packBytes = header[8];
	_fileIndex = header[9];

	in.seekg(sizeof(__int64), ios::beg);
	ReadPart(in, (char*)&_version, sizeof(__int64));
	in.seekg(HEADER_LENGTH*sizeof(__int64), ios::beg);

	_unpackBytes += HEADER_LENGTH * sizeof(__int64);

	in.seekg(_fileIndex, ios::beg);
	_vecFileIndex.clear();
	__int64 index = 0;
	for (int i = 0; i < _numItems; i++)
	{
		ReadPart(in, (char *)&index, sizeof(__int64));
		_vecFileIndex.push_back(index);
	}

	delete[] header;
	in.close();

	if (_numPart > 0)
	{
		cout << "[INFO]多线程只支持单个包解压" << endl;
		return 0;
	}
	
	char pMsg[256];
	strcpy(pMsg, packPath.c_str());
	unsigned int thread_id = 0;
	ThreadArgList* arglist = new ThreadArgList[_threadNum];
	HANDLE* handle = new HANDLE[_threadNum];
	for (int i = 0; i < _threadNum; i++)
	{
		arglist[i].pthis = this;
		arglist[i].index = i;
		arglist[i].path = new char[packPath.length()];
		strcpy(arglist[i].path, packPath.c_str());
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, &ThreadFileWrite, &arglist[i], 0, &thread_id);
		if (handle[i] == NULL)
		{
			printf("Create thread %d failed!\n", i);
			return false;
		}
		else
			printf("Create thread %d success!  thread id=%d\n", i, thread_id);
		Sleep(500);// ms
	}
	WaitForMultipleObjects(_threadNum, handle, TRUE, INFINITE);
	printf("All threads execute complete.\n");
	PrintPackInfo(0);

	return true;
}

unsigned int __stdcall UPack::ThreadFileWrite(void* lp)
{
	//char *packPath = (char *)pMsg;
	ThreadArgList* arglist = (ThreadArgList*)lp;
	UPack *pthis = (UPack*)(arglist->pthis);
	int num_thread = arglist->index;
	string packPath(arglist->path);
	ifstream in(packPath, ios::binary);
	if (!in)
	{
		cout << "[ERROR] read upk file failed." << packPath << endl;
		return 0;
	}
	//=====================读取数据=======================//
	// |  IteamFlag  |  file_info  |     path     |   data_len  |      data      |
	// |    int64    |   int64     |  char[256]   |    int64    | char[data_len] |
	while (1)
	{
		__int64 index = 0;
		pthis->GetIndex(index);
		if (pthis->_Index >= pthis->_numItems)
			break;
		in.seekg(index, ios::beg);
		__int64 file_info;
		__int64 data_len;
		char path[MAX_PATH];
		pthis->ReadPart(in, (char*)&pthis->_iteamFlag, sizeof(__int64));
		//in.seekg(sizeof(__int64));
		pthis->ReadPart(in, (char*)&file_info, sizeof(__int64));
		pthis->ReadPart(in, path, MAX_PATH);
		pthis->ReadPart(in, (char*)&data_len, sizeof(__int64));
		char *buffer = NULL;
		if (data_len>0)
			buffer = new char[data_len];

		//是文件夹则创建，是文件则写入
		if (file_info >= 0) // folder
		{
			//pthis->CreatDir(pthis->_rootDir + string(path));
			pthis->_countFolder++;
			pthis->_countItems++;
		}
		else
		{
			string img_path = pthis->_rootDir + path;
			string str = img_path.substr(0, img_path.find_last_of("\\"));
			pthis->CreatDir(str);
			ofstream fout(img_path, ios::binary);
			if (!fout){
				cout << "[ERROR] can not creat file: " << pthis->_rootDir + path << endl;
				continue;
			}
			pthis->ReadPart(in, buffer, data_len);
			fout.write(buffer, data_len);
			fout.close();
			// |    int64    |   int64     |  char[256]   |    int64    | char[data_len] |
			//_unpackBytes += sizeof(__int64) + sizeof(__int64)+MAX_PATH + sizeof(__int64)+data_len;
			pthis->_countFiles++;
			pthis->_countItems++;
		}

		if (buffer != NULL)
			delete[] buffer;

		pthis->PrintProgress(0);
	}
	return 0;
}

bool UPack::GetIndex(__int64 &index)
{
	//加锁
	CLock lock(_Lock);
	vector<__int64>::iterator it;
	it = _vecFileIndex.begin() + _Index;
	index = *it;
	_Index++;
	return true;
}

void UPack::GetList(string packPath, vector<string>& iteams)
{
	init();
	ifstream in(packPath, ios::binary);
	if (!in)
	{
		cout << "[ERROR] read upk file failed." << packPath << endl;
		return;
	}

	__int64 num_iteams = 0;
	in.seekg(5 * sizeof(__int64), ios::beg);
	cout << in.tellg() << endl;
	ReadPart(in, (char *)&num_iteams, sizeof(__int64));
	//header + iteam_flag + is_dir
	in.seekg(HEADER_LENGTH*sizeof(__int64)+sizeof(__int64)+sizeof(__int64), ios::beg);
	for (__int64 i = 0; i < num_iteams; i++)
	{
		char dir[MAX_PATH];
		__int64 data_len = 0;
		ReadPart(in, dir, MAX_PATH);
		iteams.push_back(dir);
		ReadPart(in, (char *)& data_len, sizeof(__int64));
		in.seekg(data_len + sizeof(__int64)* 2, ios::cur);
	}
}

void UPack::PrintList(vector<string>& iteams)
{
	vector<string>::iterator it;
	for (it = iteams.begin(); it != iteams.end(); it++)
	{
		cout << *it << endl;
	}
}

void UPack::GetFoldersCurrent(string path, vector<string>& folders)
{
	if (path[path.length() - 1] != '\\')
		path += "\\";

	//文件句柄  
	long   hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					folders.push_back(p.assign(path).append(fileinfo.name).append("\\"));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

void UPack::GetFilesCurrent(string path, vector<string>& files)
{
	if (path[path.length() - 1] != '\\')
		path += "\\";
	//文件句柄  
	long   hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if (!(fileinfo.attrib &  _A_SUBDIR))
				files.push_back(p.assign(path).append(fileinfo.name));
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

bool UPack::CreatDir(const string& path)
{
	string dir = path;
	if (dir.back() != '\\')
		dir += "\\";
	int e_pos = dir.length();
	int f_pos = dir.find("\\", 0);
	string subdir;
	do
	{
		e_pos = dir.find("\\", f_pos + 2);
		if (e_pos != -1)
		{
			subdir = dir.substr(0, e_pos);
			_mkdir(subdir.c_str());
			/*if (_mkdir(subdir.c_str()) == 0)
			cout << "OK" << endl;
			else cout << "FAIL" << endl;*/
		}
		f_pos = e_pos;
	} while (f_pos != -1);
	return true;
}

