// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "codegen/style/processor.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include "codegen/common/cpp_file.h"
#include "codegen/style/parsed_file.h"
#include "codegen/style/generator.h"

namespace codegen {
namespace style {
namespace {

constexpr int kErrorCantWritePath = 821;
constexpr int kErrorMaterial     = 822;

QString destFileBaseName(const structure::Module &module) {
	return "style_" + QFileInfo(module.filepath()).baseName();
}

// Material overrides only change values, so both parses must give
// the same variables of the same types.
bool sameVariables(
		const structure::Module &module,
		const structure::Module &material) {
	if (module.variablesCount() != material.variablesCount()) {
		return false;
	}
	return module.enumVariables([&](const structure::Variable &variable) {
		const auto other = structure::Module::findVariableInModule(
			variable.name,
			material);
		return other && (other->value.type() == variable.value.type());
	});
}

} // namespace

Processor::Processor(const Options &options)
: options_(options) {
}

int Processor::launch() {
	auto cache = std::map<QString, std::shared_ptr<const structure::Module>>();
	auto materialCache = cache;
	auto materialOptions = options_;
	materialOptions.material = true;
	for (auto i = 0; i != options_.inputPaths.size(); ++i) {
		auto parser = ParsedFile(cache, options_, i);
		if (!parser.read()) {
			return -1;
		}

		const auto module = parser.getResult();
		auto material = std::unique_ptr<const structure::Module>();
		if (!options_.isPalette) {
			auto materialParser = ParsedFile(materialCache, materialOptions, i);
			if (!materialParser.read()) {
				common::logError(kErrorMaterial, module->filepath())
					<< "could not apply Material overrides and icons.";
				return -1;
			}
			material = materialParser.getResult();
			if (!sameVariables(*module, *material)) {
				common::logError(kErrorMaterial, module->filepath())
					<< "Material variables differ from the module ones.";
				return -1;
			}
		}
		if (!write(*module, material.get())) {
			return -1;
		}
	}
	if (!common::TouchTimestamp(options_.timestampPath)) {
		return -1;
	}
	return 0;
}

bool Processor::write(
		const structure::Module &module,
		const structure::Module *material) const {
	bool forceReGenerate = false;
	QDir dir(options_.outputPath);
	if (!dir.mkpath(".")) {
		common::logError(kErrorCantWritePath, "Command Line") << "can not open path for writing: " << dir.absolutePath().toStdString();
		return false;
	}

	QFileInfo srcFile(module.filepath());
	QString dstFilePath = dir.absolutePath() + '/' + (options_.isPalette ? "palette" : destFileBaseName(module));

	common::ProjectInfo project = {
		"codegen_style",
		srcFile.fileName(),
		forceReGenerate
	};

	Generator generator(
		module,
		material,
		dstFilePath,
		project,
		options_.isPalette);
	if (!generator.writeHeader() || !generator.writeSource()) {
		return false;
	}
	return true;
}

Processor::~Processor() = default;

} // namespace style
} // namespace codegen
