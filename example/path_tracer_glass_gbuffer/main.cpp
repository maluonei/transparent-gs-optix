#include "system/system.h"
#include "pt_pass.h"
#include "static.h"

namespace fs = std::filesystem;

int main() {
    const std::filesystem::path root_folder_path = "../../../data/static/dolphin32";  //write your dataset path
    const std::filesystem::path xml_folder_path = root_folder_path / "xmls";         
    const std::filesystem::path save_folder_path = root_folder_path / "gbuffers";    
    
    try {
        if (!fs::exists(save_folder_path)) {
            if (fs::create_directory(save_folder_path)) {
                std::cout << "folder: " << save_folder_path << std::endl;
            } else {
                std::cerr << "cannot create folder: " << save_folder_path << std::endl;
            }
        } else {
            std::cout << "folder already exists: " << save_folder_path << std::endl;
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << "error: " << e.what() << std::endl;
    }
    
    std::vector<std::string> entryNames;
    try {
        // ±éÀúÄ¿Â¼
        for (const auto &entry : fs::directory_iterator(xml_folder_path)) {
            if (fs::is_regular_file(entry)) {                     
                std::cout << entry.path().filename() << std::endl;

                std::filesystem::path entry_name = entry;
                entryNames.push_back(entry_name.stem().string());
            }
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    auto system = Pupil::util::Singleton<Pupil::System>::instance();
    system->Init(true);
    auto pt_pass = std::make_unique<Pupil::pt::PTPass>("Path Tracing");
    system->AddPass(pt_pass.get());
    system->SetScene(xml_folder_path / (entryNames[0] + ".xml"));
    system->Run_Xmls(save_folder_path, xml_folder_path, entryNames);
    system->Destroy();
}