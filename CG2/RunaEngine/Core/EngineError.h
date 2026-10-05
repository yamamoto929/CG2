#pragma once
#include "Log.h"
#include <Windows.h>
#include <format>
#include <stdexcept>
#include <string>

// Releaseでも検査を消さず、原因をログに残して呼び出し元へ知らせる。
inline void Require(bool condition, const std::string& message) {
    if (!condition) {
        Log("ERROR: " + message);
        throw std::runtime_error(message);
    }
}

inline void CheckHR(HRESULT result, const std::string& operation) {
    Require(SUCCEEDED(result), std::format("{} (HRESULT=0x{:08X})",
        operation, static_cast<unsigned long>(result)));
}
