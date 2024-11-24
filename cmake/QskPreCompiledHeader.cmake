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

set(ALL_STANDARD_HEADERS
    ${C_STANDARD_HEADERS}
    ${CPLUSPLUS_STANDARD_HEADERS}
    # ${CPLUSPLUS20_HEADERS}
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pch.cpp "enum {};")
add_library(PCH OBJECT ${CMAKE_CURRENT_BINARY_DIR}/pch.cpp)
target_precompile_headers(PCH PRIVATE ${ALL_STANDARD_HEADERS})
set_target_properties(PCH PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
if(MSVC)
    # TODO check why we need this
    # Set the desired compiler options
    target_compile_options(PCH PRIVATE
        /Zc:referenceBinding
        /Zc:__cplusplus
        /Zc:hiddenFriend
        /Zc:externC
        /Zc:externConstexpr
        /permissive-
        /source-charset:utf-8
        /execution-charset:utf-8
    )
endif()
