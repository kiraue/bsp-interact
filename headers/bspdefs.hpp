#pragma once
#ifndef BSP_DEFINITIONS_H
#define BSP_DEFINITIONS_H

#define HEADER_LUMPS 64
#define IDBSPHEADER	(('P'<<24)+('S'<<16)+('B'<<8)+'V')
#define IDPSBHEADER	('P'+('S'<<8)+('B'<<16)+('V'<<24))

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


// Mostly copied from https://developer.valvesoftware.com/wiki/BSP_(Source)

struct lump_l4d2_t;
struct lump_t;

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

    operator lump_l4d2_t() const;
};

struct lump_l4d2_t
{
	int	version;
	int	fileofs;
	int	filelen;
	char	fourCC[4];

    operator lump_t() const;
};

lump_t::operator lump_l4d2_t() const {
    lump_l4d2_t result;
    result.version = version;
    result.fileofs = fileofs;
    result.filelen = filelen;
    memcpy(&result.fourCC, fourCC, 4);
    return result;
}

lump_l4d2_t::operator lump_t() const {
    lump_t result;
    result.version = version;
    result.fileofs = fileofs;
    result.filelen = filelen;
    memcpy(&result.fourCC, fourCC, 4);
    return result;
}

struct dheader_t
{
	int     ident;
	int     version;
	lump_t  lumps[HEADER_LUMPS];
	int     mapRevision;
};

struct dgamelump_t
{
	int             id;
	unsigned short  flags;
	unsigned short  version;
	int             fileofs;
	int             filelen;
};

struct dgamelumpheader_t
{
	int lumpCount;  // number of game lumps
	dgamelump_t gamelump[];
};

struct Vector
{
	float x;
	float y;
	float z;
};

struct dplane_t
{
	Vector  normal;   // normal vector
	float   dist;     // distance from origin
	int     type;     // plane axis identifier
};

struct dedge_t
{
	unsigned short  v[2];  // vertex indices
};

typedef unsigned char byte;
typedef int dsuredge_t;

struct dface_t
{
	unsigned short  planenum;               // the plane number
	byte            side;                   // faces opposite to the node's plane direction
	byte            onNode;                 // 1 of on node, 0 if in leaf
	int             firstedge;              // index into surfedges
	short           numedges;               // number of surfedges
	short           texinfo;                // texture info
	short           dispinfo;               // displacement info
	short           surfaceFogVolumeID;     // ?
	byte            styles[4];              // switchable lighting info
	int             lightofs;               // offset into lightmap lump
	float           area;                   // face area in units^2
	int             LightmapTextureMinsInLuxels[2]; // texture lighting info
	int             LightmapTextureSizeInLuxels[2]; // texture lighting info
	int             origFace;               // original face this was split from
	unsigned short  numPrims;               // primitives
	unsigned short  firstPrimID;
	unsigned int    smoothingGroups;        // lightmap smoothing group
};

struct dbrush_t
{
	int    firstside;     // first brushside
	int    numsides;      // number of brushsides
	int    contents;      // contents flags
};

struct dbrushside_t
{
	unsigned short  planenum;     // facing out of the leaf
	short           texinfo;      // texture info
	short           dispinfo;     // displacement info
	short           bevel;        // is the side a bevel plane?
};

struct dnode_t
{
	int             planenum;       // index into plane array
	int             children[2];    // negative numbers are -(leafs + 1), not nodes
	short           mins[3];        // for frustum culling
	short           maxs[3];
	unsigned short  firstface;      // index into face array
	unsigned short  numfaces;       // counting both sides
	short           area;           // If all leaves below this node are in the same area, then
	                                // this is the area index. If not, this is -1.
	short           paddding;       // pad to 32 bytes length
};

struct dleaf_t
{
	int             contents;             // OR of all brushes (not needed?)
	short           cluster;              // cluster this leaf is in
	short           area:9;               // area this leaf is in
	short           flags:7;              // flags
	short           mins[3];              // for frustum culling
	short           maxs[3];
	unsigned short  firstleafface;        // index into leaffaces
	unsigned short  numleaffaces;
	unsigned short  firstleafbrush;       // index into leafbrushes
	unsigned short  numleafbrushes;
	short           leafWaterDataID;      // -1 for not in water

	//!!! NOTE: for lump version 0 (usually in maps of version 19 or lower) uncomment the next line
	//CompressedLightCube   ambientLighting;      // Precaculated light info for entities.
	short                 padding;              // padding to 4-byte boundary
};

typedef unsigned short leafface_t;
typedef unsigned short leafbrush_t;

struct texinfo_t
{
	float   textureVecs[2][4];    // [s/t][xyz offset]
	float   lightmapVecs[2][4];   // [s/t][xyz offset] - length is in units of texels/area
	int     flags;                // miptex flags overrides
	int     texdata;              // Pointer to texture name, size, etc.
};

struct dtexdata_t
{
	Vector  reflectivity;            // RGB reflectivity
	int     nameStringTableID;       // index into TexdataStringTable
	int     width, height;           // source image
	int     view_width, view_height;
};

typedef int texdatastr_t;

struct dmodel_t
{
	Vector  mins, maxs;            // bounding box
	Vector  origin;                // for sounds or lights
	int     headnode;              // index into node array
	int     firstface, numfaces;   // index into face array
};

struct dvis_t
{
	int	numclusters;
    int byteofs[][2]; // Equivalent to int byteofs[numclusters][2];
};

enum
{
    PROP_STATIC = 0x73707270, // 'sprp'
    PROP_DETAIL = 0x64707270, // 'dprp'
    PROP_LIGHT_LDR = 0x64706c74, // 'dplt'
    PROP_LIGHT_HDR = 0x64706c68, // 'dplh'
};

struct StaticPropDictLump_t
{
	int	dictEntries;
	char	name[][128];	// model name
};

struct StaticPropLeafLump_t
{
	int leafEntries;
	unsigned short	leaf[];
};

struct QAngle
{
    float pitch;
    float yaw;
    float roll;
};

// I have no idea how to decode this struct
typedef int color32;

// In the bsp, this struct is preceeded by an int which dictates how many of the following struct are going to be present.
struct StaticPropLump_t
{
	// v4
	Vector          Origin;            // origin
	QAngle          Angles;            // orientation (pitch yaw roll)
	
	// v4
	unsigned short  PropType;          // index into model name dictionary
	unsigned short  FirstLeaf;         // index into leaf array
	unsigned short  LeafCount;
	unsigned char   Solid;             // solidity type
#if PROP_VERSION != 7
	// every version except v7*
	unsigned char   Flags;
#endif
	// v4 still
	int             Skin;              // model skin numbers
	float           FadeMinDist;
	float           FadeMaxDist;
	Vector          LightingOrigin;    // for lighting
#if PROP_VERSION >= 5
	// since v5
	float           ForcedFadeScale;   // fade distance scale
#endif
#if PROP_VERSION == 6 || PROP_VERSION == 7
	// v6, v7, and v7* only
	unsigned short  MinDXLevel;        // minimum DirectX version to be visible
	unsigned short  MaxDXLevel;        // maximum DirectX version to be visible
#endif
#if PROP_VERSION == 7
	// v7* only
	unsigned int    Flags;
	unsigned short  LightmapResX;      // lightmap image width
	unsigned short	LightmapResY;      // lightmap image height
#endif
#if PROP_VERSION >= 8
	// since v8
	unsigned char   MinCPULevel;
	unsigned char   MaxCPULevel;
	unsigned char   MinGPULevel;
	unsigned char   MaxGPULevel;
#endif
#if PROP_VERSION >= 7
	// since v7
	color32         DiffuseModulation; // per instance color and alpha modulation
#endif
#if PROP_VERSION == 9 || PROP_VERSION == 10
	// v9 and v10 only
	bool            DisableX360;       // if true, don't show on XBox 360 (4-bytes long)
#endif
#if PROP_VERSION >= 10
	// since v10
	unsigned int    FlagsEx;           // Further bitflags.
#endif
#if PROP_VERSION >= 11
	// since v11
	float           UniformScale;      // Prop scale
#endif
};

struct StaticPropLumpV4_t
{
	// v4
	Vector          Origin;            // origin
	QAngle          Angles;            // orientation (pitch yaw roll)
	
	// v4
	unsigned short  PropType;          // index into model name dictionary
	unsigned short  FirstLeaf;         // index into leaf array
	unsigned short  LeafCount;
	unsigned char   Solid;             // solidity type
	// every version except v7*
	unsigned char   Flags;
	// v4 still
	int             Skin;              // model skin numbers
	float           FadeMinDist;
	float           FadeMaxDist;
	Vector          LightingOrigin;    // for lighting
};
struct StaticPropLumpV5_t
{
	// v4
	Vector          Origin;            // origin
	QAngle          Angles;            // orientation (pitch yaw roll)
	
	// v4
	unsigned short  PropType;          // index into model name dictionary
	unsigned short  FirstLeaf;         // index into leaf array
	unsigned short  LeafCount;
	unsigned char   Solid;             // solidity type
	// every version except v7*
	unsigned char   Flags;
	// v4 still
	int             Skin;              // model skin numbers
	float           FadeMinDist;
	float           FadeMaxDist;
	Vector          LightingOrigin;    // for lighting
	float           ForcedFadeScale;   // fade distance scale
};
struct StaticPropLumpV6_t
{
	// v4
	Vector          Origin;            // origin
	QAngle          Angles;            // orientation (pitch yaw roll)
	
	// v4
	unsigned short  PropType;          // index into model name dictionary
	unsigned short  FirstLeaf;         // index into leaf array
	unsigned short  LeafCount;
	unsigned char   Solid;             // solidity type
	// every version except v7*
	unsigned char   Flags;
	// v4 still
	int             Skin;              // model skin numbers
	float           FadeMinDist;
	float           FadeMaxDist;
	Vector          LightingOrigin;    // for lighting
	float           ForcedFadeScale;   // fade distance scale
	// v6, v7, and v7* only
	unsigned short  MinDXLevel;        // minimum DirectX version to be visible
	unsigned short  MaxDXLevel;        // maximum DirectX version to be visible
};
struct StaticPropLumpV7_t
{
	// v4
	Vector          Origin;            // origin
	QAngle          Angles;            // orientation (pitch yaw roll)
	
	// v4
	unsigned short  PropType;          // index into model name dictionary
	unsigned short  FirstLeaf;         // index into leaf array
	unsigned short  LeafCount;
	unsigned char   Solid;             // solidity type
	// v4 still
	int             Skin;              // model skin numbers
	float           FadeMinDist;
	float           FadeMaxDist;
	Vector          LightingOrigin;    // for lighting
	float           ForcedFadeScale;   // fade distance scale
	// v6, v7, and v7* only
	unsigned short  MinDXLevel;        // minimum DirectX version to be visible
	unsigned short  MaxDXLevel;        // maximum DirectX version to be visible
	// since v7
	color32         DiffuseModulation; // per instance color and alpha modulation
};
struct StaticPropLumpV7_star_t
{
	// v4
	Vector          Origin;            // origin
	QAngle          Angles;            // orientation (pitch yaw roll)
	
	// v4
	unsigned short  PropType;          // index into model name dictionary
	unsigned short  FirstLeaf;         // index into leaf array
	unsigned short  LeafCount;
	unsigned char   Solid;             // solidity type
	// v4 still
	int             Skin;              // model skin numbers
	float           FadeMinDist;
	float           FadeMaxDist;
	Vector          LightingOrigin;    // for lighting
	float           ForcedFadeScale;   // fade distance scale
	// v6, v7, and v7* only
	unsigned short  MinDXLevel;        // minimum DirectX version to be visible
	unsigned short  MaxDXLevel;        // maximum DirectX version to be visible
	// v7* only
	unsigned int    Flags;
	unsigned short  LightmapResX;      // lightmap image width
	unsigned short	LightmapResY;      // lightmap image height
	// since v7
	color32         DiffuseModulation; // per instance color and alpha modulation
};
struct StaticPropLumpV8_t
{
	// v4
	Vector          Origin;            // origin
	QAngle          Angles;            // orientation (pitch yaw roll)
	
	// v4
	unsigned short  PropType;          // index into model name dictionary
	unsigned short  FirstLeaf;         // index into leaf array
	unsigned short  LeafCount;
	unsigned char   Solid;             // solidity type
	// v4 still
	int             Skin;              // model skin numbers
	float           FadeMinDist;
	float           FadeMaxDist;
	Vector          LightingOrigin;    // for lighting
	float           ForcedFadeScale;   // fade distance scale
	// since v8
	unsigned char   MinCPULevel;
	unsigned char   MaxCPULevel;
	unsigned char   MinGPULevel;
	unsigned char   MaxGPULevel;
	// since v7
	color32         DiffuseModulation; // per instance color and alpha modulation
};
struct StaticPropLumpV9_t
{
	// v4
	Vector          Origin;            // origin
	QAngle          Angles;            // orientation (pitch yaw roll)
	
	// v4
	unsigned short  PropType;          // index into model name dictionary
	unsigned short  FirstLeaf;         // index into leaf array
	unsigned short  LeafCount;
	unsigned char   Solid;             // solidity type
	// v4 still
	int             Skin;              // model skin numbers
	float           FadeMinDist;
	float           FadeMaxDist;
	Vector          LightingOrigin;    // for lighting
	float           ForcedFadeScale;   // fade distance scale
	// since v8
	unsigned char   MinCPULevel;
	unsigned char   MaxCPULevel;
	unsigned char   MinGPULevel;
	unsigned char   MaxGPULevel;
	// since v7
	color32         DiffuseModulation; // per instance color and alpha modulation
	// v9 and v10 only
	bool            DisableX360;       // if true, don't show on XBox 360 (4-bytes long)
};
struct StaticPropLumpV10_t
{
	// v4
	Vector          Origin;            // origin
	QAngle          Angles;            // orientation (pitch yaw roll)
	
	// v4
	unsigned short  PropType;          // index into model name dictionary
	unsigned short  FirstLeaf;         // index into leaf array
	unsigned short  LeafCount;
	unsigned char   Solid;             // solidity type
	// v4 still
	int             Skin;              // model skin numbers
	float           FadeMinDist;
	float           FadeMaxDist;
	Vector          LightingOrigin;    // for lighting
	float           ForcedFadeScale;   // fade distance scale
	// since v8
	unsigned char   MinCPULevel;
	unsigned char   MaxCPULevel;
	unsigned char   MinGPULevel;
	unsigned char   MaxGPULevel;
	// since v7
	color32         DiffuseModulation; // per instance color and alpha modulation
	// v9 and v10 only
	bool            DisableX360;       // if true, don't show on XBox 360 (4-bytes long)
	// since v10
	unsigned int    FlagsEx;           // Further bitflags.
};
struct StaticPropLumpV11_t
{
	// v4
	Vector          Origin;            // origin
	QAngle          Angles;            // orientation (pitch yaw roll)
	
	// v4
	unsigned short  PropType;          // index into model name dictionary
	unsigned short  FirstLeaf;         // index into leaf array
	unsigned short  LeafCount;
	unsigned char   Solid;             // solidity type
	// v4 still
	int             Skin;              // model skin numbers
	float           FadeMinDist;
	float           FadeMaxDist;
	Vector          LightingOrigin;    // for lighting
	float           ForcedFadeScale;   // fade distance scale
	// since v8
	unsigned char   MinCPULevel;
	unsigned char   MaxCPULevel;
	unsigned char   MinGPULevel;
	unsigned char   MaxGPULevel;
	// since v7
	color32         DiffuseModulation; // per instance color and alpha modulation
	// since v10
	unsigned int    FlagsEx;           // Further bitflags.
	// since v11
	float           UniformScale;      // Prop scale
};


struct dcubemapsample_t
{
	int    origin[3];    // position of light snapped to the nearest integer
	int    size;         // resolution of cubemap, 0 - default
};


#define OVERLAY_BSP_FACE_COUNT 64
struct doverlay_t
{
	int             Id;
	short           TexInfo;
    union
    {
        unsigned short  FaceCountAndRenderOrder;
        unsigned short  FaceCount : 14;
        unsigned short  RenderOrder: 2;
    };
    int             Ofaces[OVERLAY_BSP_FACE_COUNT];
	float           U[2];
	float           V[2];
	Vector          UVPoints[4];
	Vector          Origin;
	Vector          BasisNormal;
};

struct ColorRGBExp32
{
	byte r, g, b;
	signed char exponent;
};

struct CompressedLightCube
{
	ColorRGBExp32 m_Color[6];
};

struct dleafambientlighting_t
{
	CompressedLightCube	cube;
	byte x;		// fixed point fraction of leaf bounds
	byte y;		// fixed point fraction of leaf bounds
	byte z;		// fixed point fraction of leaf bounds
	byte pad;	// unused
};

struct dleafambientindex_t
{
	unsigned short ambientSampleCount;
	unsigned short firstAmbientSample;
};

struct doccluderdata_t
{
	int	flags;
	int	firstpoly;	// index into doccluderpolys
	int	polycount;	// amount of polygons
	Vector	mins;	        // minima of all vertices
	Vector	maxs;	        // maxima of all vertices
	// since v1
	int	area;
};

struct doccluderpolydata_t
{
	int	firstvertexindex;	// index into doccludervertindices
	int	vertexcount;		// amount of vertex indices
	int	planenum;
};

// TODO: add read write methods
struct doccluder_t
{
    int count;
    doccluderdata_t *data;
    int polyDataCount;
    doccluderpolydata_t *polyData;
    int vertexIndexCount;
    int *vertexIndices;
};

// TODO: add a function which parses the physcollide lump.
struct dphysmodel_t
{
    int modelIndex;  // Perhaps the index of the model to which this physics model applies?
    int dataSize;    // Total size of the collision data sections
    int keydataSize; // Size of the text section
    int solidCount;  // Number of collision data sections
};

// lights that were used to illuminate the world
enum emittype_t
{
	emit_surface,		// 90 degree spotlight
	emit_point,			// simple point light source
	emit_spotlight,		// spotlight with penumbra
	emit_skylight,		// directional light with no falloff (surface must trace to SKY texture)
	emit_quakelight,	// linear falloff, non-lambertian
	emit_skyambient,	// spherical light source with no falloff (surface must trace to SKY texture)
};

// Flags for dworldlight_t::flags
#define DWL_FLAGS_INAMBIENTCUBE		0x0001	// This says that the light was put into the per-leaf ambient cubes.

struct dworldlight_t
{
	Vector		origin;					
	Vector		intensity;
	Vector		normal;		// for surfaces and spotlights
	int		cluster;
	emittype_t	type; //int, 0 - surface, 1 - point, 2 - spot, 3 - sky, 4 - quakelight, 5 - sky ambient 
	int		style;
	float		stopdot;		// start of penumbra for emit_spotlight
	float		stopdot2;		// end of penumbra for emit_spotlight
	float		exponent;		// 
	float		radius;			// cutoff distance
	// falloff for emit_spotlight + emit_point: 
	// 1 / (constant_attn + linear_attn * dist + quadratic_attn * dist^2)
	float		constant_attn;	
	float		linear_attn;
	float		quadratic_attn;
	int		flags;			// Uses a combination of the DWL_FLAGS_ defines.
	int		texinfo;		// 
	int		owner;			// entity that this light it relative to
};

struct dDispVert
{
	Vector  vec;    // Vector field defining displacement volume.
	float   dist;   // Displacement distances.
	float   alpha;  // "per vertex" alpha values.
};

typedef unsigned short area_t;

// Taken from the tf2 source code dump https://github.com/sr2echa/TF2-Source-Code


enum NeighborSpan
{
	CORNER_TO_CORNER=0,
	CORNER_TO_MIDPOINT=1,
	MIDPOINT_TO_CORNER=2
};

// Where is this used inside the bsp?
struct lumpfileheader_t
{
	int				lumpOffset;
	int				lumpID;
	int				lumpVersion;	
	int				lumpLength;
	int				mapRevision;
};

enum NeighborOrientation
{
	ORIENTATION_CCW_0=0,
	ORIENTATION_CCW_90=1,
	ORIENTATION_CCW_180=2,
	ORIENTATION_CCW_270=3
};

struct dphysdisp_t
{
	unsigned short numDisplacements;
	//unsigned short dataSize[numDisplacements];
};

struct doccluderdataV1_t
{
	int			flags;
	int			firstpoly;				// index into doccluderpolys
	int			polycount;
	Vector		mins;
	Vector		maxs;
};

struct CDispSubNeighbor
{
	unsigned short		m_iNeighbor;		// This indexes into ddispinfos.
											// 0xFFFF if there is no neighbor here.

	unsigned char		m_NeighborOrientation;		// (CCW) rotation of the neighbor wrt this displacement.

	// These use the NeighborSpan type.
	unsigned char		m_Span;						// Where the neighbor fits onto this side of our displacement.
	unsigned char		m_NeighborSpan;				// Where we fit onto our neighbor.
};

struct CDispNeighbor
{
	CDispSubNeighbor	m_SubNeighbors[2];
};

#define MAX_DISP_CORNER_NEIGHBORS	4
struct CDispCornerNeighbors
{
	unsigned short	m_Neighbors[MAX_DISP_CORNER_NEIGHBORS];	// indices of neighbors.
	unsigned char	m_nNeighbors;
};


struct ddispinfo_t
{
	Vector                startPosition;                // start position used for orientation
	int                   DispVertStart;                // Index into LUMP_DISP_VERTS.
	int                   DispTriStart;                 // Index into LUMP_DISP_TRIS.
	int                   power;                        // power - indicates size of surface (2^power 1)
	int                   minTess;                      // minimum tesselation allowed
	float                 smoothingAngle;               // lighting smoothing angle
	int                   contents;                     // surface contents
	unsigned short        MapFace;                      // Which map face this displacement comes from.
	int                   LightmapAlphaStart;           // Index into ddisplightmapalpha.
	int                   LightmapSamplePositionStart;  // Index into LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS.
	CDispNeighbor         EdgeNeighbors[4];             // Indexed by NEIGHBOREDGE_ defines.
	CDispCornerNeighbors  CornerNeighbors[4];           // Indexed by CORNER_ defines.
	unsigned int          AllowedVerts[10];             // active verticies
};

struct CDispTri
{
	unsigned short m_uiTags;		// Displacement triangle tags.
};

#endif // BSP_DEFINITIONS_H