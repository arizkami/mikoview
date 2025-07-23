#include "mikoapp.hpp"
#include "include/cef_scheme.h"
#include "include/wrapper/cef_helpers.h"
#include <fstream>
#include <filesystem>
#include <cstdint>

SimpleApp::SimpleApp() {
}

void SimpleApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) {
    // Register the custom scheme with proper flags
    registrar->AddCustomScheme("app", 
        CEF_SCHEME_OPTION_STANDARD | 
        CEF_SCHEME_OPTION_LOCAL | 
        CEF_SCHEME_OPTION_CORS_ENABLED |
        CEF_SCHEME_OPTION_SECURE);
}

class AppSchemeHandler : public CefResourceHandler {
public:
    AppSchemeHandler() : offset_(0) {}
    
    virtual bool Open(CefRefPtr<CefRequest> request,
                     bool& handle_request,
                     CefRefPtr<CefCallback> callback) override {
        handle_request = true;
        
        std::string url = request->GetURL();
        // Remove "app://" prefix
        std::string path = url.substr(6);
        
        // Remove trailing slash if present
        if (!path.empty() && path.back() == '/') {
            path.pop_back();
        }
        
        // Get current directory and construct full path
        std::filesystem::path currentPath = std::filesystem::current_path();
        std::filesystem::path filePath = currentPath / "assets" / path;
        
        if (std::filesystem::exists(filePath)) {
            std::ifstream file(filePath, std::ios::binary);
            if (file) {
                data_ = std::string((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
                
                // Set MIME type based on file extension
                std::string ext = filePath.extension().string();
                if (ext == ".html") mime_type_ = "text/html";
                else if (ext == ".css") mime_type_ = "text/css";
                else if (ext == ".js") mime_type_ = "application/javascript";
                else if (ext == ".svg") mime_type_ = "image/svg+xml";
                else mime_type_ = "application/octet-stream";
                
                return true;
            }
        }
        
        return false;
    }
    
    virtual void GetResponseHeaders(CefRefPtr<CefResponse> response,
                                  int64_t& response_length,
                                  CefString& redirectUrl) override {
        response->SetMimeType(mime_type_);
        response->SetStatus(200);
        response->SetStatusText("OK");
        response_length = static_cast<int64_t>(data_.length());
    }
    
    virtual bool Read(void* data_out,
                     int bytes_to_read,
                     int& bytes_read,
                     CefRefPtr<CefResourceReadCallback> callback) override {
        bytes_read = 0;
        
        if (offset_ < data_.length()) {
            int transfer_size = (std::min)(bytes_to_read, static_cast<int>(data_.length() - offset_));
            memcpy(data_out, data_.c_str() + offset_, transfer_size);
            offset_ += transfer_size;
            bytes_read = transfer_size;
            return true;
        }
        
        return false;
    }
    
    virtual void Cancel() override {
        // Nothing to cancel
    }
    
private:
    std::string data_;
    std::string mime_type_;
    size_t offset_;
    
    IMPLEMENT_REFCOUNTING(AppSchemeHandler);
};

class AppSchemeHandlerFactory : public CefSchemeHandlerFactory {
public:
    virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                               CefRefPtr<CefFrame> frame,
                                               const CefString& scheme_name,
                                               CefRefPtr<CefRequest> request) override {
        return new AppSchemeHandler();
    }
    
    IMPLEMENT_REFCOUNTING(AppSchemeHandlerFactory);
};

void SimpleApp::OnContextInitialized() {
    CEF_REQUIRE_UI_THREAD();
    CefRegisterSchemeHandlerFactory("app", "", new AppSchemeHandlerFactory());
}

void SimpleApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) {
    // Add command-line switches to enable file access and disable web security
    command_line->AppendSwitch("--allow-file-access-from-files");
    command_line->AppendSwitch("--disable-web-security");
    command_line->AppendSwitch("--allow-running-insecure-content");
    command_line->AppendSwitch("--disable-features=VizDisplayCompositor");
}