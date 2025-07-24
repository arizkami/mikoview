#pragma once
#include "cef_app.h"
#include "cef_scheme.h"

class SimpleApp : public CefApp, public CefBrowserProcessHandler {
public:
    SimpleApp();

    // CefApp methods
    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return this;
    }

    // CefBrowserProcessHandler methods
    virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;
    virtual void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override;
    virtual CefRefPtr<CefResourceBundleHandler> GetResourceBundleHandler() override { return nullptr; }
    virtual void OnContextInitialized() override;

private:
    IMPLEMENT_REFCOUNTING(SimpleApp);
};