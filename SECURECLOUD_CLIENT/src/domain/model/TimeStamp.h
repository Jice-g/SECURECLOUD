#pragma once

#include <chrono>
#include <compare> // for three-way comparison : provides ==, !=, <, <=, >, >=

namespace domain {

/// @brief Value Object representing a point in time.
/// @details Encapsulates a std::chrono::system_clock::time_point
/// in order to provide a domain-specific type for TimeStamps.
///
class TimeStamp final {

private:

    std::chrono::system_clock::time_point m_value;

public:

    /// @brief Constructs a TimeStamp.
	/// @param value: The time point at the moment of creation.
	/// defaults to the current system time if no argument is provided.
    TimeStamp(std::chrono::system_clock::time_point value = std::chrono::system_clock::now())
        : m_value(value)
    {  }

    /// @brief Returns a TimeStamp representing the current system time.
    /// this static function may be used to get the current time without creating a TimeStamp object first.
    /// usefull all along the program
    static TimeStamp now()
    {  return TimeStamp(std::chrono::system_clock::now()); }


	/// @brief gets the time_point value.
    /// @returns a reference to the TimeStamp value.
    const std::chrono::system_clock::time_point& value() const noexcept
    {  return m_value;  }


    /// @brief Defaulted three-way comparison (see <compare> library)
    auto operator<=>(const TimeStamp&) const noexcept = default;
    bool operator==(const TimeStamp&) const noexcept = default;
};

}