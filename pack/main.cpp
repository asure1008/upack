#include "UPack.h"
#include <windows.h>


void ShowHelp()
{
	printf("Usage:\n");
	printf("    Upk /p  <SourceFiles> <UpkFile>\n");
	printf("    Upk /p  <SourceFiles> <UpkFile> [size]\n");
	printf("    Upk /u  <UpkFile> <DestFolder>\n");
	printf("    Upk /ut <UpkFile> <DestFolder> [TreadNum]\n");
}

int main(int argc,char* argv[])
{
	if (argc < 4)
	{
		ShowHelp();
		return 0;
	}

	if (strcmp(argv[1], "/p") == 0)
	{
		if (argc == 5){
			UPack bp(argv[4]);
			bp.Pack(argv[2], argv[3]);
		}
		else if(argc == 4)
		{
			UPack bp("-1");
			bp.Pack(argv[2], argv[3]);
		}
		else{
			ShowHelp();
		}

	}
	else if (argc == 4 && strcmp(argv[1], "/u") == 0)
	{
		UPack bp("-1");
		bp.Unpack(argv[2], argv[3]);
	}
	else if (argc == 5 && strcmp(argv[1], "/ut") == 0){
		int tread_num = atoi(argv[4]);
		UPack bp("-1");
		bp.UnpackThread(argv[2], argv[3], tread_num);
	}
	else
		ShowHelp();

	return 0;
}