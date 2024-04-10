//
// Created by sigsegv on 4/9/24.
//

#include "WxDebouncer.h"
#include <wx/wx.h>

void WxDebouncer::Worker() {
    while (true) {
        sema.acquire();
        {
            std::lock_guard lock{mtx};
            if (stop) {
                return;
            }
            if (!scheduled) {
                continue;
            }
            scheduled = false;
        }
        func();
    }
}

void WxDebouncer::Apply() {
    std::function<void ()> apply = this->apply;
    this->apply = [] () {};
    wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([apply]() {
        apply();
    });
}

void WxDebouncer::Schedule() {
    {
        std::lock_guard lock{mtx};
        scheduled = true;
    }
    sema.release();
}

WxDebouncer::~WxDebouncer() {
    {
        std::lock_guard lock{mtx};
        stop = true;
    }
    sema.release();
    thread.join();
}
