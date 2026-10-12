// WARNING! All changes made in this file will be lost!
// Created from 'test_user.style' by 'codegen_style'
//
// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "style_test_user.h"

#include "styles/palette.h"
#include "styles/style_test_base.h"
#include "ui/style/style_core_design.h"

namespace {

bool inited = false;

class Module_style_test_user : public style::internal::ModuleBase {
public:
	Module_style_test_user() { style::internal::registerModule(this); }

	void start(int scale) override {
		style::internal::init_style_test_user(scale);
	}
};
Module_style_test_user registrator;

style::margins _testUserPadding = { 0, 0, 0, 0 };
int _testUserSkip = 0;
int _testUserGap = 0;
int _testUserDuration = 200;
style::TestButton _testUserButton = { 0, 0, { 0, 0, 0, 0 }, 0, 0, { Qt::Uninitialized }, { Qt::Uninitialized } };
style::size _testUserSize = { 0, 0 };

} // namespace

namespace st {
const style::margins &testUserPadding(_testUserPadding);
const int &testUserSkip(_testUserSkip);
const int &testUserGap(_testUserGap);
const int &testUserDuration(_testUserDuration);
const style::TestButton &testUserButton(_testUserButton);
const style::size &testUserSize(_testUserSize);
} // namespace st

namespace style {
namespace internal {

void init_style_test_user(int scale) {
	if (inited) return;
	inited = true;

	init_style_test_base(scale);

	const int px5 = ConvertScale(5, scale);
	const int px6 = ConvertScale(6, scale);
	const int px8 = ConvertScale(8, scale);
	const int px10 = ConvertScale(10, scale);
	const int px11 = ConvertScale(11, scale);
	const int px12 = ConvertScale(12, scale);
	const int px13 = ConvertScale(13, scale);
	const int px20 = ConvertScale(20, scale);
	const int px48 = ConvertScale(48, scale);
	const int px120 = ConvertScale(120, scale);

	_testUserPadding = st::testPadding;
	if (style::IsMaterial()) {
		_testUserSkip = px13;
		_testUserGap = px12;
		_testUserDuration = 250;
		_testUserButton = { px120, px48, { px12, px6, px12, px6 }, st::testRadius, 150, st::windowBgActive, st::testArrow };
	} else {
		_testUserSkip = px11;
		_testUserGap = px8;
		_testUserDuration = 200;
		_testUserButton = { px120, px48, { px10, px5, px10, px5 }, st::testRadius, 150, st::windowFg, st::testArrow };
	}
	_testUserSize = { px20, px20 };
}

} // namespace internal
} // namespace style
