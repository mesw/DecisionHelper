cmake_minimum_required(VERSION 3.22)
project(DecisionHelper LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(DecisionHelper
    src/main.cpp
    src/core/Session.cpp
    src/core/RankingEngine.cpp
    src/core/AttributionEngine.cpp
    src/core/AnalysisEngine.cpp
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
        src/qml/components/SweetSpotProgressBar.qml
        src/qml/components/DisclosurePanel.qml
        src/qml/components/PropertyDetailSheet.qml
        src/qml/phase1/AddOptionsPage.qml
        src/qml/phase2/OptionCard.qml
        src/qml/phase2/PairwiseRankingPage.qml
        src/qml/phase2/RankingResultPage.qml
        src/qml/phase3/DefinePropertiesPage.qml
        src/qml/phase4/PrioritisePropertiesPage.qml
        src/qml/phase5/AttributionCard.qml
        src/qml/phase5/AttributionPage.qml
        src/qml/phase6/InsightCard.qml
        src/qml/phase6/RankingComparison.qml
        src/qml/phase6/CorrelationChart.qml
        src/qml/phase6/ScatterPlot.qml
        src/qml/phase6/AnalysisPage.qml
)

target_link_libraries(DecisionHelper PRIVATE
    Qt6::Core Qt6::Quick Qt6::QuickControls2
)

if(EMSCRIPTEN)
    # Qt's cmake integration sets WASM=1, MODULARIZE, EXPORT_NAME, and
    # EXPORTED_RUNTIME_METHODS automatically — adding them again causes conflicts.
    # Only set what Qt does not already provide.
    set_target_properties(DecisionHelper PROPERTIES OUTPUT_NAME "index")
endif()
