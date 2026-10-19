#ifdef _WIN32
#include <windows.h>
#include <wtypes.h>
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "IMemoryManager.h"
#pragma GCC diagnostic pop

#include <cstddef>
#include <cstring>
#include <string>
#include <thread>
#include <atomic>
#include <fstream>
#include <cstdlib>
#include <mpv/client.h>

// ============================================================================
// Логирование в файл (для диагностики)
// ============================================================================
static void LogMsg(const std::string& msg) {
    std::ofstream log("/tmp/mpv1c_debug.log", std::ios::app);
    if (log.is_open()) {
        log << msg << std::endl;
    }
}

// ============================================================================
// Перечисление методов
// ============================================================================
enum Methods {
    eVersion = 0,
    eTestFunc,
    eInitialize,
    eSetParentWindow,
    ePlay,
    eMethLast
};

static const WCHAR_T g_MethodNamesRu[][32] = {
    { u'В', u'е', u'р', u'с', u'и', u'я', 0 },
    { u'Т', u'е', u'с', u'т', u'о', u'в', u'а', u'я', u'Ф', u'у', u'н', u'к', u'ц', u'и', u'я', 0 },
    { u'И', u'н', u'и', u'ц', u'и', u'а', u'л', u'и', u'з', u'и', u'р', u'о', u'в', u'а', u'т', u'ь', 0 },
    { u'У', u'с', u'т', u'а', u'н', u'о', u'в', u'и', u'т', u'ь', u'Р', u'о', u'д', u'и', u'т', u'е', u'л', u'ь', u'с', u'к', u'о', u'е', u'О', u'к', u'н', u'о', 0 },
    { u'В', u'о', u'с', u'п', u'р', u'о', u'и', u'з', u'в', u'е', u'с', u'т', u'и', 0 }
};

static const WCHAR_T g_MethodNamesEn[][32] = {
    { u'V', u'e', u'r', u's', u'i', u'o', u'n', 0 },
    { u'T', u'e', u's', u't', u'F', u'u', u'n', u'c', 0 },
    { u'I', u'n', u'i', u't', u'i', u'a', u'l', u'i', u'z', u'e', 0 },
    { u'S', u'e', u't', u'P', u'a', u'r', u'e', u'n', u't', u'W', u'i', u'n', u'd', u'o', u'w', 0 },
    { u'P', u'l', u'a', u'y', 0 }
};

// ============================================================================
// Класс компоненты
// ============================================================================
class CMpv1c : public IComponentBase {
private:
    IAddInDefBase* m_iConnect;
    IMemoryManager* m_iMemory;
    
    mpv_handle* m_mpv;
    int64_t m_windowId;
    
    std::thread m_eventThread;
    std::atomic<bool> m_stopEventLoop{false};

    void eventLoop() {
        LogMsg("[EventThread] Start");
        while (!m_stopEventLoop && m_mpv) {
            mpv_event* event = mpv_wait_event(m_mpv, 0.1);
            if (event->event_id == MPV_EVENT_NONE) continue;
            
            // Логируем все события mpv
            std::string evName = mpv_event_name(event->event_id);
            LogMsg("[EventThread] Event: " + evName);
            
            if (event->event_id == MPV_EVENT_LOG_MESSAGE) {
                mpv_event_log_message* msg = (mpv_event_log_message*)event->data;
                LogMsg(std::string("[mpv log] ") + msg->prefix + ": " + msg->text);
            }
        }
        LogMsg("[EventThread] Stop");
    }

    std::string WcharToUtf8(const WCHAR_T* wstr, size_t len) {
        if (!wstr || len == 0) return "";
        std::string result;
        result.reserve(len * 3);
        for (size_t i = 0; i < len; ++i) {
            char16_t ch = static_cast<char16_t>(wstr[i]);
            if (ch < 0x80) {
                result.push_back(static_cast<char>(ch));
            } else if (ch < 0x800) {
                result.push_back(static_cast<char>(0xC0 | (ch >> 6)));
                result.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
            } else {
                result.push_back(static_cast<char>(0xE0 | (ch >> 12)));
                result.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
            }
        }
        return result;
    }

    bool VariantToInt64(const tVariant* pv, int64_t& result) {
        if (!pv) return false;
        switch (pv->vt) {
            case VTYPE_I2:
            case VTYPE_I4:
            case VTYPE_ERROR:
            case VTYPE_UI1:
                result = static_cast<int64_t>(pv->lVal);
                return true;
            case VTYPE_BOOL:
                result = pv->bVal ? 1 : 0;
                return true;
            case VTYPE_R4:
            case VTYPE_R8:
                result = static_cast<int64_t>(pv->dblVal);
                return true;
            default:
                return false;
        }
    }

public:
    CMpv1c() : m_iConnect(nullptr), m_iMemory(nullptr), m_mpv(nullptr), m_windowId(0) {
        LogMsg("=== CMpv1c constructor ===");
    }
    
    virtual ~CMpv1c() {
        LogMsg("=== CMpv1c destructor ===");
        m_stopEventLoop = true;
        if (m_eventThread.joinable()) {
            m_eventThread.join();
        }
        if (m_mpv) {
            mpv_terminate_destroy(m_mpv);
            m_mpv = nullptr;
        }
    }

    bool ADDIN_API Init(void* disp) override {
        LogMsg("Init() called");
        m_iConnect = static_cast<IAddInDefBase*>(disp);
        return true;
    }

    bool ADDIN_API setMemManager(void* mem) override {
        LogMsg("setMemManager() called");
        m_iMemory = static_cast<IMemoryManager*>(mem);
        return true;
    }

    long ADDIN_API GetInfo() override { return 2000; }
    void ADDIN_API Done() override { LogMsg("Done() called"); }

    bool ADDIN_API RegisterExtensionAs(WCHAR_T** wsExtensionName) override {
        LogMsg("RegisterExtensionAs() called");
        const WCHAR_T* wsExtension = u"Mpv1c";
        size_t len = 0;
        while (wsExtension[len] != u'\0') len++;
        size_t iActualSize = len + 1;

        if (m_iMemory && m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T))) {
            WCHAR_T* dest = static_cast<WCHAR_T*>(*wsExtensionName);
            for (size_t i = 0; i < iActualSize; i++) dest[i] = wsExtension[i];
            return true;
        }
        return false;
    }

    long ADDIN_API GetNProps() override { return 0; }
    long ADDIN_API FindProp(const WCHAR_T* wsPropName) override { return -1; }
    const WCHAR_T* ADDIN_API GetPropName(long lPropNum, long lPropAlias) override { return nullptr; }
    bool ADDIN_API GetPropVal(long lPropNum, tVariant* pvarPropVal) override { return false; }
    bool ADDIN_API SetPropVal(long lPropNum, tVariant* pvarPropVal) override { return false; }
    bool ADDIN_API IsPropReadable(long lPropNum) override { return false; }
    bool ADDIN_API IsPropWritable(long lPropNum) override { return false; }

    long ADDIN_API GetNMethods() override { return eMethLast; }

    long ADDIN_API FindMethod(const WCHAR_T* wsMethodName) override {
        if (!wsMethodName) return -1;
        for (long i = 0; i < eMethLast; ++i) {
            const WCHAR_T* ru = g_MethodNamesRu[i];
            bool match = true;
            for (size_t k = 0; ; ++k) {
                if (ru[k] == 0 && wsMethodName[k] == 0) break;
                if (ru[k] != wsMethodName[k]) { match = false; break; }
            }
            if (match) return i;

            const WCHAR_T* en = g_MethodNamesEn[i];
            match = true;
            for (size_t k = 0; ; ++k) {
                if (en[k] == 0 && wsMethodName[k] == 0) break;
                if (en[k] != wsMethodName[k]) { match = false; break; }
            }
            if (match) return i;
        }
        return -1;
    }

    const WCHAR_T* ADDIN_API GetMethodName(long lMethodNum, long lMethodAlias) override {
        if (lMethodNum < 0 || lMethodNum >= eMethLast) return nullptr;
        
        const WCHAR_T* wsCurrentName = (lMethodAlias == 1) ? g_MethodNamesEn[lMethodNum] : g_MethodNamesRu[lMethodNum];
        
        size_t len = 0;
        while (wsCurrentName[len] != u'\0') len++;
        size_t iActualSize = len + 1;
        
        WCHAR_T* wsMethodName = nullptr;
        if (m_iMemory && m_iMemory->AllocMemory((void**)&wsMethodName, iActualSize * sizeof(WCHAR_T))) {
            for (size_t i = 0; i < iActualSize; i++) {
                wsMethodName[i] = wsCurrentName[i];
            }
        }
        return wsMethodName;
    }

    long ADDIN_API GetNParams(long lMethodNum) override {
        switch (lMethodNum) {
            case eVersion:         return 0;
            case eTestFunc:        return 0;
            case eInitialize:      return 0;
            case eSetParentWindow: return 1;
            case ePlay:            return 1;
        }
        return 0;
    }

    bool ADDIN_API GetParamDefValue(long lMethodNum, long lParamNum, tVariant *pvarParamDefValue) override {
        pvarParamDefValue->vt = VTYPE_EMPTY;
        return true;
    }

    bool ADDIN_API HasRetVal(long lMethodNum) override {
        switch (lMethodNum) {
            case eVersion:  return true;
            case eTestFunc: return true;
            case ePlay:     return true;
            case eInitialize:      return false;
            case eSetParentWindow: return false;
        }
        return false;
    }

    bool ADDIN_API CallAsProc(long lMethodNum, tVariant* paParams, long lSizeArray) override {
        LogMsg("CallAsProc: method=" + std::to_string(lMethodNum));
        
        switch (lMethodNum) {
            case eInitialize: {
                LogMsg("[Init] Starting mpv initialization...");
                
                // Логируем переменную окружения DISPLAY
                const char* display = std::getenv("DISPLAY");
                LogMsg(std::string("[Init] DISPLAY=") + (display ? display : "(null)"));
                
                // Останавливаем предыдущий поток
                m_stopEventLoop = true;
                if (m_eventThread.joinable()) {
                    m_eventThread.join();
                }
                
                if (m_mpv) {
                    mpv_terminate_destroy(m_mpv);
                    m_mpv = nullptr;
                }
                
                m_mpv = mpv_create();
                if (!m_mpv) {
                    LogMsg("[Init] ERROR: mpv_create() returned NULL");
                    return false;
                }
                LogMsg("[Init] mpv_create() OK");
                
                // Включаем подробное логирование mpv
                int level = MPV_LOG_LEVEL_V;
                mpv_request_log_messages(m_mpv, "v");
                
                // Устанавливаем опции через mpv_set_option_string (проще и надёжнее)
                int r1 = mpv_set_option_string(m_mpv, "force-window", "yes");
                LogMsg("[Init] force-window=yes: " + std::to_string(r1));
                
                int r2 = mpv_set_option_string(m_mpv, "idle", "yes");
                LogMsg("[Init] idle=yes: " + std::to_string(r2));
                
                int r3 = mpv_set_option_string(m_mpv, "keep-open", "yes");
                LogMsg("[Init] keep-open=yes: " + std::to_string(r3));
                
                // Если окно задано — применяем
                if (m_windowId != 0) {
                    char wid_str[32];
                    snprintf(wid_str, sizeof(wid_str), "%lld", (long long)m_windowId);
                    int r4 = mpv_set_option_string(m_mpv, "wid", wid_str);
                    LogMsg("[Init] wid=" + std::string(wid_str) + ": " + std::to_string(r4));
                }
                
                // Инициализация
                int initResult = mpv_initialize(m_mpv);
                LogMsg("[Init] mpv_initialize() returned: " + std::to_string(initResult));
                
                if (initResult < 0) {
                    LogMsg("[Init] ERROR: mpv_initialize failed with code " + std::to_string(initResult));
                    mpv_terminate_destroy(m_mpv);
                    m_mpv = nullptr;
                    return false;
                }
                
                LogMsg("[Init] mpv_initialize() SUCCESS");
                
                // Запускаем поток обработки событий
                m_stopEventLoop = false;
                m_eventThread = std::thread(&CMpv1c::eventLoop, this);
                LogMsg("[Init] Event thread started");
                
                return true;
            }
            
            case eSetParentWindow: {
                if (lSizeArray < 1) return false;
                
                int64_t wid;
                if (!VariantToInt64(&paParams[0], wid)) return false;
                m_windowId = wid;
                LogMsg("[SetWindow] wid=" + std::to_string(wid));
                
                if (m_mpv) {
                    char wid_str[32];
                    snprintf(wid_str, sizeof(wid_str), "%lld", (long long)m_windowId);
                    mpv_set_option_string(m_mpv, "wid", wid_str);
                }
                return true;
            }
        }
        return false;
    }

    bool ADDIN_API CallAsFunc(long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, long lSizeArray) override {
        LogMsg("CallAsFunc: method=" + std::to_string(lMethodNum));
        
        switch (lMethodNum) {
            case eVersion: {
                const WCHAR_T* res = u"1.0";
                size_t len = 0;
                while (res[len] != u'\0') len++;
                
                if (m_iMemory && m_iMemory->AllocMemory((void**)&pvarRetValue->pwstrVal, (len + 1) * sizeof(WCHAR_T))) {
                    for (size_t i = 0; i <= len; i++) {
                        pvarRetValue->pwstrVal[i] = res[i];
                    }
                    pvarRetValue->wstrLen = len;
                    pvarRetValue->vt = VTYPE_PWSTR;
                    return true;
                }
                return false;
            }
            
            case eTestFunc: {
                const WCHAR_T* res = u"OK";
                size_t len = 0;
                while (res[len] != u'\0') len++;
                
                if (m_iMemory && m_iMemory->AllocMemory((void**)&pvarRetValue->pwstrVal, (len + 1) * sizeof(WCHAR_T))) {
                    for (size_t i = 0; i <= len; i++) {
                        pvarRetValue->pwstrVal[i] = res[i];
                    }
                    pvarRetValue->wstrLen = len;
                    pvarRetValue->vt = VTYPE_PWSTR;
                    return true;
                }
                return false;
            }
            
            case ePlay: {
                if (!m_mpv || lSizeArray < 1) {
                    LogMsg("[Play] ERROR: mpv not initialized or no params");
                    pvarRetValue->vt = VTYPE_BOOL;
                    pvarRetValue->bVal = false;
                    return true;
                }
                
                const WCHAR_T* wstr = paParams[0].pwstrVal;
                size_t wlen = paParams[0].wstrLen;
                std::string url = WcharToUtf8(wstr, wlen);
                LogMsg("[Play] URL: " + url);
                
                if (url.empty()) {
                    pvarRetValue->vt = VTYPE_BOOL;
                    pvarRetValue->bVal = false;
                    return true;
                }
                
                const char* cmd[] = {"loadfile", url.c_str(), NULL};
                int err = mpv_command(m_mpv, cmd);
                LogMsg("[Play] mpv_command(loadfile) returned: " + std::to_string(err));
                
                pvarRetValue->vt = VTYPE_BOOL;
                pvarRetValue->bVal = (err >= 0);
                return true;
            }
        }
        return false;
    }

    void ADDIN_API SetLocale(const WCHAR_T* loc) override {}
    void ADDIN_API SetUserInterfaceLanguageCode(const WCHAR_T* lang) override {}
};

// ============================================================================
// Экспортные функции
// ============================================================================

static WCHAR_T g_classNames[] = { u'M', u'p', u'v', u'1', u'c', u'\0' };

extern "C" const WCHAR_T* ADDIN_API GetClassNames() { return g_classNames; }

extern "C" long ADDIN_API GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface) {
    if (!pInterface) return 0;
    *pInterface = new CMpv1c();
    return 1;
}

extern "C" long ADDIN_API DestroyObject(IComponentBase** pInterface) {
    if (!pInterface || !*pInterface) return 0;
    delete *pInterface;
    *pInterface = nullptr;
    return 0;
}

extern "C" AppCapabilities ADDIN_API SetPlatformCapabilities(const AppCapabilities capabilities) {
    return eAppCapabilities1;
}

extern "C" AttachType ADDIN_API GetAttachType() {
    return eCanAttachNotIsolated;
}
