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
	aabb.hpp
	animation.hpp
        animation_constant.hpp
	bvh.hpp
	camera.hpp
	color.hpp
        envmap.hpp
	envmap_cube.hpp
	envmap_equirect.hpp
	fresnel.hpp
        imgsave.hpp
	import.hpp
	material.hpp
	material_light.hpp
	material_lambertian.hpp
	material_mirror.hpp
	material_glass.hpp
	material_phong.hpp
	material_twosided.hpp
	math.hpp
	mesh.hpp
	prng.hpp
	ray.hpp
	sampler.hpp
	scene.hpp
	surface.hpp
	surface_sphere.hpp
	surface_triangle.hpp
	tangentspace.hpp
	texture.hpp
        texture_checker.hpp
	texture_constant.hpp
	texture_image.hpp
        texture_transformer.hpp
	transformation.hpp
	pathtracer.cpp
	stb_image.h
	tiny_obj_loader.h)
target_link_libraries(pathtracer OpenMP::OpenMP_CXX)
install(TARGETS pathtracer RUNTIME DESTINATION bin)
