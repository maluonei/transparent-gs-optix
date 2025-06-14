set(pupil_application_example example)

if(${pupil_build_example})
    add_subdirectory(empty_gui)
    add_subdirectory(path_tracer)
    add_subdirectory(cuda_test)
    add_subdirectory(path_tracer_offline)
    add_subdirectory(path_tracer_glass_gbuffer)
endif()
