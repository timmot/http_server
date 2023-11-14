// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include "Bytes.hpp"
#include <fstream>
#include <string>
#include <string_view>

class File {
public:
    static std::optional<std::unique_ptr<File>> open(std::string_view path, std::string_view mode = "rb")
    {
        auto file_handle = fopen(path.data(), mode.data());

        if (file_handle)
            return std::make_unique<File>(file_handle);

        return {};
    }

    File(FILE* file_handle)
        : m_file_handle(file_handle)
    {
    }

    bool is_open() const
    {
        return m_file_handle != nullptr;
    }

    Bytes read(size_t size) const
    {
        Bytes buffer(size);
        fread(buffer.data(), size, 1, m_file_handle);
        return buffer;
    }

    Bytes read_all() const
    {
        fseek(m_file_handle, 0, SEEK_END);
        auto size = ftell(m_file_handle);
        rewind(m_file_handle);

        Bytes buffer(size);
        fread(buffer.data(), size, 1, m_file_handle);
        return buffer;
    }

    ~File()
    {
        if (m_file_handle)
            fclose(m_file_handle);
    }

private:
    FILE* m_file_handle = nullptr;
};
