
function( copy_dir_to_target_bin target dir )
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/${dir}
        $<TARGET_FILE_DIR:${target}>/${dir})
endfunction()