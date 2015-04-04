

#pragma once

/*
=====================================================================
	OPAQUE HANDLES
=====================================================================
*/
#define LLGL_DEFAULT_ID		(0)
#define LLGL_NULL_HANDLE	(~0)

mxDECLARE_8BIT_HANDLE(HDepthStencilState);
mxDECLARE_8BIT_HANDLE(HRasterizerState);
mxDECLARE_8BIT_HANDLE(HSamplerState);
mxDECLARE_8BIT_HANDLE(HBlendState);
mxDECLARE_8BIT_HANDLE(HInputLayout);
mxDECLARE_8BIT_HANDLE(HColorTarget);
mxDECLARE_8BIT_HANDLE(HDepthTarget);
mxDECLARE_16BIT_HANDLE(HTexture);
mxDECLARE_16BIT_HANDLE(HBuffer);	// generic buffer handle (e.g. vertex, index, uniform)
mxDECLARE_16BIT_HANDLE(HShader);	// handle to a shader object (e.g. vertex, pixel)
mxDECLARE_16BIT_HANDLE(HProgram);	// handle to a program object
// shader resource is everything that can be sampled in a shader: textures, render targets, UAVs, etc.
mxDECLARE_16BIT_HANDLE(HResource);
mxDECLARE_POINTER_HANDLE(HContext);	// device window
mxDECLARE_POINTER_HANDLE(HPipeline);

mxREFLECT_AS_BUILT_IN_INTEGER(HDepthStencilState);
mxREFLECT_AS_BUILT_IN_INTEGER(HRasterizerState);
mxREFLECT_AS_BUILT_IN_INTEGER(HSamplerState);
mxREFLECT_AS_BUILT_IN_INTEGER(HBlendState);
mxREFLECT_AS_BUILT_IN_INTEGER(HResource);
mxREFLECT_AS_BUILT_IN_INTEGER(HProgram);
mxREFLECT_AS_BUILT_IN_INTEGER(HShader);
mxREFLECT_AS_BUILT_IN_INTEGER(HBuffer);

// string names are used for debugging
#define mxHASH_STR(STATIC_STRING)	GetStaticStringHash(STATIC_STRING)
#define mxHASH_ID(NAME)				GetStaticStringHash(TO_STR(NAME))

struct NamedObject : CStruct
{
	String		name;	//<= can be empty in release builds
	UINT32		hash;	// name hash, always valid
public:
	mxDECLARE_CLASS(NamedObject, CStruct);
	mxDECLARE_REFLECTION;
	NamedObject();
	void UpdateNameHash();	// uses GetDynamicStringHash()
	bool Equals( const NamedObject& other ) const;
};

/*
=====================================================================
	TEXTURES AND RENDER TARGETS
=====================================================================
*/
// texture format for storing color information
struct TextureFormat {
	enum Enum {
		// Block-compressed formats start here:
		BC1, // DXT1 (RGB compression: 8:1, 8 bytes per block)
		BC2, // DXT3 (RGBA compression: 4:1, 16 bytes per block)
		BC3, // DXT5 (RGBA compression: 4:1, 16 bytes per block)
		BC4, // LATC1/ATI1 1 component texture compression (also 3DC+/ATI1N, 8 bytes per block)
		BC5, // LATC2/ATI2 2 component texture compression (luminance & alpha compression 4:1 -> normal map compression, also 3DC/ATI2N, 16 bytes per block)
		// Uncompressed formats start here:
		Unknown,
		//L8,
		BGRA8,	// DXGI_FORMAT_B8G8R8A8_UNORM / D3DFMT_A8R8G8B8
		//RGBA16,
		//RGBA16F,
		//R5G6B5,
		//RGBA4,
		RGBA8,	// DXGI_FORMAT_R8G8B8A8_UNORM
		//RGB5A1,
		//RGB10A2,
		R11G11B10,
		//R32f,	// DXGI_FORMAT_R32_FLOAT

		Count	//<= Marker. Don't use!
	};
	static bool IsCompressed( Enum _format );	// returns true if the given format is block-compressed
	static UINT BitsPerPixel( Enum _format );
	static UINT GetBlockSize( Enum _format );	// returns the texture block size in elements
};
mxDECLARE_ENUM( TextureFormat::Enum, UINT8, TextureFormatT );

struct DepthStencilFormat {
	enum Enum {
		D16,
		D24S8,
		D32,
		Count	//<= Marker. Don't use!
	};
	static UINT BitsPerPixel( Enum format );
};
mxDECLARE_ENUM( DepthStencilFormat::Enum, UINT8, DepthStencilFormatT );

struct TextureType {
	enum Enum {
		TEXTURE_1D,
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_CUBE,
		TEXTURE_1D_ARRAY,
		TEXTURE_2D_ARRAY,
		TEXTURE_CUBE_ARRAY
	};
};
mxDECLARE_ENUM( TextureType::Enum, UINT8, TextureTypeT );

struct CubeFace {
	enum Enum {
		PosX,	NegX,
		PosY,	NegY,
		PosZ,	NegZ
	};
};
mxDECLARE_ENUM( CubeFace::Enum, UINT8, CubeFaceT );

struct Texture2DDescription : NamedObject {
	TextureFormatT	format;
	UINT16			width;
	UINT16			height;
	UINT8			numMips;
	bool			dynamic;	// can be updated by CPU
public:
	mxDECLARE_CLASS(Texture2DDescription, NamedObject);
	mxDECLARE_REFLECTION;
	Texture2DDescription();
};
struct Texture3DDescription : NamedObject {
	TextureFormatT	format;
	UINT16			width;
	UINT16			height;
	UINT8			depth;
	UINT8			numMips;
public:
	mxDECLARE_CLASS(Texture3DDescription, NamedObject);
	mxDECLARE_REFLECTION;
};

struct ColorTargetDescription : NamedObject {
	TextureFormatT		format;
	UINT16				width;
	UINT16				height;
public:
	mxDECLARE_CLASS(ColorTargetDescription, NamedObject);
	mxDECLARE_REFLECTION;
};
// describes a depth-stencil surface
struct DepthTargetDescription : NamedObject {
	DepthStencilFormatT	format;
	UINT16				width;
	UINT16				height;
	bool				sample;	// Can it be bound as a shader resource?
public:
	mxDECLARE_CLASS(DepthTargetDescription, NamedObject);
	mxDECLARE_REFLECTION;
	DepthTargetDescription();
};

struct Resolution
{
	UINT16	width;
	UINT16	height;
};
struct Viewport : Resolution
{
	UINT16	x;	// top left x
	UINT16	y;	// top left y
};

struct Rectangle64
{
	UINT16	left;	// The upper-left corner x-coordinate.
	UINT16	top;	// The upper-left corner y-coordinate.
	UINT16	right;	// The lower-right corner x-coordinate.
	UINT16	bottom;	// The lower-right corner y-coordinate.
};


/*
=====================================================================
	RENDER STATES
=====================================================================
*/
struct ComparisonFunc {
	enum Enum {
		Always,
		Never,
		Less,
		Equal,
		Greater,
		Not_Equal,
		Less_Equal,
		Greater_Equal,
	};
};
mxDECLARE_ENUM( ComparisonFunc::Enum, UINT8, ComparisonFuncT );

struct FillMode {
	enum Enum {
		Solid,
		Wireframe,
	};
};
mxDECLARE_ENUM( FillMode::Enum, UINT8, FillModeT );

struct CullMode {
	enum Enum {
		None,
		Back,
		Front,
	};
};
mxDECLARE_ENUM( CullMode::Enum, UINT8, CullModeT );

// Filtering options to use for minification, magnification and mip-level sampling during texture sampling.
struct TextureFilter {
    enum Enum {
        Min_Mag_Mip_Point,
        Min_Mag_Point_Mip_Linear,
        Min_Point_Mag_Linear_Mip_Point,
        Min_Point_Mag_Mip_Linear,
        Min_Linear_Mag_Mip_Point,
        Min_Linear_Mag_Point_Mip_Linear,
        Min_Mag_Linear_Mip_Point,
        Min_Mag_Mip_Linear,
        Anisotropic,
    };
};
mxDECLARE_ENUM( TextureFilter::Enum, UINT8, TextureFilterT );

struct TextureAddressMode {
	enum Enum {
		Wrap,
		Clamp,
		Border,		//<= not support on all platforms
		Mirror,
		MirrorOnce	//<= not support on all platforms
	};
};
mxDECLARE_ENUM( TextureAddressMode::Enum, UINT8, TextureAddressModeT );

// RGB or alpha blending operation.
struct BlendOp {
	enum Enum {
		MIN,
		MAX,
		ADD,
		SUBTRACT,
		REV_SUBTRACT,
	};
};
mxDECLARE_ENUM( BlendOp::Enum, UINT8, BlendOpT );

// Blend factors modulate values for the pixel shader and render target.
struct BlendFactor {
	enum Enum {
		ZERO,
		ONE,

		SRC_COLOR,
		DST_COLOR,
		SRC_ALPHA,
		DST_ALPHA,
		SRC1_COLOR,
		SRC1_ALPHA,
		BLEND_FACTOR,

		INV_SRC_COLOR,
		INV_DST_COLOR,
		INV_SRC_ALPHA,		
		INV_DST_ALPHA,
		INV_SRC1_COLOR,
		INV_SRC1_ALPHA,
		INV_BLEND_FACTOR,

		SRC_ALPHA_SAT,		
	};
};
mxDECLARE_ENUM( BlendFactor::Enum, UINT8, BlendFactorT );

struct StencilOp {
	enum Enum {
		KEEP,		// Keeps the current value.
		ZERO,		// Sets the stencil buffer value to 0.
		INVERT,		// Bitwise inverts the current stencil buffer value.
		REPLACE,	// Sets the stencil buffer value to the reference value.
		INCR_SAT,	// Increment the stencil value by 1, and clamp the result to maximum unsigned value.
		DECR_SAT,	// Decrement the stencil value by 1, and clamp the result to zero.
		INCR_WRAP,	// Increment the stencil value by 1, and wrap the result if necessary.
		DECR_WRAP,	// Decrement the stencil value by 1, and wrap the result if necessary.		
	};
};
mxDECLARE_ENUM( StencilOp::Enum, UINT8, StencilOpT );

struct ColorWriteMask {
	enum Flags {
		ENABLE_RED		= BIT(0),
		ENABLE_GREEN	= BIT(1),
		ENABLE_BLUE		= BIT(2),
		ENABLE_ALPHA	= BIT(3),
		ENABLE_ALL		= ENABLE_RED|ENABLE_GREEN|ENABLE_BLUE|ENABLE_ALPHA,
	};
};
mxDECLARE_FLAGS( ColorWriteMask::Flags, UINT8, BColorWriteMask );

struct ClearMask {
	enum Flags {
		CLEAR_DEPTH		= BIT(0),
		CLEAR_STENCIL	= BIT(1),
		CLEAR_COLOR		= BIT(2),
		CLEAR_ALL		= CLEAR_DEPTH|CLEAR_STENCIL|CLEAR_COLOR,
	};
};
mxDECLARE_FLAGS( ClearMask::Flags, UINT8, BClearMask );

struct DepthStencilSide : CStruct
{
	ComparisonFuncT	stencilFunction;
	StencilOpT		stencilPassOp;
    StencilOpT		stencilFailOp;
	StencilOpT		depthFailOp;
public:
	mxDECLARE_CLASS(DepthStencilSide, CStruct);
	mxDECLARE_REFLECTION;
	DepthStencilSide(ENoInit) {}
	DepthStencilSide();
};
struct DepthStencilDescription : NamedObject
{
	bool				enableDepthTest;
	bool				enableDepthWrite;
	ComparisonFuncT		depthFunction;
	bool				enableStencil;		// default = false
	UINT8				stencilReadMask;	// default = 0xFF
	UINT8				stencilWriteMask;	// default = 0xFF
	//bool				enableTwoSidedStencil;
	DepthStencilSide	frontFace;
	DepthStencilSide	backFace;
public:
	mxDECLARE_CLASS(DepthStencilDescription, NamedObject);
	mxDECLARE_REFLECTION;
	DepthStencilDescription(ENoInit) : frontFace(_NoInit), backFace(_NoInit) {}
	DepthStencilDescription();
};

struct RasterizerDescription : NamedObject
{
	FillModeT	fillMode;
	CullModeT	cullMode;
	bool		enableDepthClip;
	bool		enableScissor;
	bool		enableMultisample;
	bool		enableAntialiasedLine;
public:
	mxDECLARE_CLASS(RasterizerDescription, NamedObject);
	mxDECLARE_REFLECTION;
	RasterizerDescription(ENoInit) {}
	RasterizerDescription();
};

// @see D3D11_SAMPLER_DESC
struct SamplerDescription : NamedObject
{
	TextureFilterT		filter;
	TextureAddressModeT	addressU;	// The texture addressing mode for the u-coordinate.
	TextureAddressModeT	addressV;	// The texture addressing mode for the v-coordinate.
	TextureAddressModeT	addressW;	// The texture addressing mode for the w-coordinate.
	UINT8				maxAnisotropy;	// The maximum anisotropy value.
	ComparisonFuncT		comparison;		// Compare the sampled result to the comparison value?
	float				borderColor[4];	// Border color to use if TextureAddressMode::Border is specified for addressU, addressV, or addressW.
	float				minLOD;			// Lower end of the mipmap range to clamp access to.
	float				maxLOD;			// Upper end of the mipmap range to clamp access to.
	float				mipLODBias;		// Offset from the calculated mipmap level.
public:
	mxDECLARE_CLASS(SamplerDescription, NamedObject);
	mxDECLARE_REFLECTION;
	SamplerDescription(ENoInit) {}
	SamplerDescription();
};

struct BlendChannel : CStruct
{
	BlendOpT		operation;
	BlendFactorT	sourceFactor;
	BlendFactorT	destinationFactor;
public:
	mxDECLARE_CLASS(BlendChannel, CStruct);
	mxDECLARE_REFLECTION;
	BlendChannel();
};
mxTODO("independent render target blending?");
struct BlendDescription : NamedObject
{
	bool				enableBlending;

	BlendChannel		color;
	BlendChannel		alpha;
	BColorWriteMask		writeMask;

	bool				enableAlphaToCoverage;
	//bool				enableIndependentBlend;
public:
	mxDECLARE_CLASS(BlendDescription, NamedObject);
	mxDECLARE_REFLECTION;
	BlendDescription(ENoInit) {}
	BlendDescription();
};

/*
=====================================================================
	GEOMETRY
=====================================================================
*/

//	enumerates all allowed types of elementary graphics primitives used for rasterization.
struct Topology {
	enum Enum {
		Undefined	  = 0,	// Error.
		PointList	  = 1,	// A collection of isolated points.
		LineList	  = 2,	// A list of points, one vertex per point.
		LineStrip	  = 3,	// A strip of connected lines, 1 vertex per line plus one 1 start vertex.
		TriangleList  = 4,	// A list of triangles, 3 vertices per triangle.
		TriangleStrip = 5,	// A string of triangles, 3 vertices for the first triangle, and 1 per triangle after that.
		TriangleFan	  = 6,	// A string of triangles, 3 vertices for the first triangle, and 1 per triangle after that.
	};
};
mxDECLARE_ENUM( Topology::Enum, UINT8, TopologyT );

// enumerates data types of vertex attributes (vector dimension (1-4) is stored separately)
struct AttributeType {
	enum Enum {
		Byte,	// 8-bit signed integer number
		UByte,	// 8-bit unsigned integer number
		Short,	// 16-bit signed integer number
		UShort,	// 16-bit unsigned integer number
		Half,	// 16-bit floating-point number
		Float,	// 32-bit floating-point number
		//Double,	// 64-bit floating-point number
		Count	//<= Marker. Don't use!
	};
};
mxDECLARE_ENUM( AttributeType::Enum, UINT8, AttributeTypeT );

// describes the meaning (semantics) of vertex components.
struct VertexAttribute {
	enum Enum {
		Position,	// Position, 3 floats per vertex.

		Color0,		// Vertex color.
		Color1,

		Normal,		// Normal, 3 floats per vertex.
		Tangent,	// X axis if normal is Z
		Binormal,	// Y axis if normal is Z (aka Bitangent)

		TexCoord0,	// Texture coordinates.
		TexCoord1,
		TexCoord2,
		TexCoord3,
		TexCoord4,
		TexCoord5,
		TexCoord6,
		TexCoord7,

		BoneWeights,	// 4 weighting factors to matrices
		BoneIndices,	// 4 indices to bone/joint matrices		

		Count	//<= Marker. Don't use!
	};
};
mxDECLARE_ENUM( VertexAttribute::Enum, UINT8, VertexAttributeT );

struct VertexElement
{
	BITFIELD	type			: 3;	// AttributeTypeT
	BITFIELD	semantic		: 5;	// VertexAttributeT
	BITFIELD	dimension		: 2;	// {1,2,3,4} => [0..3]
	BITFIELD	inputSlot		: 5;	// [0..LLGL_MAX_VERTEX_STREAMS)
	BITFIELD	normalized		: 1;
};

// This structure describes the memory layout and format of a vertex buffer.
struct VertexDescription
{
	VertexElement	attribsArray[ LLGL_MAX_VERTEX_ATTRIBS ];	// array of all vertex elements
	UINT8			attribOffsets[ LLGL_MAX_VERTEX_ATTRIBS ];	// offsets within vertex streams
	UINT8			streamStrides[ LLGL_MAX_VERTEX_STREAMS ];	// strides of each vertex buffer
	UINT8			attribCount;	// total number of vertex components
public:
	void Begin();
	void End();

	void Add(
		AttributeTypeT type, UINT dimension,
		VertexAttributeT semantic,
		bool isNormalized = false,
		UINT inputSlot = 0
	);
};

/*
=====================================================================
    SHADER PROGRAMS
=====================================================================
*/
enum EShaderType
{
	ShaderVertex,
	ShaderHull,
	ShaderDomain,
	ShaderGeometry,
	ShaderFragment,	// Pixel shader
	//new shader types can be added here:
	//ShaderCompute,
	ShaderTypeCount,	//<= Marker. Don't use!
};
extern const Chars g_shaderTypeName[ShaderTypeCount];
const char* EShaderTypeToChars( EShaderType shaderType );

struct CBufferBindingOGL : CStruct
{
	String	name;
	UINT8	slot;// constant buffer slot
public:
	mxDECLARE_CLASS(CBufferBindingOGL,CStruct);
	mxDECLARE_REFLECTION;
};
struct SamplerBindingOGL : CStruct
{
	String	name;
	UINT8	slot;	// texture unit index
public:
	mxDECLARE_CLASS(SamplerBindingOGL,CStruct);
	mxDECLARE_REFLECTION;
};
struct ProgramBindingsOGL : CStruct
{
	TArray< CBufferBindingOGL >	uniforms;	// uniform block bindings
	TArray< SamplerBindingOGL >	samplers;	// shader sampler bindings
	UINT32			activeVertexAttributes;	// enabled attributes mask
public:
	mxDECLARE_CLASS(ProgramBindingsOGL,CStruct);
	mxDECLARE_REFLECTION;
	void Clear();
};

struct ProgramDescription : NamedObject
{
	HShader	shaders[ShaderTypeCount];

	// platform-specific data for defining binding points;
	// the data can be discarded after linking the program
	const ProgramBindingsOGL* bindings;	// used only by OpenGL back-end

public:
	mxDECLARE_CLASS(ProgramDescription,NamedObject);
	mxDECLARE_REFLECTION;
	ProgramDescription();
};

/*
=====================================================================
    MISCELLANEOUS
=====================================================================
*/
enum EBufferType
{
	Buffer_Uniform,
	Buffer_Vertex,
	Buffer_Index,
};
mxDECLARE_ENUM( EBufferType, UINT8, BufferTypeT );
//const char* EBufferType_To_Chars( EBufferType type );

enum EMapMode
{
    Map_Read,
    Map_Write,
    Map_Read_Write,
    Map_Write_Discard,
    Map_Write_DiscardRange,
};


struct PipelineStateDescription
{
	float				blendFactor[4];
	UINT32				sampleMask;
	HBlendState			blendState;

	HRasterizerState	rasterizerState;

	HDepthStencilState	depthStencilState;
	UINT8				stencilRef;

	HInputLayout		inputLayout;

	HProgram			program;
};

struct FrameBufferDescription
{
	HColorTarget	colorTargets[LLGL_MAX_BOUND_TARGETS];
	HDepthTarget	depthTarget;	// optional depth-stencil surface
	UINT8			numTargets;	// number of color targets to bind
};

/*
    The bytecode for all shaders including, vertex, pixel, domain, hull, and geometry shaders.
    The input vertex format.
    The primitive topology type. Note that the input-assembler primitive topology type (point, line, triangle, patch) is set within the PSO using the D3D12_PRIMITIVE_TOPOLOGY_TYPE enumeration. The primitive adjacency and ordering (line list, line strip, line strip with adjacency data, etc.) is set from within a command list using the ID3D12GraphicsCommandList::IASetPrimitiveTopology method.
    The blend state, rasterizer state, depth stencil state.
    The depth stencil and render target formats, as well as the render target count.
    Multi-sampling parameters.
    A streaming output buffer.
    The root signature. For more information, see Root Signatures.
*/




/*
=======================================================================
	ENGINE-SPECIFIC FORMATS
=======================================================================
*/
static const UINT32 TEXTURE_MAGIC_NUM = 'PAMT';	// "TMAP"
// "DDS " or ' SDD' on little-endian machines
static const UINT32 DDS_MAGIC_NUM = ' SDD';	// 0x20534444

#pragma pack(push,1)
struct TextureHeader
{
	UINT32		magic;
	UINT32		size;	// total size of image data
	UINT16		width;	// texture width (in texels)
	UINT16		height;	// texture height (in texels)
	UINT8		depth;	// depth of a volume texture
	UINT8		flags;
	UINT8		format;	// TextureFormatT
	UINT8		numMips;// mip level count
};
#pragma pack(pop)

ASSERT_SIZEOF(TextureHeader, 16);


struct TextureImage
{
	const void *	data;	// pointer to raw image data
	UINT32			size;	// total size of raw image data
	UINT16			width;	// texture width (in texels)
	UINT16			height;	// texture height (in texels)
	UINT16			depth;	// depth of a volume texture
	TextureFormatT	format;	// engine-specific texture format
	UINT8			numMips;// mip level count
	bool			isCubeMap;
};

UINT32 CalculateTextureSize( UINT16 _width, UINT16 _height, TextureFormatT _format, UINT8 _numMips );

struct MipLevel
{
	const void *	data;	// pointer to data of this mipmap level
	UINT32			size;	// size of this mipmap level, in bytes
	UINT32			pitch;	// size of a row of blocks, in bytes
	UINT16			width;	// width of this mipmap level (in texels)
	UINT16			height;	// height of this mipmap level (in texels)
};

// parses the texture mipmap levels into the user-supplied array
ERet ParseMipLevels( const TextureImage& _image, UINT8 _side, MipLevel *_mips, UINT8 _maxMips );

ERet AddBindings(
	ProgramBindingsOGL &destination,
	const ProgramBindingsOGL &source
);
// simply does register assignment
ERet AssignBindPoints(
	ProgramBindingsOGL & bindings
);
// validates that the shader input bindings are the same
// across different shader stages and merges them
ERet MergeBindings(
	ProgramBindingsOGL &destination,
	const ProgramBindingsOGL &source
);

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
