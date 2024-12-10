#pragma once
#include <stdio.h>
#include <vector>
#include <cstring>

class File
{
private:
    FILE *fileptr;
    size_t size;
    char file_path[256];

protected:
    size_t count[2]; // Data-read and Data-written
    ssize_t seek[2]; // Read-seek and Write-seek
    ssize_t old_seek[2]; // For RevertReadPtr() and RevertWritePtr()

public:
    // File is always opened in read and write mode
    File(const char *__restrict__ path)
    {
        fileptr = fopen(path, "wb+");
        if (fileptr == nullptr)
            return;

        fseek(fileptr, 0, SEEK_END);
        size = ftell(fileptr);
        fseek(fileptr, 0, SEEK_SET);

        memset(file_path, 0, sizeof(file_path));
        strncpy(file_path, path, sizeof(file_path) - 1);
    }
    virtual ~File()
    {
        if (fileptr != nullptr)
            fclose(fileptr);
        fileptr = nullptr;
        size = 0;
        memset(seek, 0, sizeof(seek));
        memset(old_seek, 0, sizeof(old_seek));
        memset(count, 0, sizeof(count));
        memset(file_path, 0, sizeof(file_path));
    }

    enum
    {
        READ = 0,
        WRITE = 1,
    };

    File(const File &other)
    {
        fileptr = fopen(other.file_path, "wb+");
        size = other.size;
        memcpy(seek, other.seek, sizeof(seek));
        memcpy(old_seek, other.old_seek, sizeof(other.old_seek));
        memcpy(count, other.count, sizeof(count));
        strncpy(file_path, other.file_path, sizeof(file_path));
    }

    File& operator=(const File& other)
    {
        if (this != &other)
        {
            if (fileptr != nullptr)
                fclose(fileptr);

            fileptr = fopen(other.file_path, "wb+");
            size = other.size;
            memcpy(seek, other.seek, sizeof(seek));
            memcpy(old_seek, other.old_seek, sizeof(other.old_seek));
            memcpy(count, other.count, sizeof(count));
            strncpy(file_path, other.file_path, sizeof(file_path));
        }
        return *this;
    }

    File(File&& other) noexcept
    {
        fileptr = other.fileptr;
        size = other.size;
        memcpy(seek, other.seek, sizeof(seek));
        memcpy(old_seek, other.old_seek, sizeof(old_seek));
        memcpy(count, other.count, sizeof(count));
        strncpy(file_path, other.file_path, sizeof(file_path));

        other.fileptr = nullptr;
        other.size = 0;
        memset(other.seek, 0, sizeof(other.seek));
        memset(other.old_seek, 0, sizeof(other.old_seek));
        memset(other.count, 0, sizeof(other.count));
        memset(other.file_path, 0, sizeof(other.file_path));
    }

    File& operator=(File&& other) noexcept
    {
        if (this != &other)
        {
            if (fileptr != nullptr)
                fclose(fileptr);

            fileptr = other.fileptr;
            size = other.size;
            memcpy(seek, other.seek, sizeof(seek));
            memcpy(old_seek, other.old_seek, sizeof(other.old_seek));
            memcpy(count, other.count, sizeof(count));
            strncpy(file_path, other.file_path, sizeof(file_path));

            other.fileptr = nullptr;
            other.size = 0;
            memset(other.seek, 0, sizeof(other.seek));
            memset(other.old_seek, 0, sizeof(other.old_seek));
            memset(other.count, 0, sizeof(other.count));
            memset(other.file_path, 0, sizeof(other.file_path));
        }
        return *this;
    }

    inline constexpr size_t GetSize() const {
        return size;
    }

    inline constexpr const char* GetPath() const {
        return file_path;
    }

    inline constexpr ssize_t GetReadPtr() const {
        return seek[READ];
    }

    inline constexpr ssize_t GetWritePtr() const {
        return seek[WRITE];
    }

    inline constexpr void SetReadPtr(ssize_t offset) {
        old_seek[READ] = seek[READ];
        seek[READ] = offset;
    }

    inline constexpr void SetWritePtr(ssize_t offset) {
        old_seek[WRITE] = seek[WRITE];
        seek[WRITE] = offset;
    }

    inline constexpr size_t GetReadBytes() const {
        return count[READ];
    }

    inline constexpr size_t GetWrittenBytes() const {
        return count[WRITE];
    }

    inline constexpr void RevertReadPtr() {
        seek[READ] = old_seek[READ];
    }

    inline constexpr void RevertWritePtr() {
        seek[WRITE] = old_seek[WRITE];
    }

    inline void Update() {
        fclose(fileptr);
        fileptr = fopen(file_path, "wb+");
    }

    inline constexpr bool IsValid() const {
        return fileptr != nullptr;
    }

    template<typename T>
    constexpr size_t Read(const T& buffer, size_t elements = 1, ssize_t element_offset = 0) {
        seek[READ] += element_offset * sizeof(T);
        fseek(fileptr, seek[READ], SEEK_SET);

        size_t bytes_written = fread(buffer, sizeof(T), elements, fileptr);

        seek[READ] += bytes_written;
        count[READ] += bytes_written;
        return bytes_written;
    };

    template<typename T>
    constexpr size_t Write(const T& buffer, size_t elements = 1, ssize_t element_offset = 0) {
        seek[WRITE] += element_offset;
        fseek(fileptr, seek[WRITE], SEEK_SET);

        size_t bytes_written = fwrite(buffer, sizeof(T), elements, fileptr);

        seek[WRITE] += bytes_written;
        count[WRITE] += bytes_written;
        return bytes_written;
    };

};