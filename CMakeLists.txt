cmake_minimum_required(VERSION 3.25)

project(dasGame)


set(CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
find_package(Gorgon REQUIRED)

add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/main.cpp)


target_link_libraries(${PROJECT_NAME} PRIVATE Gorgon::Gorgon)
configure_file(${CMAKE_SOURCE_DIR}/Assets/tank.png ${CMAKE_SOURCE_DIR}/Bin/Assets/tank.png COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Assets/bullet.png ${CMAKE_SOURCE_DIR}/Bin/Assets/bullet.png COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Assets/Nasa.ttf ${CMAKE_SOURCE_DIR}/Bin/Assets/Nasa.ttf COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Assets/brick.jpg ${CMAKE_SOURCE_DIR}/Bin/Assets/brick.jpg COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Assets/Sounds/gameplay.wav ${CMAKE_SOURCE_DIR}/Bin/Assets/Sounds/gameplay.wav COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Assets/Sounds/gun.wav ${CMAKE_SOURCE_DIR}/Bin/Assets/Sounds/gun.wav COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Assets/Sounds/Impact.wav ${CMAKE_SOURCE_DIR}/Bin/Assets/Sounds/Impact.wav COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Assets/Sounds/ring.wav ${CMAKE_SOURCE_DIR}/Bin/Assets/Sounds/ring.wav COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Assets/Sounds/ui_beep.wav ${CMAKE_SOURCE_DIR}/Bin/Assets/Sounds/ui_beep.wav COPYONLY)