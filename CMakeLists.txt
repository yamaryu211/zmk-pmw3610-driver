zephyr_library()

zephyr_library_sources_ifdef(CONFIG_PMW3610 src/pmw3610.c)
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)
target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_PMW3610 app PRIVATE src/behaviors/begavior_pmw3610.c)