#pragma once

#include <my.h>

typedef struct
{
	HDC                 DC;
	HFONT               Font;
	HFONT               OldFont;
	ULONG_PTR           FontType;
	LPENUMLOGFONTEXW    EnumLogFontEx;

} ADJUST_FONT_DATA, *PADJUST_FONT_DATA;

typedef struct TEXT_METRIC_INTERNAL
{
	ULONG       Magic;
	BOOL        Filled;
	TEXTMETRICA TextMetricA;
	TEXTMETRICW TextMetricW;

	TEXT_METRIC_INTERNAL()
	{
		this->Magic = TAG4('TMIN');
		this->Filled = FALSE;
	}

	BOOL VerifyMagic()
	{
		return this->Magic == TAG4('TMIN');
	}

} TEXT_METRIC_INTERNAL, *PTEXT_METRIC_INTERNAL;



typedef struct HookKernel
{
	HWND        MainWindow;
	WNDPROC     StubMainWindowProc;

	ULONG_PTR   OriginalLocaleID;
	ULONG       LocaleID;
	CHAR        ScriptNameA[LF_FACESIZE];
	WCHAR       ScriptNameW[LF_FACESIZE];
	ULONG_PTR   OriginalCharset;

	ml::HashTableT<TEXT_METRIC_INTERNAL>     TextMetricCache;

	API_POINTER(GetVersionExA)       StubGetVersionExA;
	API_POINTER(RegisterClassExA)    StubRegisterClassExA;
	API_POINTER(CreateWindowExA)     StubCreateWindowExA;
	API_POINTER(DefWindowProcA)      StubDefWindowProcA;
	API_POINTER(DispatchMessageA)    StubDispatchMessageA;

	struct
	{
		API_POINTER(GetStockObject)             StubGetStockObject;
		API_POINTER(DeleteObject)               StubDeleteObject;
		API_POINTER(CreateFontIndirectExW)      StubCreateFontIndirectExW;
		API_POINTER(NtGdiHfontCreate)           StubNtGdiHfontCreate;
		API_POINTER(CreateCompatibleDC)         StubCreateCompatibleDC;
		API_POINTER(EnumFontsA)                 StubEnumFontsA;
		API_POINTER(EnumFontsW)                 StubEnumFontsW;
		API_POINTER(EnumFontFamiliesA)          StubEnumFontFamiliesA;
		API_POINTER(EnumFontFamiliesW)          StubEnumFontFamiliesW;
		API_POINTER(EnumFontFamiliesExA)        StubEnumFontFamiliesExA;
		API_POINTER(EnumFontFamiliesExW)        StubEnumFontFamiliesExW;

	} HookStub;

	struct HookRoutineData
	{
		struct
		{
			BOOLEAN StockObjectInitialized : 1;

			RTL_CRITICAL_SECTION GdiLock;

			HGDIOBJ StockObject[STOCK_LAST + 1];

		} Gdi32;

	} HookRoutineData;

	HookKernel()
	{
		HookRoutineData.Gdi32.StockObjectInitialized = FALSE;
		RtlZeroMemory(HookRoutineData.Gdi32.StockObject, 0, sizeof(HookRoutineData.Gdi32.StockObject));

		HookStub.StubGetStockObject = nullptr;
		HookStub.StubDeleteObject   = nullptr;
		HookStub.StubCreateFontIndirectExW = nullptr;
		HookStub.StubNtGdiHfontCreate = nullptr;
		HookStub.StubCreateCompatibleDC = nullptr;
		HookStub.StubEnumFontsA = nullptr;
		HookStub.StubEnumFontsW = nullptr;
		HookStub.StubEnumFontFamiliesA = nullptr;
		HookStub.StubEnumFontFamiliesW = nullptr;
		HookStub.StubEnumFontFamiliesExA = nullptr;
		HookStub.StubEnumFontFamiliesExW = nullptr; 

		StubGetVersionExA    = nullptr;
		StubRegisterClassExA = nullptr;
		StubCreateWindowExA  = nullptr;
		StubDefWindowProcA   = nullptr;
		StubDispatchMessageA = nullptr;

		MainWindow = nullptr;
		StubMainWindowProc = nullptr;

		OriginalLocaleID = 0;
		LocaleID = 0;
		RtlZeroMemory(ScriptNameA, sizeof(ScriptNameA));
		RtlZeroMemory(ScriptNameW, sizeof(ScriptNameW));
		OriginalCharset = 0;

	}


	NTSTATUS AdjustFontData(HDC DC, LPENUMLOGFONTEXW EnumLogFontEx, PTEXT_METRIC_INTERNAL TextMetric, ULONG_PTR FontType);
	NTSTATUS AdjustFontDataInternal(PADJUST_FONT_DATA AdjustData);
	NTSTATUS GetNameRecordFromNameTable(PVOID TableBuffer, ULONG_PTR TableSize, ULONG_PTR NameID, ULONG_PTR LanguageID, PUNICODE_STRING Name);

	VOID GetTextMetricsWFromLogFont(PTEXTMETRICW TextMetricW, CONST LOGFONTW *LogFont);

	PTEXT_METRIC_INTERNAL GetTextMetricFromCache(LPENUMLOGFONTEXW LogFont);
	VOID AddTextMetricToCache(LPENUMLOGFONTEXW LogFont, PTEXT_METRIC_INTERNAL TextMetric);

	HDC CreateCompatibleDC(HDC hDC)
	{
		return HookStub.StubCreateCompatibleDC(hDC);
	}

	HGDIOBJ GetStockObject(LONG Object)
	{
		return HookStub.StubGetStockObject(Object);
	}

	BOOL DeleteObject(HGDIOBJ GdiObject)
	{
		return HookStub.StubDeleteObject(GdiObject);
	}

	int EnumFontsA(HDC hdc, PCSTR lpFaceName, FONTENUMPROCA lpFontFunc, LPARAM lParam)
	{
		return HookStub.StubEnumFontsA(hdc, lpFaceName, lpFontFunc, lParam);
	}

	int EnumFontsW(HDC hdc, PCWSTR lpFaceName, FONTENUMPROCW lpFontFunc, LPARAM lParam)
	{
		return HookStub.StubEnumFontsW(hdc, lpFaceName, lpFontFunc, lParam);
	}

	int EnumFontFamiliesA(HDC hdc, LPCSTR lpFaceName, FONTENUMPROCA lpProc, LPARAM lParam)
	{
		return HookStub.StubEnumFontFamiliesA(hdc, lpFaceName, lpProc, lParam);
	}

	int EnumFontFamiliesW(HDC hdc, LPCWSTR lpFaceName, FONTENUMPROCW lpProc, LPARAM lParam)
	{
		return HookStub.StubEnumFontFamiliesW(hdc, lpFaceName, lpProc, lParam);
	}

	int EnumFontFamiliesExA(HDC hdc, LPLOGFONTA lpLogfont, FONTENUMPROCA lpProc, LPARAM lParam, DWORD dwFlags)
	{
		return HookStub.StubEnumFontFamiliesExA(hdc, lpLogfont, lpProc, lParam, dwFlags);
	}

	int EnumFontFamiliesExW(HDC hdc, LPLOGFONTW lpLogfont, FONTENUMPROCW lpProc, LPARAM lParam, DWORD dwFlags)
	{
		return HookStub.StubEnumFontFamiliesExW(hdc, lpLogfont, lpProc, lParam, dwFlags);
	}

	NTSTATUS HookGdi32Routines(PVOID Gdi32);
	NTSTATUS UnHookGdi32Routines();
	VOID     InitFontCharsetInfo();

}HookKernel, *PHookKernel;


PHookKernel LeGetGlobalData();


