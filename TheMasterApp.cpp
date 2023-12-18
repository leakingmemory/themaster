//
// Created by sigsegv on 12/13/23.
//

#include "TheMasterApp.h"
#include "TheMasterFrame.h"

bool TheMasterApp::OnInit() {
    auto *whatsFrame = new TheMasterFrame();
    whatsFrame->Show();
    return true;
}

wxIMPLEMENT_APP(TheMasterApp);