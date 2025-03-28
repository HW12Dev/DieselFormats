#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "DieselIdstringVisualiserService.h"

class CDieselVisualiserModule : public CAtlDllModuleT<CDieselVisualiserModule> {};

CDieselVisualiserModule _AtlModule;


// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
  hInstance;
  return _AtlModule.DllMain(dwReason, lpReserved);
}

// Used to determine whether the DLL can be unloaded by OLE
__control_entrypoint(DllExport)
STDAPI DllCanUnloadNow(void) {
  return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type
_Check_return_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv) {
  return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}
