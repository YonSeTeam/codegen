// WARNING! All changes made in this file will be lost!
// Created from 'test_plain.style' by 'codegen_style'
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

void init_style_test_plain(int scale);

} // namespace internal

struct TestPlainBox {
	style::margins margin;
	style::color fg;
};

} // namespace style

namespace st {
extern const int &testPlainSkip;
constexpr int testPlainCount = 3;
extern const style::font &testPlainFont;
extern const style::icon &testPlainCheck;
extern const style::TestPlainBox &testPlainBox;
} // namespace st
