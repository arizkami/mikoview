#pragma once

#include "include/cef_v8.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_process_message.h"
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <memory>

namespace MikoView {
namespace JSAPI {

// Forward declarations
class InvokeHandler;
class InvokeRequest;
class InvokeResponse;

// Callback types
using InvokeCallback = std::function<void(const std::string& result, bool success)>;
using NativeHandler = std::function<void(const InvokeRequest& request, InvokeResponse& response)>;

// Request/Response structures
class InvokeRequest {
public:
    InvokeRequest(const std::string& method, const std::string& data, int requestId);
    
    const std::string& GetMethod() const { return method_; }
    const std::string& GetData() const { return data_; }
    int GetRequestId() const { return requestId_; }
    
    // Parse JSON data
    template<typename T>
    bool GetParam(const std::string& key, T& value) const;
    
private:
    std::string method_;
    std::string data_;
    int requestId_;
};

class InvokeResponse {
public:
    InvokeResponse(int requestId);
    
    void SetSuccess(const std::string& data);
    void SetError(const std::string& error, int code = -1);
    
    bool IsSuccess() const { return success_; }
    const std::string& GetData() const { return data_; }
    const std::string& GetError() const { return error_; }
    int GetErrorCode() const { return errorCode_; }
    int GetRequestId() const { return requestId_; }
    
    std::string ToJSON() const;
    
private:
    int requestId_;
    bool success_;
    std::string data_;
    std::string error_;
    int errorCode_;
};

// Main invoke handler
class InvokeHandler {
public:
    static InvokeHandler* GetInstance();
    
    // Register native handlers
    void RegisterHandler(const std::string& method, NativeHandler handler);
    void UnregisterHandler(const std::string& method);
    
    // Handle invoke from renderer
    void HandleInvoke(CefRefPtr<CefBrowser> browser, 
                     CefRefPtr<CefFrame> frame,
                     const std::string& method,
                     const std::string& data,
                     int requestId);
    
    // Send response back to renderer
    void SendResponse(CefRefPtr<CefBrowser> browser,
                     const InvokeResponse& response);
    
    // Invoke from native to renderer
    void InvokeRenderer(CefRefPtr<CefBrowser> browser,
                       const std::string& method,
                       const std::string& data,
                       InvokeCallback callback = nullptr);
    
private:
    InvokeHandler() = default;
    static std::unique_ptr<InvokeHandler> instance_;
    
    std::map<std::string, NativeHandler> handlers_;
    std::map<int, InvokeCallback> pendingCallbacks_;
    int nextRequestId_;
    
    int GenerateRequestId();
};

// V8 Handler for JavaScript side
class V8InvokeHandler : public CefV8Handler {
public:
    V8InvokeHandler();
    
    bool Execute(const CefString& name,
                CefRefPtr<CefV8Value> object,
                const CefV8ValueList& arguments,
                CefRefPtr<CefV8Value>& retval,
                CefString& exception) override;
    
private:
    IMPLEMENT_REFCOUNTING(V8InvokeHandler);
};

// Utility functions
namespace Utils {
    std::string V8ValueToJSON(CefRefPtr<CefV8Value> value);
    CefRefPtr<CefV8Value> JSONToV8Value(const std::string& json);
    std::string EscapeJSON(const std::string& str);
    std::string UnescapeJSON(const std::string& str);
}

} // namespace JSAPI
} // namespace MikoView