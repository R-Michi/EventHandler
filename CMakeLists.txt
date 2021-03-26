# requiered CMAKE version to build the project
cmake_minimum_required (VERSION 3.8)

# current project
project ("EventHandler")

# set comiler flags
set(CMAKE_CXX_STANDARD 17)

# add external include directories
# no search directories needed

# link event handler library
add_library(libevent_handler STATIC 
			"${CMAKE_CURRENT_SOURCE_DIR}/src/event_handler_src.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/listener_src.cpp") 

# compile and link final executable
add_executable(event_handler_test 
			   "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

# link external libraries to the executable
target_link_libraries(event_handler_test
					  "libevent_handler")

# additional work
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
