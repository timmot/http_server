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
        : file_handle(file_handle)
    {
    }

    bool is_open() const
    {
        return file_handle != nullptr;
    }

    Bytes read(size_t size) const
    {
        Bytes buffer(size);
        fread(buffer.data(), size, 1, file_handle);
        return buffer;
    }

    Bytes read_all() const
    {
        fseek(file_handle, 0, SEEK_END);
        auto size = ftell(file_handle);
        rewind(file_handle);

        Bytes buffer(size);
        fread(buffer.data(), size, 1, file_handle);
        return buffer;
    }

    ~File()
    {
        if (file_handle)
            fclose(file_handle);
    }

private:
    FILE* file_handle = nullptr;
};
