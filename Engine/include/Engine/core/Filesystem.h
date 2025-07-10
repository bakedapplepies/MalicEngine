#pragma once

#include <string>
#include <filesystem>

#include "Engine/core/Defines.h"

MLC_NAMESPACE_START

class File
{
public:
    explicit File(const std::filesystem::path& path);
    ~File() = default;
    File(const File& file) = default;
    File& operator=(const File& file) = default;
    File(File&& file) noexcept = default;
    File& operator=(File&& file) noexcept = default;

    MLC_NODISCARD const char* GetPath() const;

private:
    std::string m_path;
};

MLC_NAMESPACE_END