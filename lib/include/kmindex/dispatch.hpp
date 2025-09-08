
#include <cstddef>
#include <functional>
#include <type_traits>

namespace kmq {

    template<std::size_t Max,
             std::size_t Value,
             std::size_t Step,
             typename Compare = std::less<std::size_t>,
             typename Op = void>
    struct runtime_dispatch
    {
      static constexpr Compare compare = Compare{};

      template<template<std::size_t> typename Functor, typename... Args>
      static auto execute(std::size_t value, Args&&... args)
        -> decltype(Functor<Value>()(std::forward<Args>(args)...))
      {
        if (compare(value, Value))
        {
          return Functor<Value>()(std::forward<Args>(args)...);
        }

        if constexpr(std::is_same_v<Op, void>)
        {
          return runtime_dispatch<Max, Value + Step, Step, Compare>::template execute<Functor, Args...>(
            value, std::forward<Args>(args)...
          );
        }
        else
        {
          return runtime_dispatch<Max, Op{}(Value, Step), Step, Compare>::template execute<Functor, Args...>(
            value, std::forward<Args>(args)...
          );
        }
      }
    };

    template<std::size_t Max, std::size_t Step, typename Compare, typename Op>
    struct runtime_dispatch<Max, Max, Step, Compare, Op>
    {
      static constexpr Compare compare = Compare{};

      template<template<std::size_t> typename Functor, typename... Args>
      static auto execute(std::size_t value, Args&&... args)
        -> decltype(Functor<Max>()(std::forward<Args>(args)...))
      {
        if (compare(value, Max))
          return Functor<Max>()(std::forward<Args>(args)...);
        else
          throw std::runtime_error("No implementation found.");
      }
    };

}