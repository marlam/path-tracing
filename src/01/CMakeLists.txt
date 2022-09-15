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
	camera.hpp
	imgsave.hpp
	material.hpp
	material_lambertian.hpp
	material_light.hpp
	math.hpp
	prng.hpp
	ray.hpp
	sampler.hpp
	surface.hpp
	surface_sphere.hpp
	pathtracer.cpp)
target_link_libraries(pathtracer OpenMP::OpenMP_CXX)
install(TARGETS pathtracer RUNTIME DESTINATION bin)
