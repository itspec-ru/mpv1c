// Для Windows нужно подключить заголовки SDK ДО официальных заголовков 1С,
// так как в types.h используются типы DATE и IID из Windows SDK
#ifdef _WIN32
#include <windows.h>
#include <wtypes.h>
#endif

// Подавляем предупреждения GCC 15 о visibility в официальных заголовках 1С
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "IMemoryManager.h"

#pragma GCC diagnostic pop

#include <cstddef>

class CMpv1c : public IComponentBase
{
private:
    IAddInDefBase* m_iConnect;
    IMemoryManager* m_iMemory;

public:
    CMpv1c() : m_iConnect(nullptr), m_iMemory(nullptr) {}
    virtual ~CMpv1c() {}

    bool ADDIN_API Init(void* disp) override
    {
        m_iConnect = static_cast<IAddInDefBase*>(disp);
        return true;
    }

    bool ADDIN_API setMemManager(void* mem) override
    {
        m_iMemory = static_cast<IMemoryManager*>(mem);
        return true;
    }

    long ADDIN_API GetInfo() override
    {
        return 2100;
    }

    void ADDIN_API Done() override {}

    bool ADDIN_API RegisterExtensionAs(WCHAR_T** wsExtensionName) override
    {
        // Используем u"" для char16_t (WCHAR_T в современном SDK на Linux)
        const WCHAR_T* wsExtension = u"Mpv1c";
        
        // Безопасный подсчет длины без wcslen
        size_t len = 0;
        while (wsExtension[len] != u'\0') {
            len++;
        }
        size_t iActualSize = len + 1;
        
        if (m_iMemory)
        {
            if (m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
            {
                WCHAR_T* dest = static_cast<WCHAR_T*>(*wsExtensionName);
                for (size_t i = 0; i < iActualSize; i++)
                {
                    dest[i] = wsExtension[i];
                }
                return true;
            }
        }
        return false;
    }

    // --- Свойства ---
    long ADDIN_API GetNProps() override { return 0; }
    long ADDIN_API FindProp(const WCHAR_T* wsPropName) override { return -1; }
    const WCHAR_T* ADDIN_API GetPropName(long lPropNum, long lPropAlias) override { return nullptr; }
    bool ADDIN_API GetPropVal(long lPropNum, tVariant* pvarPropVal) override { return false; }
    bool ADDIN_API SetPropVal(long lPropNum, tVariant* pvarPropVal) override { return false; }
    
    // ИСПРАВЛЕНО: В современном SDK эти методы принимают только один аргумент
    bool ADDIN_API IsPropReadable(long lPropNum) override { return false; }
    bool ADDIN_API IsPropWritable(long lPropNum) override { return false; }

    // --- Методы ---
    long ADDIN_API GetNMethods() override { return 0; }
    long ADDIN_API FindMethod(const WCHAR_T* wsMethodName) override { return -1; }
    const WCHAR_T* ADDIN_API GetMethodName(long lMethodNum, long lMethodAlias) override { return nullptr; }
    long ADDIN_API GetNParams(long lMethodNum) override { return 0; }
    bool ADDIN_API GetParamDefValue(long lMethodNum, long lParamNum, tVariant *pvarParamDefValue) override { return false; }
    bool ADDIN_API HasRetVal(long lMethodNum) override { return false; }
    bool ADDIN_API CallAsProc(long lMethodNum, tVariant* paParams, long lSizeArray) override { return false; }
    bool ADDIN_API CallAsFunc(long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, long lSizeArray) override { return false; }

    // ИСПРАВЛЕНО: В современном SDK этот метод возвращает void, а не bool
    void ADDIN_API SetLocale(const WCHAR_T* loc) override {}
    
    // ИСПРАВЛЕНО: Добавлен обязательный метод из современного SDK
    void ADDIN_API SetUserInterfaceLanguageCode(const WCHAR_T* lang) override {}
};

// ============================================================================
// ЭКСПОРТНЫЕ ФУНКЦИИ (СТРОГО по официальным заголовкам ComponentBase.h)
// ============================================================================

static WCHAR_T g_classNames[] = { u'M', u'p', u'v', u'1', u'c', u'\0' };

// ИСПРАВЛЕНО: Нет параметров, возвращает const WCHAR_T*
extern "C" const WCHAR_T* ADDIN_API GetClassNames()
{
    return g_classNames;
}

extern "C" long ADDIN_API GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
    if (!pInterface) return 0;
    *pInterface = new CMpv1c();
    return 1;
}

extern "C" long ADDIN_API DestroyObject(IComponentBase** pInterface)
{
    if (!pInterface || !*pInterface) return 0;
    delete *pInterface;
    *pInterface = nullptr;
    return 0;
}

// ИСПРАВЛЕНО: Используем типы enum из SDK, а не long
extern "C" AppCapabilities ADDIN_API SetPlatformCapabilities(const AppCapabilities capabilities)
{
    return eAppCapabilities1; 
}

extern "C" AttachType ADDIN_API GetAttachType()
{
    return eCanAttachNotIsolated; 
}
