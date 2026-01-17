#pragma once

// Lightweight assertion utilities usable in both Debug and Release builds.
// Features:
// - Triggers in Release as well (throws std::runtime_error after reporting)
// - Two reporting modes: console-only and GUI dialog (Windows MessageBox)
// - Allows caller to supply custom message text
// - Header-only; no external dependencies required
//
// Usage examples (do NOT auto-insert into project; opt-in where needed):
//   FV_ASSERT(x > 0);
//   FV_ASSERT_MSG(x > 0, "x must be positive");
//   FV_ASSERT_GUI_MSG(ptr != nullptr, "ptr should not be null");
//
// Notes:
// - These macros do not rely on NDEBUG; they are active in all builds.
// - GUI dialog is only available on Windows; on other platforms it falls back
// to console reporting.

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace fv_assert {
inline std::string make_message(const char *expr, const char *file, int line,
                                const char *func, const std::string &userMsg) {
  std::ostringstream oss;
  oss << "Assertion failed\n"
      << "  Expression: " << (expr ? expr : "<none>") << "\n"
      << "  Location  : " << (file ? file : "<unknown>") << ':' << line << "\n";
  if (func && *func)
    oss << "  Function  : " << func << "\n";
  if (!userMsg.empty())
    oss << "  Message   : " << userMsg << "\n";
  return oss.str();
}

inline void report_and_throw(const char *expr, const char *file, int line,
                             const char *func, const std::string &userMsg,
                             bool useGui) {
  const std::string full = make_message(expr, file, line, func, userMsg);

  // Console/stderr output
  std::fwrite(full.c_str(), 1, full.size(), stderr);
  std::fflush(stderr);

  // Optional GUI dialog on Windows
#if defined(_WIN32)
  if (useGui) {
    MessageBoxA(nullptr, full.c_str(), "Assertion Failed",
                MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
  }
#else
  (void)useGui; // suppress unused warning on non-Windows
#endif

  // Always throw to ensure it triggers in release as well
  throw std::runtime_error(full);
}
} // namespace fv_assert

// Internal helper to select the right function signature across compilers
#if defined(__GNUC__) || defined(__clang__)
#define FV_ASSERT_FUNC __PRETTY_FUNCTION__
#else
#define FV_ASSERT_FUNC __func__
#endif

// Public macros
// Base macro: no message, console-only
#define FV_ASSERT(cond)                                                        \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fv_assert::report_and_throw(#cond, __FILE__, __LINE__, FV_ASSERT_FUNC,   \
                                  std::string(), false);                       \
    }                                                                          \
  } while (0)

// With custom message, console-only
#define FV_ASSERT_MSG(cond, msg)                                               \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fv_assert::report_and_throw(#cond, __FILE__, __LINE__, FV_ASSERT_FUNC,   \
                                  std::string(msg), false);                    \
    }                                                                          \
  } while (0)

// Base macro: no message, GUI dialog (Windows). On non-Windows it behaves like
// console-only
#define FV_ASSERT_GUI(cond)                                                    \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fv_assert::report_and_throw(#cond, __FILE__, __LINE__, FV_ASSERT_FUNC,   \
                                  std::string(), true);                        \
    }                                                                          \
  } while (0)

// With custom message, GUI dialog (Windows). On non-Windows it behaves like
// console-only
#define FV_ASSERT_GUI_MSG(cond, msg)                                           \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fv_assert::report_and_throw(#cond, __FILE__, __LINE__, FV_ASSERT_FUNC,   \
                                  std::string(msg), true);                     \
    }                                                                          \
  } while (0)

// Note: Do NOT undefine FV_ASSERT_FUNC here; it must remain defined for macro
// expansion sites that include this header.
