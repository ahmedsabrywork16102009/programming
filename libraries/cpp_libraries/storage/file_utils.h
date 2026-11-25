#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace storage {

	// =============================================================
	//  File Operation Result
	// =============================================================
	struct stFileResult {
		bool succeeded;
		std::string message;
	};

	inline stFileResult makeFileSuccess() { return {true, {}}; }

	inline stFileResult makeFileFailure(std::string_view action, std::string_view fileName) { return {false, std::string(action) + ": " + std::string(fileName)}; }

	inline void reportFileError(const stFileResult &result) {
		if (!result.succeeded) {
			std::cerr << "File error - " << result.message << "\n";
		}
	}

	// =============================================================
	//  Checked Operations
	// =============================================================
	inline stFileResult appendLineToFileChecked(std::string_view fileName, std::string_view lineData) {
		const std::string path(fileName);
		std::ofstream file(path, std::ios::out | std::ios::app);
		if (!file.is_open()) {
			return makeFileFailure("Could not open file for appending", path);
		}

		file << lineData << '\n';
		file.flush();
		if (!file) {
			return makeFileFailure("Could not write to file", path);
		}

		file.close();
		if (!file) {
			return makeFileFailure("Could not close file after appending", path);
		}

		return makeFileSuccess();
	}

	inline stFileResult writeLinesToFileChecked(std::string_view fileName, const std::vector<std::string> &vLines) {
		const std::string path(fileName);
		std::ofstream file(path, std::ios::out | std::ios::trunc);
		if (!file.is_open()) {
			return makeFileFailure("Could not open file for writing", path);
		}

		for (const std::string &line : vLines) {
			file << line << '\n';
			if (!file) {
				return makeFileFailure("Could not write to file", path);
			}
		}

		file.flush();
		if (!file) {
			return makeFileFailure("Could not finish writing file", path);
		}

		file.close();
		if (!file) {
			return makeFileFailure("Could not close file after writing", path);
		}

		return makeFileSuccess();
	}

	inline stFileResult loadLinesFromFileChecked(std::string_view fileName, std::vector<std::string> &vLines, bool includeEmptyLines = false) {
		vLines.clear();
		const std::string path(fileName);
		std::ifstream file(path);
		if (!file.is_open()) {
			return makeFileFailure("Could not open file for reading", path);
		}

		std::string line;
		while (std::getline(file, line)) {
			if (includeEmptyLines || !line.empty()) {
				vLines.push_back(line);
			}
		}

		if (file.bad()) {
			vLines.clear();
			return makeFileFailure("Could not finish reading file", path);
		}

		return makeFileSuccess();
	}

	// =============================================================
	//  Backward-Compatible Operations
	// =============================================================
	inline bool appendLineToFile(std::string_view fileName, std::string_view lineData) {
		const stFileResult result = appendLineToFileChecked(fileName, lineData);
		reportFileError(result);
		return result.succeeded;
	}

	inline bool writeLinesToFile(std::string_view fileName, const std::vector<std::string> &vLines) {
		const stFileResult result = writeLinesToFileChecked(fileName, vLines);
		reportFileError(result);
		return result.succeeded;
	}

	inline std::vector<std::string> loadLinesFromFile(std::string_view fileName) {
		std::vector<std::string> vLines;
		const stFileResult result = loadLinesFromFileChecked(fileName, vLines);
		reportFileError(result);
		return vLines;
	}

}  // namespace storage
