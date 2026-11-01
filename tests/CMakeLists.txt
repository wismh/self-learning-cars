find_package(GTest REQUIRED)

add_executable(cars_tests
    test_neural_network.cpp
    test_genetic_algorithm.cpp
    test_track_generator.cpp
    test_help.cpp
    test_geometry.cpp
    test_locale_prefs.cpp
    ../src/ai/genetic_algorithm.cpp
    ../src/track/track_generator.cpp
    ../src/loc/locale_prefs.cpp)

target_include_directories(cars_tests PRIVATE
    ../src
    ${CMAKE_BINARY_DIR}/generated/self-learning-cars)

target_compile_features(cars_tests PRIVATE cxx_std_23)

target_link_libraries(cars_tests PRIVATE GTest::gtest_main)

if(MSVC)
    set_target_properties(cars_tests PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
endif()

gtest_discover_tests(cars_tests)
