cmake_minimum_required(VERSION 3.22)
project(DecisionHelper LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(DecisionHelper
    src/main.cpp
    src/core/Session.cpp
    src/core/DecisionEngine.cpp
    src/core/Storage.cpp
    src/controller/SessionController.cpp
)

qt_add_qml_module(DecisionHelper
    URI DecisionHelper
    VERSION 1.0
    QML_FILES
        src/qml/main.qml
        src/qml/PhaseRouter.qml
        src/qml/components/AppHeader.qml
        src/qml/components/DecisionChoiceCard.qml
        src/qml/phase1/NameDecisionPage.qml
        src/qml/phase2/AddOptionsPage.qml
        src/qml/phase3/DefineCriteriaPage.qml
        src/qml/phase4/OptionMicroDecisionPage.qml
        src/qml/phase5/CriteriaWeightingPage.qml
        src/qml/phase6/ResultsPage.qml
)

# ── Emoji Font (WASM & Cross-Platform) ─────────────────────────────────────────
# WASM environments do not have host system emoji fonts (e.g. Segoe UI Emoji / Apple Color Emoji).
# We bundle Google Noto Color Emoji to render emojis cleanly in the browser.
set(EMOJI_FONT "")
foreach(candidate
    "${CMAKE_SOURCE_DIR}/fonts/NotoColorEmoji.ttf"
    "${CMAKE_SOURCE_DIR}/third_party/noto-emoji/fonts/NotoColorEmoji.ttf"
    "${CMAKE_SOURCE_DIR}/../checklist/third_party/noto-emoji/fonts/NotoColorEmoji.ttf"
    "${CMAKE_SOURCE_DIR}/../taskWizard/third_party/noto-emoji/fonts/NotoColorEmoji.ttf"
)
    if(EXISTS "${candidate}")
        set(EMOJI_FONT "${candidate}")
        break()
    endif()
endforeach()

# Automatically copy font into fonts/ if found in a sibling repository
if(EXISTS "${EMOJI_FONT}" AND NOT EXISTS "${CMAKE_SOURCE_DIR}/fonts/NotoColorEmoji.ttf" AND NOT EXISTS "${CMAKE_SOURCE_DIR}/third_party/noto-emoji/fonts/NotoColorEmoji.ttf")
    file(COPY "${EMOJI_FONT}" DESTINATION "${CMAKE_SOURCE_DIR}/fonts")
    set(EMOJI_FONT "${CMAKE_SOURCE_DIR}/fonts/NotoColorEmoji.ttf")
endif()

if(EXISTS "${EMOJI_FONT}")
    set_source_files_properties("${EMOJI_FONT}"
        PROPERTIES QT_RESOURCE_ALIAS "NotoColorEmoji.ttf"
    )
    qt_add_resources(DecisionHelper "appfonts"
        PREFIX "/fonts"
        FILES "${EMOJI_FONT}"
        BIG_RESOURCES
    )
    target_compile_definitions(DecisionHelper PRIVATE HAS_EMOJI_FONT)
    message(STATUS "Emoji font found: ${EMOJI_FONT}")
else()
    message(WARNING "Emoji font not found at ${EMOJI_FONT}. Emojis may render as boxes in WASM.")
endif()

target_link_libraries(DecisionHelper PRIVATE
    Qt6::Core Qt6::Quick Qt6::QuickControls2
)

if(EMSCRIPTEN)
    target_compile_definitions(DecisionHelper PRIVATE DECISIONHELPER_WASM)
    target_link_options(DecisionHelper PRIVATE
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    )
    set_target_properties(DecisionHelper PROPERTIES
        OUTPUT_NAME "index"
        QT_WASM_INITIAL_MEMORY "64MB"
    )
endif()
