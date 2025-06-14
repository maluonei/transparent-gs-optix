#include "system/system.h"
#include "pt_pass.h"
#include "static.h"

namespace fs = std::filesystem;

int main() {
    std::filesystem::path root_folder_path = "G:/codes/PupilOptixLab/data/static/dolphin32";
    std::filesystem::path xml_folder_path = root_folder_path / "xmls";// 文件夹路径
    std::filesystem::path save_folder_path = root_folder_path / "gbuffers";// 文件夹路径
    
    try {
        // 检查文件夹是否存在
        if (!fs::exists(save_folder_path)) {
            // 如果文件夹不存在，创建它
            if (fs::create_directory(save_folder_path)) {
                std::cout << "文件夹已创建: " << save_folder_path << std::endl;
            } else {
                std::cerr << "无法创建文件夹: " << save_folder_path << std::endl;
            }
        } else {
            std::cout << "文件夹已存在: " << save_folder_path << std::endl;
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << "错误: " << e.what() << std::endl;
    }
    
    std::vector<std::string> entryNames;
    try {
        // 遍历目录
        for (const auto &entry : fs::directory_iterator(xml_folder_path)) {
            //std::cout << "entry.path().filename():" << entry.path().filename() << std::endl;
            if (fs::is_regular_file(entry)) {                     // 如果是常规文件（排除目录等）
                std::cout << entry.path().filename() << std::endl;// 打印文件名

                std::filesystem::path entry_name = entry;
                entryNames.push_back(entry_name.stem().string());
            } else {
                //std::cout << "fs::is_regular_file(entry) is false";
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

    //auto system = Pupil::util::Singleton<Pupil::System>::instance();
    //system->Init(true);
    //
    //{
    //    auto pt_pass = std::make_unique<Pupil::pt::PTPass>("Path Tracing");
    //    system->AddPass(pt_pass.get());
    //    std::filesystem::path scene_file_path{ Pupil::DATA_DIR };
    //    std::filesystem::path default_scene_file_path = scene_file_path / "static/default_ply3.xml";
    //    system->SetScene(default_scene_file_path);
    //
    //    system->Run_Once(scene_file_path / "static/test");
    //}
    //
    //system->Destroy();
    //
    //return 0;
}