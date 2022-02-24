#include <Windows.h>
#include <stdio.h>
#if 0
//对目录进行加锁
int main()
{
	//先查找并关闭窗口
	HWND hWnd = FindWindow(L"CabinetWClass", L"empty");
	if (hWnd)
	{
		// close the window using API        
		SendMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
	}
	else {
		wprintf_s(L"Close directory window failed with %d\n", GetLastError());
		return -1;
	}

	//锁住目录
	HANDLE hDir = CreateFileW(L"D:\\empty", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hDir != INVALID_HANDLE_VALUE)
	{
		//if (DeleteFileW(L"F:\\Test\\TestFile.txt"))
		//	wprintf(L"F:\\Test\\TestFile.txt was deleted");
		//else
		//	wprintf_s(L"DeleteFile failed with %d\n", GetLastError());

		Sleep(60 * 1000);
		CloseHandle(hDir);
	}
	else
		wprintf_s(L"Open directory failed with %d\n", GetLastError());

	return 0;
}
#endif