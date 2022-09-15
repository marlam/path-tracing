cmake_minimum_required(VERSION 3.12)

project(pathtracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenMP)

if(UNIX)
    # works with gcc and clang:
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fopenmp")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -Ofast -DNDEBUG")
endif()

add_executable(pathtracer
	animation.hpp
        animation_constant.hpp
	camera.hpp
	fresnel.hpp
	imgsave.hpp
	material.hpp
	material_light.hpp
	material_lambertian.hpp
	material_mirror.hpp
	material_glass.hpp
	material_phong.hpp
	material_twosided.hpp
	math.hpp
	prng.hpp
	ray.hpp
	sampler.hpp
	surface.hpp
	surface_sphere.hpp
	texture.hpp
	texture_constant.hpp
	texture_image.hpp
	transformation.hpp
	pathtracer.cpp
	stb_image.h)
target_link_libraries(pathtracer OpenMP::OpenMP_CXX)
install(TARGETS pathtracer RUNTIME DESTINATION bin)
