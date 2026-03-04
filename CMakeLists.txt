# CMakeList.txt : CMake project for Trie, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Trie")

# Add source to this project's executable.
add_executable (Trie  "src/main.cpp" "headers/imported/IHashTable.h" "headers/imported/OpenHashTable.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Trie PROPERTY CXX_STANDARD 20)
endif()

# Указываем где искать заголовки
target_include_directories(Trie PRIVATE
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/src    
)

# Опция для указания пути к тестам
set(TEST_DIR "${CMAKE_SOURCE_DIR}/TestData" CACHE PATH "Path to test files")

# Копируем тесты в build directory
file(COPY ${TEST_DIR} DESTINATION ${CMAKE_BINARY_DIR})

# Передаем путь к тестам в код
target_compile_definitions(Trie PRIVATE 
   TEST_DIR="${CMAKE_BINARY_DIR}/TestData")