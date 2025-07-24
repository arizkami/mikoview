#include "invoke.hpp"
#include "../logger.hpp"
#include "cef_task.h"
#include "wrapper/cef_helpers.h"
#include <json/json.h>
#include <sstream>

namespace MikoView {
namespace JSAPI {

// Static instance
std::unique_ptr<InvokeHandler> InvokeHandler::instance_ = nullptr;

// InvokeRequest implementation
InvokeRequest::InvokeRequest(const std::string& method, const std::string& data, int requestId)
    : method_(method), data_(data), requestId_(requestId) {
}

template<typename T>
bool InvokeRequest::GetParam(const std::string& key, T& value) const {
    try {
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(data_, root)) {
            return false;
        }
        
        if (!root.isMember(key)) {
            return false;
        }
        
        // Type-specific extraction
        if constexpr (std::is_same_v<T, std::string>) {
            if (root[key].isString()) {
                value = root[key].asString();
                return true;
            }
        } else if constexpr (std::is_same_v<T, int>) {
            if (root[key].isInt()) {
                value = root[key].asInt();
                return true;
            }
        } else if constexpr (std::is_same_v<T, bool>) {
            if (root[key].isBool()) {
                value = root[key].asBool();
                return true;
            }
        } else if constexpr (std::is_same_v<T, double>) {
            if (root[key].isDouble()) {
                value = root[key].asDouble();
                return true;
            }
        }
        
        return false;
    } catch (...) {
        return false;
    }
}

// InvokeResponse implementation
InvokeResponse::InvokeResponse(int requestId)
    : requestId_(requestId), success_(false), errorCode_(0) {
}

void InvokeResponse::SetSuccess(const std::string& data) {
    success_ = true;
    data_ = data;
    error_.clear();
    errorCode_ = 0;
}

void InvokeResponse::SetError(const std::string& error, int code) {
    success_ = false;
    error_ = error;
    errorCode_ = code;
    data_.clear();
}

std::string InvokeResponse::ToJSON() const {
    Json::Value root;
    root["requestId"] = requestId_;
    root["success"] = success_;
    
    if (success_) {
        // Try to parse data as JSON, if it fails, treat as string
        Json::Value dataJson;
        Json::Reader reader;
        if (reader.parse(data_, dataJson)) {
            root["data"] = dataJson;
        } else {
            root["data"] = data_;
        }
    } else {
        root["error"] = error_;
        root["errorCode"] = errorCode_;
    }
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

// InvokeHandler implementation
InvokeHandler* InvokeHandler::GetInstance() {
    if (!instance_) {
        instance_ = std::make_unique<InvokeHandler>();
        instance_->nextRequestId_ = 1;
    }
    return instance_.get();
}

void InvokeHandler::RegisterHandler(const std::string& method, NativeHandler handler) {
    handlers_[method] = handler;
    Logger::Info("Registered invoke handler: " + method);
}

void InvokeHandler::UnregisterHandler(const std::string& method) {
    handlers_.erase(method);
    Logger::Info("Unregistered invoke handler: " + method);
}

void InvokeHandler::HandleInvoke(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                const std::string& method,
                                const std::string& data,
                                int requestId) {
    auto it = handlers_.find(method);
    if (it == handlers_.end()) {
        InvokeResponse response(requestId);
        response.SetError("Method not found: " + method, 404);
        SendResponse(browser, response);
        return;
    }
    
    try {
        InvokeRequest request(method, data, requestId);
        InvokeResponse response(requestId);
        
        // Execute handler
        it->second(request, response);
        
        // Send response
        SendResponse(browser, response);
    } catch (const std::exception& e) {
        InvokeResponse response(requestId);
        response.SetError("Handler exception: " + std::string(e.what()), 500);
        SendResponse(browser, response);
    }
}

void InvokeHandler::SendResponse(CefRefPtr<CefBrowser> browser,
                                const InvokeResponse& response) {
    if (!browser || !browser->GetMainFrame()) {
        return;
    }
    
    std::string script = "if (window.mikoview && window.mikoview._handleInvokeResponse) { "
                        "window.mikoview._handleInvokeResponse(" + response.ToJSON() + "); }";
    
    browser->GetMainFrame()->ExecuteJavaScript(script, "", 0);
}

void InvokeHandler::InvokeRenderer(CefRefPtr<CefBrowser> browser,
                                  const std::string& method,
                                  const std::string& data,
                                  InvokeCallback callback) {
    if (!browser || !browser->GetMainFrame()) {
        if (callback) {
            callback("", false);
        }
        return;
    }
    
    int requestId = GenerateRequestId();
    if (callback) {
        pendingCallbacks_[requestId] = callback;
    }
    
    Json::Value request;
    request["method"] = method;
    request["data"] = data;
    request["requestId"] = requestId;
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string requestJson = Json::writeString(builder, request);
    
    std::string script = "if (window.mikoview && window.mikoview._handleNativeInvoke) { "
                        "window.mikoview._handleNativeInvoke(" + requestJson + "); }";
    
    browser->GetMainFrame()->ExecuteJavaScript(script, "", 0);
}

int InvokeHandler::GenerateRequestId() {
    return nextRequestId_++;
}

// V8InvokeHandler implementation
V8InvokeHandler::V8InvokeHandler() {
}

bool V8InvokeHandler::Execute(const CefString& name,
                             CefRefPtr<CefV8Value> object,
                             const CefV8ValueList& arguments,
                             CefRefPtr<CefV8Value>& retval,
                             CefString& exception) {
    if (name == "invoke") {
        if (arguments.size() < 2) {
            exception = "invoke requires at least 2 arguments: method and data";
            return true;
        }
        
        if (!arguments[0]->IsString()) {
            exception = "First argument (method) must be a string";
            return true;
        }
        
        std::string method = arguments[0]->GetStringValue();
        std::string data = Utils::V8ValueToJSON(arguments[1]);
        int requestId = 0;
        
        if (arguments.size() > 2 && arguments[2]->IsInt()) {
            requestId = arguments[2]->GetIntValue();
        }
        
        // Send message to browser process
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("invoke");
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        args->SetString(0, method);
        args->SetString(1, data);
        args->SetInt(2, requestId);
        
        CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
        if (context && context->GetFrame()) {
            context->GetFrame()->SendProcessMessage(PID_BROWSER, message);
        }
        
        retval = CefV8Value::CreateBool(true);
        return true;
    }
    
    return false;
}

// Utility functions
namespace Utils {

std::string V8ValueToJSON(CefRefPtr<CefV8Value> value) {
    if (!value) {
        return "null";
    }
    
    if (value->IsNull() || value->IsUndefined()) {
        return "null";
    }
    
    if (value->IsBool()) {
        return value->GetBoolValue() ? "true" : "false";
    }
    
    if (value->IsInt()) {
        return std::to_string(value->GetIntValue());
    }
    
    if (value->IsDouble()) {
        return std::to_string(value->GetDoubleValue());
    }
    
    if (value->IsString()) {
        return "\"" + EscapeJSON(value->GetStringValue()) + "\"";
    }
    
    if (value->IsArray()) {
        std::ostringstream oss;
        oss << "[";
        int length = value->GetArrayLength();
        for (int i = 0; i < length; i++) {
            if (i > 0) oss << ",";
            oss << V8ValueToJSON(value->GetValue(i));
        }
        oss << "]";
        return oss.str();
    }
    
    if (value->IsObject()) {
        std::ostringstream oss;
        oss << "{";
        std::vector<CefString> keys;
        value->GetKeys(keys);
        
        bool first = true;
        for (const auto& key : keys) {
            if (!first) oss << ",";
            oss << "\"" << EscapeJSON(key) << "\":" << V8ValueToJSON(value->GetValue(key));
            first = false;
        }
        oss << "}";
        return oss.str();
    }
    
    return "null";
}

CefRefPtr<CefV8Value> JSONToV8Value(const std::string& json) {
    try {
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) {
            return CefV8Value::CreateNull();
        }
        
        return JsonValueToV8Value(root);
    } catch (...) {
        return CefV8Value::CreateNull();
    }
}

std::string EscapeJSON(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (c < 0x20) {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    oss << c;
                }
                break;
        }
    }
    return oss.str();
}

std::string UnescapeJSON(const std::string& str) {
    // Implementation for unescaping JSON strings
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case '"': result += '"'; i++; break;
                case '\\': result += '\\'; i++; break;
                case 'b': result += '\b'; i++; break;
                case 'f': result += '\f'; i++; break;
                case 'n': result += '\n'; i++; break;
                case 'r': result += '\r'; i++; break;
                case 't': result += '\t'; i++; break;
                default: result += str[i]; break;
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

} // namespace Utils

} // namespace JSAPI
} // namespace MikoView