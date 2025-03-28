#include "DieselIdstringVisualiserService.h"

/*#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "DieselFormats.lib")*/

extern "C" { // hacky way to avoid needing to link against zlib.lib, shouldn't cause any problems since it doesn't get called for Idstring lookups
  int inflate(void* strm, int flush) {}
  int inflateEnd(void* strm) {}
  int inflateInit2_(void* strm, int windowBits, const char* version, int stream_size) {};
}

#include <diesel/modern/hash.h>
#include <diesel/modern/hashlist.h>

#include <string>

CDieselIdstringVisualiserService::CDieselIdstringVisualiserService()
{
  Reader hashlist_file("X:\\Projects\\DieselEngineExplorer\\hashlist.txt"); // ADD HASHLIST PATH TO VS2022 PROPERTY PAGES!!!!
  diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlist_file);
  hashlist_file.Close();
}

CDieselIdstringVisualiserService::~CDieselIdstringVisualiserService()
{
}

std::wstring narrow_to_wide_string(const std::string& str) {
  if (str.empty()) return std::wstring();

  auto length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), 0, 0);

  std::wstring wide = std::wstring(length, '\0');

  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), wide.data(), wide.length());

  return wide;
}

HRESULT FormatIdstringToATLCString(const diesel::modern::Idstring& idstring, ATL::CString& out_cstring) {
  std::string narrow_idstring;
  bool success = diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(idstring, narrow_idstring);

  // begin 1.1 change
  if (success) {
      narrow_idstring = "Idstring(\"" + narrow_idstring + "\")";
  }
  else {
      narrow_idstring = "@ID" + narrow_idstring + "@";
  }


  // end 1.1 change


  out_cstring = ATL::CString(narrow_to_wide_string(narrow_idstring).c_str());

  return S_OK;
}

HRESULT STDMETHODCALLTYPE CDieselIdstringVisualiserService::EvaluateVisualizedExpression(
  _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
  _Deref_out_opt_ Evaluation::DkmEvaluationResult** ppResultObject
) {
  HRESULT hr;

  Evaluation::DkmPointerValueHome* pointer_value_home = Evaluation::DkmPointerValueHome::TryCast(pVisualizedExpression->ValueHome());
  if (pointer_value_home == nullptr) {
    return E_NOTIMPL;
  }

  Evaluation::DkmRootVisualizedExpression* root_visualised_expression = Evaluation::DkmRootVisualizedExpression::TryCast(pVisualizedExpression);
  if (root_visualised_expression == nullptr) {
    return E_NOTIMPL;
  }

  DkmProcess* target_process = pVisualizedExpression->RuntimeInstance()->Process();

  diesel::modern::Idstring idstring;
  hr = target_process->ReadMemory(pointer_value_home->Address(), DkmReadMemoryFlags::None, &idstring, sizeof(idstring), nullptr);
  if (FAILED(hr)) {
    return E_NOTIMPL;
  }

  ATL::CString formatted;
  hr = FormatIdstringToATLCString(idstring, formatted);
  if (FAILED(hr)) {
    formatted = "<Invalid Idstring Value>";
  }

  ATL::CString ptr_str;
  ATL::CComPtr<DkmString> type = root_visualised_expression->Type();
  if(type != nullptr && wcschr(type->Value(), '*') != nullptr) {
    UINT64 address = pointer_value_home->Address();
    if ((target_process->SystemInformation()->Flags() & DefaultPort::DkmSystemInformationFlags::Is64Bit) != 0) {
      ptr_str.Format(L"0x%08x%08x", static_cast<DWORD>(address >> 32), static_cast<DWORD>(address));
    }
    else {
      ptr_str.Format(L"0x%08x", static_cast<DWORD>(address));
    }

    ATL::CString formatted_with_addr;
    formatted_with_addr.Format(L"%s {%s}", static_cast<LPCWSTR>(ptr_str), static_cast<LPCWSTR>(formatted));
    formatted = formatted_with_addr;
  }
  
  Evaluation::DkmEvaluationResultFlags_t result_flags = Evaluation::DkmEvaluationResultFlags::Expandable;
  if (ptr_str.IsEmpty()) {
    result_flags |= Evaluation::DkmEvaluationResultFlags::ReadOnly; // allow only editing pointer values
  }

  ATL::CComPtr<DkmString> value;
  ATL::CComPtr<DkmString> editable_value;
  ATL::CComPtr<Evaluation::DkmDataAddress> address;

  hr = DkmString::Create(DkmSourceString(formatted), &value);
  if (FAILED(hr)) return hr;
  hr = DkmString::Create(ptr_str, &editable_value);
  if (FAILED(hr)) return hr;
  hr = Evaluation::DkmDataAddress::Create(pVisualizedExpression->RuntimeInstance(), pointer_value_home->Address(), nullptr, &address);
  if (FAILED(hr)) return hr;

  ATL::CComPtr<Evaluation::DkmSuccessEvaluationResult> evaluation_result;

  Evaluation::DkmSuccessEvaluationResult::Create(
    pVisualizedExpression->InspectionContext(),
    pVisualizedExpression->StackFrame(),
    root_visualised_expression->Name(),
    root_visualised_expression->FullName(),
    result_flags,
    value,
    editable_value,
    root_visualised_expression->Type(),
    Evaluation::DkmEvaluationResultCategory::Class,
    Evaluation::DkmEvaluationResultAccessType::None,
    Evaluation::DkmEvaluationResultStorageType::None,
    Evaluation::DkmEvaluationResultTypeModifierFlags::None,
    address,
    nullptr,
    nullptr,
    DkmDataItem::Null(),
    &evaluation_result
  );

  *ppResultObject = evaluation_result.Detach();

  return S_OK;
}

HRESULT STDMETHODCALLTYPE CDieselIdstringVisualiserService::UseDefaultEvaluationBehavior(
  _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
  _Out_ bool* pUseDefaultEvaluationBehavior,
  _Deref_out_opt_ Evaluation::DkmEvaluationResult** ppDefaultEvaluationResult
) {
  Evaluation::DkmRootVisualizedExpression* root_visualised_expression = Evaluation::DkmRootVisualizedExpression::TryCast(pVisualizedExpression);
  if (root_visualised_expression == nullptr)
    return E_NOTIMPL;

  Evaluation::DkmInspectionContext* inspection_context = pVisualizedExpression->InspectionContext();

  CAutoDkmClosePtr<Evaluation::DkmLanguageExpression> language_expression;
  HRESULT hr = Evaluation::DkmLanguageExpression::Create(
    inspection_context->Language(),
    Evaluation::DkmEvaluationFlags::TreatAsExpression,
    root_visualised_expression->FullName(),
    DkmDataItem::Null(),
    &language_expression
  );

  if (FAILED(hr)) {
    return hr;
  }

  ATL::CComPtr<Evaluation::DkmInspectionContext> new_inspection_context;

  if (DkmComponentManager::IsApiVersionSupported(DkmApiVersion::VS16RTMPreview)) {
    hr = Evaluation::DkmInspectionContext::Create(
      inspection_context->InspectionSession(),
      inspection_context->RuntimeInstance(),
      inspection_context->Thread(),
      inspection_context->Timeout(),
      Evaluation::DkmEvaluationFlags::TreatAsExpression | Evaluation::DkmEvaluationFlags::ShowValueRaw,
      inspection_context->FuncEvalFlags(),
      inspection_context->Radix(),
      inspection_context->Language(),
      inspection_context->ReturnValue(),
      nullptr,
      Evaluation::DkmCompiledVisualizationDataPriority::None,
      inspection_context->ReturnValues(),
      inspection_context->SymbolsConnection(),
      &new_inspection_context
    );
  }
  else {
    hr = Evaluation::DkmInspectionContext::Create(
      inspection_context->InspectionSession(),
      inspection_context->RuntimeInstance(),
      inspection_context->Thread(),
      inspection_context->Timeout(),
      Evaluation::DkmEvaluationFlags::TreatAsExpression | Evaluation::DkmEvaluationFlags::ShowValueRaw,
      inspection_context->FuncEvalFlags(),
      inspection_context->Radix(),
      inspection_context->Language(),
      inspection_context->ReturnValue(),
      nullptr,
      Evaluation::DkmCompiledVisualizationDataPriority::None,
      inspection_context->ReturnValues(),
      &new_inspection_context
    );
  }
  if (FAILED(hr)) {
    return hr;
  }

  ATL::CComPtr<Evaluation::DkmEvaluationResult> evaluation_result;

  hr = pVisualizedExpression->EvaluateExpressionCallback(
    new_inspection_context,
    language_expression,
    pVisualizedExpression->StackFrame(),
    &evaluation_result
  );
  if (FAILED(hr)) {
    return hr;
  }

  *ppDefaultEvaluationResult = evaluation_result.Detach();
  *pUseDefaultEvaluationBehavior = true;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE CDieselIdstringVisualiserService::GetChildren(
  _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
  _In_ UINT32 InitialRequestSize,
  _In_ Evaluation::DkmInspectionContext* pInspectionContext,
  _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* pInitialChildren,
  _Deref_out_ Evaluation::DkmEvaluationResultEnumContext** ppEnumContext
) {
  return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CDieselIdstringVisualiserService::GetItems(
  _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
  _In_ Evaluation::DkmEvaluationResultEnumContext* pEnumContext,
  _In_ UINT32 StartIndex,
  _In_ UINT32 Count,
  _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* pItems
) {
  return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CDieselIdstringVisualiserService::SetValueAsString(
  _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
  _In_ DkmString* pValue,
  _In_ UINT32 Timeout,
  _Deref_out_opt_ DkmString** ppErrorText
) {
  return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CDieselIdstringVisualiserService::GetUnderlyingString(
  _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
  _Deref_out_opt_ DkmString** ppStringValue
) {
  return E_NOTIMPL;
}
