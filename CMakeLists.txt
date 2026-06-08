cmake_minimum_required(VERSION 3.16)

project(HomeCareScheduler)

set(CMAKE_CXX_STANDARD 17)

set(SFML_DIR "C:/SFML/SFML-2.6.1/lib/cmake/SFML")

set(SOURCES
    Main.cpp

    CareCenter/CareCenter.cpp
    CareGiver/CareGiver.cpp
    Customer/Customer.cpp
    Route/Route.cpp
    Problem/Problem.cpp
    Objectives/Objectives.cpp
    Solution/Solution.cpp
    Decoder/Decoder.cpp
    Evaluator/Evaluator.cpp
    ParetoTools/ParetoTools.cpp
    Mobso/Mobso.cpp

    Tests/BasicTests.cpp
    Tests/TestBigProblem.cpp
    Tests/TestLineProblem.cpp

    Visualizer/Visualizer.cpp
)

find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

add_executable(HomeCareScheduler ${SOURCES})

target_include_directories(HomeCareScheduler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(HomeCareScheduler
    sfml-graphics
    sfml-window
    sfml-system
)