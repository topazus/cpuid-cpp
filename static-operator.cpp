#include <assert.h>

#include <format>
#include <iostream>
#include <string_view>
enum class Color { red, green, blue };
// static operator[]
struct kEnumToStringViewBimap {
  static constexpr std::string_view operator[](Color color) noexcept {
    switch (color) {
      case Color::red:
        return "red";
      case Color::green:
        return "green";
      case Color::blue:
        return "blue";
    }
  }

  static constexpr Color operator[](std::string_view color) noexcept {
    if (color == "red") {
      return Color::red;
    } else if (color == "green") {
      return Color::green;
    } else if (color == "blue") {
      return Color::blue;
    }
  }
};
auto test_static_operator() {
  assert(kEnumToStringViewBimap{}["red"] == Color::red);
  assert(kEnumToStringViewBimap{}[Color::red] == "red");
}
char xdigit(int n) {
  static constexpr char digits[] = "0123456789abcdef";
  return digits[n];
}
// compile time function
constexpr char xdigit_(int n) {
  static constexpr char digits[] = "0123456789abcdef";
  return digits[n];
}
auto test_xdigit() {
  std::cout << std::format("xdigit(0) = {}\n", xdigit(0));
  std::cout << std::format("xdigit_(0) = {}\n", xdigit_(0));
}

constexpr std::optional<int> to_int(std::string_view s) {
  int value;
  if (auto [p, err] = std::from_chars(s.begin(), s.end(), value);
      err == std::errc{}) {
    return value;
  } else {
    return std::nullopt;
  }
}
auto test_to_int() {
  static_assert(to_int("42") == 42);
  static_assert(to_int("foo") == std::nullopt);
}
// ...
int main() {
  test_xdigit();
  test_to_int();
  return 0;
}