# ==============================================================================
# BrazaReader - CMakeLists.txt
# Leitor de PDF otimizado para Raspberry Pi Zero (ARMv6 hard-float)
# ==============================================================================

cmake_minimum_required(VERSION 3.16)

project(BrazaReader
    VERSION 1.0.0
    DESCRIPTION "Leitor de PDF minimalista otimizado para Raspberry Pi Zero"
    LANGUAGES CXX
)

# ==============================================================================
# Configuracoes do compilador C++17
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ==============================================================================
# Dependencias - Qt5
# ==============================================================================
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ==============================================================================
# Dependencias - SQLite3
# ==============================================================================
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# ==============================================================================
# Dependencias - MuPDF (busca robusta)
# ==============================================================================

# Detectar arquitetura para paths
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(ARCH_LIB_DIR "x86_64-linux-gnu")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(ARCH_LIB_DIR "aarch64-linux-gnu")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    set(ARCH_LIB_DIR "arm-linux-gnueabihf")
else()
    set(ARCH_LIB_DIR "")
endif()

set(LIB_SEARCH_PATHS
    /usr/lib
    /usr/local/lib
    /usr/lib/${ARCH_LIB_DIR}
)

# MuPDF principal (obrigatorio)
find_path(MUPDF_INCLUDE_DIR
    NAMES mupdf/fitz.h
    PATHS /usr/include /usr/local/include
)

find_library(MUPDF_LIBRARY
    NAMES mupdf
    PATHS ${LIB_SEARCH_PATHS}
)

if(NOT MUPDF_INCLUDE_DIR OR NOT MUPDF_LIBRARY)
    message(FATAL_ERROR
        "MuPDF NAO encontrado!\n"
        "Instale com: sudo apt-get install libmupdf-dev"
    )
endif()

message(STATUS "MuPDF include: ${MUPDF_INCLUDE_DIR}")
message(STATUS "MuPDF library: ${MUPDF_LIBRARY}")

# MuPDF-third (dependencia interna do MuPDF)
find_library(MUPDF_THIRD_LIBRARY
    NAMES mupdf-third
    PATHS ${LIB_SEARCH_PATHS}
)
if(MUPDF_THIRD_LIBRARY)
    message(STATUS "MuPDF-third:   ${MUPDF_THIRD_LIBRARY}")
endif()

# MuJS (JavaScript engine usado pelo MuPDF)
find_library(MUJS_LIBRARY
    NAMES mujs
    PATHS ${LIB_SEARCH_PATHS}
)
if(MUJS_LIBRARY)
    message(STATUS "MuJS:          ${MUJS_LIBRARY}")
endif()

# Gumbo (HTML parser usado pelo MuPDF)
find_library(GUMBO_LIBRARY
    NAMES gumbo
    PATHS ${LIB_SEARCH_PATHS}
)
if(GUMBO_LIBRARY)
    message(STATUS "Gumbo:         ${GUMBO_LIBRARY}")
endif()

# Extract (document extraction usado pelo MuPDF)
find_library(EXTRACT_LIBRARY
    NAMES extract
    PATHS ${LIB_SEARCH_PATHS}
)
if(EXTRACT_LIBRARY)
    message(STATUS "Extract:       ${EXTRACT_LIBRARY}")
endif()

# Dependencias graficas/compressao
find_library(FREETYPE_LIBRARY NAMES freetype PATHS ${LIB_SEARCH_PATHS})
find_library(HARFBUZZ_LIBRARY NAMES harfbuzz PATHS ${LIB_SEARCH_PATHS})
find_library(JPEG_LIBRARY NAMES jpeg PATHS ${LIB_SEARCH_PATHS})
find_library(OPENJP2_LIBRARY NAMES openjp2 PATHS ${LIB_SEARCH_PATHS})
find_library(JBIG2DEC_LIBRARY NAMES jbig2dec PATHS ${LIB_SEARCH_PATHS})
find_library(ZLIB_LIBRARY NAMES z PATHS ${LIB_SEARCH_PATHS})
find_library(PNG_LIBRARY NAMES png PATHS ${LIB_SEARCH_PATHS})
find_library(TIFF_LIBRARY NAMES tiff PATHS ${LIB_SEARCH_PATHS})
find_library(LEPTONICA_LIBRARY NAMES leptonica lept PATHS ${LIB_SEARCH_PATHS})
find_library(TESSERACT_LIBRARY NAMES tesseract PATHS ${LIB_SEARCH_PATHS})
find_library(SSL_LIBRARY NAMES ssl PATHS ${LIB_SEARCH_PATHS})
find_library(CRYPTO_LIBRARY NAMES crypto PATHS ${LIB_SEARCH_PATHS})

# ==============================================================================
# Arquivos fonte - Core
# ==============================================================================
set(CORE_SOURCES
    src/main.cpp
    src/app.h
    src/app.cpp
)

# ==============================================================================
# Arquivos fonte - UI Layer
# ==============================================================================
set(UI_SOURCES
    src/ui/mainwindow.h
    src/ui/mainwindow.cpp
    src/ui/libraryscreen.h
    src/ui/libraryscreen.cpp
    src/ui/readerscreen.h
    src/ui/readerscreen.cpp
    src/ui/settingsscreen.h
    src/ui/settingsscreen.cpp
    src/ui/aboutscreen.h
    src/ui/aboutscreen.cpp
    src/ui/sidebarmenu.h
    src/ui/sidebarmenu.cpp
    src/ui/termsscreen.h
    src/ui/termsscreen.cpp
    src/ui/collectionscreen.h
    src/ui/collectionscreen.cpp
    src/ui/topbar_helper.h
    src/ui/pagewidget.h
    src/ui/pagewidget.cpp

)

# ==============================================================================
# Arquivos fonte - Engine
# ==============================================================================
set(ENGINE_SOURCES
    src/engine/pdfloader.h
    src/engine/pdfloader.cpp
    src/engine/pdfrenderer.h
    src/engine/pdfrenderer.cpp
    src/engine/pdfcache.h
    src/engine/pdfcache.cpp
    src/engine/textextractor.h
    src/engine/textextractor.cpp
)

# ==============================================================================
# Arquivos fonte - Storage
# ==============================================================================
set(STORAGE_SOURCES
    src/storage/librarymanager.h
    src/storage/librarymanager.cpp
    src/storage/progressmanager.h
    src/storage/progressmanager.cpp
    src/storage/favoritemanager.h
    src/storage/favoritemanager.cpp
    src/storage/settingsmanager.h
    src/storage/settingsmanager.cpp
    src/storage/collectionmanager.h
    src/storage/collectionmanager.cpp
    src/storage/annotationmanager.h
    src/storage/annotationmanager.cpp
)

# ==============================================================================
# Arquivos fonte - System
# ==============================================================================
set(SYSTEM_SOURCES
    src/system/touchhandler.h
    src/system/touchhandler.cpp
)

# ==============================================================================
# Recursos Qt
# ==============================================================================
set(RESOURCE_FILES
    resources/resources.qrc
)

# Criar resources.qrc se nao existir
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/resources/resources.qrc")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/resources")
    file(WRITE "${CMAKE_SOURCE_DIR}/resources/resources.qrc"
        "<RCC>\n  <qresource prefix=\"/\">\n  </qresource>\n</RCC>\n"
    )
    message(STATUS "resources.qrc criado automaticamente")
endif()

# ==============================================================================
# Executavel
# ==============================================================================
add_executable(${PROJECT_NAME}
    ${CORE_SOURCES}
    ${UI_SOURCES}
    ${ENGINE_SOURCES}
    ${STORAGE_SOURCES}
    ${SYSTEM_SOURCES}
    ${RESOURCE_FILES}
)

# ==============================================================================
# Includes
# ==============================================================================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/engine
    ${CMAKE_SOURCE_DIR}/src/storage
    ${CMAKE_SOURCE_DIR}/src/system
    ${MUPDF_INCLUDE_DIR}
    ${SQLITE3_INCLUDE_DIRS}
)

# ==============================================================================
# Linkagem - ORDEM IMPORTA para bibliotecas estaticas
# ==============================================================================

# Bibliotecas principais
set(LINK_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    ${MUPDF_LIBRARY}
)

# MuPDF-third (contem freetype, zlib, etc embutidos)
if(MUPDF_THIRD_LIBRARY)
    list(APPEND LINK_LIBS ${MUPDF_THIRD_LIBRARY})
endif()

# MuJS (JavaScript - resolve js_* symbols)
if(MUJS_LIBRARY)
    list(APPEND LINK_LIBS ${MUJS_LIBRARY})
else()
    message(WARNING "libmujs NAO encontrada. Instale: sudo apt-get install libmujs-dev")
endif()

# Gumbo (HTML parser - resolve gumbo_* symbols)
if(GUMBO_LIBRARY)
    list(APPEND LINK_LIBS ${GUMBO_LIBRARY})
else()
    message(WARNING "libgumbo NAO encontrada. Instale: sudo apt-get install libgumbo-dev")
endif()

# Extract (document extraction - resolve extract_* symbols)
if(EXTRACT_LIBRARY)
    list(APPEND LINK_LIBS ${EXTRACT_LIBRARY})
else()
    message(WARNING "libextract NAO encontrada. Pode precisar compilar do fonte.")
endif()

# SQLite3
list(APPEND LINK_LIBS ${SQLITE3_LIBRARIES})

# Dependencias graficas/compressao
if(FREETYPE_LIBRARY)
    list(APPEND LINK_LIBS ${FREETYPE_LIBRARY})
endif()
if(HARFBUZZ_LIBRARY)
    list(APPEND LINK_LIBS ${HARFBUZZ_LIBRARY})
endif()
if(JPEG_LIBRARY)
    list(APPEND LINK_LIBS ${JPEG_LIBRARY})
endif()
if(OPENJP2_LIBRARY)
    list(APPEND LINK_LIBS ${OPENJP2_LIBRARY})
endif()
if(JBIG2DEC_LIBRARY)
    list(APPEND LINK_LIBS ${JBIG2DEC_LIBRARY})
endif()
if(ZLIB_LIBRARY)
    list(APPEND LINK_LIBS ${ZLIB_LIBRARY})
endif()
if(PNG_LIBRARY)
    list(APPEND LINK_LIBS ${PNG_LIBRARY})
endif()
if(TIFF_LIBRARY)
    list(APPEND LINK_LIBS ${TIFF_LIBRARY})
endif()
if(LEPTONICA_LIBRARY)
    list(APPEND LINK_LIBS ${LEPTONICA_LIBRARY})
endif()
if(TESSERACT_LIBRARY)
    list(APPEND LINK_LIBS ${TESSERACT_LIBRARY})
endif()
if(SSL_LIBRARY)
    list(APPEND LINK_LIBS ${SSL_LIBRARY})
endif()
if(CRYPTO_LIBRARY)
    list(APPEND LINK_LIBS ${CRYPTO_LIBRARY})
endif()

# Sistema
list(APPEND LINK_LIBS
    m
    pthread
    dl
)

# Aplicar linkagem
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBS})

# Diretorio de linkagem do SQLite
if(SQLITE3_LIBRARY_DIRS)
    target_link_directories(${PROJECT_NAME} PRIVATE ${SQLITE3_LIBRARY_DIRS})
endif()

# ==============================================================================
# Flags de compilacao
# ==============================================================================
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-O2 -s -DNDEBUG>
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra -Wpedantic>
    $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
    $<$<CONFIG:MinSizeRel>:-Os -s -DNDEBUG>
)

# Flags ARM (Raspberry Pi Zero)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv6" OR CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    message(STATUS "Otimizacoes ARM habilitadas")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -march=armv6 -mfpu=vfp -mfloat-abi=hard
    )
endif()

# ==============================================================================
# Definicoes
# ==============================================================================
target_compile_definitions(${PROJECT_NAME} PRIVATE
    APP_NAME="${PROJECT_NAME}"
    APP_VERSION="${PROJECT_VERSION}"
    LIBRARY_PATH="/library"
    DATA_DIR="brazareader"
)

# ==============================================================================
# Instalacao
# ==============================================================================
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# ==============================================================================
# Resumo
# ==============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "  BrazaReader v${PROJECT_VERSION}")
message(STATUS "========================================")
message(STATUS "  Build type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler:      ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Processor:     ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  Qt5:           ${Qt5Core_VERSION}")
message(STATUS "  SQLite3:       ${SQLITE3_VERSION}")
message(STATUS "  Output:        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "========================================")
message(STATUS "")