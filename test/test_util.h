
namespace bdm {

template <typename T>
struct abs_error {
  static_assert(std::is_same<T, double>::value || std::is_same<T, float>::value,
                "abs_error<T> may only be used with T = { float, double }");
  static constexpr double value = 1e-24;
};

template <>
struct abs_error<float> {
  static constexpr double value = 1e-6;
};

template <>
struct abs_error<double> {
  static constexpr double value = 1e-9;
};

}  // namespace bdm