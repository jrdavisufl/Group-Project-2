#include <iostream>
#include <sstream>
#include <stdexcept>

#include "error.hpp"

Error::Error(ErrorType type, std::string message) {
    this->message = message;
    this->type = type;
}

// checks if all fields are equal
bool Error::operator==(const Error& other) const {
    return this->type == other.type && this->message == other.message;
}

// checks if just the message is equal
bool Error::operator==(const std::string& message) const {
    return this->message == message;
}

// checks if just the type is equal
bool Error::operator==(const ErrorType& type) const {
    return this->type == type;
}

std::string Error::ToString() const {
    std::string outgoing = Error::ErrorTypeToString(this->type);
    if (this->message != "") {
        outgoing = outgoing + ": " + this->message;
    }

    return outgoing;
}

// operator override so error class can be passed into cout stream
std::ostream& operator<<(std::ostream& os, const Error& e) {
    os << e.ToString();
    return os;
}

std::string Error::ErrorTypeToString(ErrorType type) {
    static constexpr std::string_view names[] = {
        "Empty",
        "InvalidInput",
        "NotFound",
        "OutOfBounds",
        "DuplicateValue"
    };

    auto index = static_cast<size_t>(type);
    if (index < std::size(names)) {
        return std::string(names[index]);
    }
    
    return "UnknownError";
}
