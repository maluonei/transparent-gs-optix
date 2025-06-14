#include "system/system.h"
#include "pt_pass.h"
#include "static.h"

int main() {
    auto system = Pupil::util::Singleton<Pupil::System>::instance();
    system->Init(true);

    {
        auto pt_pass = std::make_unique<Pupil::pt::PTPass>("Path Tracing");
        system->AddPass(pt_pass.get());
        std::filesystem::path scene_file_path{ Pupil::DATA_DIR };
        //scene_file_path /= "static/default_ply2.xml";
        std::filesystem::path default_scene_file_path = scene_file_path / "static/default_ply2.xml";
        system->SetScene(default_scene_file_path);

        //system->Run();
        system->Run_Once(scene_file_path, "test");
    }

    system->Destroy();

    return 0;
}