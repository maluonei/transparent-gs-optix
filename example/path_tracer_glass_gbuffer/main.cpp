#include "system/system.h"
#include "pt_pass.h"
#include "static.h"

namespace fs = std::filesystem;

int main() {
    std::filesystem::path root_folder_path = "G:/codes/PupilOptixLab/data/static/dolphin32";
    std::filesystem::path xml_folder_path = root_folder_path / "xmls";// �ļ���·��
    std::filesystem::path save_folder_path = root_folder_path / "gbuffers";// �ļ���·��
    
    try {
        // ����ļ����Ƿ����
        if (!fs::exists(save_folder_path)) {
            // ����ļ��в����ڣ�������
            if (fs::create_directory(save_folder_path)) {
                std::cout << "�ļ����Ѵ���: " << save_folder_path << std::endl;
            } else {
                std::cerr << "�޷������ļ���: " << save_folder_path << std::endl;
            }
        } else {
            std::cout << "�ļ����Ѵ���: " << save_folder_path << std::endl;
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << "����: " << e.what() << std::endl;
    }
    
    std::vector<std::string> entryNames;
    try {
        // ����Ŀ¼
        for (const auto &entry : fs::directory_iterator(xml_folder_path)) {
            //std::cout << "entry.path().filename():" << entry.path().filename() << std::endl;
            if (fs::is_regular_file(entry)) {                     // ����ǳ����ļ����ų�Ŀ¼�ȣ�
                std::cout << entry.path().filename() << std::endl;// ��ӡ�ļ���

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