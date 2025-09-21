#ifndef LUTILS_H
#define LUTILS_H

#include <algorithm>
#include <string>
#include <vector>

namespace Louvre {
template <typename, bool>
struct _numerical_underlying_type {};

template <typename T>
struct _numerical_underlying_type<T, true> {
  using type = std::underlying_type<T>::type;
};

template <typename T>
struct _numerical_underlying_type<T, false> {
  using type = T;
};

template <typename T>
struct numerical_underlying_type
    : _numerical_underlying_type<T, std::is_enum<T>::value> {};

template <typename T>
static inline void LVectorRemoveOne(std::vector<T>& vec, T val) noexcept {
  const auto it{std::find(vec.begin(), vec.end(), val)};
  if (it != vec.end()) vec.erase(it);
}

template <typename T>
static inline void LVectorRemoveAll(std::vector<T>& vec, T val) noexcept {
  for (auto it = vec.begin(); it != vec.end();) {
    if (*it == val)
      it = vec.erase(it);
    else
      it++;
  }
}

template <typename T>
static inline void LVectorRemoveOneUnordered(std::vector<T>& vec,
                                             T val) noexcept {
  auto it = std::find(vec.begin(), vec.end(), val);
  if (it != vec.end()) {
    *it = std::move(vec.back());
    vec.pop_back();
  }
}

template <typename T>
static inline void LVectorPushBackIfNonexistent(std::vector<T>& vec,
                                                T val) noexcept {
  auto it = std::find(vec.begin(), vec.end(), val);
  if (it == vec.end()) {
    vec.push_back(val);
  }
}

template <typename T>
static inline void LVectorRemoveAllUnordered(std::vector<T>& vec,
                                             T val) noexcept {
  for (auto it = vec.begin(); it != vec.end();) {
  retry:
    if (*it == val) {
      *it = std::move(vec.back());
      vec.pop_back();

      if (it != vec.end()) goto retry;
    } else
      it++;
  }
}

inline const std::string getenvString(const char* env) noexcept {
  const char* val{getenv(env)};

  if (val != NULL) return std::string(val);

  return std::string();
}

int createSHM(std::size_t size);
};  // namespace Louvre

#endif  // LUTILS_H
