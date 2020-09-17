
/*****************************************************************************
 * FLT: An OpenFlight file loader
 *
 * Copyright (C) 2007  Michael M. Morrison   All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *****************************************************************************/

/*
 * OpenFlight Parser
 * 
 * Author: Mike Morrison
 *         morrison@users.sourceforge.net
 */

#ifndef _FLT_H_
#define _FLT_H_

// ignore all of this for the docs
//#if ! defined( DOXYGEN_IGNORE )

#if defined(_WIN32)
# if defined(FLTLIB_EXPORTS)
#  define FLTLIB_API __declspec(dllexport)
# else
#  define FLTLIB_API __declspec(dllimport)
# endif
#else
# define FLTLIB_API 
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef float real32;
typedef double real64;

typedef enum {
	FLT_TYPE_NULL	= 0x00,
	FLT_UNKNOWN	= 0x01,
	FLT_UINT8 	= 0x02,
	FLT_UINT16	= 0x04,
	FLT_UINT32	= 0x08,
	FLT_INT8		= 0x10,
	FLT_INT16		= 0x20,
	FLT_INT32		= 0x40,
	FLT_REAL32	= 0x80,
	FLT_REAL64	= 0x0100,
	FLT_ARRAY		= 0x0200,
	FLT_POINTER	= 0x0400,
	FLT_POINTER2	= 0x0800,
	FLT_CHAR	= 0x1000
} FltType;

#ifndef MAX_PATHLEN
#define MAX_PATHLEN 256
#endif

/* macros for endian swapping ptr to ptr */
#define _ENDIAN_16( a, b )	((uint8*)(a))[0] = ((uint8*)(b))[1], \
														((uint8*)(a))[1] = ((uint8*)(b))[0];

#define _ENDIAN_32( a, b )	((uint8*)(a))[0] = ((uint8*)(b))[3], \
														((uint8*)(a))[1] = ((uint8*)(b))[2], \
														((uint8*)(a))[2] = ((uint8*)(b))[1], \
														((uint8*)(a))[3] = ((uint8*)(b))[0];

#define _ENDIAN_64( a, b )	((uint8*)(a))[0] = ((uint8*)(b))[7], \
														((uint8*)(a))[1] = ((uint8*)(b))[6], \
														((uint8*)(a))[2] = ((uint8*)(b))[5], \
														((uint8*)(a))[3] = ((uint8*)(b))[4], \
														((uint8*)(a))[4] = ((uint8*)(b))[3], \
														((uint8*)(a))[5] = ((uint8*)(b))[2], \
														((uint8*)(a))[6] = ((uint8*)(b))[1], \
														((uint8*)(a))[7] = ((uint8*)(b))[0];

/////// taken from SGL sgldb/sgldb.h

#if defined(linux)
#include <endian.h>
#elif defined( _PowerMAXOS )
#include <sys/byteorder.h>
#endif

// are there no WIN32 big-endian targets, or vxworks little endian?

#if defined(WIN32) || defined(_WIN32) || (defined(sgi) && defined(unix) && defined(_MIPSEL)) || (defined(sun) && defined(unix) && !defined(_BIG_ENDIAN)) || (defined(__BYTE_ORDER) && (__BYTE_ORDER == __LITTLE_ENDIAN)) || (defined(__APPLE__) && defined(__LITTLE_ENDIAN__)) || (defined( _PowerMAXOS ) && (BYTE_ORDER == LITTLE_ENDIAN ))
#  define FLTLIB_LITTLE_ENDIAN
#elif (defined(sgi) && defined(unix) && defined(_MIPSEB)) || (defined(sun) && defined(unix) && defined(_BIG_ENDIAN)) || defined(vxw) || (defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN)) || ( defined(__APPLE__) && defined(__BIG_ENDIAN__)) || (defined( _PowerMAXOS ) && (BYTE_ORDER == BIG_ENDIAN) )
#  define FLTLIB_BIG_ENDIAN
#else
#  error unknown endian type
#endif

/////////////////////

#ifdef FLTLIB_LITTLE_ENDIAN
#define ENDIAN_16( d ) { uint16 tmp; _ENDIAN_16( &tmp, &(d) ); (d) = tmp; }
#define ENDIAN_32( d ) { uint32 tmp; _ENDIAN_32( &tmp, &(d) ); (d) = tmp; }
#define ENDIAN_32r( d ) { real32 tmp; _ENDIAN_32( &tmp, &(d) ); (d) = tmp; }
#define ENDIAN_64( d ) { real64 tmp; _ENDIAN_64( &tmp, &(d) ); (d) = tmp; }
#define ENDIAN_64r( d ) { real64 tmp; _ENDIAN_64( &tmp, &(d) ); (d) = tmp; }
#else
#define ENDIAN_16( d ) 
#define ENDIAN_32( d )
#define ENDIAN_32r( d )
#define ENDIAN_64( d )
#define ENDIAN_64r( d )
#endif

typedef struct _FltNode {
	uint32 type;
	uint32 length;
	uint32 treeDepth;
	uint32 byteOffset;
	struct _FltNode * parent;
	struct _FltNode * prev, * next;
	struct _FltNode ** child;
	struct _FltNode * attr;
	uint32 numChildren;
} FltNode;

// coord nodes
#define FLT_COORDS_METERS 0
#define FLT_COORDS_KILOMETERS 1
#define FLT_COORDS_FEET 4
#define FLT_COORDS_INCHES 5
#define FLT_COORDS_NAUTICALMILES 8

// projections
#define FLT_PROJECTION_FLATEARTH 0
#define FLT_PROJECTION_TRAPEZOIDAL 1
#define FLT_PROJECTION_ROUNDEARTH 2
#define FLT_PROJECTION_LAMBERT 3
#define FLT_PROJECTION_UTM 4
#define FLT_PROJECTION_GEOCENTRIC 5
#define FLT_PROJECTION_GEODETIC 6

// ellipsoids
#define FLT_ELLIPSOID_WGS84 0
#define FLT_ELLIPSOID_WGS72 1
#define FLT_ELLIPSOID_BESSEL 2
#define FLT_ELLIPSOID_CLARKE 3
#define FLT_ELLIPSOID_NAD27 4
#define FLT_ELLIPSOID_USER 5

// flags
#define FLTHDRFLAGS_SAVE_VTX_NORMALS (1<<(31-0))
#define FLTHDRFLAGS_RGB_COLOR        (1<<(31-1))  // 'RGB Mode'
#define FLTHDRFLAGS_CADVIEW_MODE     (1<<(31-2))

#define FLTRECORD_HEADER 1

// macros to help parsing differences
// note - this actually checks to see if we are < 15.4 because there were
// most changes between 14.2/15.0 and 15.41, and then after 16.0
#define FLT_IS_V13X(_ff) ( (_ff)->header->formatRevision < 1420 )
#define FLT_IS_V14X(_ff) ( ((_ff)->header->formatRevision < 1541) && \
                           ((_ff)->header->formatRevision > 1410) )
#define FLT_IS_V15X(_ff) ( ((_ff)->header->formatRevision < 1600) && \
                           ((_ff)->header->formatRevision > 1540) )
#define FLT_IS_V16X(_ff) ( ((_ff)->header->formatRevision < 1700) && \
                           ((_ff)->header->formatRevision > 1599) )

typedef struct _FltHeader {
	FltNode node;
	char		ID[8];
	uint32	formatRevision;
	uint32	editRevision;
	char		dateTime[32];
	uint16	nextGroupNodeID;
	uint16	nextLODNodeID;
	uint16	nextObjectNodeID;
	uint16	nextFaceNodeID;
	uint16	unitMultiplier;
	uint8		coordUnits;
	uint8		setTexWhiteOnNewFaces;
	uint32	flags;
	uint32	reserved0[6];
	uint32	projectionType;
	uint32	reserved1[7];
	uint16	nextDOFNodeID;
	uint16	vertexStorageLength;
	uint32	databaseOrigin;
	real64	swDBCoordX;
	real64	swDBCoordY;
	real64	deltaXPlace;
	real64	deltaYPlace;
	uint16	nextSoundNodeID;
	uint16	nextPathNodeID;
	uint32	reserved2[2];
	uint16	nextClipNodeID;
	uint16	nextTextNodeID;
	uint16	nextBSPNodeID;
	uint16	nextSwitchNodeID;
	uint32	reserved3;
	real64	swDBLatitude;
	real64	swDBLongitude;
	real64	neDBLatitude;
	real64	neDBLongitude;
	real64	originDBLatitude;
	real64	originDBLongitude;
	real64	lambertUpperLatitude;
	real64	lambertLowerLatitude;
	uint16	nextLightSourceNodeID;
	uint16	nextLightPointNodeID;
	uint16	nextRoadNodeID;
	uint16	nextCATNodeID;
	uint16	reserved4;
	uint16	reserved5;
	uint16	reserved6;
	uint16	reserved7;
	uint32	earthEllipsoidModel;
	uint16	nextAdaptiveNodeID;
	uint16	nextCurveNodeID;
	int16		utmZone;
	uint8		reserved8[6];
	real64	deltaZPlace;
	real64	dbRadius;
	uint16	nextMeshNodeID;
	uint16	nextLightPointSystemID;
	uint32	reserved9;
	real64	earthMajorAxis;
	real64	earthMinorAxis;
} FltHeader;

typedef enum {
	FVHAS_NORMAL =	(1<<0),
	FVHAS_COLOR = 	(1<<1),
	FVHAS_TEXTURE = (1<<2)
} FltVertexFlags;

#define FVSTART_HARD_EDGE (1<<(15-0))
#define FVNORMAL_FROZEN		(1<<(15-1))
#define FVNO_COLOR				(1<<(15-2))
#define FVPACKED_COLOR		(1<<(15-3))

#define FLTRECORD_VERTEXC 68			// with color
#define FLTRECORD_VERTEXCN 69			// with color + normal
#define FLTRECORD_VERTEXCNUV 70		// with color + normal + uv tex
#define FLTRECORD_VERTEXCUV 71		// with color + uv tex

typedef struct _FltVertex {
	FltNode node;
	uint16 colorNameIndex;
	uint16 flags;
	real64 x, y, z;
	real32 i, j, k;
	real32 u, v;
	uint32 packedColor;
	uint32 colorIndex;
	FltVertexFlags localFlags;
	uint32 indx;
	uint32 mtMask;									// multitexture mask (not in spec)
	real32 mtU[7], mtV[7];					// multitexture UV if available
																	// max of 7
} FltVertex;

#define FLTRECORD_LIGHTPOINT 111 
#define FLTRECORD_INDEXEDLIGHTPOINT 130 

typedef struct _FltLightPoint {
	FltNode node;
	char ID[8];       // we only support the name for the moment!!
} FltLightPoint;

#define FLTRECORD_LIGHTPOINTSYSTEM 131 

#define FLTLPS_ANIM_ON       0
#define FLTLPS_ANIM_OFF      1
#define FLTLPS_ANIM_RAND     2
#define FLTLPS_FLAGS_ENABLED (1<<(31-0))

typedef struct _FltLightPointSystem {
	FltNode node;
	char ID[8];       
  real32 intensity;
  uint32 animationState;
  uint32 flags;
} FltLightPointSystem;

#define FLTRECORD_SWITCH 96

typedef struct _FltSwitch {
	FltNode node;
	char ID[8];
	uint32 reserved0;
	uint32 currentMask;
	uint32 numUInt32sPerMask;
	uint32 numMasks;
	uint32 * masks; // = numUInt32sPerMask * numMasks * 4
} FltSwitch;

#define FLTRECORD_COMMENT 31
#define FLTRECORD_COMMENT_MAX_LEN 256

typedef struct _FltComment {
	FltNode node;
	char text[FLTRECORD_COMMENT_MAX_LEN];
} FltComment;

#define FLTRECORD_LONGID 33
#define FLTRECORD_LONGID_MAX_LEN 64

typedef struct _FltLongID {
	FltNode node;
	char text[FLTRECORD_LONGID_MAX_LEN];
} FltLongID;

#define FLTRECORD_DOF 14

#define FLTDOF_LIMITX	(1<<(31-0))
#define FLTDOF_LIMITY	(1<<(31-1))
#define FLTDOF_LIMITZ	(1<<(31-2))
#define FLTDOF_LIMITPITCH	(1<<(31-3))
#define FLTDOF_LIMITROLL	(1<<(31-4))
#define FLTDOF_LIMITYAW	(1<<(31-5))
#define FLTDOF_LIMITSCALEX	(1<<(31-6))
#define FLTDOF_LIMITSCALEY	(1<<(31-7))
#define FLTDOF_LIMITSCALEZ	(1<<(31-8))

typedef struct _FltDOF {
	FltNode node;
	char ID[8];
	uint32 reserved0;

	real64 localOriginX;
	real64 localOriginY;
	real64 localOriginZ;

	real64 localPointX;
	real64 localPointY;
	real64 localPointZ;

	real64 localPlanePointX;
	real64 localPlanePointY;
	real64 localPlanePointZ;

	real64 localMinZ;
	real64 localMaxZ;
	real64 localCurZ;
	real64 localIncZ;

	real64 localMinY;
	real64 localMaxY;
	real64 localCurY;
	real64 localIncY;

	real64 localMinX;
	real64 localMaxX;
	real64 localCurX;
	real64 localIncX;

	real64 localMinPitch;		// about X
	real64 localMaxPitch;
	real64 localCurPitch;
	real64 localIncPitch;

	real64 localMinRoll;		// about Y
	real64 localMaxRoll;
	real64 localCurRoll;
	real64 localIncRoll;

	real64 localMinYaw;		// about Z
	real64 localMaxYaw;
	real64 localCurYaw;
	real64 localIncYaw;

	real64 localMinScaleZ;
	real64 localMaxScaleZ;
	real64 localCurScaleZ;
	real64 localIncScaleZ;

	real64 localMinScaleY;
	real64 localMaxScaleY;
	real64 localCurScaleY;
	real64 localIncScaleY;

	real64 localMinScaleX;
	real64 localMaxScaleX;
	real64 localCurScaleX;
	real64 localIncScaleX;

	uint32 flags;
} FltDOF;

#define FLTLOD_USEPREVSLANT (1<<(31-0))
#define FLTLOD_FREEZECENTER (1<<(31-2))

#define FLTRECORD_LOD 73

typedef struct _FltLOD {
	FltNode node;
	char ID[8];
	uint32 reserved0;
	real64 switchInDistance;
	real64 switchOutDistance;
	uint16 specialEffectID1;
	uint16 specialEffectID2;
	uint32 flags;
	real64 centerX;
	real64 centerY;
	real64 centerZ;
	real64 transitionRange;
} FltLOD;

#define FLTRECORD_TEXTUREMAPPINGPALETTE 112

#define FLTTMAPPALETTE_TYPE_NONE 0
#define FLTTMAPPALETTE_TYPE_PUT 1
#define FLTTMAPPALETTE_TYPE_4POINTPUT 2
#define FLTTMAPPALETTE_TYPE_SURFACEPROJ 3
#define FLTTMAPPALETTE_TYPE_SPHERICALPROJ 4
#define FLTTMAPPALETTE_TYPE_RADIALPROJ 5
#define FLTTMAPPALETTE_TYPE_ENVIRONMENTMAP 6

typedef struct _FltTextureMappingPaletteEntry {
  FltNode node;
  char ID[20];                // out of order
  uint32 reserved;
  uint32 index;
  uint32 type;
	uint32 warped;
  // ignore the rest
} FltTextureMappingPaletteEntry;

typedef struct _FltTextureMappingPalette {
	FltNode node;
	uint32 numMappings;
	FltTextureMappingPaletteEntry ** mappings;
} FltTextureMappingPalette;

#define FLTRECORD_SHADERPALETTE 133

#define FLTSHADER_CG   0
#define FLTSHADER_CGFX 1
#define FLTSHADER_GLSL 2

typedef struct _FltShader {
  FltNode node;
  char ID[1024];                    // out of place, but fits
  uint32 index;
  uint32 type;
  // cg shaders only for now
  char  vertexProgramFile[1024];
  char  fragmentProgramFile[1024];
  uint32  vertexProgramProfile;
  uint32  fragmentProgramProfile;
  char  vertexProgramEntry[256];
  char  fragmentProgramEntry[256];
} FltShader;

typedef struct _FltShaderPalette {
	FltNode node;
	uint32 numShaders;
	FltShader ** shaders;
} FltShaderPalette;

#define FLTRECORD_LINESTYLEPALETTE 97

typedef struct _FltLineStyle {
  FltNode node;
  uint16 index;
  uint16 patternMask;
  uint32 lineWidth;
} FltLineStyle;

typedef struct _FltLineStylePalette {
	FltNode node;
	uint32 numStyles;
	FltLineStyle ** styles;
} FltLineStylePalette;

#define FLTRECORD_VERTEXPALETTE 67

typedef struct _FltVertexPalette {
	FltNode node;
	uint32 numVerts;
	FltVertex ** verts;
	uint32 numSearchVerts;						// added to understand how many verts
																		// were in the initial palette
} FltVertexPalette;

#define FLTRECORD_VERTEXLIST 72

typedef struct _FltVertexList {
	FltNode node;
	uint32 numVerts;
	FltVertex ** list;
	uint32 *indexList;
} FltVertexList;

#define FLTGROUP_FORWARD_ANIM (1<<(31-1))
#define FLTGROUP_SWING_ANIM (1<<(31-2))
#define FLTGROUP_BOUNDINGBOX_FOLLOWS (1<<(31-3))
#define FLTGROUP_FREEZE_BOUNDINGBOX (1<<(31-4))
#define FLTGROUP_DEFAULT_PARENT (1<<(31-5))

#define FLTRECORD_GROUP 2

typedef struct _FltGroup {
	FltNode node;
	char ID[8];
	uint16 relativePriority;
	uint16 reserved0;
	uint32 flags;
	uint16 specialEffectID1;
	uint16 specialEffectID2;
	uint16 significance;
	uint8  layerCode;
	uint8  reserved1;
	uint32 reserved2;
} FltGroup;

#define FLTOBJECT_NODISPLAYDAY (1<<(31-0))
#define FLTOBJECT_NODISPLAYDUSK (1<<(31-1))
#define FLTOBJECT_NODISPLAYNIGHT (1<<(31-2))
#define FLTOBJECT_NOILLUMINATION (1<<(31-3))
#define FLTOBJECT_FLATSHADED (1<<(31-4))
#define FLTOBJECT_GROUPSSHADOWOBJECT (1<<(31-5))

#define FLTRECORD_OBJECT 4

typedef struct _FltObject {
	FltNode node;
	char ID[8];
	uint32 flags;
	uint16 relativePriority;
	uint16 transparency;
	uint16 specialEffectID1;
	uint16 specialEffectID2;
	uint16 significance;
	uint16 reserved0;
} FltObject;

#define FLTFACEDT_DRAWSOLIDBACKFACE 0
#define FLTFACEDT_DRAWSOLIDNOBACKFACE 1
#define FLTFACEDT_DRAWWIREFRAME 3
#define FLTFACEDT_DRAWWIREFRAMECLOSE 2 // docs had this backwards?
#define FLTFACEDT_SURROUNDWIREALTCOLOR 4
#define FLTFACEDT_OMNIDIRLIGHT 8
#define FLTFACEDT_UNIDIRLIGHT 9
#define FLTFACEDT_BIDIRLIGHT 10

#define FLTFACEBB_FIXEDNOALPHA 0
#define FLTFACEBB_FIXEDALPHA 1
#define FLTFACEBB_AXIALROTATE 2
#define FLTFACEBB_POINTROTATE 3

#define FLTFACEMF_TERRAIN (1<<(31-0))
#define FLTFACEMF_NOCOLOR (1<<(31-1))
#define FLTFACEMF_NOALTCOLOR (1<<(31-2))
#define FLTFACEMF_PACKEDCOLOR (1<<(31-3))
#define FLTFACEMF_TERRAINCULTURECUTOUT (1<<(31-4))
#define FLTFACEMF_HIDDEN (1<<(31-5))

#define FLTFACELM_FCNOTILLUM 0
#define FLTFACELM_VCNOTILLUM 1
#define FLTFACELM_FCVN 2
#define FLTFACELM_VCVN 3

// format is ABGR
#define FLTPACKED_COLOR_R( x ) ( ((x)>> 0) & 0xff )
#define FLTPACKED_COLOR_G( x ) ( ((x)>> 8) & 0xff )
#define FLTPACKED_COLOR_B( x ) ( ((x)>>16) & 0xff )
#define FLTPACKED_COLOR_A( x ) ( ((x)>>24) & 0xff )

#define FLTPACKEDCOLOR_Rf(c) ( (float)FLTPACKEDCOLOR_R(c) / 255.0f )
#define FLTPACKEDCOLOR_Gf(c) ( (float)FLTPACKEDCOLOR_G(c) / 255.0f )
#define FLTPACKEDCOLOR_Bf(c) ( (float)FLTPACKEDCOLOR_B(c) / 255.0f )
#define FLTPACKEDCOLOR_Af(c) ( (float)FLTPACKEDCOLOR_A(c) / 255.0f )

#define FLTRECORD_FACE 5

typedef struct _FltFace {
	FltNode node;
	char ID[8];
	uint32 irColorCode;
	uint16 relativePriority;
	uint8  drawType;
	uint8  textureWhite;
	uint16 colorNameIndex;
	uint16 alternateColorNameIndex;
	uint8  reserved0;
	uint8  billboardFlags;
	int16  detailTexturePatternIndex;
	int16  texturePatternIndex;
	int16  materialIndex;
	uint16 surfaceMaterialCode;
	uint16 featureID;
	uint32 irMaterialCode;
	uint16 transparency;
	uint8  LODGenerationControl;
	uint8  lineStyleIndex;
	uint32 miscFlags;
	uint8  lightMode;
	uint8  reserved1;
	uint16 reserved2;
	uint32 reserved3;
	uint32 packedColorPrimary;
	uint32 packedColorAlternate;
	uint16 textureMappingIndex;
	uint16 reserved4;
	uint32 primaryColorIndex;
	uint32 alternateColorIndex;
	uint16 reserved5;
	int16 shaderIndex;
} FltFace;

#define FLTRECORD_TEXTURE 64

typedef struct _FltTexture {
	FltNode node;
	char ID[200];
	uint32 index;
	uint32 xloc;
	uint32 yloc;
} FltTexture;

#define FLTRECORD_COLORPALETTE 32

typedef struct _FltColorPalette {
	FltNode node;
	uint32 color[1024];
} FltColorPalette;

#define FLTRECORD_MATRIX 49

typedef struct _FltMatrix {
	FltNode node;
	real32 matrix[16]; // single precision, row major
	real32 inverseTranspose[9]; // Convenience - 3x3 inverse transpose for normals
} FltMatrix;

#define FLTRECORD_GENERALMATRIX 94

typedef struct _FltGeneralMatrix {
	FltNode node;
	real32 matrix[16]; // single precision, row major
} FltGeneralMatrix;

#define FLTRECORD_NONUNIFORMSCALE 79

typedef struct _FltNonuniformScale {
	FltNode node;
	uint32 reserved;
	real64 centerX;
	real64 centerY;
	real64 centerZ;
	real32 scaleX;
	real32 scaleY;
	real32 scaleZ;
} FltNonuniformScale;

#define FLTRECORD_TRANSLATE 78

typedef struct _FltTranslate {
	FltNode node;
	uint32 reserved;
	real64 fromX;
	real64 fromY;
	real64 fromZ;
	real64 deltaX;
	real64 deltaY;
	real64 deltaZ;
} FltTranslate;

#define FLTRECORD_REPLICATE 60

typedef struct _FltReplicate {
	FltNode node;
	uint16 replications;
	uint16 reserved;
} FltReplicate;

#define FLTRECORD_ROTATEABOUTPOINT 80

typedef struct _FltRotateAboutPoint {
	FltNode node;
	uint32 reserved0;
	real64 centerX, centerY, centerZ;
	real32 rotI, rotJ, rotK;
	real32 rotAngle;
} FltRotateAboutPoint;


#define FLTRECORD_INSTANCEDEFINITION 62

typedef struct _FltInstanceDefinition {
	FltNode node;
	uint16 reserved0;
	uint16 instance;
} FltInstanceDefinition;

#define FLTRECORD_INSTANCEREFERENCE 61

typedef struct _FltInstanceReference {
	FltNode node;
	uint16 reserved0;
	uint16 instance;
} FltInstanceReference;

#define FLTRECORD_BSP 55

typedef struct _FltBSP {
	FltNode node;
	char ID[8];
	uint32 reserved0;
	real64 coefA, coefB, coefC, coefD;
} FltBSP;

#define FLTRECORD_MULTITEXTURE 52

// layer defs
#define FLTMT_HASLAYER1 (1<<(31-0))
#define FLTMT_HASLAYER2 (1<<(31-1))
#define FLTMT_HASLAYER3 (1<<(31-2))
#define FLTMT_HASLAYER4 (1<<(31-3))
#define FLTMT_HASLAYER5 (1<<(31-4))
#define FLTMT_HASLAYER6 (1<<(31-5))
#define FLTMT_HASLAYER7 (1<<(31-6))
#define FLTMT_HASLAYER(x) (1<<(31-((x)-1)))

// effect defs > 100 user defined
#define FLTMTEFFECT_ENV  0
#define FLTMTEFFECT_BUMP 1

typedef struct _FltMultiTexture {
	FltNode node;
	uint32  mask;
	struct {
		uint16  index;
		uint16  effect;
		uint16  mapping;
		uint16  data;
	} layer[8];
} FltMultiTexture;

#define FLTRECORD_UVLIST 53

typedef struct _FltUVList {
	FltNode node;
	uint32  mask;
	uint32  numValues;			// number of floats in list
	real32 * uvValues;			// numUVSets floats  interpretation depends on
													// mask and how many verts are in the vertexlist
													// that we follow
} FltUVList;

#define FLTRECORD_ROADSEGMENT 87

typedef struct _FltRoadSegment {
	FltNode node;
	char ID[8];
} FltRoadSegment;

#define FLTRECORD_ROADCONSTRUCTION 127

#define FLTRCROADTYPE_CURVE 0
#define FLTRCROADTYPE_HILL  1
#define FLTRCROADTYPE_STRAIGHT 2

#define FLTRCSPIRALTYPE_LINEARLENGTH 0
#define FLTRCSPIRALTYPE_LINEARANGLE  1
#define FLTRCSPIRALTYPE_COSINELENGTH 2

typedef struct _FltRoadConstruction {
	FltNode node;
	char ID[8];
	uint32 reserved0;
	uint32 roadType;
	uint32 roadtoolsVersion;
	real64 entryX;
	real64 entryY;
	real64 entryZ;
	real64 alignmentX;
	real64 alignmentY;
	real64 alignmentZ;
	real64 exitX;
	real64 exitY;
	real64 exitZ;
	real64 arcRadius;
	real64 entrySpiralLength;
	real64 exitSpiralLength;
	real64 superelevation;
	uint32 spiralType;
	uint32 verticalParabolaFlag;
	real64 verticalCurveLength;
	real64 minimumCurveLength;
	real64 entrySlope;
	real64 exitSlope;
} FltRoadConstruction;

#define FLTRECORD_ROADPATH 92

#define FLTRPNORMALTYPE_UPVEC 0
#define FLTRPNORMALTYPE_HPR   1

typedef struct _FltRoadPath {
	FltNode node;
	char ID[8];
	uint32 reserved0;
	char pathName[120];
	real64 speedLimit;
	uint32 noPassing;
	uint32 vertexNormalType;
	uint8  spare[480];
} FltRoadPath;

#define FLTRECORD_MESH 84

typedef struct _FltMesh {
	FltNode node;
	char ID[8];
	int32  irColorCode;
	int16  relativePriority;
	int8   drawType;
	int8   textureWhite;
	uint16 colorNameIndex;
	uint16 alternateColorNameIndex;
	int8   reserved0;
	int8   billboardFlags;
	int16  detailTexturePatternIndex;
	int16  texturePatternIndex;
	int16  materialIndex;
	int16  surfaceMaterialCode;
	int16  featureID;
	int32  irMaterialCode;
	uint16 transparency;
	uint8  LODGenerationControl;
	uint8  lineStyleIndex;
	uint32 miscFlags;
	uint8  lightMode;
	uint8  reserved1;
	uint16 reserved2;
	uint32 reserved3;
	uint32 packedColorPrimary;
	uint32 packedColorAlternate;
	int16  textureMappingIndex;
	int16  reserved4;
	uint32 primaryColorIndex;
	uint32 alternateColorIndex;
	int16  reserved5;
	int16  shaderIndex;
} FltMesh;

#define FLTRECORD_LOCALVERTEXPOOL 85

#define FLTLVPATTR_POSITION    (1<<(31-0))
#define FLTLVPATTR_COLORINDEX  (1<<(31-1))
#define FLTLVPATTR_PACKEDCOLOR (1<<(31-2))
#define FLTLVPATTR_NORMAL      (1<<(31-3))
#define FLTLVPATTR_UV0         (1<<(31-4))
#define FLTLVPATTR_UV1         (1<<(31-5))
#define FLTLVPATTR_UV2         (1<<(31-6))
#define FLTLVPATTR_UV3         (1<<(31-7))
#define FLTLVPATTR_UV4         (1<<(31-8))
#define FLTLVPATTR_UV5         (1<<(31-9))
#define FLTLVPATTR_UV6         (1<<(31-10))
#define FLTLVPATTR_UV7         (1<<(31-11))

typedef struct _FltLVPEntry {
	real64 x, y, z;
	uint32 color;
	real32 i, j, k;
	real32 u0,v0;
	real32 u1,v1;
	real32 u2,v2;
	real32 u3,v3;
	real32 u4,v4;
	real32 u5,v5;
	real32 u6,v6;
	real32 u7,v7;
} FltLVPEntry;

typedef struct _FltLocalVertexPool {
	FltNode node;
	uint32  numVerts;
	uint32  attrMask;
	FltLVPEntry * entries;
} FltLocalVertexPool;

#define FLTRECORD_MESHPRIMITIVE 86

#define FLTMESHPRIM_TRISTRIP  1
#define FLTMESHPRIM_TRIFAN    2
#define FLTMESHPRIM_QUADSTRIP 3
#define FLTMESHPRIM_POLYINDEX 4

typedef struct _FltMeshPrimitive {
	FltNode node;
	uint16  primitiveType;
	uint16  vertIndexLength;
	uint32  numVerts;
	uint32  *indices;
} FltMeshPrimitive;


#define FLTRECORD_MATERIAL 113

typedef struct _FltMaterial {
	FltNode node;
	char ID[12];		// out of order, but matches everything else
	uint32 index;
	uint32 flags;
	real32 ambientRed;
	real32 ambientGreen;
	real32 ambientBlue;
	real32 diffuseRed;
	real32 diffuseGreen;
	real32 diffuseBlue;
	real32 specularRed;
	real32 specularGreen;
	real32 specularBlue;
	real32 emissiveRed;
	real32 emissiveGreen;
	real32 emissiveBlue;
	real32 shininess;		// 0->128.0
	real32 alpha;				// 0->1.0 1.0 = opaque
	uint32 spare;
} FltMaterial;

// this record is defined as "obsolete"
#define FLTRECORD_MATERIAL_TABLE 66

//
// Material table defines 64 material types, rather than
// a configurable number like the later material palette.
// This struct is used as a reference only.
//
typedef struct _FltMaterialTableEntry {
	real32 ambientRed;
	real32 ambientGreen;
	real32 ambientBlue;
	real32 diffuseRed;
	real32 diffuseGreen;
	real32 diffuseBlue;
	real32 specularRed;
	real32 specularGreen;
	real32 specularBlue;
	real32 emissiveRed;
	real32 emissiveGreen;
	real32 emissiveBlue;
	real32 shininess;		// 0->128.0
	real32 alpha;				// 0->1.0 1.0 = opaque
	uint32 flags;
  char   ID[12];
	uint32 spare[28];
} FltMaterialTableEntry;

#define FLTRECORD_NAME_TABLE 114

typedef struct _FltNameTableEntry {
  uint32 length;
  uint16 index;
  char   name[80];  // max len of 80
} FltNameTableEntry;

typedef struct _FltNameTable {
  FltNode node;
  uint32 numNames;
  uint16 nextIndex;
  FltNameTableEntry ** entries;
} FltNameTable;

typedef struct _FltMaterialPalette {
	FltNode node;
	FltMaterial ** material;
	uint32 numMaterials;
} FltMaterialPalette;

#define FLTRECORD_EXTERNALREFERENCE 63

// flags bits
#define FLTEXTREF_COLORPALETTE_OVERRIDE (1<<(31-0))
#define FLTEXTREF_MATERIALPALETTE_OVERRIDE (1<<(31-1))
#define FLTEXTREF_TEXTUREPALETTE_OVERRIDE (1<<(31-2))
#define FLTEXTREF_LINESTYLEPALETTE_OVERRIDE (1<<(31-3))
#define FLTEXTREF_SOUNDPALETTE_OVERRIDE (1<<(31-4))
#define FLTEXTREF_LIGHTSOURCEPALETTE_OVERRIDE (1<<(31-5))
#define FLTEXTREF_LIGHTPOINTPALETTE_OVERRIDE (1<<(31-6))
#define FLTEXTREF_SHADERPALETTE_OVERRIDE (1<<(31-7))

typedef struct _FltExternalReference {
	FltNode node;
	char path[200];		// path to extref filename<node name>
	uint8 reserved0;
	uint8 reserved1;
	uint16 reserved2;
	uint32 flags;
	uint16 reserved3;
	uint16 reserved4;
} FltExternalReference;

#define FLTRECORD_LIGHTSOURCEPALETTEENTRY 102

#define FLTLIGHT_INFINITE 0		// directional
#define FLTLIGHT_LOCAL    1		// point
#define FLTLIGHT_SPOT     2		// spot

typedef struct _FltLightSourcePaletteEntry
{
	FltNode node;
	uint32 index;
	uint32 reserved0[2];
	char   ID[20];
	uint32 reserved1;
	real32 ambient[4];		// r g b a
	real32 diffuse[4];		// r g b a
	real32 specular[4];		// r g b a
	uint32 type;
	uint32 reserved2[10];
	real32 spotExponent;
	real32 spotCutoff;
	real32 yaw; 
	real32 pitch;
	real32 attenC;
	real32 attenL;
	real32 attenQ;
	uint32 activeDuringModeling;
	real32 reserved3[19];
} FltLightSourcePaletteEntry;

typedef struct _FltLightSourcePalette
{
	FltLightSourcePaletteEntry ** lights;
	uint32 numLights;
} FltLightSourcePalette;

#define FLTRECORD_LIGHTSOURCE 101

#define FLTRECORD_LSENABLED	(1<<(31-0))
#define FLTRECORD_LSGLOBAL	(1<<(31-1))
#define FLTRECORD_LSEXPORT	(1<<(31-3))

typedef struct _FltLightSource
{
	FltNode node;
	char   ID[8];
	uint32 reserved0;
	uint32 paletteIndex;
	uint32 reserved1;
	uint32 flags;
	uint32 reserved2;
	real64 position[3];		// local or spot only
	real32 yaw; 					// az - potential override
	real32 pitch;					// el - potentail override
} FltLightSource;

typedef struct _FltBuffer {
	uint8 *buffer;
	uint8 *curPtr;
	uint32 bytesRead;
	uint32 bytesRemaining;
	uint32 readLength; // length of last read to buffer
	uint32 bufferSize;	// current total size of buffer
} FltBuffer;

typedef struct _FltStack {
	void ** stack;
	uint32 stackPtr;
	uint32 stackSize;
} FltStack;

typedef struct {
	char * head, * tail;
	int last;
} MStrtokCtxt;

#define FLTFILE_FLAGS_FIRSTPUSHSEEN 0x01

typedef struct _FltFile {
	FltNode node;
	FILE * stdFile;
	FltHeader * header;
	FltLightSourcePalette * lightSourcePalette;
	FltVertexPalette * vertPalette;
	FltColorPalette * colorPalette;
	FltMaterialPalette * materialPalette;
	FltLineStylePalette * lineStylePalette;
	FltShaderPalette * shaderPalette;
	FltTextureMappingPalette * textureMappingPalette;
	FltBuffer * buffer;
	FltStack * stack;
	FltNode * lastNode;
	FltNode * lastParent;
	FltNode ** allNodes;
	real32 coordScale;
	uint32 numNodes;
	uint32 byteOffset;
	uint32 treeDepth;
  int32  ignoreExtension;
  int32  flags;
  uint32 fileID;
	uint32 xformUnits;
	real64 extents[6];  /* swX, swY, neX, neY */
	char fileName[MAX_PATHLEN];

	// for path building
	struct {
		char * searchPath;
		int first;
		MStrtokCtxt stc;
	} pathInfo;
} FltFile;

// add nodes that are attributes here

#define FLTNODE_ISATTRIBUTE(xn) ( (xn) == FLTRECORD_COMMENT || \
																	(xn) == FLTRECORD_MATRIX  || \
																	(xn) == FLTRECORD_GENERALMATRIX  || \
																	(xn) == FLTRECORD_NONUNIFORMSCALE  || \
																	(xn) == FLTRECORD_TRANSLATE  || \
																	(xn) == FLTRECORD_MULTITEXTURE  || \
																	(xn) == FLTRECORD_REPLICATE  || \
																	(xn) == FLTRECORD_UVLIST  || \
																	(xn) == FLTRECORD_LOCALVERTEXPOOL || \
																	(xn) == FLTRECORD_LONGID )

// add nodes that can be a parent node here

#define FLTNODE_CANPARENT(xn) ( (xn) == FLTRECORD_HEADER || \
																(xn) == FLTRECORD_SWITCH || \
																(xn) == FLTRECORD_DOF || \
																(xn) == FLTRECORD_LOD || \
																(xn) == FLTRECORD_GROUP || \
																(xn) == FLTRECORD_OBJECT || \
																(xn) == FLTRECORD_BSP || \
                                (xn) == FLTRECORD_LIGHTPOINTSYSTEM || \
																(xn) == FLTRECORD_MESH || \
																(xn) == FLTRECORD_ROADPATH || \
																(xn) == FLTRECORD_ROADCONSTRUCTION || \
																(xn) == FLTRECORD_INSTANCEDEFINITION || \
                                (xn) == FLTRECORD_LIGHTPOINT || \
                                (xn) == FLTRECORD_INDEXEDLIGHTPOINT || \
																(xn) == FLTRECORD_FACE )

// add nodes that contain an 'ID' or are named.

#define FLTNODE_HASID(xn) ( (xn) == FLTRECORD_HEADER || \
														(xn) == FLTRECORD_SWITCH || \
														(xn) == FLTRECORD_SHADERPALETTE || \
														(xn) == FLTRECORD_TEXTUREMAPPINGPALETTE || \
														(xn) == FLTRECORD_LIGHTPOINTSYSTEM || \
														(xn) == FLTRECORD_LIGHTPOINT || \
														(xn) == FLTRECORD_INDEXEDLIGHTPOINT || \
														(xn) == FLTRECORD_DOF || \
														(xn) == FLTRECORD_LOD || \
														(xn) == FLTRECORD_GROUP || \
														(xn) == FLTRECORD_OBJECT || \
														(xn) == FLTRECORD_MATERIAL || \
														(xn) == FLTRECORD_BSP || \
														(xn) == FLTRECORD_ROADPATH || \
														(xn) == FLTRECORD_ROADCONSTRUCTION || \
														(xn) == FLTRECORD_ROADSEGMENT || \
														(xn) == FLTRECORD_MESH || \
														(xn) == FLTRECORD_LIGHTSOURCE || \
														(xn) == FLTRECORD_LIGHTSOURCEPALETTEENTRY || \
														(xn) == FLTRECORD_FACE )

#define FLT_GETPARENT(f) ( FltStackGetCurrent(f->stack) )

#define FLTRECORDFUNC_ARGLIST FltFile * flt, uint16 recLength
#define FLTPOSTPROCESSFUNC_ARGLIST FltFile * flt, FltNode * node
#define FLTRECORDUSERCALLBACK_ARGLIST FltFile *flt, void *recData, void *userData

typedef void * (*FltRecordFunc)( FLTRECORDFUNC_ARGLIST );
typedef void * (*FltPostProcessFunc)( FLTPOSTPROCESSFUNC_ARGLIST );
typedef void * (*FltRecordUserCallback)( FLTRECORDUSERCALLBACK_ARGLIST );

typedef struct _FltRecordEntryName {
	char * type;
	char * name;
	uint32 offset;
} FltRecordEntryName;

typedef struct _FltRecord {
	uint16 opcode;
	char * name;
	FltRecordFunc func;
	FltRecordEntryName * entry;
	FltPostProcessFunc postProcessFunc;
	FltRecordUserCallback userFunc;
	void * userData;
} FltRecord;

typedef enum {
	FLTATTR_MIN_POINT = 0,
	FLTATTR_MIN_BILINEAR = 1,
	FLTATTR_MIN_MIPMAP = 2, // obsolete
	FLTATTR_MIN_MIPMAP_POINT = 3,
	FLTATTR_MIN_MIPMAP_LINEAR = 4,
	FLTATTR_MIN_MIPMAP_BILINEAR = 5,
	FLTATTR_MIN_MIPMAP_TRILINEAR = 6,
	FLTATTR_MIN_NONE = 7
	// todo: add others
} FltAttrMinification;

typedef enum {
	FLTATTR_MAG_POINT = 0,
	FLTATTR_MAG_BILINEAR = 1,
	FLTATTR_MAG_NONE = 2,
	FLTATTR_MAG_BICUBIC = 3,
	FLTATTR_MAG_SHARPEN = 4,
	FLTATTR_MAG_ADD_DETAIL = 5,
	FLTATTR_MAG_MODULATE_DETAIL = 6
	// todo: add others
} FltAttrMagnification;

typedef enum {
	FLTATTR_REP_REPEAT = 0,
	FLTATTR_REP_CLAMP = 1,
	FLTATTR_REP_NONE = 3,
	FLTATTR_REP_MIRROR = 4
} FltAttrRepetition;

typedef enum {
	FLTATTR_ENV_MODULATE = 0,
	FLTATTR_ENV_BLEND = 1,
	FLTATTR_ENV_DECAL = 2,
	FLTATTR_ENV_REPLACE = 3,
	FLTATTR_ENV_ADD = 4
} FltAttrEnvironment;

typedef struct _FltTxAttributes {
	uint32 uTexels;
	uint32 vTexels;
	uint32 realWorldU;
	uint32 realWorldV;
	uint32 upVectorX;
	uint32 upVectorY;
	uint32 fileFormat;
	uint32 minificationFilter;
	uint32 magnificationFilter;
	uint32 repetitionType;
	uint32 repetitionU;
	uint32 repetitionV;
	uint32 modifyFlag;
	uint32 xPivot;
	uint32 yPivot;
	uint32 environmentType;
	uint32 isIntensity;
	real64 realWorldSizeU;
	real64 realWorldSizeV;
	// skip a bunch
  uint32 detailTexture;
  uint32 detailJ, detailK, detailM, detailN;
  uint32 detailScramble;
	// add others
} FltTxAttributes;

// public
FLTLIB_API FltFile * fltOpen( const char * name );
FLTLIB_API void fltClose( FltFile *theFile );
FLTLIB_API void fltFileFree( FltFile *theFile );
FLTLIB_API int fltParse( FltFile *theFile, uint32 skip );
FLTLIB_API int fltRegisterRecordUserCallback( uint16 record, FltRecordUserCallback, 
																														void * userData );
FLTLIB_API FltRecord * fltRecordGetDefinition( uint16 record );
FLTLIB_API char * fltNodeName( FltNode * node );
FLTLIB_API char * fltSafeNodeName( FltNode * node );
FLTLIB_API void fltDumpNode( FltNode * node );
FLTLIB_API FltType fltStringToType( char * string );
FLTLIB_API void fltRecordEntryNameToString( char * string, FltNode * node, 
																										FltRecordEntryName * e );
FLTLIB_API uint32 fltLookupColor( FltFile * flt, uint32 colorIndex, real32 *r, 
																						real32 *g, real32 *b, real32 *a );
FLTLIB_API FltTexture * fltLookupTexture( FltFile * flt, uint16 indx );
FLTLIB_API FltMaterial * fltLookupMaterial( FltFile * flt, uint16 indx );
FLTLIB_API FltLightSourcePaletteEntry * 
	fltLookupLightSource( FltFile * flt, uint32 lightIndex );
FLTLIB_API FltLineStyle * fltLookupLineStyle( FltFile * flt, uint16 idx );
FLTLIB_API FltShader * fltLookupShader( FltFile * flt, uint32 idx );
FLTLIB_API FltTextureMappingPaletteEntry * fltLookupTextureMapping( 																													FltFile * flt, uint32 idx );
FLTLIB_API FltInstanceDefinition * fltLookupInstance( FltFile * flt, uint16 indx );
FLTLIB_API FltTxAttributes * fltLoadAttributes( const char * file );
FLTLIB_API void fltFreeAttributes( FltTxAttributes * );
FLTLIB_API int fltFindFile ( FltFile *, char * inFileName, char *outFileName );
FLTLIB_API void fltAddSearchPath( FltFile * flt, char * path );
FLTLIB_API void fltCopySearchPaths( FltFile * to, FltFile * from );
FLTLIB_API FltNode * fltFindFirstAttrNode( FltNode * node, uint32 type );
FLTLIB_API int fltGetMultiTextureCount( FltVertexList * vlist );
FLTLIB_API float * fltGetMultiTextureCoords( FltVertexList * vlist, int idx );

FLTLIB_API int fltFinite64( real64 value );
/* returns swX swY neX neY */
FLTLIB_API void fltGetExtentsLatLon( FltFile * flt, real64 extents[4] );
FLTLIB_API void fltGetOriginLatLon( FltFile * flt, real64 origin[2] );
/* returns swX swY neX neY zMin zMax*/
FLTLIB_API void fltGetExtents( FltFile * flt, real64 extents[6] );
FLTLIB_API uint32 fltGetProjection( FltFile * flt );
FLTLIB_API uint32 fltGetEllipsoidModel( FltFile * flt );
FLTLIB_API void fltGetUTMZoneAndHemisphere( FltFile * flt, int *z, int *h );

FLTLIB_API void fltSetFileID( FltFile * flt, uint32 value );
FLTLIB_API uint32 fltGetFileID( FltFile * flt );

// private
FLTLIB_API int fltReadRecordAttr( FltFile * flt, uint16 * type, uint16 * length );
FLTLIB_API void fltSkipRecord( FltFile * flt, uint16 length );

FLTLIB_API void * FltBufferResize( FltBuffer *, uint32 size );
FLTLIB_API void FltBufferRewind( FltBuffer * );
FLTLIB_API void FltBufferResetWithLength( FltBuffer *, uint32 len );
FLTLIB_API FltBuffer * FltBufferAlloc( FltFile * );
FLTLIB_API void FltBufferFree( FltFile * flt );

FLTLIB_API void * FltStackAlloc( FltFile * );
FLTLIB_API void FltStackFree( FltFile * flt );
FLTLIB_API void FltStackPush( FltStack *, void * );
FLTLIB_API void * FltStackPop( FltStack * );
FLTLIB_API void * FltStackGetCurrent( FltStack * );

FLTLIB_API int fltReadBlock( FltFile * flt, void * data, uint32 length );
FLTLIB_API int fltSkipBlock( FltFile * flt, uint32 len );
FLTLIB_API uint32 fltReadUInt32( FltFile * flt );
FLTLIB_API int32 fltReadInt32( FltFile * flt );
FLTLIB_API uint16 fltReadUInt16( FltFile * flt );
FLTLIB_API int16 fltReadInt16( FltFile * flt );
FLTLIB_API uint8 fltReadUInt8( FltFile * flt );
FLTLIB_API int8 fltReadInt8( FltFile * flt );
FLTLIB_API real32 fltReadReal32( FltFile * flt );
FLTLIB_API real64 fltReadReal64( FltFile * flt );


// MISC macros 

#ifndef offsetof
#define offsetof( structPointer, element ) \
													(size_t)&( ((structPointer)((void *)0))->element )
#endif

#define elementat( structPointer, theStructPtr, elemType, element ) \
		*(elemType *)( (char *)theStructPtr + \
										(size_t)offsetof( structPointer, element ) )
																											

#ifdef __cplusplus
}
#endif

//#endif /* DOXYGEN_IGNORE */

#endif /* _FLT_H_ */



