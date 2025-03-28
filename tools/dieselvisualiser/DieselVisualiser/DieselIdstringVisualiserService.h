#pragma once

#ifndef STRICT
#define STRICT
#endif

#define _ATL_FREE_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

using namespace ATL; // vsdebugeng.templates.h requires the ATL namespace to be available in the global scope for some reason

#include <vsdebugeng.h>
#include <vsdebugeng.templates.h>

#include "DieselVisualiser.Contract.h"

using namespace Microsoft::VisualStudio::Debugger;

class ATL_NO_VTABLE CDieselIdstringVisualiserService : public CDieselIdstringVisualiserServiceContract, public ATL::CComObjectRootEx<ATL::CComMultiThreadModel>, public ATL::CComCoClass<CDieselIdstringVisualiserService, &CDieselIdstringVisualiserServiceContract::ClassId> {
protected:
  CDieselIdstringVisualiserService();
  ~CDieselIdstringVisualiserService();

public:
  DECLARE_NO_REGISTRY();
  DECLARE_NOT_AGGREGATABLE(CDieselIdstringVisualiserService);


public:
  HRESULT STDMETHODCALLTYPE EvaluateVisualizedExpression(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _Deref_out_opt_ Evaluation::DkmEvaluationResult** ppResultObject
  );
  HRESULT STDMETHODCALLTYPE UseDefaultEvaluationBehavior(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _Out_ bool* pUseDefaultEvaluationBehavior,
    _Deref_out_opt_ Evaluation::DkmEvaluationResult** ppDefaultEvaluationResult
  );
  HRESULT STDMETHODCALLTYPE GetChildren(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _In_ UINT32 InitialRequestSize,
    _In_ Evaluation::DkmInspectionContext* pInspectionContext,
    _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* pInitialChildren,
    _Deref_out_ Evaluation::DkmEvaluationResultEnumContext** ppEnumContext
  );
  HRESULT STDMETHODCALLTYPE GetItems(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _In_ Evaluation::DkmEvaluationResultEnumContext* pEnumContext,
    _In_ UINT32 StartIndex,
    _In_ UINT32 Count,
    _Out_ DkmArray<Evaluation::DkmChildVisualizedExpression*>* pItems
  );
  HRESULT STDMETHODCALLTYPE SetValueAsString(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _In_ DkmString* pValue,
    _In_ UINT32 Timeout,
    _Deref_out_opt_ DkmString** ppErrorText
  );
  HRESULT STDMETHODCALLTYPE GetUnderlyingString(
    _In_ Evaluation::DkmVisualizedExpression* pVisualizedExpression,
    _Deref_out_opt_ DkmString** ppStringValue
  );

private:
};

OBJECT_ENTRY_AUTO(CDieselIdstringVisualiserService::ClassId, CDieselIdstringVisualiserService)