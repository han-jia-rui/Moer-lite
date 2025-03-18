#pragma once

#include <cstdlib>
#include <iostream>
#include <source_location>

inline void
Todo(const std::source_location location = std::source_location::current()) {
    std::cout << "TODO: Implement this function at line " << location.line()
              << " in file " << location.file_name() << std::endl;
    std::exit(1);
}
