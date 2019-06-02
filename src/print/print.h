#pragma once

#include <iostream>

auto print = [](const auto&... args) -> void {
  (std::cout << ... << args) << '\n';
};