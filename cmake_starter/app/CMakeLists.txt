# Build the app, using the HAL

include_directories(include)
file(GLOB MY_SOURCES "src/*.c")
add_executable(main ${MY_SOURCES})

# Make use of the HAL library
target_link_libraries(main LINK_PRIVATE hal)

#Get hal includes too
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/hal/include)

# Copy executable to final location (change `main` to project name as needed)
add_custom_command(TARGET main POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy
     "$<TARGET_FILE:main>"
     "~/ensc351/public/myApps/reaction_timer"
  COMMENT "Copying ARM executable to public NFS directory")
