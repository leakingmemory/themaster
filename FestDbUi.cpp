//
// Created by sigsegv on 3/11/24.
//

#include "FestDbUi.h"
#include <cpprest/http_client.h>
#include "DownloadFestDialog.h"
#include "DataDirectory.h"
#include <filesystem>
#include <medfest/FestZip.h>
#include <medfest/FestDeserializer.h>
#include "FestObjectStreamUi.h"
#include "FestSerializerUi.h"

class FestUpdateException : public std::exception {
private:
    std::string msg;
public:
    FestUpdateException(const std::string &msg) : msg(msg) {}
    const char * what() const noexcept override;
};

const char *FestUpdateException::what() const noexcept {
    return msg.c_str();
}

void FestDbUi::UpdateFromFile(DownloadFestDialog &dialog, const std::string &filename) {
    auto dbfile = DataDirectory::Data("themaster").Sub("FEST").GetPath("fest.db");
    auto dbBackupFile = DataDirectory::Data("themaster").Sub("FEST").GetPath("fest.db.bak");
    auto tmpfile = dbfile;
    {
        std::stringstream sstr{};
        sstr << "." << getpid();
        auto app = sstr.str();
        tmpfile.append(app);
    }
    std::shared_ptr<Fest> fest{};
    {
        FestZip festZip{filename};
        auto file = festZip.GetXmlFile();
        if (!file) {
            throw FestUpdateException("Unpack xml from zip failed");
        }
        FestObjectStreamUi festObjectStream{dialog, file};
        fest = festObjectStream.read();
    }
    FestSerializerUi festSerializer{dialog, tmpfile};
    if (std::filesystem::exists(dbfile)) {
        FestDeserializer festDeserializer{dbfile};
        festDeserializer.Preload(festSerializer);
        std::filesystem::copy(dbfile, dbBackupFile);
    }
    festSerializer.Serialize(*fest);
    festSerializer.Write();
    if (rename(tmpfile.c_str(), dbfile.c_str()) != 0) {
        throw FestUpdateException("Replace/insert db-file failed");
    }
}

void FestDbUi::Update() {
    web::http::client::http_client httpClient{"https://www.legemiddelsok.no"};
    web::http::http_request request{web::http::methods::GET};
    request.set_request_uri("/_layouts/15/FESTmelding/fest251.zip");
    {
        auto dbfile = DataDirectory::Data("themaster").Sub("FEST").GetPath("fest.db");
        if (std::filesystem::exists(dbfile)) {
            std::string lastMod = DataDirectory::Data("themaster").Sub("FEST").ReadFile("lastmod");
            if (!lastMod.empty()) {
                request.headers().add("If-Modified-Since", lastMod);
            }
        }
    }
    std::shared_ptr<DownloadFestDialog> downloadFestDialog = std::make_shared<DownloadFestDialog>(parent);
    auto self = shared_from_this();
    httpClient.request(request).then([self, downloadFestDialog] (const pplx::task<web::http::http_response> &responseTask) {
        try {
            auto response = responseTask.get();
            auto statusCode = response.status_code();
            if (statusCode != web::http::status_codes::NotModified) {
                std::string lastModified{};
                uint64_t contentLength{0};
                {
                    auto &headers = response.headers();
                    auto findLastModified = headers.find("Last-Modified");
                    if (findLastModified != headers.end()) {
                        lastModified = findLastModified->second;
                    }
                    contentLength = headers.content_length();
                }
                auto body = response.body();
                std::string tmpdownload{};
                {
                    std::stringstream sstr{};
                    sstr << "fest.zip." << getpid();
                    tmpdownload = sstr.str();
                }
                tmpdownload = DataDirectory::Data("themaster").Sub("FEST").GetPath(tmpdownload);
                std::ofstream outfile(tmpdownload, std::ofstream::binary);
                typeof(contentLength) offset = 0;
                std::string buf{};
                while (true) {
                    auto avail = body.streambuf().in_avail();
                    if (offset >= contentLength && avail <= 0) {
                        break;
                    }
                    if (avail <= 0) {
                        continue;
                    }
                    if (avail > 16384) {
                        avail = 16384;
                    }
                    buf.resize(avail);
                    body.streambuf().getn((uint8_t *) buf.data(), avail).get();
                    outfile.write(buf.data(), avail);
                    offset += avail;
                    wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter(
                            [offset, contentLength, &downloadFestDialog]() {
                                auto prcnt = (offset * 100) / contentLength;
                                downloadFestDialog->SetDownloadProgress(prcnt);
                            });
                }
                outfile.close();
                body.close();
                std::string festfile = DataDirectory::Data("themaster").Sub("FEST").GetPath("fest.zip");
                if (rename(tmpdownload.c_str(), festfile.c_str()) != 0) {
                    throw FestUpdateException("Fest zip insert new downloaded failed");
                }
                self->UpdateFromFile(*downloadFestDialog, festfile);
                if (!lastModified.empty()) {
                    DataDirectory::Data("themaster").Sub("FEST").WriteFile("lastmod", lastModified);
                }
            } else {
                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([]() {
                    wxMessageBox(wxT("FEST is up to date, and no need to download"), wxT("FEST is up to date"),
                                 wxICON_INFORMATION);
                });
            }
        } catch (const std::exception &e) {
            std::string msg{e.what()};
            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([msg]() {
                wxString err{msg};
                wxMessageBox(err, wxT("Download failed"), wxICON_ERROR);
            });
        } catch (...) {
            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([]() {
                wxMessageBox(wxT("Downloading fest failed"), wxT("Download failed"), wxICON_ERROR);
            });
        }
        wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([&downloadFestDialog]() {
            downloadFestDialog->EndModal(0);
        });
    });
    downloadFestDialog->ShowModal();
}
