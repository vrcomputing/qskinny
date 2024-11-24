message(STATUS "Using precompiled headers")

# C Standard Library Headers
set(C_STANDARD_HEADERS
    <cassert>
    <cctype>
    <cerrno>
    <cfloat>
    <cinttypes>
    <climits>
    <clocale>
    <cmath>
    <csetjmp>
    <csignal>
    <cstdarg>
    <cstdbool>
    <cstddef>
    <cstdint>
    <cstdio>
    <cstdlib>
    <cstring>
    <ctime>
    <cwchar>
    <cwctype>
)

# C++ Standard Library Headers
set(CPLUSPLUS_STANDARD_HEADERS
    <algorithm>
    <array>
    <atomic>
    <bitset>
    <chrono>
    <codecvt>
    <complex>
    <condition_variable>
    <deque>
    <exception>
    <execution>
    <filesystem>
    <forward_list>
    <fstream>
    <functional>
    <future>
    <initializer_list>
    <iomanip>
    <ios>
    <iosfwd>
    <iostream>
    <istream>
    <iterator>
    <limits>
    <list>
    <locale>
    <map>
    <memory>
    <mutex>
    <new>
    <numeric>
    <optional>
    <ostream>
    <queue>
    <random>
    <ratio>
    <regex>
    <set>
    <shared_mutex>
    <sstream>
    <stack>
    <stdexcept>
    <streambuf>
    <string>
    <string_view>
    <strstream>
    <system_error>
    <thread>
    <tuple>
    <type_traits>
    <typeindex>
    <typeinfo>
    <unordered_map>
    <unordered_set>
    <utility>
    <valarray>
    <variant>
    <vector>
    <version>
)

# C++20 Specific Headers
set(CPLUSPLUS20_HEADERS
    <concepts>
    <ranges>
    <span>
    <source_location>
    <stop_token>
    <synchronization>
    <barrier>
    <latch>
    <semaphore>
)

set(QSK_QT_HEADERS
    <QApplication>
    <QBrush>
    <QByteArray>
    <QColor>
    <QCoreApplication>
    <QDebug>
    <QDir>
    <QDoubleValidator>
    <QElapsedTimer>
    <QEvent>
    <QFile>
    <QFont>
    <QFontDatabase>
    <QFontMetrics>
    <QFontMetricsF>
    <QGraphicsGridLayout>
    <QGraphicsScene>
    <QGraphicsView>
    <QGraphicsWidget>
    <QGridLayout>
    <QGuiApplication>
    <QImage>
    <QKeySequence>
    <QLabel>
    <QLineF>
    <QListWidget>
    <QLocale>
    <QMetaMethod>
    <QMetaProperty>
    <QMetaType>
    <QObject>
    <QPainter>
    <QPainterPath>
    <QPen>
    <QPointer>
    <QPolygonF>
    <QQmlApplicationEngine>
    <QQuickFramebufferObject>
    <QQuickItem>
    <QQuickWidget>
    <QQuickWindow>
    <QRectF>
    <QSGFlatColorMaterial>
    <QSGGeometryNode>
    <QSGMaterialRhiShader>
    <QSGNode>
    <QSize>
    <QStringList>
    <QSvgRenderer>
    <Qt>
    <QThread>
    <QTime>
    <QTimer>
    <QtMath>
    <QtQml>
    <QTransform>
    <QUrl>
    <QVariant>
    <QVector>
    <QWidget>
)

if(TRUE) # build break on gcc
    list(REMOVE_ITEM CPLUSPLUS_STANDARD_HEADERS <execution>)
endif()

if(TRUE) # NOT TARGET Qt::Widgets
    list(REMOVE_ITEM QSK_QT_HEADERS <QApplication>)
    list(REMOVE_ITEM QSK_QT_HEADERS <QGraphicsWidget>)
    list(REMOVE_ITEM QSK_QT_HEADERS <QGraphicsGridLayout>)
    list(REMOVE_ITEM QSK_QT_HEADERS <QGraphicsScene>)
    list(REMOVE_ITEM QSK_QT_HEADERS <QWidget>)
endif()

if(NOT TARGET Qt::QuickWidgets)
    list(REMOVE_ITEM QSK_QT_HEADERS <QQuickWidget>)
endif()

if(QT_VERSION_MAJOR VERSION_GREATER_EQUAL 6)
    list(REMOVE_ITEM QSK_QT_HEADERS <QSGMaterialRhiShader>)
endif()

set(ALL_STANDARD_HEADERS
    ${C_STANDARD_HEADERS}
    ${CPLUSPLUS_STANDARD_HEADERS}
    # ${CPLUSPLUS20_HEADERS}
    ${QSK_QT_HEADERS}
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pch.cpp "enum {};")
add_library(PCH OBJECT ${CMAKE_CURRENT_BINARY_DIR}/pch.cpp)
target_precompile_headers(PCH PRIVATE ${ALL_STANDARD_HEADERS})
set_target_properties(PCH PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
if(MSVC)
    # TODO check why we need this
    # Set the desired compiler options
    # target_compile_options(PCH PRIVATE
    #     /Zc:referenceBinding
    #     /Zc:__cplusplus
    #     /Zc:hiddenFriend
    #     /Zc:externC
    #     /Zc:externConstexpr
    #     /permissive-
    #     #/source-charset:utf-8
    #     #/execution-charset:utf-8
    #     /utf-8
    # )
endif()

target_link_libraries(PCH PRIVATE
    Qt::Core
    Qt::CorePrivate
    Qt::Gui
    Qt::GuiPrivate
    Qt::Quick
    Qt::QuickPrivate)

if(TARGET Qt::OpenGL)
    target_link_libraries(PCH PRIVATE Qt::OpenGL)
endif()
if(TARGET Qt::OpenGLPrivate)
    target_link_libraries(PCH PRIVATE Qt::OpenGLPrivate)
endif()
if(TARGET Qt::Widgets)
    target_link_libraries(PCH PRIVATE Qt::Widgets)
endif()
if(TARGET Qt::Svg)
    target_link_libraries(PCH PRIVATE Qt::Svg)
endif()
if(TARGET Qt::QuickShapesPrivate)
    target_link_libraries(PCH PRIVATE Qt::QuickShapesPrivate)
endif()
if(TARGET Qt::QuickWidgets)
    target_link_libraries(PCH PRIVATE Qt::QuickWidgets)
endif()
if(TARGET Qt::WebEngine)
    target_link_libraries(PCH PRIVATE Qt::WebEngine)
endif()
if(TARGET Qt::WebEngineCore)
    target_link_libraries(PCH PRIVATE Qt::WebEngineCore)
endif()
if(TARGET Qt::WebEngineQuick)
    target_link_libraries(PCH PRIVATE Qt::WebEngineQuick)
endif()
if(TARGET Qt::WebEngineQuickPrivate)
    target_link_libraries(PCH PRIVATE Qt::WebEngineQuickPrivate)
endif()