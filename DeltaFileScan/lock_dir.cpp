#include <Windows.h>
#include <stdio.h>
#if 0
//��Ŀ¼���м���
int main()
{
	//�Ȳ��Ҳ��رմ���
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

	//��סĿ¼
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