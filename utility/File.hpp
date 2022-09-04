// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include "Bytes.hpp"
#include <fstream>
#include <string>
#include <string_view>

class File {
public:
    File(std::string_view path, std::string const& mode = "rb")
        : file_handle(fopen(std::string(path).c_str(), mode.c_str()))
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
        fclose(file_handle);
    }

private:
    FILE* file_handle;
};