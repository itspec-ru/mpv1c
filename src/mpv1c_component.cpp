// Отключаем предупреждение GCC 15 о visibility для неклассовых типов 
// при включении официальных заголовков 1С
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "IMemoryManager.h"

#pragma GCC diagnostic pop

#include <string>
#include <cstring>
#include <cstdint>
#include <fstream>


void LogToFile(const std::string& msg) {
    std::ofstream log("/tmp/mpv1c_debug.log", std::ios::app);
    if (log.is_open()) {
        log << msg << std::endl;
        log.flush();
    }
}

class Mpv1cComponent : public IComponentBase {
private:
    IAddInDefBase*  m_iConnect = nullptr;
    IMemoryManager* m_iMemory = nullptr;

    WCHAR_T* AllocWString(const std::string& str) {
        if (!m_iMemory) return nullptr;
        size_t len = str.length();
        size_t bytes = (len + 1) * sizeof(WCHAR_T);
        WCHAR_T* buf = nullptr;
        if (m_iMemory->AllocMemory((void**)&buf, bytes)) {
            for (size_t i = 0; i < len; ++i) {
                buf[i] = static_cast<WCHAR_T>(str[i]);
            }
            buf[len] = 0;
            return buf;
        }
        return nullptr;
    }

public:
    Mpv1cComponent() { LogToFile("=== Mpv1cComponent constructor ==="); }
    virtual ~Mpv1cComponent() { LogToFile("=== Mpv1cComponent destructor ==="); }

    bool ADDIN_API Init(void* disp) override { 
        LogToFile("Init() called");
        m_iConnect = static_cast<IAddInDefBase*>(disp);
        return true; 
    }
    
    bool ADDIN_API setMemManager(void* mem) override { 
        LogToFile("setMemManager() called");
        m_iMemory = static_cast<IMemoryManager*>(mem); 
        
        // Тестовое сообщение в журнал регистрации 1С при успешной инициализации
        if (m_iConnect && m_iMemory) {
            WCHAR_T* msg = AllocWString("Mpv1c plugin initialized successfully");
            WCHAR_T* source = AllocWString("Mpv1c");
            
            if (msg && source) {
                // 1005 = ADDIN_E_INFO (информационное сообщение в журнал 1С)
                m_iConnect->AddError(1005, source, msg, 0);
                m_iMemory->FreeMemory((void**)&msg);
                m_iMemory->FreeMemory((void**)&source);
            } else {
                // Освобождаем память, если одна из строк не выделилась
                if (msg) m_iMemory->FreeMemory((void**)&msg);
                if (source) m_iMemory->FreeMemory((void**)&source);
            }
        }
        return true; 
    }
    
    long ADDIN_API GetInfo() override { 
        return 2100; // API версия 2.10
    }
    
    void ADDIN_API Done() override {
        LogToFile("Done() called");
    }
    
    bool ADDIN_API RegisterExtensionAs(WCHAR_T** wsExtensionName) override {
        *wsExtensionName = AllocWString("Mpv1c");
        return *wsExtensionName != nullptr;
    }
    
    long ADDIN_API GetNProps() override { return 0; }
    long ADDIN_API FindProp(const WCHAR_T* wsPropName) override { return -1; }
    const WCHAR_T* GetPropName(long lPropNum, long lPropAlias) override { return nullptr; }
    bool ADDIN_API GetPropVal(long lPropNum, tVariant* pvarPropVal) override { return false; }
    bool ADDIN_API SetPropVal(long lPropNum, tVariant* pvarPropVal) override { return false; }
    
    // КРИТИЧЕСКИ ВАЖНО: сигнатура с bool*
    bool ADDIN_API IsPropReadable(long lPropNum, bool *pbVal) override { 
        if (pbVal) *pbVal = false; 
        return true; 
    }
    bool ADDIN_API IsPropWritable(long lPropNum, bool *pbVal) override { 
        if (pbVal) *pbVal = false; 
        return true; 
    }
    
    long ADDIN_API GetNMethods() override { return 0; }
    long ADDIN_API FindMethod(const WCHAR_T* wsMethodName) override { return -1; }
    const WCHAR_T* GetMethodName(long lMethodNum, long lMethodAlias) override { return nullptr; }
    long ADDIN_API GetNParams(long lMethodNum) override { return 0; }
    bool ADDIN_API GetParamDefValue(long lMethodNum, long lParamNum, tVariant *pvarParamDefValue) override { return false; }
    bool ADDIN_API HasRetVal(long lMethodNum) override { return false; }
    bool ADDIN_API CallAsProc(long lMethodNum, tVariant* paParams, long lSizeArray) override { return false; }
    bool ADDIN_API CallAsFunc(long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, long lSizeArray) override { return false; }
    
    bool ADDIN_API SetLocale(const WCHAR_T* loc) override { return true; }
};

// ============================================================================
// ЭКСПОРТНЫЕ ФУНКЦИИ
// ============================================================================

static WCHAR_T g_classNames[] = { 'M', 'p', 'v', '1', 'c', '\0' };

extern "C" long ADDIN_API GetClassNames(WCHAR_T** names) {
    *names = g_classNames;
    return 1;
}

extern "C" long ADDIN_API GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface) {
    if (!pInterface) return 0;
    *pInterface = new Mpv1cComponent();
    return 1;
}

extern "C" long ADDIN_API DestroyObject(IComponentBase** pInterface) {
    if (!pInterface || !*pInterface) return 0;
    delete *pInterface; 
    *pInterface = nullptr;
    return 0;
}

extern "C" long ADDIN_API SetPlatformCapabilities(long capabilities) {
    return 1; // eAppCapabilities1
}

extern "C" long ADDIN_API GetAttachType() {
    return 1; // eCanAttachNotIsolated
}
