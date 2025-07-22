#pragma once
#include "include/cef_client.h"
#include "include/cef_display_handler.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/cef_task.h"
#include <list>

class SimpleClient;

// Task class for CefPostTask compatibility
class CloseBrowserTask : public CefTask {
public:
    CloseBrowserTask(CefRefPtr<SimpleClient> client, bool force_close);
    void Execute() override;

private:
    CefRefPtr<SimpleClient> client_;
    bool force_close_;
    IMPLEMENT_REFCOUNTING(CloseBrowserTask);
};

// Simple CEF client implementation
class SimpleClient : public CefClient,
                    public CefDisplayHandler,
                    public CefLifeSpanHandler,
                    public CefLoadHandler {
public:
    SimpleClient();

    // CefClient methods
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override;
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override;

    // CefDisplayHandler methods
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                              const CefString& title) override;

    // CefLifeSpanHandler methods
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler methods
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            ErrorCode errorCode,
                            const CefString& errorText,
                            const CefString& failedUrl) override;
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            TransitionType transition_type) override;

    // Browser management
    void CloseAllBrowsers(bool force_close);
    void DoCloseAllBrowsers(bool force_close);
    CefRefPtr<CefBrowser> GetFirstBrowser();
    bool HasBrowsers();

private:
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browser_list_;

    IMPLEMENT_REFCOUNTING(SimpleClient);
};