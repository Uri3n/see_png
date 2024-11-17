// This file provides a set of macros
// that help with specifying compile-time attributes in
// a platform independant manner. This includes things like
// force-inline, packed structs, etc.

#ifndef COMPILEATTRS_HPP
#define COMPILEATTRS_HPP


// ~ Noinline, forceinline ~
// Niche usecases, possibly useful for performance reasons.
#if defined(__clang__) || defined(__GNUC__)
  #define BITHAT_NOINLINE    __attribute__((noinline))
  #define BITHAT_FORCEINLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
  #define SPNG_NOINLINE    __declspec(noinline)
  #define SPNG_FORCEINLINE __forceinline inline
#else
  static_assert("Unsupported compiler.")
#endif


// ~ Struct and Class Packing Attributes ~
// For packed structs and classes.
#if defined(__clang__) || defined(__GNUC__)
  #define SPNG_PACKED_IMPL(__KIND__, __NAME__, __BODY__) \
    __KIND__ __attribute__((packed)) __NAME__ __BODY__;
#elif defined(_MSC_VER)
  #define SPNG_PACKED_IMPL(__KIND__, __NAME__, __BODY__) \
    __pragma(pack(push, 1))       \
      __KIND__ __NAME__ __BODY__; \
    __pragma(pack(pop))
#else
  static_assert("Unsupported compiler.")
#endif

#define PACKED_STRUCT(__NAME__, __BODY__) SPNG_PACKED_IMPL(struct, __NAME__, __BODY__)
#define PACKED_CLASS(__NAME__, __BODY__)  SPNG_PACKED_IMPL(class, __NAME__, __BODY__)

#endif //COMPILEATTRS_HPP
