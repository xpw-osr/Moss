// Exception Handling support header for -*- C++ -*-

// Copyright (C) 1995-2020 Free Software Foundation, Inc.
//
// This file is part of GCC.
//
// GCC is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// GCC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file exception
 *  This is a Standard C++ Library header.
 */

#ifndef __EXCEPTION__
#define __EXCEPTION__

#pragma GCC system_header

#pragma GCC visibility push(default)

#include "libcxx/gcc/bits/c++config.hh"
#include "libcxx/gcc/bits/exception.hh"

extern "C++" {

namespace std {
  /** @addtogroup exceptions
   *  @{
   */

  /** If an %exception is thrown which is not listed in a function's
   *  %exception specification, one of these may be thrown.  */
  class bad_exception : public exception {
  public:
    bad_exception() _GLIBCXX_USE_NOEXCEPT {}

    // This declaration is not useless:
    // http://gcc.gnu.org/onlinedocs/gcc-3.0.2/gcc_6.html#SEC118
    virtual ~bad_exception() _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT;

    // See comment in eh_exception.cc.
    virtual const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT;
  };

  /// If you write a replacement %terminate handler, it must be of this type.
  typedef void (*terminate_handler)();

  /// If you write a replacement %unexpected handler, it must be of this type.
  typedef void (*unexpected_handler)();

  /// Takes a new handler function as an argument, returns the old function.
  terminate_handler set_terminate(terminate_handler) _GLIBCXX_USE_NOEXCEPT;

#if __cplusplus >= 201103L
  /// Return the current terminate handler.
  terminate_handler get_terminate() noexcept;
#endif

  /** The runtime will call this function if %exception handling must be
   *  abandoned for any reason.  It can also be called by the user.  */
  void terminate() _GLIBCXX_USE_NOEXCEPT __attribute__((__noreturn__));

  /// Takes a new handler function as an argument, returns the old function.
  unexpected_handler set_unexpected(unexpected_handler) _GLIBCXX_USE_NOEXCEPT;

#if __cplusplus >= 201103L
  /// Return the current unexpected handler.
  unexpected_handler get_unexpected() noexcept;
#endif

  /** The runtime will call this function if an %exception is thrown which
   *  violates the function's %exception specification.  */
  void unexpected() __attribute__((__noreturn__));

  /** [18.6.4]/1:  'Returns true after completing evaluation of a
   *  throw-expression until either completing initialization of the
   *  exception-declaration in the matching handler or entering @c unexpected()
   *  due to the throw; or after entering @c terminate() for any reason
   *  other than an explicit call to @c terminate().  [Note: This includes
   *  stack unwinding [15.2].  end note]'
   *
   *  2: 'When @c uncaught_exception() is true, throwing an
   *  %exception can result in a call of @c terminate()
   *  (15.5.1).'
   */
  _GLIBCXX17_DEPRECATED
  bool uncaught_exception() _GLIBCXX_USE_NOEXCEPT __attribute__((__pure__));

#if __cplusplus >= 201703L || !defined(__STRICT_ANSI__) // c++17 or gnu++98
#define __cpp_lib_uncaught_exceptions 201411L
  /// The number of uncaught exceptions.
  int uncaught_exceptions() _GLIBCXX_USE_NOEXCEPT __attribute__((__pure__));
#endif

  // @} group exceptions
} // namespace std

namespace __gnu_cxx {
  _GLIBCXX_BEGIN_NAMESPACE_VERSION

  /**
   *  @brief A replacement for the standard terminate_handler which
   *  prints more information about the terminating exception (if any)
   *  on stderr.
   *
   *  @ingroup exceptions
   *
   *  Call
   *   @code
   *     std::set_terminate(__gnu_cxx::__verbose_terminate_handler)
   *   @endcode
   *  to use.  For more info, see
   *  http://gcc.gnu.org/onlinedocs/libstdc++/manual/bk01pt02ch06s02.html
   *
   *  In 3.4 and later, this is on by default.
   */
  void __verbose_terminate_handler();

  _GLIBCXX_END_NAMESPACE_VERSION
} // namespace __gnu_cxx

} // extern "C++"

#pragma GCC visibility pop

#if (__cplusplus >= 201103L)
#include "libcxx/gcc/bits/exception_ptr.hh"
#include "libcxx/gcc/bits/nested_exception.hh"
#endif

#endif
