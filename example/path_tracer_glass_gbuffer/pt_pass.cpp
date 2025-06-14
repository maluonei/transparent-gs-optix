#include "pt_pass.h"
#include "imgui.h"

#include "cuda/context.h"
#include "optix/context.h"
#include "optix/module.h"

#include "util/event.h"
#include "system/system.h"
#include "system/gui/gui.h"
#include "world/world.h"
#include "world/render_object.h"

extern "C" char embedded_ptx_code[];

namespace Pupil {
extern uint32_t g_window_w;
extern uint32_t g_window_h;
}// namespace Pupil

#include <tinyexr.h>
#include <vector>
#include <iostream>
namespace Pupil::pt {
int saveToEXR(cuda::RWArrayView<float4> view, int width, int height, const std::string &filename) {
    // ����һ���洢ͼ�����ݵ� vector
    //std::vector<float> rgbaData(width * height * 4);

    // �� CUDA �ڴ��е����ݸ��Ƶ� rgbaData
    float4 *cudaDeviceData = view.GetDataPtr();// ���� view.data() �����豸�ڴ�ָ��
    float4 *deviceData = new float4[view.GetNum()];
    cudaMemcpy(deviceData, cudaDeviceData, view.GetNum() * sizeof(float4), cudaMemcpyDeviceToHost);

    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = 4;
    std::vector<float> images[4];
    images[0].resize(width * height);
    images[1].resize(width * height);
    images[2].resize(width * height);
    images[3].resize(width * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            int flip_y_index = (height - 1 - y) * width + x;
            //int write_index = x * height + (height - 1 - y);

            float4 pixel = deviceData[index];

            // �� float4 ת��Ϊ TinyEXR ��ʽ�� RGBA ��������
            images[0][flip_y_index] = pixel.x;// Red
            images[1][flip_y_index] = pixel.y;// Green
            images[2][flip_y_index] = pixel.z;// Blue
            images[3][flip_y_index] = pixel.w;// Alpha
        }
    }

    float *image_ptr[4];
    image_ptr[0] = &(images[3].at(0));// A
    image_ptr[1] = &(images[2].at(0));// B
    image_ptr[2] = &(images[1].at(0));// G
    image_ptr[3] = &(images[0].at(0));// R

    image.images = (unsigned char **)image_ptr;
    image.width = width;
    image.height = height;

    header.num_channels = 4;
    header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
    // Must be (A)BGR order, since most of EXR viewers expect this channel order.
    strncpy(header.channels[0].name, "A", 255);
    header.channels[0].name[strlen("A")] = '\0';
    strncpy(header.channels[1].name, "B", 255);
    header.channels[1].name[strlen("B")] = '\0';
    strncpy(header.channels[2].name, "G", 255);
    header.channels[2].name[strlen("G")] = '\0';
    strncpy(header.channels[3].name, "R", 255);
    header.channels[3].name[strlen("R")] = '\0';

    header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++) {
        header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;         // pixel type of input image
        header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF;// pixel type of output image to be stored in .EXR
    }

    const char *err = NULL;// or nullptr in C++11 or later.
    int ret = SaveEXRImageToFile(&image, &header, filename.c_str(), &err);
    if (ret != TINYEXR_SUCCESS) {
        fprintf(stderr, "Save EXR err: %s\n", err);
        FreeEXRErrorMessage(err);// free's buffer for an error message
        return ret;
    }
    printf("Saved exr file. [ %s ] \n", filename.c_str());

    //free(rgb);
    delete[] deviceData;
    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);
}

int saveToEXR(cuda::RWArrayView<float3> view, int width, int height, const std::string &filename) {
    // ����һ���洢ͼ�����ݵ� vector
    //std::vector<float> rgbaData(width * height * 4);

    // �� CUDA �ڴ��е����ݸ��Ƶ� rgbaData
    float3 *cudaDeviceData = view.GetDataPtr();// ���� view.data() �����豸�ڴ�ָ��
    float3 *deviceData = new float3[view.GetNum()];
    cudaMemcpy(deviceData, cudaDeviceData, view.GetNum() * sizeof(float3), cudaMemcpyDeviceToHost);

    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = 3;
    std::vector<float> images[3];
    images[0].resize(width * height);
    images[1].resize(width * height);
    images[2].resize(width * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            int flip_y_index = (height - 1 - y) * width + x;
            //float3 pixel = view[index];
            float3 pixel = deviceData[index];

            // �� float4 ת��Ϊ TinyEXR ��ʽ�� RGBA ��������
            images[0][flip_y_index] = pixel.x;// Red
            images[1][flip_y_index] = pixel.y;// Green
            images[2][flip_y_index] = pixel.z;// Blue
        }
    }

    float *image_ptr[4];
    image_ptr[0] = &(images[2].at(0));// A
    image_ptr[1] = &(images[1].at(0));// B
    image_ptr[2] = &(images[0].at(0));// G

    image.images = (unsigned char **)image_ptr;
    image.width = width;
    image.height = height;

    header.num_channels = 3;
    header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
    // Must be (A)BGR order, since most of EXR viewers expect this channel order.

    strncpy(header.channels[0].name, "B", 255);
    header.channels[0].name[strlen("B")] = '\0';
    strncpy(header.channels[1].name, "G", 255);
    header.channels[1].name[strlen("G")] = '\0';
    strncpy(header.channels[2].name, "R", 255);
    header.channels[2].name[strlen("R")] = '\0';

    header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++) {
        header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;         // pixel type of input image
        header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF;// pixel type of output image to be stored in .EXR
    }

    const char *err = NULL;// or nullptr in C++11 or later.
    int ret = SaveEXRImageToFile(&image, &header, filename.c_str(), &err);
    if (ret != TINYEXR_SUCCESS) {
        fprintf(stderr, "Save EXR err: %s\n", err);
        FreeEXRErrorMessage(err);// free's buffer for an error message
        return ret;
    }
    printf("Saved exr file. [ %s ] \n", filename.c_str());

    //free(rgb);
    delete[] deviceData;

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);
}

}// namespace Pupil::pt

namespace {
int m_max_depth;
bool m_accumulated_flag;

Pupil::world::World *m_world = nullptr;
}// namespace

namespace Pupil::pt {
PTPass::PTPass(std::string_view name) noexcept
    : Pass(name) {
    auto optix_ctx = util::Singleton<optix::Context>::instance();
    auto cuda_ctx = util::Singleton<cuda::Context>::instance();
    m_stream = std::make_unique<cuda::Stream>();
    m_optix_pass = std::make_unique<optix::Pass<SBTTypes, OptixLaunchParams>>(optix_ctx->context, m_stream->GetStream());
    InitOptixPipeline();
    BindingEventCallback();
}

void PTPass::SaveFrame(const std::filesystem::path &path, const std::string& filename) noexcept {
    std::filesystem::path reflect_path = path / (filename + "_reflect_direction.exr");
    std::filesystem::path refract_path = path / (filename + "_refract_direction.exr");
    std::filesystem::path reflect_position_path = path / (filename + "_reflect_position.exr");
    std::filesystem::path refract_position_path = path / (filename + "_refract_position.exr");
    std::filesystem::path normal_path = path / (filename + "_normal.exr");

    std::cout << "**********************\nfilename:" << filename << std::endl;

    saveToEXR(m_optix_launch_params.reflect_buffer, m_optix_launch_params.config.frame.width, m_optix_launch_params.config.frame.height, reflect_path.string());
    saveToEXR(m_optix_launch_params.refract_buffer, m_optix_launch_params.config.frame.width, m_optix_launch_params.config.frame.height, refract_path.string());
    saveToEXR(m_optix_launch_params.reflect_position_buffer, m_optix_launch_params.config.frame.width, m_optix_launch_params.config.frame.height, reflect_position_path.string());
    saveToEXR(m_optix_launch_params.refract_position_buffer, m_optix_launch_params.config.frame.width, m_optix_launch_params.config.frame.height, refract_position_path.string());
    saveToEXR(m_optix_launch_params.normal_buffer, m_optix_launch_params.config.frame.width, m_optix_launch_params.config.frame.height, normal_path.string());
}

void PTPass::OnRun() noexcept {
    if (m_dirty) {
        m_optix_launch_params.camera.SetData(m_world_camera->GetCudaMemory());
        m_optix_launch_params.config.max_depth = m_max_depth;
        m_optix_launch_params.config.accumulated_flag = m_accumulated_flag;
        m_optix_launch_params.sample_cnt = 0;
        m_optix_launch_params.random_seed = 0;
        m_optix_launch_params.handle = m_world->GetIASHandle(2, true);
        m_optix_launch_params.emitters = m_world->emitters->GetEmitterGroup();
        m_dirty = false;
    }

    m_optix_pass->Run(m_optix_launch_params, m_optix_launch_params.config.frame.width,
                      m_optix_launch_params.config.frame.height);
    m_optix_pass->Synchronize();

    m_optix_launch_params.sample_cnt += m_optix_launch_params.config.accumulated_flag;
    ++m_optix_launch_params.random_seed;
}

void PTPass::InitOptixPipeline() noexcept {
    auto module_mngr = util::Singleton<optix::ModuleManager>::instance();

    auto sphere_module = module_mngr->GetModule(optix::EModuleBuiltinType::SpherePrimitive);
    // auto curve_linear_module = module_mngr->GetModule(optix::EModuleBuiltinType::RoundLinearPrimitive);
    // auto curve_quadratic_module = module_mngr->GetModule(optix::EModuleBuiltinType::RoundQuadraticBsplinePrimitive);
    auto curve_cubic_module = module_mngr->GetModule(optix::EModuleBuiltinType::RoundCubicBsplinePrimitive);
    // auto curve_catmullrom_module = module_mngr->GetModule(optix::EModuleBuiltinType::RoundCatmullromPrimitive);
    auto pt_module = module_mngr->GetModule(embedded_ptx_code);

    optix::PipelineDesc pipeline_desc;
    {
        // for mesh(triangle) geo
        optix::RayTraceProgramDesc forward_ray_desc{
            .module_ptr = pt_module,
            .ray_gen_entry = "__raygen__main",
            .miss_entry = "__miss__default",
            .hit_group = { .ch_entry = "__closesthit__default" },
        };
        pipeline_desc.ray_trace_programs.push_back(forward_ray_desc);
        optix::RayTraceProgramDesc shadow_ray_desc{
            .module_ptr = pt_module,
            .miss_entry = "__miss__shadow",
            .hit_group = { .ch_entry = "__closesthit__shadow" },
        };
        pipeline_desc.ray_trace_programs.push_back(shadow_ray_desc);
    }

    {
        optix::RayTraceProgramDesc forward_ray_desc{
            .module_ptr = pt_module,
            .hit_group = { .ch_entry = "__closesthit__default_sphere",
                           .intersect_module = sphere_module },
        };
        pipeline_desc.ray_trace_programs.push_back(forward_ray_desc);
        optix::RayTraceProgramDesc shadow_ray_desc{
            .module_ptr = pt_module,
            .hit_group = { .ch_entry = "__closesthit__shadow_sphere",
                           .intersect_module = sphere_module },
        };
        pipeline_desc.ray_trace_programs.push_back(shadow_ray_desc);
    }

    auto curve_module = curve_cubic_module;
    {
        optix::RayTraceProgramDesc forward_ray_desc{
            .module_ptr = pt_module,
            .hit_group = { .ch_entry = "__closesthit__default_curve",
                           .intersect_module = curve_module },
        };
        pipeline_desc.ray_trace_programs.push_back(forward_ray_desc);
        optix::RayTraceProgramDesc shadow_ray_desc{
            .module_ptr = pt_module,
            .hit_group = { .ch_entry = "__closesthit__shadow_curve",
                           .intersect_module = curve_module },
        };
        pipeline_desc.ray_trace_programs.push_back(shadow_ray_desc);
    }

    {
        auto mat_programs = Pupil::resource::GetMaterialProgramDesc();
        pipeline_desc.callable_programs.insert(
            pipeline_desc.callable_programs.end(),
            mat_programs.begin(), mat_programs.end());
    }
    m_optix_pass->InitPipeline(pipeline_desc);
}

void PTPass::SetScene(world::World *world) noexcept {
    m_world_camera = world->camera.get();

    m_optix_launch_params.config.frame.width = world->scene->sensor.film.w;
    m_optix_launch_params.config.frame.height = world->scene->sensor.film.h;
    m_optix_launch_params.config.max_depth = world->scene->integrator.max_depth;
    m_optix_launch_params.config.accumulated_flag = true;

    m_max_depth = m_optix_launch_params.config.max_depth;
    m_accumulated_flag = m_optix_launch_params.config.accumulated_flag;

    m_optix_launch_params.random_seed = 0;
    m_optix_launch_params.sample_cnt = 0;

    m_output_pixel_num = m_optix_launch_params.config.frame.width *
                         m_optix_launch_params.config.frame.height;
    auto buf_mngr = util::Singleton<BufferManager>::instance();
    {
        m_optix_launch_params.frame_buffer.SetData(buf_mngr->GetBuffer(buf_mngr->DEFAULT_FINAL_RESULT_BUFFER_NAME)->cuda_ptr, m_output_pixel_num);

        BufferDesc desc{
            .name = "pt accum buffer",
            .flag = EBufferFlag::None,
            .width = static_cast<uint32_t>(world->scene->sensor.film.w),
            .height = static_cast<uint32_t>(world->scene->sensor.film.h),
            .stride_in_byte = sizeof(float) * 4
        };
        m_optix_launch_params.accum_buffer.SetData(buf_mngr->AllocBuffer(desc)->cuda_ptr, m_output_pixel_num);

        desc.name = "albedo";
        desc.flag = EBufferFlag::AllowDisplay;
        desc.stride_in_byte = sizeof(float3);
        m_optix_launch_params.albedo_buffer.SetData(buf_mngr->AllocBuffer(desc)->cuda_ptr, m_output_pixel_num);

        desc.name = "normal";
        m_optix_launch_params.normal_buffer.SetData(buf_mngr->AllocBuffer(desc)->cuda_ptr, m_output_pixel_num);

        desc.name = "refract";
        desc.stride_in_byte = sizeof(float4);
        m_optix_launch_params.refract_buffer.SetData(buf_mngr->AllocBuffer(desc)->cuda_ptr, m_output_pixel_num);

        desc.name = "reflect";
        m_optix_launch_params.reflect_buffer.SetData(buf_mngr->AllocBuffer(desc)->cuda_ptr, m_output_pixel_num);

        desc.name = "refract_position";
        desc.stride_in_byte = sizeof(float4);
        m_optix_launch_params.refract_position_buffer.SetData(buf_mngr->AllocBuffer(desc)->cuda_ptr, m_output_pixel_num);

        desc.name = "reflect_position";
        m_optix_launch_params.reflect_position_buffer.SetData(buf_mngr->AllocBuffer(desc)->cuda_ptr, m_output_pixel_num);


        desc.name = "test";
        desc.stride_in_byte = sizeof(float);
        m_optix_launch_params.test.SetData(buf_mngr->AllocBuffer(desc)->cuda_ptr, m_output_pixel_num);
    }

    m_world = world;
    m_optix_launch_params.handle = m_world->GetIASHandle(2, true);

    {
        optix::SBTDesc<SBTTypes> desc{};
        desc.ray_gen_data = {
            .program = "__raygen__main"
        };
        {
            int emitter_index_offset = 0;
            using HitGroupDataRecord = optix::ProgDataDescPair<SBTTypes::HitGroupDataType>;
            for (auto &&ro : world->GetRenderobjects()) {
                HitGroupDataRecord hit_default_data{};
                if (ro->geo.type == optix::Geometry::EType::TriMesh)
                    hit_default_data.program = "__closesthit__default";
                else if (ro->geo.type == optix::Geometry::EType::Sphere)
                    hit_default_data.program = "__closesthit__default_sphere";
                else
                    hit_default_data.program = "__closesthit__default_curve";
                hit_default_data.data.mat = ro->mat;
                hit_default_data.data.geo = ro->geo;
                if (ro->is_emitter) {
                    hit_default_data.data.emitter_index_offset = emitter_index_offset;
                    emitter_index_offset += ro->sub_emitters_num;
                }

                desc.hit_datas.push_back(hit_default_data);

                HitGroupDataRecord hit_shadow_data{};
                if (ro->geo.type == optix::Geometry::EType::TriMesh)
                    hit_shadow_data.program = "__closesthit__shadow";
                else if (ro->geo.type == optix::Geometry::EType::Sphere)
                    hit_shadow_data.program = "__closesthit__shadow_sphere";
                else
                    hit_shadow_data.program = "__closesthit__shadow_curve";

                hit_shadow_data.data.mat.type = ro->mat.type;
                desc.hit_datas.push_back(hit_shadow_data);
            }
        }
        {
            optix::ProgDataDescPair<SBTTypes::MissDataType> miss_data = {
                .program = "__miss__default"
            };
            desc.miss_datas.push_back(miss_data);
            optix::ProgDataDescPair<SBTTypes::MissDataType> miss_shadow_data = {
                .program = "__miss__shadow"
            };
            desc.miss_datas.push_back(miss_shadow_data);
        }
        {
            auto mat_programs = Pupil::resource::GetMaterialProgramDesc();
            for (auto &mat_prog : mat_programs) {
                if (mat_prog.cc_entry) {
                    optix::ProgDataDescPair<SBTTypes::CallablesDataType> cc_data = {
                        .program = mat_prog.cc_entry
                    };
                    desc.callables_datas.push_back(cc_data);
                }
                if (mat_prog.dc_entry) {
                    optix::ProgDataDescPair<SBTTypes::CallablesDataType> dc_data = {
                        .program = mat_prog.dc_entry
                    };
                    desc.callables_datas.push_back(dc_data);
                }
            }
        }
        m_optix_pass->InitSBT(desc);
    }

    m_dirty = true;
}

void PTPass::BindingEventCallback() noexcept {
    EventBinder<EWorldEvent::CameraChange>([this](void *) {
        m_dirty = true;
    });

    EventBinder<EWorldEvent::RenderInstanceUpdate>([this](void *) {
        m_dirty = true;
    });

    EventBinder<ESystemEvent::SceneLoad>([this](void *p) {
        SetScene((world::World *)p);
    });
}

void PTPass::Inspector() noexcept {
    Pass::Inspector();
    ImGui::Text("sample count: %d", m_optix_launch_params.sample_cnt + 1);
    ImGui::InputInt("max trace depth", &m_max_depth);
    m_max_depth = clamp(m_max_depth, 1, 128);
    if (m_optix_launch_params.config.max_depth != m_max_depth) {
        m_dirty = true;
    }

    if (ImGui::Checkbox("accumulate radiance", &m_accumulated_flag)) {
        m_dirty = true;
    }
}
}// namespace Pupil::pt