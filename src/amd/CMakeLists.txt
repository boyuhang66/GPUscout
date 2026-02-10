set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(analysis_register_spilling analysis_register_spilling.cpp)
add_executable(analysis_restrict analysis_restrict.cpp)
add_executable(analysis_vectorized_load analysis_vectorized_load.cpp)
add_executable(analysis_atomic_instruction analysis_atomic_instruction.cpp)
add_executable(analysis_wavefront_divergence analysis_wavefront_divergence.cpp)
add_executable(analysis_shared_memory analysis_shared_memory.cpp)
add_executable(analysis_datatype_conversion analysis_datatype_conversion.cpp)
add_executable(analysis_deadlock_detection analysis_deadlock_detection.cpp)
add_executable(save_to_json save_to_json.cpp)

install(TARGETS analysis_register_spilling
                analysis_restrict
                analysis_vectorized_load
                analysis_atomic_instruction
                analysis_wavefront_divergence
                analysis_shared_memory
                analysis_datatype_conversion
                analysis_deadlock_detection
                save_to_json
                DESTINATION analysis_amd)

install(PROGRAMS measurements.sh DESTINATION analysis_amd)