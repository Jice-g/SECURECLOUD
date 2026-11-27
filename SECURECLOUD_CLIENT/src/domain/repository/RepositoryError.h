#pragma once

#include <stdexcept>

/// @file   RepositoryError.h
/// @brief  Technical failure raised by a repository implementation.

namespace domain {

/// @brief Thrown by an I...Repository when storage itself fails (disk full,
///        corrupted database, I/O error...).
/// @note  "Not found" is NOT an error: find...() returns std::optional / nullptr.
///        The Infrastructure layer converts SQLite/Qt errors into this type,
///        so that the Domain and Application layers never see a vendor exception.
class RepositoryError : public std::runtime_error {
	// simple and easier to manage the errors thrown thrue the program
public:
    using std::runtime_error::runtime_error;
};

} // namespace domain
