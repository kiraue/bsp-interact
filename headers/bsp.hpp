#pragma once
#include "fileio.hpp"
#include <cstring>

#define HEADER_LUMPS 64
#define IDBSPHEADER	(('P'<<24)+('S'<<16)+('B'<<8)+'V')
#define IDPSBHEADER	('P'+('S'<<8)+('B'<<16)+('V'<<24))
#define CLAMP(x, min, max)  \
    ( (x) > (max) ? (max) : ( (x) < (min) ? (min) : (x) ) )

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

struct dgamelump_t
{
	int             id;        // gamelump ID
	unsigned short  flags;     // flags
	unsigned short  version;   // gamelump version
	int             fileofs;   // offset to this gamelump
	int             filelen;   // length
};

struct dgamelumpheader_t
{
	int lumpCount;  // number of game lumps
	dgamelump_t gamelump[];
};

// TODO: add handling for the game lump
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

    enum
    {
        LUMP_ENTITIES,
        LUMP_PLANES,
        LUMP_TEXDATA,
        LUMP_VERTEXES,
        LUMP_VISIBILITY,
        LUMP_NODES,
        LUMP_TEXINFO,
        LUMP_FACES,
        LUMP_LIGHTING,
        LUMP_OCCLUSION,
        LUMP_LEAFS,
        LUMP_FACEIDS,
        LUMP_EDGES,
        LUMP_SURFEDGES,
        LUMP_MODELS,
        LUMP_WORLDLIGHTS,
        LUMP_LEAFFACES,
        LUMP_LEAFBRUSHES,
        LUMP_BRUSHES,
        LUMP_BRUSHSIDES,
        LUMP_AREAS,
        LUMP_AREAPORTALS,
        LUMP_PORTALS,
        LUMP_UNUSED0 = 22,
        LUMP_PROPCOLLISION = 22,
        LUMP_CLUSTERS,
        LUMP_UNUSED1 = 23,
        LUMP_PROPHULLS = 23,
        LUMP_PORTALVERTS,
        LUMP_UNUSED2 = 24,
        LUMP_FAKEENTITIES = 24,
        LUMP_PROPHULLVERTS = 24,
        LUMP_CLUSTERPORTALS,
        LUMP_UNUSED3 = 25,
        LUMP_PROPTRIS = 25,
        LUMP_DISPINFO,
        LUMP_ORIGINALFACES,
        LUMP_PHYSDISP,
        LUMP_PHYSCOLLIDE,
        LUMP_VERTNORMALS,
        LUMP_VERTNORMALINDICES,
        LUMP_DISP_LIGHTMAP_ALPHAS,
        LUMP_DISP_VERTS,
        LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS,
        LUMP_GAME_LUMP,
        LUMP_LEAFWATERDATA,
        LUMP_PRIMITIVES,
        LUMP_PRIMVERTS,
        LUMP_PRIMINDICES,
        LUMP_PAKFILE,
        LUMP_CLIPPORTALVERTS,
        LUMP_CUBEMAPS,
        LUMP_TEXDATA_STRING_DATA,
        LUMP_TEXDATA_STRING_TABLE,
        LUMP_OVERLAYS,
        LUMP_LEAFMINDISTTOWATER,
        LUMP_FACE_MACRO_TEXTURE_INFO,
        LUMP_DISP_TRIS,
        LUMP_PHYSCOLLIDESURFACE,
        LUMP_PROP_BLOB = 49,
        LUMP_WATEROVERLAYS,
        LUMP_LIGHTMAPPAGES,
        LUMP_LEAF_AMBIENT_INDEX_HDR = 51,
        LUMP_LIGHTMAPPAGEINFOS,
        LUMP_LEAF_AMBIENT_INDEX = 52,
        LUMP_LIGHTING_HDR,
        LUMP_WORLDLIGHTS_HDR,
        LUMP_LEAF_AMBIENT_LIGHTING_HDR,
        LUMP_LEAF_AMBIENT_LIGHTING,
        LUMP_XZIPPAKFILE,
        LUMP_FACES_HDR,
        LUMP_MAP_FLAGS,
        LUMP_OVERLAY_FADES,
        LUMP_OVERLAY_SYSTEM_LEVELS,
        LUMP_PHYSLEVEL,
        LUMP_DISP_MULTIBLEND
    };

    constexpr int GetIdent() const {
        return header->ident;
    }

    template<typename T>
    constexpr void SelectLump(char n) {
        lump_id = n;
        lump = header->lumps[n];
        lumpdata_size = lump.filelen;
        lumpdata_off = lump.fileofs;
        lumpdata_num = lumpdata_size / sizeof(T);
        lumpdata_remain[READ] = lumpdata_remain[WRITE] = lumpdata_size;
        SetReadPtr(lumpdata_off);
        SetWritePtr(lumpdata_off);
    }

    inline constexpr int GetLumpDataSize() const {
        return lumpdata_size;
    }

    inline constexpr int GetLumpElementCount() const {
        return lumpdata_num;
    }

    template<typename T, size_t elements>
    // Offset is calculated by sizeof(T) * offset, its not in raw bytes.
    // Elements is the number of elements, this function is meant to be used with arrays.
    // Returns the amount of bytes read.
    // Value can be less than expected since this function will make sure to not read data outside of the range of the lump.
    // This does increase the read pointer by the correct amount.
    constexpr size_t ReadLumpElements(const T *buffer, size_t offset = 0) {
        size_t elem_remain = lumpdata_remain[READ] / sizeof(T);
        offset = CLAMP(offset, 0, elem_remain);
        size_t read = Read(buffer, CLAMP(elements, 0, elem_remain - offset), offset);
        lumpdata_remain[READ] -= read;
        return read;
    }

    template<typename T, size_t elements>
    // Offset is calculated by sizeof(T) * offset, its not in raw bytes.
    // Elements is the number of elements, this function is meant to be used with arrays.
    // Automaticly rewrites the lump if the size (in bytes) of the written elements is different from the size of the previous elements.
    // Returns the amount of bytes written.
    // Value can be less than expected since this function will make sure to not write data outside of the range of the lump.
    // This does increase the write pointer by the correct amount.
    constexpr size_t WriteLumpElements(const T *buffer, size_t offset = 0) {
        size_t elem_remain = lumpdata_remain[WRITE] / sizeof(T);
        offset = CLAMP(offset, 0, elem_remain);
        size_t written = Write(buffer, CLAMP(elements, 0, elem_remain - offset), offset);
        lumpdata_remain[WRITE] -= written;
        return written;
    }

    inline constexpr int GetMapRevision() const {
        return header->mapRevision;
    }

    inline constexpr int GetBspVersion() const {
        return header->version;
    }

    inline constexpr int GetLumpVersion() const {
        return lump.version;
    }

    inline constexpr char* GetFourCC() const {
        return (char*)lump.fourCC;
    }

    inline constexpr int GetCompressedSize() const {
        return lump.compressed;
    }

    // Overwrite the currently selected lump with a new one.
    // Does not affect write pointer.
    void SetLump(const lump_t& new_lump) {
        SetWritePtr((ssize_t)(&((dheader_t*)0)->lumps[lump_id]));
        lump = new_lump;
        Write(&new_lump, sizeof(lump_t));
        RevertWritePtr();
    }

    // Returns the currently selected lump.
    // Does not affect read pointer.
    inline constexpr lump_t GetLump() const {
        return lump;
    }

    template<typename T>
    // Basicly equivalent to WriteLumpElements<T, 1>(buffer, index) except it can go backwards.
    // Does not affect write pointer.
    // Clamps the index to a valid range.
    void SetLumpElement(const T& new_elem, size_t index) {
        index = CLAMP(index, 0, lumpdata_num);
        SetWritePtr(lumpdata_off + index * sizeof(T));
        Write(new_elem, sizeof(T));
        RevertWritePtr();
    }

    template<typename T>
    // Basicly equivalent to ReadLumpElements<T, 1>(buffer, index) except it can go backwards.
    // Does not affect read pointer.
    // Clamps the index to a valid range.
    constexpr T GetLumpElement(size_t index) {
        index = CLAMP(index, 0, lumpdata_num);
        SetReadPtr(lumpdata_off + index * sizeof(T));
        T elem;
        Read(&elem, sizeof(T));
        RevertReadPtr();
        return elem;
    }

    template<typename T>
    constexpr T* GetAllLumpElements() {
        T *buffer = new T[lumpdata_num];
        SetReadPtr(lumpdata_off);
        Read(buffer, lumpdata_num);
        RevertReadPtr();
        return buffer;
    }
};
