// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "codegen/style/module.h"

#include <algorithm>

namespace codegen {
namespace style {
namespace structure {
namespace {

QString fullNameKey(const FullName &name) {
	return name.join('.');
}

} // namespace

Module::Module(const QString &fullpath) : fullpath_(fullpath) {
}

void Module::addIncluded(std::shared_ptr<const Module> value) {
	included_.push_back(std::move(value));
	lookupOrder_.clear();
}

const std::vector<const Module*> &Module::lookupOrder() const {
	if (lookupOrder_.empty()) {
		lookupOrder_.push_back(this);
		for (const auto &module : included_) {
			for (const auto other : module->lookupOrder()) {
				const auto i = std::find(
					lookupOrder_.begin(),
					lookupOrder_.end(),
					other);
				if (i == lookupOrder_.end()) {
					lookupOrder_.push_back(other);
				}
			}
		}
	}
	return lookupOrder_;
}

bool Module::addStruct(const Struct &value) {
	if (findStruct(value.name)) {
		return false;
	}
	structsByName_.insert(fullNameKey(value.name), structs_.size());
	structs_.push_back(value);
	return true;
}

const Struct *Module::findStruct(const FullName &name) const {
	const auto key = fullNameKey(name);
	for (const auto module : lookupOrder()) {
		const auto index = module->structsByName_.value(key, -1);
		if (index >= 0) {
			return &module->structs_.at(index);
		}
	}
	return nullptr;
}

bool Module::addVariable(const Variable &value) {
	if (findVariable(value.name)) {
		return false;
	}
	variablesByName_.insert(fullNameKey(value.name), variables_.size());
	variables_.push_back(value);
	return true;
}

const Variable *Module::findVariable(const FullName &name, bool *outFromThisModule) const {
	const auto key = fullNameKey(name);
	for (const auto module : lookupOrder()) {
		const auto index = module->variablesByName_.value(key, -1);
		if (index >= 0) {
			if (outFromThisModule) *outFromThisModule = (module == this);
			return &module->variables_.at(index);
		}
	}
	return nullptr;
}

const Struct *Module::findStructInModule(const FullName &name, const Module &module) {
	auto index = module.structsByName_.value(fullNameKey(name), -1);
	if (index < 0) {
		return nullptr;
	}
	return &module.structs_.at(index);
}

const Variable *Module::findVariableInModule(const FullName &name, const Module &module) {
	auto index = module.variablesByName_.value(fullNameKey(name), -1);
	if (index < 0) {
		return nullptr;
	}
	return &module.variables_.at(index);
}

} // namespace structure
} // namespace style
} // namespace codegen
