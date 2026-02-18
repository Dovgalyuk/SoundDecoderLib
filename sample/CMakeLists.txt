add_executable(sample sample.c script.c)

target_include_directories(sample PUBLIC ../include)
target_link_libraries(sample SoundDecoderVMLib)
