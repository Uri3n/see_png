#ifndef DEFER_HPP
#define DEFER_HPP
#include <type_traits>

#define spng_defer(obj)          ::spng::Defer _(cond);
#define spng_defer_if(cond, obj) ::spng::Defer _([&](){ if((cond))obj(); })

namespace spng {
  template<typename T> requires std::is_invocable_v<T>
  class Defer {
    T obj_;
  public:
    Defer(const Defer&)             = delete;
    Defer& operator=(const Defer&)  = delete;
    Defer(const Defer&&)            = delete;
    Defer& operator=(const Defer&&) = delete;

    explicit Defer(T obj) : obj_(obj) {}
    ~Defer() { obj_(); }
  };
}

#endif //DEFER_HPP
