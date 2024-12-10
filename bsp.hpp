#pragma once
#include "fileio.hpp"
#include <cstring>

#define HEADER_LUMPS 64
#define IDBSPHEADER	(('P'<<24)+('S'<<16)+('B'<<8)+'V')
#define IDPSBHEADER	('P'+('S'<<8)+('B'<<16)+('V'<<24))
#define indexof(SIZE, ELEMENT) SIZE * ELEMENT

struct lump_t
{
	int    fileofs;
	int    filelen;
	int    version;
    union
    {
        char    fourCC[4];
        int     compressed;
    };
};

struct dheader_t
{
	int     ident;
	int     version;
	lump_t  lumps[HEADER_LUMPS];
	int     mapRevision;
};

class Bsp : public File
{
private:
    char lump_id;
    lump_t lump;
    size_t lumpdata_size;
    size_t lumpdata_off;
    dheader_t *header;

public:
    Bsp(const char *__restrict__ path) : File(path)
    {
        header = new dheader_t;
        Read(header, sizeof(dheader_t));
        lump_id = -1;
        lumpdata_size = -1;
        lumpdata_off = -1;
        memset(&lump, 0, sizeof(lump_t));
    }

    ~Bsp()
    {
        delete header;
        header = nullptr;
        lumpdata_size = 0;
        lumpdata_off = 0;
        lump_id = 0;
        memset(&lump, 0, sizeof(lump_t));
    }

    enum
    {
        LITTLE_ENDIAN = 1,
        BIG_ENDIAN = 2,
        UNKNOWN = 3,
    };

    constexpr char CheckIdent() const {
        if (header->ident == IDBSPHEADER)
            return LITTLE_ENDIAN;
        if (header->ident == IDPSBHEADER)
            return BIG_ENDIAN;
        return UNKNOWN;
    }

    inline constexpr void SelectLump(char n) {
        lump_id = n;
        lump = header->lumps[n];
        lumpdata_size = lump.filelen;
        lumpdata_off = lump.fileofs;
        SetReadPtr(lumpdata_off);
        SetWritePtr(lumpdata_off);
    }

    inline constexpr int GetLumpDataSize() const {
        return lumpdata_size;
    }

    template<typename T>
    inline constexpr int GetLumpElementCount(T type) const {
        return lumpdata_size / sizeof(T);
    }

    template<typename T, size_t elements>
    // Offset is calculated by sizeof(T) * offset, its not in raw bytes.
    // Elements is the number of elements, this function is meant to be used with arrays.
    // Returns the amount of bytes read.
    inline constexpr size_t ReadLumpElements(const T& buffer, size_t offset) const {
        return Read(buffer, elements, offset);
    }

    template<typename T, size_t elements>
    // Offset is calculated by sizeof(T) * offset, its not in raw bytes.
    // Elements is the number of elements, this function is meant to be used with arrays.
    // Automaticly rewrites the lump if the size (in bytes) of the written elements is different from the size of the previous elements.
    // ^ This behaviour may cause accidental overwrite of data from other lumps if the new size is bigger than the old size, be careful.
    // Returns the amount of bytes written.
    constexpr size_t WriteLumpElements(const T& buffer, size_t offset) const {
        Write(buffer, elements, offset);
        int new_size = indexof(sizeof(T), elements);

        if (new_size != lumpdata_size)
        {
            SetWritePtr(offsetof(dheader_t, lumps[lump_id].filelen));
            Write(&new_size);
            RevertWritePtr();
        }

        return new_size;
    }

    inline constexpr int GetMapRevision() const {
        return header->mapRevision;
    }

    inline constexpr int GetBspVersion() const {
        return header->version;
    }

    // Overwrite the currently selected lump with a new one.
    // Does not affect write pointer.
    void SetLump(const lump_t& new_lump) {
        SetWritePtr(offsetof(dheader_t, lumps[lump_id]));
        lump = new_lump;
        Write(new_lump, sizeof(lump_t));
        RevertWritePtr();
    }

    // Returns the currently selected lump.
    // Does not affect read pointer.
    inline constexpr lump_t GetLump() {
        return lump;
    }

    template<typename T>
    // Equivalent to ReadLumpElements<T, 1>(buffer, <offset>).
    // Increments read pointer by the size of the element.
    constexpr T GetLumpElement(size_t index) const {
        T elem;
        Read(&elem, sizeof(T));
        return elem;
    }

    template<typename T>
    // Equivalent to WriteLumpElements<T, 1>(buffer, <offset>).
    // Increments write pointer by the size of the element.
    void SetLumpElement(T new_elem, size_t index) {
        Write(new_elem, sizeof(T));
    }
};
