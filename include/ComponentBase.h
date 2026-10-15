#ifndef __COMPONENT_BASE_H__
#define __COMPONENT_BASE_H__
#include "types.h"

class IComponentBase {
public:
    virtual bool ADDIN_API Init(void* disp) = 0;
    virtual bool ADDIN_API setMemManager(void* mem) = 0;
    virtual long ADDIN_API GetInfo() = 0;
    virtual void ADDIN_API Done() = 0;
    virtual bool ADDIN_API RegisterExtensionAs(WCHAR_T** wsExtensionName) = 0;
    
    virtual long ADDIN_API GetNProps() = 0;
    virtual long ADDIN_API FindProp(const WCHAR_T* wsPropName) = 0;
    virtual const WCHAR_T* GetPropName(long lPropNum, long lPropAlias) = 0;
    virtual bool ADDIN_API GetPropVal(long lPropNum, tVariant* pvarPropVal) = 0;
    virtual bool ADDIN_API SetPropVal(long lPropNum, tVariant* pvarPropVal) = 0;
    virtual bool ADDIN_API IsPropReadable(long lPropNum, bool *pbVal) = 0;
    virtual bool ADDIN_API IsPropWritable(long lPropNum, bool *pbVal) = 0;
    
    virtual long ADDIN_API GetNMethods() = 0;
    virtual long ADDIN_API FindMethod(const WCHAR_T* wsMethodName) = 0;
    virtual const WCHAR_T* GetMethodName(long lMethodNum, long lMethodAlias) = 0;
    virtual long ADDIN_API GetNParams(long lMethodNum) = 0;
    virtual bool ADDIN_API GetParamDefValue(long lMethodNum, long lParamNum, tVariant *pvarParamDefValue) = 0;
    virtual bool ADDIN_API HasRetVal(long lMethodNum) = 0;
    virtual bool ADDIN_API CallAsProc(long lMethodNum, tVariant* paParams, long lSizeArray) = 0;
    virtual bool ADDIN_API CallAsFunc(long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, long lSizeArray) = 0;
    
    virtual bool ADDIN_API SetLocale(const WCHAR_T* loc) = 0;
    
    virtual ~IComponentBase() {}
};
#endif
