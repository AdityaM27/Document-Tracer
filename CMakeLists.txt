cmake_minimum_required(VERSION 3.16)
project(DocuTrace VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization and warning flags
if(MSVC)
    add_compile_options(/W4 /EHsc)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Core Library Sources
set(CORE_SOURCES
    src/Document.cpp
    src/Tokenizer.cpp
    src/TextProcessor.cpp
    src/DocumentLoader.cpp
    src/ExactMatchStrategy.cpp
    src/NGramSimilarity.cpp
    src/JaccardSimilarity.cpp
    src/SemanticSimilarity.cpp
    src/ReferenceIndex.cpp
    src/AcademicSourceProvider.cpp
    src/WebSourceProvider.cpp
    src/SourceDiscovery.cpp
    src/WritingPatternAnalyzer.cpp
    src/AIAnalysisClient.cpp
    src/ThreadPool.cpp
    src/ReportGenerator.cpp
    src/AnalysisService.cpp
)

add_library(docutrace_core STATIC ${CORE_SOURCES})

# Windows socket and crypto libraries required for networking and HTTP server
set(PLATFORM_LIBS "")
if(WIN32)
    set(PLATFORM_LIBS ws2_32 crypt32)
endif()

# 1. Main CLI Executable
add_executable(docutrace_cli src/main.cpp)
target_link_libraries(docutrace_cli PRIVATE docutrace_core ${PLATFORM_LIBS})

# 2. HTTP Backend REST API Server Executable
add_executable(docutrace_server
    backend/Server.cpp
    backend/server_main.cpp
)
target_link_libraries(docutrace_server PRIVATE docutrace_core ${PLATFORM_LIBS})

# 3. Comprehensive Unit & Integration Tests Executable
add_executable(docutrace_tests tests/test_main.cpp)
target_link_libraries(docutrace_tests PRIVATE docutrace_core ${PLATFORM_LIBS})

# 4. Performance & Accuracy Benchmark Executable
add_executable(docutrace_benchmark tests/benchmark.cpp)
target_link_libraries(docutrace_benchmark PRIVATE docutrace_core ${PLATFORM_LIBS})

enable_testing()
add_test(NAME AllTests COMMAND docutrace_tests)
