cmake_minimum_required(VERSION 3.1)

set(project_name async_curl)

project(${project_name}
	VERSION 1.0
	DESCRIPTION "Async tcp server forwarding to curl"
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#add_compile_options(-Wall)
add_compile_options( -O2 -Wall)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(
	Boost 1.69 REQUIRED
	#COMPONENTS program_options
	COMPONENTS filesystem
)
include_directories(${Boost_INCLUDE_DIR})

aux_source_directory(. sources)
add_executable(${project_name} ${sources})
target_link_libraries(${project_name} PRIVATE Threads::Threads ${Boost_LIBRARIES})

install(TARGETS ${project_name} DESTINATION ${CMAKE_HOME_DIRECTORY})
