#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#define NULLIFYSTACK(array) \
    memset(array, 0, sizeof(array))
#define COPYSTACK(array, other) \
    memcpy(array, other, sizeof(array))
#define STRSTACKCPY(str, other) \
    strncpy(str, other, sizeof(str) - 1)

class File
{
private:
    FILE *fileptr;
    size_t size;
    char mode[4];
    char filepath[256];

protected:
    size_t count[2]; // Data-read and Data-written
    ssize_t seek[2]; // Read-seek and Write-seek
    ssize_t old_seek[2]; // For RevertReadPtr() and RevertWritePtr()
    // Access with <member>[READ] and <member>[WRITE]

public:
    File(const char *__restrict__ path)
    {
        NULLIFYSTACK(mode);
        STRSTACKCPY(mode, "rb+");
        STRSTACKCPY(filepath, path);
        if (Exists(path))
            fileptr = fopen(path, "rb+");
        else if (Accessible(path))
            fileptr = fopen(path, "wb+");
        else
        {
            perror("fopen");
            abort();
        }

        fseek(fileptr, 0, SEEK_END);
        size = ftell(fileptr);
        fseek(fileptr, 0, SEEK_SET);

        NULLIFYSTACK(seek);
        NULLIFYSTACK(old_seek);
        NULLIFYSTACK(count);

    }
    virtual ~File()
    {
        if (fileptr != nullptr)
            fclose(fileptr);
        fileptr = nullptr;
        size = 0;
        NULLIFYSTACK(seek);
        NULLIFYSTACK(old_seek);
        NULLIFYSTACK(count);
        NULLIFYSTACK(filepath);
        NULLIFYSTACK(mode);
    }

    enum
    {
        READ = 0,
        WRITE = 1,
    };

    File(const File &other)
    {
        fileptr = fopen(other.filepath, other.mode);
        size = other.size;
        COPYSTACK(seek, other.seek);
        COPYSTACK(old_seek, other.old_seek);
        COPYSTACK(count, other.count);
        STRSTACKCPY(filepath, other.filepath);
        STRSTACKCPY(mode, other.mode);
    }

    File& operator=(const File& other)
    {
        if (this != &other)
        {
            if (fileptr != nullptr)
                fclose(fileptr);

            fileptr = fopen(other.filepath, other.mode);
            size = other.size;
            COPYSTACK(seek, other.seek);
            COPYSTACK(old_seek, other.old_seek);
            COPYSTACK(count, other.count);
            STRSTACKCPY(filepath, other.filepath);
            STRSTACKCPY(mode, other.mode);
        }
        return *this;
    }

    File(File&& other) noexcept
    {
        fileptr = other.fileptr;
        size = other.size;
        COPYSTACK(seek, other.seek);
        COPYSTACK(old_seek, other.old_seek);
        COPYSTACK(count, other.count);
        STRSTACKCPY(filepath, other.filepath);
        STRSTACKCPY(mode, other.mode);

        other.fileptr = nullptr;
        other.size = 0;
        NULLIFYSTACK(other.seek);
        NULLIFYSTACK(other.old_seek);
        NULLIFYSTACK(other.count);
        NULLIFYSTACK(other.filepath);
        NULLIFYSTACK(other.mode);
    }

    File& operator=(File&& other) noexcept
    {
        if (this != &other)
        {
            if (fileptr != nullptr)
                fclose(fileptr);

            fileptr = other.fileptr;
            size = other.size;
            COPYSTACK(seek, other.seek);
            COPYSTACK(old_seek, other.old_seek);
            COPYSTACK(count, other.count);
            STRSTACKCPY(filepath, other.filepath);
            STRSTACKCPY(mode, other.mode);

            other.fileptr = nullptr;
            other.size = 0;
            NULLIFYSTACK(other.seek);
            NULLIFYSTACK(other.old_seek);
            NULLIFYSTACK(other.count);
            NULLIFYSTACK(other.filepath);
            NULLIFYSTACK(other.mode);
        }
        return *this;
    }

    inline size_t GetSize() const {
        return size;
    }

    inline const char* GetPath() const {
        return filepath;
    }

    inline ssize_t GetReadPtr() const {
        return seek[READ];
    }

    inline ssize_t GetWritePtr() const {
        return seek[WRITE];
    }

    inline void SetReadPtr(ssize_t offset) {
        old_seek[READ] = seek[READ];
        seek[READ] = offset;
    }

    inline void SetWritePtr(ssize_t offset) {
        old_seek[WRITE] = seek[WRITE];
        seek[WRITE] = offset;
    }

    inline size_t GetReadBytes() const {
        return count[READ];
    }

    inline size_t GetWrittenBytes() const {
        return count[WRITE];
    }

    inline void RevertReadPtr() {
        seek[READ] = old_seek[READ];
    }

    inline void RevertWritePtr() {
        seek[WRITE] = old_seek[WRITE];
    }

    template<typename T>
    size_t Read(const T *buffer, size_t elements = 1, ssize_t element_offset = 0) {
        seek[READ] += element_offset * sizeof(T);
        fseek(fileptr, seek[READ], SEEK_SET);

        size_t bytes_read = fread((void*)buffer, sizeof(T), elements, fileptr);

        seek[READ] += bytes_read;
        count[READ] += bytes_read;
        return bytes_read;
    };

    template<typename T>
    size_t Write(const T *buffer, size_t elements = 1, ssize_t element_offset = 0) {
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
    int Backup(const char *__restrict__ append, const size_t block_size = BUFSIZ, bool overwrite = false, transform_t transformer_func = nullptr) {
        char *buffer = new char[block_size];
        memset(buffer, 0, block_size);
        strncpy(buffer, filepath, block_size);
        strncat(buffer, append, block_size - strlen(filepath));
        if (strcmp(buffer, filepath) == 0)
            return 1;

        FILE *backup = nullptr;
        if (overwrite)
            backup = fopen(buffer, "wb");
        else
        {
            if (Exists(buffer))
                return 2;
            else
                backup = fopen(buffer, "wb");
        }
        size_t transfered = 0;
        memset(buffer, 0, block_size);
        fseek(fileptr, 0, SEEK_SET);
        if (transformer_func == nullptr)
        {
            while ((transfered = fread(buffer, 1, block_size, fileptr)) > 0)
            {
                fwrite(buffer, 1, transfered, backup);
            }
        }
        else
        {
            size_t index = 0;
            while ((transfered = fread(buffer, 1, block_size, fileptr)) > 0)
            {
                for (size_t i = 0; i < transfered; i++)
                {
                    buffer[i] = transformer_func(buffer[i], i, index);
                }
                index++;
                fwrite(buffer, 1, transfered, backup);
            }
        }

        fclose(backup);
        delete[] buffer;
        return 0;
    }

    static bool Exists(const char *__restrict__ path) {
        FILE *file = nullptr;
        if ((file = fopen(path, "rb")) != nullptr)
        {
            fclose(file);
            return true;
        }
        return false;
    }
    static bool Accessible(const char *__restrict__ path) {
        FILE *file = nullptr;
        // File exists
        if ((file = fopen(path, "rb")) != nullptr)
        {
            fclose(file);
            return true;
        }
        // File does not exist but can be created
        if ((file = fopen(path, "wb")) != nullptr)
        {
            fclose(file);
            return true;
        }
        return false;
    }
};