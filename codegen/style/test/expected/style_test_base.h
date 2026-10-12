// WARNING! All changes made in this file will be lost!
// Created from 'test_base.style' by 'codegen_style'
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

void init_style_test_base(int scale);

} // namespace internal

struct TestButton {
	int width;
	int height;
	style::margins padding;
	int radius;
	int duration;
	style::color textFg;
	style::icon icon;
};

} // namespace style

namespace st {
extern const int &testRadius;
extern const int &testDuration;
constexpr double testRatio = 0.5;
extern const style::margins &testPadding;
extern const style::icon &testArrow;
extern const style::icon &testClose;
extern const style::TestButton &testButton;
extern const style::TestButton &testButtonBig;
extern const style::TestButton &testButtonCopy;
} // namespace st
