// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <vector>
#include "codegen/style/structure_types.h"

namespace codegen {
namespace style {
namespace structure {

class Module {
public:

	explicit Module(const QString &fullpath);
	Module(const Module &other) = delete;
	Module &operator=(const Module &other) = delete;

	QString filepath() const {
		return fullpath_;
	}

	void addIncluded(std::shared_ptr<const Module> value);

	bool hasIncludes() const {
		return !included_.empty();
	}
	template <typename F>
	bool enumIncludes(F functor) const {
		for (const auto &module : included_) {
			if (!functor(*module)) {
				return false;
			}
		}
		return true;
	}

	// Returns false if there is a struct with such name already.
	bool addStruct(const Struct &value);
	// Returns nullptr if there is no such struct in result_ or any of included modules.
	const Struct *findStruct(const FullName &name) const;
	bool hasStructs() const {
		return !structs_.isEmpty();
	}

	template <typename F>
	bool enumStructs(F functor) const {
		for (const auto &value : structs_) {
			if (!functor(value)) {
				return false;
			}
		}
		return true;
	}

	// Returns false if there is a variable with such name already.
	bool addVariable(const Variable &value);
	// Returns nullptr if there is no such variable in result_ or any of included modules.
	const Variable *findVariable(const FullName &name, bool *outFromThisModule = nullptr) const;
	bool hasVariables() const {
		return !variables_.isEmpty();
	}
	int variablesCount() const {
		return variables_.size();
	}

	template <typename F>
	bool enumVariables(F functor) const {
		for (const auto &value : variables_) {
			if (!functor(value)) {
				return false;
			}
		}
		return true;
	}

	explicit operator bool() const {
		return !fullpath_.isEmpty();
	}

	static const Struct *findStructInModule(const FullName &name, const Module &module);
	static const Variable *findVariableInModule(const FullName &name, const Module &module);

private:
	// This module and every module it includes, directly or not, each once,
	// in the order a depth-first search through the includes reaches them.
	// Looking names up in it finds what the recursive search would find,
	// without walking shared includes again for every path to them.
	const std::vector<const Module*> &lookupOrder() const;

	QString fullpath_;
	std::vector<std::shared_ptr<const Module>> included_;
	mutable std::vector<const Module*> lookupOrder_;
	QList<Struct> structs_;
	QList<Variable> variables_;
	QMap<QString, int> structsByName_;
	QMap<QString, int> variablesByName_;

};

} // namespace structure
} // namespace style
} // namespace codegen
