// WARNING! All changes made in this file will be lost!
// Created from 'test_user.style' by 'codegen_style'
//
// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/style/style_core.h"

namespace style {
namespace internal {

void init_style_test_user(int scale);

} // namespace internal

struct TestButton;

} // namespace style

namespace st {
extern const style::margins &testUserPadding;
extern const int &testUserSkip;
extern const int &testUserGap;
extern const int &testUserDuration;
extern const style::TestButton &testUserButton;
extern const style::size &testUserSize;
} // namespace st
