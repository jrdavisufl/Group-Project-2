#pragma once

#include <iostream>
#include <string>

// valid error codes
enum class ErrorType {
    Empty,
    Exception,
    InvalidInput,
    NotFound,
    OutOfBounds,
    DuplicateValue
};

// Error class returned by all outward facing methods that have a chance at failing
class Error {
    public:
        Error(ErrorType type, std::string message = "");

        // equals operators for ease of testing
        bool operator==(const Error& other) const;
        bool operator==(const std::string& message) const;
        bool operator==(const ErrorType& type) const;

        // to string for printing to console
        std::string ToString() const;
        friend std::ostream& operator<<(std::ostream& os, const Error& p);

    private:
        static std::string ErrorTypeToString(ErrorType type);

        std::string message;
        ErrorType type;
};