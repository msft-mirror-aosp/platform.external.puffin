// Copyright 2017 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LOGGING_H_
#define SRC_LOGGING_H_

#if defined(BASE_VER) && BASE_VER >= 822064
#include "base/check.h"  // CHECK-related macros are defined in base/check.h on Chrome OS.
#include "base/logging.h"
#elif USE_BRILLO
#include "base/logging.h"
#else
#include "glog/logging.h"
#endif

#define TEST_OP(_x, _y, op)                                                \
  do {                                                                     \
    const auto& x = _x;                                                    \
    const auto& y = _y;                                                    \
    if (!(x op y)) {                                                       \
      LOG(ERROR) << #_x " " #op " " #_y << " failed: " << x << " " #op " " \
                 << y;                                                     \
      return {};                                                           \
    }                                                                      \
  } while (0)

#define TEST_EQ(_x, _y) TEST_OP(_x, _y, ==)
#define TEST_NE(_x, _y) TEST_OP(_x, _y, !=)
#define TEST_LE(_x, _y) TEST_OP(_x, _y, <=)
#define TEST_GE(_x, _y) TEST_OP(_x, _y, >=)
#define TEST_LT(_x, _y) TEST_OP(_x, _y, <)
#define TEST_GT(_x, _y) TEST_OP(_x, _y, >)

#define TEST_AND_RETURN_FALSE(_x)   \
  do {                              \
    if (!(_x)) {                    \
      LOG(ERROR) << #_x " failed."; \
      return false;                 \
    }                               \
  } while (0)

#define TEST_AND_RETURN_VALUE(_x, _v) \
  do {                                \
    if (!(_x)) {                      \
      LOG(ERROR) << #_x " failed.";   \
      return (_v);                    \
    }                                 \
  } while (0)

#endif  // SRC_LOGGING_H_
