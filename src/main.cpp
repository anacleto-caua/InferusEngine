#include <exception>

#include <spdlog/spdlog.h>

#include "Engine/Types.hpp"
#include "Engine/InferusEngine.hpp"

int main() {
    // [Time] [Log Level] Message
    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");
    #ifdef NDEBUG
        spdlog::set_level(spdlog::level::off);
    #else
        spdlog::set_level(spdlog::level::debug);
    #endif

    InferusEngine Engine = InferusEngine();

    if ( Engine.Init() != InferusResult::SUCCESS ) {
        spdlog::critical("Couldn't open engine.");
        return -1;
    }

    try {
        Engine.Run();
    } catch (const std::exception &e) {
        spdlog::critical("runtime exception - ", e.what());
        return -1;
    }

    return 0;
}
