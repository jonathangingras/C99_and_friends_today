#include <iostream>

template <typename type>
struct type_traits {
  static const bool is_pod = false;
};

template <typename type>
struct type_traits<type &> {
  static const bool is_pod = false;
};

template <typename type>
struct type_traits<type *> {
  static const bool is_pod = true;
};

template <>
struct type_traits<char> {
  static const bool is_pod = true;
};

template <>
struct type_traits<short> {
  static const bool is_pod = true;
};

template <>
struct type_traits<int> {
  static const bool is_pod = true;
};

template <>
struct type_traits<long> {
  static const bool is_pod = true;
};

template <>
struct type_traits<float> {
  static const bool is_pod = true;
};

template <>
struct type_traits<double> {
  static const bool is_pod = true;
};

template <typename type>
inline bool is_pod(const type &) {
  return type_traits<type>::is_pod;
}

int main() {

  int i;
  float f;

  std::cout << std::boolalpha;
  std::cout << is_pod(i) << std::endl;
  std::cout << is_pod(f) << std::endl;
  std::cout << is_pod(&std::cout) << std::endl;

  return 0;
}
