#pragma once
#include <stdio.h>
#include <cstring>

class File
{
private:
    FILE *fileptr;
    size_t size;
    char filepath[256];

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

        memset(filepath, 0, sizeof(filepath));
        strncpy(filepath, path, sizeof(filepath) - 1);
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
        memset(filepath, 0, sizeof(filepath));
    }

    enum
    {
        READ = 0,
        WRITE = 1,
    };

    File(const File &other)
    {
        fileptr = fopen(other.filepath, "wb+");
        size = other.size;
        memcpy(seek, other.seek, sizeof(seek));
        memcpy(old_seek, other.old_seek, sizeof(other.old_seek));
        memcpy(count, other.count, sizeof(count));
        strncpy(filepath, other.filepath, sizeof(filepath));
    }

    File& operator=(const File& other)
    {
        if (this != &other)
        {
            if (fileptr != nullptr)
                fclose(fileptr);

            fileptr = fopen(other.filepath, "wb+");
            size = other.size;
            memcpy(seek, other.seek, sizeof(seek));
            memcpy(old_seek, other.old_seek, sizeof(other.old_seek));
            memcpy(count, other.count, sizeof(count));
            strncpy(filepath, other.filepath, sizeof(filepath));
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
        strncpy(filepath, other.filepath, sizeof(filepath));

        other.fileptr = nullptr;
        other.size = 0;
        memset(other.seek, 0, sizeof(other.seek));
        memset(other.old_seek, 0, sizeof(other.old_seek));
        memset(other.count, 0, sizeof(other.count));
        memset(other.filepath, 0, sizeof(other.filepath));
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
            strncpy(filepath, other.filepath, sizeof(filepath));

            other.fileptr = nullptr;
            other.size = 0;
            memset(other.seek, 0, sizeof(other.seek));
            memset(other.old_seek, 0, sizeof(other.old_seek));
            memset(other.count, 0, sizeof(other.count));
            memset(other.filepath, 0, sizeof(other.filepath));
        }
        return *this;
    }

    inline constexpr size_t GetSize() const {
        return size;
    }

    inline constexpr const char* GetPath() const {
        return filepath;
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
        fileptr = fopen(filepath, "wb+");
    }

    inline constexpr bool IsValid() const {
        return fileptr != nullptr;
    }

    template<typename T>
    constexpr size_t Read(const T *buffer, size_t elements = 1, ssize_t element_offset = 0) {
        seek[READ] += element_offset * sizeof(T);
        fseek(fileptr, seek[READ], SEEK_SET);

        size_t bytes_read = fread((void*)buffer, sizeof(T), elements, fileptr);

        seek[READ] += bytes_read;
        count[READ] += bytes_read;
        return bytes_read;
    };

    template<typename T>
    constexpr size_t Write(const T *buffer, size_t elements = 1, ssize_t element_offset = 0) {
        seek[WRITE] += element_offset * sizeof(T);
        fseek(fileptr, seek[WRITE], SEEK_SET);

        size_t bytes_written = fwrite((void*)buffer, sizeof(T), elements, fileptr);

        seek[WRITE] += bytes_written;
        count[WRITE] += bytes_written;
        return bytes_written;
    };

    typedef char (*transform_t)(char, unsigned int, size_t);
    // transformer_func is a function pointer (can be nullptr/NULL) which takes in as an input the character, its index inside the block, and the index of its block, and outputs a character.
    // A return value of 0 indicates the function worked properly.
    int Backup(const char *__restrict__ append, const size_t block_size = BUFSIZ, transform_t transformer_func = nullptr) {
        char *buffer = new char[block_size]{};
        strncpy(buffer, filepath, block_size);
        strncat(buffer, append, block_size - strlen(filepath));
        if (strcmp(buffer, filepath) == 0)
            return 1;
        FILE *backup = fopen(buffer, "wb");

        size_t transfered = 0;
        if (transformer_func == nullptr)
        {
            while ((transfered = fread(buffer, sizeof(char), block_size, fileptr)) > 0)
            {
                fwrite(buffer, transfered, sizeof(char), backup);
            }
        }
        else
        {
            size_t index = 0;
            while ((transfered = fread(buffer, sizeof(char), block_size, fileptr)) > 0)
            {
                for (size_t i = 0; i < transfered; i++)
                {
                    buffer[i] = transformer_func(buffer[i], i, index);
                }
                index++;
                fwrite(buffer, transfered, sizeof(char), backup);
            }
        }

        fclose(backup);
        delete[] buffer;
        return 0;
    }
};