#pragma once

#ifndef BSP_INTERACT_H
#define BSP_INTERACT_H

#include "fileio.hpp"
#include "bspdefs.hpp"
#include <cstring>
#include <vector>
#include <iostream>


#define CLAMP(x, min, max)  \
    ( (x) > (max) ? (max) : ( (x) < (min) ? (min) : (x) ) )
#define CPTRCAST(x, TYPE)   \
    ( *(TYPE *)&x )

// TODO: add more handling for the game lump
// add handling for different versions from other games
class Bsp : public File
{
private:
    char lump_id;
    lump_t lump;
    size_t lumpdata_size;
    size_t lumpdata_off;
    size_t lumpdata_num;
    size_t lumpdata_remain[2]; // How much remains in the lump that hasnt been read/written yet
    dheader_t *header;
    dgamelumpheader_t *gameheader;
    std::vector<char> lumpdata;

public:
    Bsp(const char *__restrict__ path) : File(path)
    {
        header = new dheader_t;
        Read(header);
        SetReadPtr(header->lumps[LUMP_GAME_LUMP].fileofs);
        gameheader = (dgamelumpheader_t *)(new char[header->lumps[LUMP_GAME_LUMP].filelen]);
        Read<char>((char *)gameheader, header->lumps[LUMP_GAME_LUMP].filelen);
        RevertReadPtr();
        lump_id = LUMP_ENTITIES;
        lump = header->lumps[LUMP_ENTITIES];
        lumpdata_off = lump.fileofs;
        lumpdata_remain[READ] = lumpdata_remain[WRITE] = lumpdata_num = lumpdata_size = lump.filelen;
        SetReadPtr(lumpdata_off);
        lumpdata.reserve(lumpdata_size);
        Read(lumpdata.data(), lumpdata_size);
        RevertReadPtr();
    }

    ~Bsp()
    {
        delete header;
        delete gameheader;
        header = nullptr;
        gameheader = nullptr;
        lumpdata_size = 0;
        lumpdata_off = 0;
        lumpdata_num = 0;
        lump_id = 0;
        memset(lumpdata_remain, 0, sizeof(size_t[2]));
        memset(&lump, 0, sizeof(lump_t));
    }

    int GetIdent() const {
        return header->ident;
    }

    // Always use this before interacting with the bsp.
    // By default, the chosen lump is the entity lump.
    template<typename T>
    void SelectLump(char n) {
        lump_id = n;
        lump = header->lumps[n];
        lumpdata_size = lump.filelen;
        lumpdata_off = lump.fileofs;
        lumpdata_num = lumpdata_size / sizeof(T);
        lumpdata_remain[READ] = lumpdata_remain[WRITE] = lumpdata_size;
        SetReadPtr(lumpdata_off);
        lumpdata.reserve(lumpdata_size);
        Read<char>(lumpdata.data(), lumpdata_size);
        SetReadPtr(lumpdata_off);
        SetWritePtr(lumpdata_off);
    }

    inline int GetLumpDataSize() const {
        return lumpdata_size;
    }

    inline int GetElementCount() const {
        return lumpdata_num;
    }

    // Offset is calculated by sizeof(T) * offset, its not in raw bytes.
    // Elements is the number of elements, this function is meant to be used with arrays.
    // Returns the amount of bytes read.
    // Value can be less than expected since this function will make sure to not read data outside of the range of the lump.
    // This does increase the read pointer by the correct amount.
    template<typename T>
    size_t ReadLumpElements(const T *buffer, size_t elements = 1, size_t offset = 0) {
        size_t elem_remain = lumpdata_remain[READ] / sizeof(T);
        offset = CLAMP(offset, 0, elem_remain);
        elements = CLAMP(elements, 0, elem_remain - offset);
        size_t read = Read(buffer, elements, offset);
        lumpdata_remain[READ] -= read;
        return read;
    }

    // Offset is calculated by sizeof(T) * offset, its not in raw bytes.
    // Elements is the number of elements, this function is meant to be used with arrays.
    // Automaticly rewrites the lump if the size (in bytes) of the written elements is different from the size of the previous elements.
    // Returns the amount of bytes written.
    // Value can be less than expected since this function will make sure to not write data outside of the range of the lump.
    // This does increase the write pointer by the correct amount.
    template<typename T>
    size_t WriteLumpElements(const T *buffer, size_t elements = 1, size_t offset = 0) {
        size_t elem_remain = lumpdata_remain[WRITE] / sizeof(T);
        offset = CLAMP(offset, 0, elem_remain);
        elements = CLAMP(elements, 0, elem_remain - offset);
        size_t written = Write(buffer, elements, offset);
        lumpdata_remain[WRITE] -= written;
        return written;
    }

    inline int GetMapRevision() const {
        return header->mapRevision;
    }

    inline int GetBspVersion() const {
        return header->version;
    }

    inline int GetLumpVersion() const {
        return lump.version;
    }

    inline char* GetFourCC() const {
        return (char*)lump.fourCC;
    }

    inline int GetCompressedSize() const {
        return lump.compressed;
    }

    // Overwrite the currently selected lump with a new one.
    void SetLump(const lump_t& new_lump) {
        SetWritePtr((ssize_t)(&((dheader_t*)0)->lumps[lump_id])); // offsetof(dheader_t, lumps[lump_id])
        lump = new_lump;
        Write(&new_lump, sizeof(lump_t));
        RevertWritePtr();
    }

    // Returns the currently selected lump.
    inline lump_t GetLump() const {
        return lump;
    }

    // Basicly equivalent to WriteLumpElements<T>(buffer, 1, index) except it can go backwards and it doesnt change the write pointer.
    // Clamps the index to a valid range.
    template<typename T>
    void SetLumpElement(const T& new_elem, size_t index) {
        index = CLAMP(index, 0, lumpdata_num);
        SetWritePtr(lumpdata_off + index * sizeof(T));
        Write(new_elem, sizeof(T));
        RevertWritePtr();
    }

    // Basicly equivalent to ReadLumpElements<T>(buffer, 1, index) except it can go backwards and it doesnt change the read pointer.
    // Clamps the index to a valid range.
    template<typename T>
    T GetLumpElement(size_t index) {
        index = CLAMP(index, 0, lumpdata_num);
        SetReadPtr(lumpdata_off + index * sizeof(T));
        T elem;
        Read(&elem, sizeof(T));
        RevertReadPtr();
        return elem;
    }

    // This function can be pretty slow.
    // May not work correctly with lumps that use variable length structures.
    template<typename T>
    std::vector<T> GetAllLumpElements() {
        std::vector<T> result(lumpdata_num);
        // TODO: replace this.
        memcpy(result.data(), lumpdata.data(), lumpdata_size);
        return result;
    }

    // Returns the byte offsets of the PVS and the PAS inside the vis lump.
    std::vector<int[2]> GetVisData() {
        int visnum = -1;

        SetReadPtr(header->lumps[LUMP_VISIBILITY].fileofs);
        Read(&visnum);

        std::vector<int[2]> result(visnum);
        Read<int[2]>(result.data(), visnum);

        RevertReadPtr();

        return result;
    }
    // TODO: add method to decompress the PVS and PAS

    // Returns the number of visclusters
    int GetVisClusterCount() {
        int visnum = -1;

        SetReadPtr(header->lumps[LUMP_VISIBILITY].fileofs);
        Read(&visnum);
        RevertReadPtr();

        return visnum;
    }

    // Returns the number of gamelumps inside the bsp
    int GetGameLumpCount() {
        return gameheader->lumpCount;
    }

    // Returns all the gamelumps inside the bsp.
    std::vector<dgamelump_t> GetAllGameLumps() {
        return std::vector<dgamelump_t>(&gameheader->gamelump[0], &gameheader->gamelump[0] + gameheader->lumpCount);
    }
};

#endif // BSP_INTERACT_H