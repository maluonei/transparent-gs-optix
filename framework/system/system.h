#pragma once

#include "util/util.h"
#include "util/timer.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Pupil {
class Pass;
class GuiPass;

enum class ESystemEvent {
    Quit,
    Precompute,
    StartRendering,
    StopRendering,
    SceneLoad,
    FrameFinished
};

class System : public util::Singleton<System> {
    friend class GuiPass;

public:
    bool render_flag = true;
    bool quit_flag = false;

    void Init(bool has_window = true) noexcept;
    void Run() noexcept;
    void Run_Once(const std::filesystem::path& path, const std::string& filename) noexcept;
    void Run_Xmls(
        const std::filesystem::path path, 
        const std::filesystem::path xml_folder_path, 
        std::vector<std::string> pathes) noexcept;
    void Run_Xmls2(
        const std::filesystem::path path) noexcept;
    void Destroy() noexcept;

    void AddPass(Pass *) noexcept;
    void SetScene(std::filesystem::path) noexcept;
    void ReSetCamera(std::filesystem::path) noexcept;

private:
    std::vector<Pass *> m_passes;
    std::vector<Pass *> m_pre_passes;
    GuiPass *m_gui_pass = nullptr;
    Timer m_render_timer;
};
}// namespace Pupil