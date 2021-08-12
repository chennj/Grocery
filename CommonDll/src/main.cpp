#include <Windows.h>

#ifdef _WIN32
#define EXPORT_DLL _declspec(dllexport)
#else
#define EXPORT_DLL
#endif

extern "C"
{
	// ------------------------ test ------------------------------------------
	// ���ⲿϵͳ����
	EXPORT_DLL int export_test(int a, int b)
	{
		return a + b;
	}

	EXPORT_DLL BOOL ExecuteMAXScriptScript(const wchar_t* s, BOOL quietErrors = FALSE, void* fpv = NULL)
	{
		return TRUE;
	}
}

namespace Hook
{
	HHOOK m_hHook;
	// HOOK ���


	// HOOK ����
	LRESULT CALLBACK HookProc(INT iCode, WPARAM wParam, LPARAM lParam)
	{
		if (iCode > 0)
		{
			;
		}

		return CallNextHookEx(m_hHook, iCode, wParam, lParam);
	}

	// Hook
	inline BOOL WINAPI Hook(INT iHookId = WH_CALLWNDPROC)
	{
		m_hHook = SetWindowsHookEx(iHookId, HookProc, NULL, GetCurrentThreadId());
		return (m_hHook != NULL);
	}

	// Unhook
	inline VOID WINAPI Unhook()
	{
		if (m_hHook)
		{
			UnhookWindowsHookEx(m_hHook);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////
// ��ں���
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		//Hook::Hook();
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//Hook::Unhook();

	}

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////