#include <Graphics/Graphics_PCH.h>
#pragma hdrstop

#include <algorithm>
//#include <vector>

//#include <XNACollision/XNACollision.h>
//#if MX_AUTOLINK
//#pragma comment( lib, "XNACollision.lib" )
//#endif //MX_AUTOLINK

#include <Core/VectorMath.h>
#include <Graphics/Device.h>
#include <Graphics/Geometry.h>
#include <Graphics/Legacy.h>

#if LLGL_CONFIG_DRIVER_D3D11
	#include "Driver_D3D11.h"
#endif

mxDEFINE_CLASS(NamedObject);
mxBEGIN_REFLECTION(NamedObject)
	mxMEMBER_FIELD( name ),
	mxMEMBER_FIELD( hash ),
mxEND_REFLECTION;
NamedObject::NamedObject()
{
	this->hash = 0;
}
void NamedObject::UpdateNameHash()
{
	this->hash = GetDynamicStringHash(this->name.SafeGetPtr());
}
bool NamedObject::Equals( const NamedObject& other ) const
{
	return this->name == other.name && this->hash == other.hash;
}

mxBEGIN_REFLECT_ENUM( ComparisonFuncT )
	mxREFLECT_ENUM_ITEM( Always, ComparisonFunc::Always ),
	mxREFLECT_ENUM_ITEM( Never, ComparisonFunc::Never ),
	mxREFLECT_ENUM_ITEM( Less, ComparisonFunc::Less ),
	mxREFLECT_ENUM_ITEM( Equal, ComparisonFunc::Equal ),
	mxREFLECT_ENUM_ITEM( Greater, ComparisonFunc::Greater ),
	mxREFLECT_ENUM_ITEM( Not_Equal, ComparisonFunc::Not_Equal ),
	mxREFLECT_ENUM_ITEM( Less_Equal, ComparisonFunc::Less_Equal ),
	mxREFLECT_ENUM_ITEM( Greater_Equal, ComparisonFunc::Greater_Equal ),
mxEND_REFLECT_ENUM

mxBEGIN_REFLECT_ENUM( FillModeT )
	mxREFLECT_ENUM_ITEM( Solid, FillMode::Solid ),
	mxREFLECT_ENUM_ITEM( Wireframe, FillMode::Wireframe ),
mxEND_REFLECT_ENUM

mxBEGIN_REFLECT_ENUM( CullModeT )
	mxREFLECT_ENUM_ITEM( None, CullMode::None ),
	mxREFLECT_ENUM_ITEM( Back, CullMode::Back ),
	mxREFLECT_ENUM_ITEM( Front, CullMode::Front ),
mxEND_REFLECT_ENUM

//mxBEGIN_REFLECT_ENUM( EFilteringMode )
//	mxREFLECT_ENUM_ITEM( Filter_Point, FilteringMode::Filter_Point ),
//	mxREFLECT_ENUM_ITEM( Filter_Linear, FilteringMode::Filter_Linear ),
//	mxREFLECT_ENUM_ITEM( Filter_Anisotropic, FilteringMode::Filter_Anisotropic ),
//mxEND_REFLECT_ENUM
mxBEGIN_REFLECT_ENUM( TextureFilterT )
	mxREFLECT_ENUM_ITEM( Min_Mag_Mip_Point, TextureFilter::Min_Mag_Mip_Point ),
	mxREFLECT_ENUM_ITEM( Min_Mag_Point_Mip_Linear, TextureFilter::Min_Mag_Point_Mip_Linear ),
	mxREFLECT_ENUM_ITEM( Min_Point_Mag_Linear_Mip_Point, TextureFilter::Min_Point_Mag_Linear_Mip_Point ),
	mxREFLECT_ENUM_ITEM( Min_Point_Mag_Mip_Linear, TextureFilter::Min_Point_Mag_Mip_Linear ),
	mxREFLECT_ENUM_ITEM( Min_Linear_Mag_Mip_Point, TextureFilter::Min_Linear_Mag_Mip_Point ),
	mxREFLECT_ENUM_ITEM( Min_Linear_Mag_Point_Mip_Linear, TextureFilter::Min_Linear_Mag_Point_Mip_Linear ),
	mxREFLECT_ENUM_ITEM( Min_Mag_Linear_Mip_Point, TextureFilter::Min_Mag_Linear_Mip_Point ),
	mxREFLECT_ENUM_ITEM( Min_Mag_Mip_Linear, TextureFilter::Min_Mag_Mip_Linear ),
	mxREFLECT_ENUM_ITEM( Anisotropic, TextureFilter::Anisotropic ),
mxEND_REFLECT_ENUM

mxBEGIN_REFLECT_ENUM( TextureAddressModeT )
	mxREFLECT_ENUM_ITEM( Wrap, TextureAddressMode::Wrap ),
	mxREFLECT_ENUM_ITEM( Clamp, TextureAddressMode::Clamp ),
	mxREFLECT_ENUM_ITEM( Border, TextureAddressMode::Border ),
	mxREFLECT_ENUM_ITEM( Mirror, TextureAddressMode::Mirror ),
	mxREFLECT_ENUM_ITEM( MirrorOnce, TextureAddressMode::MirrorOnce ),
mxEND_REFLECT_ENUM

mxBEGIN_REFLECT_ENUM( BlendOpT )
	mxREFLECT_ENUM_ITEM( MIN, BlendOp::MIN ),
	mxREFLECT_ENUM_ITEM( MAX, BlendOp::MAX ),
	mxREFLECT_ENUM_ITEM( ADD, BlendOp::ADD ),
	mxREFLECT_ENUM_ITEM( SUBTRACT, BlendOp::SUBTRACT ),
	mxREFLECT_ENUM_ITEM( REV_SUBTRACT, BlendOp::REV_SUBTRACT ),
mxEND_REFLECT_ENUM

mxBEGIN_REFLECT_ENUM( BlendFactorT )
	mxREFLECT_ENUM_ITEM( ZERO, BlendFactor::ZERO ),
	mxREFLECT_ENUM_ITEM( ONE, BlendFactor::ONE ),
	mxREFLECT_ENUM_ITEM( SRC_COLOR, BlendFactor::SRC_COLOR ),
	mxREFLECT_ENUM_ITEM( INV_SRC_COLOR, BlendFactor::INV_SRC_COLOR ),
	mxREFLECT_ENUM_ITEM( SRC_ALPHA, BlendFactor::SRC_ALPHA ),
	mxREFLECT_ENUM_ITEM( INV_SRC_ALPHA, BlendFactor::INV_SRC_ALPHA ),
	mxREFLECT_ENUM_ITEM( DST_ALPHA, BlendFactor::DST_ALPHA ),
	mxREFLECT_ENUM_ITEM( INV_DST_ALPHA, BlendFactor::INV_DST_ALPHA ),
	mxREFLECT_ENUM_ITEM( DST_COLOR, BlendFactor::DST_COLOR ),
	mxREFLECT_ENUM_ITEM( INV_DST_COLOR, BlendFactor::INV_DST_COLOR ),
	mxREFLECT_ENUM_ITEM( SRC_ALPHA_SAT, BlendFactor::SRC_ALPHA_SAT ),
	mxREFLECT_ENUM_ITEM( BLEND_FACTOR, BlendFactor::BLEND_FACTOR ),
	mxREFLECT_ENUM_ITEM( INV_BLEND_FACTOR, BlendFactor::INV_BLEND_FACTOR ),
	mxREFLECT_ENUM_ITEM( SRC1_COLOR, BlendFactor::SRC1_COLOR ),
	mxREFLECT_ENUM_ITEM( INV_SRC1_COLOR, BlendFactor::INV_SRC1_COLOR ),
	mxREFLECT_ENUM_ITEM( SRC1_ALPHA, BlendFactor::SRC1_ALPHA ),
	mxREFLECT_ENUM_ITEM( INV_SRC1_ALPHA, BlendFactor::INV_SRC1_ALPHA ),
mxEND_REFLECT_ENUM

mxBEGIN_REFLECT_ENUM( StencilOpT )
	mxREFLECT_ENUM_ITEM( KEEP, StencilOp::KEEP ),
	mxREFLECT_ENUM_ITEM( ZERO, StencilOp::ZERO ),
	mxREFLECT_ENUM_ITEM( INCR, StencilOp::INCR_WRAP ),
	mxREFLECT_ENUM_ITEM( DECR, StencilOp::DECR_WRAP ),
	mxREFLECT_ENUM_ITEM( REPLACE, StencilOp::REPLACE ),
	mxREFLECT_ENUM_ITEM( INCR_SAT, StencilOp::INCR_SAT ),
	mxREFLECT_ENUM_ITEM( DECR_SAT, StencilOp::DECR_SAT ),
	mxREFLECT_ENUM_ITEM( INVERT, StencilOp::INVERT ),
mxEND_REFLECT_ENUM

mxBEGIN_FLAGS( BColorWriteMask )
	mxREFLECT_BIT( ENABLE_RED, ColorWriteMask::ENABLE_RED ),
	mxREFLECT_BIT( ENABLE_GREEN, ColorWriteMask::ENABLE_GREEN ),
	mxREFLECT_BIT( ENABLE_BLUE, ColorWriteMask::ENABLE_BLUE ),
	mxREFLECT_BIT( ENABLE_ALPHA, ColorWriteMask::ENABLE_ALPHA ),
	mxREFLECT_BIT( ENABLE_ALL, ColorWriteMask::ENABLE_ALL ),
mxEND_FLAGS

mxBEGIN_FLAGS( BClearMask )
	mxREFLECT_BIT( CLEAR_DEPTH, ClearMask::CLEAR_DEPTH ),
	mxREFLECT_BIT( CLEAR_STENCIL, ClearMask::CLEAR_STENCIL ),
	mxREFLECT_BIT( CLEAR_COLOR, ClearMask::CLEAR_COLOR ),
	mxREFLECT_BIT( CLEAR_ALL, ClearMask::CLEAR_ALL ),
mxEND_FLAGS

mxDEFINE_CLASS(DepthStencilSide);
mxBEGIN_REFLECTION(DepthStencilSide)
	mxMEMBER_FIELD(stencilFunction),
	mxMEMBER_FIELD(stencilPassOp),
	mxMEMBER_FIELD(stencilFailOp),
	mxMEMBER_FIELD(depthFailOp),
mxEND_REFLECTION
DepthStencilSide::DepthStencilSide()
{
	stencilFunction = ComparisonFunc::Always;
	stencilPassOp = StencilOp::KEEP;
	stencilFailOp = StencilOp::KEEP;
	depthFailOp = StencilOp::KEEP;
}

mxDEFINE_CLASS(DepthStencilDescription);
mxBEGIN_REFLECTION(DepthStencilDescription)
	mxMEMBER_FIELD(enableDepthTest),
	mxMEMBER_FIELD(enableDepthWrite),
	mxMEMBER_FIELD(depthFunction),
	mxMEMBER_FIELD(enableStencil),
	mxMEMBER_FIELD(stencilReadMask),
	mxMEMBER_FIELD(stencilWriteMask),
	mxMEMBER_FIELD(frontFace),
	mxMEMBER_FIELD(backFace),
mxEND_REFLECTION
DepthStencilDescription::DepthStencilDescription()
{
	enableDepthTest = true;
	enableDepthWrite = true;
	depthFunction = ComparisonFunc::Less_Equal;
	enableStencil = false;
	stencilReadMask = ~0;
	stencilWriteMask = ~0;
}

mxDEFINE_CLASS(RasterizerDescription);
mxBEGIN_REFLECTION(RasterizerDescription)
	mxMEMBER_FIELD(fillMode),
	mxMEMBER_FIELD(cullMode),
	mxMEMBER_FIELD(enableDepthClip),
	mxMEMBER_FIELD(enableScissor),
	mxMEMBER_FIELD(enableMultisample),
	mxMEMBER_FIELD(enableAntialiasedLine),
mxEND_REFLECTION
RasterizerDescription::RasterizerDescription()
{
	fillMode = FillMode::Solid;
	cullMode = CullMode::Back;
	enableDepthClip = true;
	enableScissor = false;
	enableMultisample = false;
	enableAntialiasedLine = false;
}

mxDEFINE_CLASS(SamplerDescription);
mxBEGIN_REFLECTION(SamplerDescription)
	//mxMEMBER_FIELD(min_filter),
	//mxMEMBER_FIELD(mag_filter),
	//mxMEMBER_FIELD(mip_filter),
	//mxMEMBER_FIELD(comparison),
	mxMEMBER_FIELD(filter),

	mxMEMBER_FIELD(addressU),
	mxMEMBER_FIELD(addressV),
	mxMEMBER_FIELD(addressW),
	mxMEMBER_FIELD(comparison),
	mxMEMBER_FIELD(borderColor),
	mxMEMBER_FIELD(minLOD),
	mxMEMBER_FIELD(maxLOD),
	mxMEMBER_FIELD(mipLODBias),
	mxMEMBER_FIELD(maxAnisotropy),
mxEND_REFLECTION
SamplerDescription::SamplerDescription()
{
	filter			= TextureFilter::Min_Mag_Mip_Point;
	addressU		= TextureAddressMode::Clamp;
	addressV		= TextureAddressMode::Clamp;
	addressW		= TextureAddressMode::Clamp;
	comparison		= ComparisonFunc::Never;
	borderColor[0]	= 0.0f;
	borderColor[1]	= 0.0f;
	borderColor[2]	= 0.0f;
	borderColor[3]	= 0.0f;
	minLOD			= -FLT_MAX;
	maxLOD			= +FLT_MAX;
	mipLODBias		= 0.0f;
	maxAnisotropy	= 1;
}

mxDEFINE_CLASS(BlendChannel);
mxBEGIN_REFLECTION(BlendChannel)
	mxMEMBER_FIELD(operation),
	mxMEMBER_FIELD(sourceFactor),
	mxMEMBER_FIELD(destinationFactor),
mxEND_REFLECTION
BlendChannel::BlendChannel()
{
	operation = BlendOp::ADD;
	sourceFactor = BlendFactor::ONE;
	destinationFactor = BlendFactor::ZERO;
}

mxDEFINE_CLASS(BlendDescription);
mxBEGIN_REFLECTION(BlendDescription)
	mxMEMBER_FIELD(enableBlending),

	mxMEMBER_FIELD(color),
	mxMEMBER_FIELD(alpha),
	mxMEMBER_FIELD(writeMask),

	mxMEMBER_FIELD(enableAlphaToCoverage),
	//mxMEMBER_FIELD(enableIndependentBlend),
mxEND_REFLECTION

BlendDescription::BlendDescription()
{
	enableBlending = false;

	writeMask = ColorWriteMask::ENABLE_ALL;

	enableAlphaToCoverage	= false;
	//enableIndependentBlend	= false;
}

mxBEGIN_REFLECT_ENUM( TopologyT )
	mxREFLECT_ENUM_ITEM( PT_Undefined, Topology::Undefined ),
	mxREFLECT_ENUM_ITEM( PT_PointList, Topology::PointList ),
	mxREFLECT_ENUM_ITEM( PT_LineList, Topology::LineList ),
	mxREFLECT_ENUM_ITEM( PT_LineStrip, Topology::LineStrip ),
	mxREFLECT_ENUM_ITEM( PT_TriangleList, Topology::TriangleList ),
	mxREFLECT_ENUM_ITEM( PT_TriangleStrip, Topology::TriangleStrip ),
	mxREFLECT_ENUM_ITEM( PT_TriangleFan, Topology::TriangleFan ),
mxEND_REFLECT_ENUM

mxBEGIN_REFLECT_ENUM( AttributeTypeT )
	mxREFLECT_ENUM_ITEM( Byte, AttributeType::Byte ),
	mxREFLECT_ENUM_ITEM( Short, AttributeType::Short ),
	mxREFLECT_ENUM_ITEM( Half, AttributeType::Half ),
	mxREFLECT_ENUM_ITEM( Float, AttributeType::Float ),
	//mxREFLECT_ENUM_ITEM( Double, AttributeType::Double ),
mxEND_REFLECT_ENUM

mxBEGIN_REFLECT_ENUM( VertexAttributeT )
	mxREFLECT_ENUM_ITEM( Position, VertexAttribute::Position ),

	mxREFLECT_ENUM_ITEM( Color0, VertexAttribute::Color0 ),
	mxREFLECT_ENUM_ITEM( Color1, VertexAttribute::Color1 ),

	mxREFLECT_ENUM_ITEM( Normal, VertexAttribute::Normal ),
	mxREFLECT_ENUM_ITEM( Tangent, VertexAttribute::Tangent ),
	mxREFLECT_ENUM_ITEM( Binormal, VertexAttribute::Binormal ),

	mxREFLECT_ENUM_ITEM( TexCoord0, VertexAttribute::TexCoord0 ),
	mxREFLECT_ENUM_ITEM( TexCoord1, VertexAttribute::TexCoord1 ),
	mxREFLECT_ENUM_ITEM( TexCoord2, VertexAttribute::TexCoord2 ),
	mxREFLECT_ENUM_ITEM( TexCoord3, VertexAttribute::TexCoord3 ),
	mxREFLECT_ENUM_ITEM( TexCoord4, VertexAttribute::TexCoord4 ),
	mxREFLECT_ENUM_ITEM( TexCoord5, VertexAttribute::TexCoord5 ),
	mxREFLECT_ENUM_ITEM( TexCoord6, VertexAttribute::TexCoord6 ),
	mxREFLECT_ENUM_ITEM( TexCoord7, VertexAttribute::TexCoord7 ),

	mxREFLECT_ENUM_ITEM( BoneWeights, VertexAttribute::BoneWeights ),
	mxREFLECT_ENUM_ITEM( BoneIndices, VertexAttribute::BoneIndices ),	
mxEND_REFLECT_ENUM

void VertexDescription::Begin()
{
	mxZERO_OUT(attribsArray);
	mxZERO_OUT(attribOffsets);
	mxZERO_OUT(streamStrides);
	attribCount = 0;
}
void VertexDescription::End()
{
	mxASSERT(attribCount > 0);
}

// gs_attributeSize [ ATTRIB_TYPE ] [ VECTOR_DIMENSION ]
#if (LLGL_Driver == LLGL_Driver_Direct3D_11)
	static const UINT8 gs_attributeSize[AttributeType::Count][4] = {
		{  1,  2,  4,  4 },	// Byte
		{  1,  2,  4,  4 },	// UByte
		{  2,  4,  8,  8 },	// Short
		{  2,  4,  8,  8 },	// UShort
		{  2,  4,  8,  8 },	// Half
		{  4,  8, 12, 16 },	// Float
		//{  8, 16, 24, 32 },	// Double
	};
#elif (LLGL_Driver_Is_OpenGL)
	static const UINT8 gs_attributeSize[AttributeType::Count][4] = {
		{  1,  2,  4,  4 },	// Byte
		{  1,  2,  4,  4 },	// UByte
		{  2,  4,  6,  8 },	// Short
		{  2,  4,  6,  8 },	// UShort
		{  2,  4,  6,  8 },	// Half
		{  4,  8, 12, 16 },	// Float
		//{  8, 16, 24, 32 },	// Double
	};
#else
	#error Unsupported driver type!
#endif

void VertexDescription::Add(
	AttributeTypeT type, UINT dimension,
	VertexAttributeT semantic,
	bool isNormalized,
	UINT inputSlot
)
{
	const UINT elementIndex = attribCount++;
	VertexElement & newElement = attribsArray[elementIndex];
	{
		newElement.type = type;
		newElement.semantic = semantic;
		newElement.normalized = isNormalized;
		newElement.dimension = dimension-1;
		newElement.inputSlot = inputSlot;
	}
	const UINT elementSize = gs_attributeSize[type][dimension-1];
	attribOffsets[elementIndex] = streamStrides[inputSlot];
	streamStrides[inputSlot] += elementSize;
}

mxBEGIN_REFLECT_ENUM( TextureFormatT )
	mxREFLECT_ENUM_ITEM( BC1, TextureFormat::BC1 ),
	mxREFLECT_ENUM_ITEM( BC2, TextureFormat::BC2 ),
	mxREFLECT_ENUM_ITEM( BC3, TextureFormat::BC3 ),
	mxREFLECT_ENUM_ITEM( BC4, TextureFormat::BC4 ),
	mxREFLECT_ENUM_ITEM( BC5, TextureFormat::BC5 ),
	mxREFLECT_ENUM_ITEM( Unknown, TextureFormat::Unknown ),
	//mxREFLECT_ENUM_ITEM( L8, TextureFormat::L8 ),
	//mxREFLECT_ENUM_ITEM( BGRX8, TextureFormat::BGRX8 ),
	mxREFLECT_ENUM_ITEM( BGRA8, TextureFormat::BGRA8 ),
	//mxREFLECT_ENUM_ITEM( RGBA16, TextureFormat::RGBA16 ),
	//mxREFLECT_ENUM_ITEM( RGBA16F, TextureFormat::RGBA16F ),
	//mxREFLECT_ENUM_ITEM( R5G6B5, TextureFormat::R5G6B5 ),
	//mxREFLECT_ENUM_ITEM( RGBA4, TextureFormat::RGBA4 ),
	mxREFLECT_ENUM_ITEM( RGBA8, TextureFormat::RGBA8 ),
	//mxREFLECT_ENUM_ITEM( RGB5A1, TextureFormat::RGB5A1 ),
	//mxREFLECT_ENUM_ITEM( RGB10A2, TextureFormat::RGB10A2 ),
	mxREFLECT_ENUM_ITEM( R11G11B10, TextureFormat::R11G11B10 ),
	//mxREFLECT_ENUM_ITEM( R32f, TextureFormat::R32f ),
mxEND_REFLECT_ENUM

bool TextureFormat::IsCompressed( TextureFormat::Enum _format )
{
	return _format < TextureFormat::Unknown;
}
UINT TextureFormat::BitsPerPixel( TextureFormat::Enum _format )
{
	static const UINT s_sizes[TextureFormat::Count] = {
		4 ,	// BC1
		8 ,	// BC2
		8 ,	// BC3
		4 ,	// BC4
		8 ,	// BC5
		0 ,	// Unknown
		//8 ,	// L8
		//32,	// BGRX8
		32,	// BGRA8
		//64,	// RGBA16
		//64,	// RGBA16F
		//16,	// R5G6B5
		//16,	// RGBA4
		32,	// RGBA8
		//16,	// RGB5A1
		//32,	// RGB10A2
		//32,	// R32f
	};
	return s_sizes[_format];
}
UINT TextureFormat::GetBlockSize( Enum _format )
{
	// block-compressed formats organize texture data in 4x4 texel blocks
	return TextureFormat::IsCompressed(_format) ? 16 : 1;
}

mxBEGIN_REFLECT_ENUM( DepthStencilFormatT )
	mxREFLECT_ENUM_ITEM( D16, DepthStencilFormat::D16 ),
	mxREFLECT_ENUM_ITEM( D24S8, DepthStencilFormat::D24S8 ),
	mxREFLECT_ENUM_ITEM( D32, DepthStencilFormat::D32 ),
mxEND_REFLECT_ENUM

UINT DepthStencilFormat::BitsPerPixel( DepthStencilFormat::Enum format )
{
	static const UINT s_sizes[DepthStencilFormat::Count] = {
		16,	// D16
		32,	// D24S8
		32,	// D32
	};
	return s_sizes[format];
}

mxBEGIN_REFLECT_ENUM( TextureTypeT )
	mxREFLECT_ENUM_ITEM( TEXTURE_1D, TextureType::TEXTURE_1D ),
	mxREFLECT_ENUM_ITEM( TEXTURE_2D, TextureType::TEXTURE_2D ),
	mxREFLECT_ENUM_ITEM( TEXTURE_3D, TextureType::TEXTURE_3D ),
	mxREFLECT_ENUM_ITEM( TEXTURE_CUBE, TextureType::TEXTURE_CUBE ),
	mxREFLECT_ENUM_ITEM( TEXTURE_1D_ARRAY, TextureType::TEXTURE_1D_ARRAY ),
	mxREFLECT_ENUM_ITEM( TEXTURE_2D_ARRAY, TextureType::TEXTURE_2D_ARRAY ),
	mxREFLECT_ENUM_ITEM( TEXTURE_CUBE_ARRAY, TextureType::TEXTURE_CUBE_ARRAY ),
mxEND_REFLECT_ENUM

mxDEFINE_CLASS(Texture2DDescription);
mxBEGIN_REFLECTION(Texture2DDescription)
	mxMEMBER_FIELD(format),
	mxMEMBER_FIELD(width),
	mxMEMBER_FIELD(height),
	mxMEMBER_FIELD(numMips),
	mxMEMBER_FIELD(dynamic),
	//mxMEMBER_FIELD(readOnly),
mxEND_REFLECTION
Texture2DDescription::Texture2DDescription()
{
	format = TextureFormat::Unknown;
	width = 0;
	height = 0;
	numMips = 0;
	dynamic = false;
}

mxDEFINE_CLASS(Texture3DDescription);
mxBEGIN_REFLECTION(Texture3DDescription)
	mxMEMBER_FIELD(format),
	mxMEMBER_FIELD(width),
	mxMEMBER_FIELD(height),
	mxMEMBER_FIELD(depth),
	mxMEMBER_FIELD(numMips),
mxEND_REFLECTION

mxDEFINE_CLASS(ColorTargetDescription);
mxBEGIN_REFLECTION(ColorTargetDescription)
	mxMEMBER_FIELD(format),
	mxMEMBER_FIELD(width),
	mxMEMBER_FIELD(height),
mxEND_REFLECTION

mxDEFINE_CLASS(DepthTargetDescription);
mxBEGIN_REFLECTION(DepthTargetDescription)
	mxMEMBER_FIELD(format),
	mxMEMBER_FIELD(width),
	mxMEMBER_FIELD(height),
	mxMEMBER_FIELD(sample),
mxEND_REFLECTION
DepthTargetDescription::DepthTargetDescription()
{
	format = DepthStencilFormat::D24S8;
	width = 0;
	height = 0;
	sample = false;
}

const Chars g_shaderTypeName[ShaderTypeCount] =
{
	"Vertex shader",
	"Hull shader",
	"Domain shader",
	"Geometry shader",
	"Fragment shader",
};
const char* EShaderTypeToChars( EShaderType shaderType )
{
	if( shaderType >= ShaderTypeCount ) {
		return NULL;
	}
	return g_shaderTypeName[ shaderType ].buffer;
}

mxDEFINE_CLASS(CBufferBindingOGL);
mxBEGIN_REFLECTION(CBufferBindingOGL)
	mxMEMBER_FIELD(name),
	mxMEMBER_FIELD(slot),
mxEND_REFLECTION;
mxDEFINE_CLASS(SamplerBindingOGL);
mxBEGIN_REFLECTION(SamplerBindingOGL)
	mxMEMBER_FIELD(name),
	mxMEMBER_FIELD(slot),
mxEND_REFLECTION;
mxDEFINE_CLASS(ProgramBindingsOGL);
mxBEGIN_REFLECTION(ProgramBindingsOGL)
	mxMEMBER_FIELD(uniforms),
	mxMEMBER_FIELD(samplers),
mxEND_REFLECTION;

void ProgramBindingsOGL::Clear()
{
	uniforms.Empty();
	samplers.Empty();
	activeVertexAttributes = 0;
}

mxDEFINE_CLASS(ProgramDescription);
mxBEGIN_REFLECTION(ProgramDescription)
	mxMEMBER_FIELD(shaders),
	mxMEMBER_FIELD(bindings),
mxEND_REFLECTION
ProgramDescription::ProgramDescription()
{
	memset(shaders, LLGL_NULL_HANDLE, sizeof(shaders));
	bindings = NULL;
}

mxBEGIN_REFLECT_ENUM( BufferTypeT )
	mxREFLECT_ENUM_ITEM( Uniform, EBufferType::Buffer_Uniform ),
	mxREFLECT_ENUM_ITEM( Vertex, EBufferType::Buffer_Vertex ),
	mxREFLECT_ENUM_ITEM( Index, EBufferType::Buffer_Index ),
mxEND_REFLECT_ENUM

UINT32 CalculateTextureSize( UINT16 _width, UINT16 _height, TextureFormatT _format, UINT8 _numMips )
{
	const UINT8 _depth = 1;

	UINT32 textureSize = 0;

	const bool blockCompressed = TextureFormat::IsCompressed(_format);
	const UINT32 bitsPerPixel = TextureFormat::BitsPerPixel(_format);
	const UINT32 texelsInBlock = TextureFormat::GetBlockSize(_format);
	const UINT32 blockSizeBytes = (texelsInBlock * bitsPerPixel) / BITS_IN_BYTE;

	UINT32 width  = largest( _width, 1 );
	UINT32 height = largest( _height, 1 );
	UINT32 depth  = largest( _depth, 1 );

	for( UINT lodIndex = 0; lodIndex < _numMips; lodIndex++ )
	{
		// Determine the size of this mipmap level, in bytes.
		UINT32 levelSize = 0;

		// And find out the size of a row of blocks, in bytes.
		UINT32 pitchSize = 0;

		// Pictures are encoded in blocks (1x1 or 4x4) and this is the height of the block matrix.
		UINT32 rowCount = 0;

		if( blockCompressed )
		{
			// A block-compressed texture must be a multiple of 4 in all dimensions
			// because the block-compression algorithms operate on 4x4 texel blocks.
			// There's memory padding to make the size multiple of 4 on every level.
			const UINT32 numBlocksWide = largest( 1, (width + 3) / 4 );		// round up to 4
			const UINT32 numBlocksHigh = largest( 1, (height + 3) / 4 );	// round up to 4
			//const UINT32 physicalWidth  = numBlocksWide * 4;
			//const UINT32 physicalHeight = numBlocksHigh * 4;
			levelSize = numBlocksWide * numBlocksHigh * depth * blockSizeBytes;
			pitchSize = numBlocksWide * blockSizeBytes;
			rowCount = numBlocksHigh;
		}
		else
		{
			pitchSize = (width * bitsPerPixel + (BITS_IN_BYTE-1)) / BITS_IN_BYTE;	// round up to nearest byte
			rowCount = height;
			levelSize = pitchSize * rowCount;
			mxASSERT(levelSize == (width * height * depth * bitsPerPixel) / BITS_IN_BYTE);
		}

		textureSize += levelSize;

		width  = largest( width/2, 1 );
		height = largest( height/2, 1 );
	}

	return textureSize;
}

ERet ParseMipLevels( const TextureImage& _image, UINT8 _side, MipLevel *_mips, UINT8 _maxMips )
{
	mxASSERT(_image.width  >= 1);
	mxASSERT(_image.height >= 1);
	mxASSERT(_image.depth  >= 0);

	const bool blockCompressed = TextureFormat::IsCompressed(_image.format);
	const UINT32 bitsPerPixel = TextureFormat::BitsPerPixel(_image.format);
	const UINT32 texelsInBlock = TextureFormat::GetBlockSize(_image.format);
	const UINT32 blockSizeBytes = (texelsInBlock * bitsPerPixel) / BITS_IN_BYTE;

	const UINT numSides = _image.isCubeMap ? 6 : 1;
	const UINT numMips = smallest(_image.numMips, _maxMips);

	// current byte offset into the image data
	UINT32 offset = 0;

	for( UINT sideIndex = 0; sideIndex < numSides; sideIndex++ )
	{
		UINT32 width  = largest( _image.width, 1 );
		UINT32 height = largest( _image.height, 1 );
		UINT32 depth  = largest( _image.depth, 1 );

		for( UINT lodIndex = 0; lodIndex < numMips; lodIndex++ )
		{
			// Determine the size of this mipmap level, in bytes.
			UINT32 levelSize = 0;

			// And find out the size of a row of blocks, in bytes.
			UINT32 pitchSize = 0;

			// Pictures are encoded in blocks (1x1 or 4x4) and this is the height of the block matrix.
			UINT32 rowCount = 0;

			if( blockCompressed )
			{
				// A block-compressed texture must be a multiple of 4 in all dimensions
				// because the block-compression algorithms operate on 4x4 texel blocks.
				// There's memory padding to make the size multiple of 4 on every level.
				const UINT32 numBlocksWide = largest( 1, (width + 3) / 4 );		// round up to 4
				const UINT32 numBlocksHigh = largest( 1, (height + 3) / 4 );	// round up to 4
				//const UINT32 physicalWidth  = numBlocksWide * 4;
				//const UINT32 physicalHeight = numBlocksHigh * 4;
				levelSize = numBlocksWide * numBlocksHigh * depth * blockSizeBytes;
				pitchSize = numBlocksWide * blockSizeBytes;
				rowCount = numBlocksHigh;
			}
			else
			{
				pitchSize = (width * bitsPerPixel + (BITS_IN_BYTE-1)) / BITS_IN_BYTE;	// round up to nearest byte
				rowCount = height;
				levelSize = pitchSize * rowCount;
				mxASSERT(levelSize == (width * height * depth * bitsPerPixel) / BITS_IN_BYTE);
			}

			if( sideIndex == _side )
			{
				MipLevel& mip = _mips[ lodIndex ];

				mip.data		= mxAddByteOffset( _image.data, offset );
				mip.size		= levelSize;
				mip.pitch		= pitchSize;
				mip.width		= width;
				mip.height		= height;
			}

			offset += levelSize;

			mxASSERT(offset <= _image.size);
			mxUNUSED(_image.size);

			width  = largest( width/2, 1 );
			height = largest( height/2, 1 );
			// For mipmap levels that are smaller than 4x4,
			// only the first four texels will be used for a 2x2 map,
			// and only the first texel will be used by a 1x1 block.
		}
	}
	return ALL_OK;
}

ERet AddBindings(
	ProgramBindingsOGL &destination,
	const ProgramBindingsOGL &source
)
{
	for( UINT bufferIndex = 0; bufferIndex < source.uniforms.Num(); bufferIndex++ )
	{
		const CBufferBindingOGL& binding = source.uniforms[ bufferIndex ];
		const CBufferBindingOGL* existing = FindByName( destination.uniforms, binding.name.ToPtr() );
		if( !existing )
		{
			destination.uniforms.Add( binding );
		}
	}
	for( UINT samplerIndex = 0; samplerIndex < source.samplers.Num(); samplerIndex++ )
	{
		const SamplerBindingOGL& binding = source.samplers[ samplerIndex ];
		const SamplerBindingOGL* existing = FindByName( destination.samplers, binding.name.ToPtr() );
		if( !existing )
		{
			destination.samplers.Add( binding );
		}
	}
	destination.activeVertexAttributes |= source.activeVertexAttributes;
	return ALL_OK;
}

ERet AssignBindPoints(
	ProgramBindingsOGL & bindings
)
{
	for( UINT bufferIndex = 0; bufferIndex < bindings.uniforms.Num(); bufferIndex++ )
	{
		CBufferBindingOGL & binding = bindings.uniforms[ bufferIndex ];
		binding.slot = bufferIndex;
	}
	for( UINT samplerIndex = 0; samplerIndex < bindings.samplers.Num(); samplerIndex++ )
	{
		SamplerBindingOGL & binding = bindings.samplers[ samplerIndex ];
		binding.slot = samplerIndex;
	}
	return ALL_OK;
}

ERet MergeBindings(
	ProgramBindingsOGL &destination,
	const ProgramBindingsOGL &source
)
{
	for( UINT bufferIndex = 0; bufferIndex < source.uniforms.Num(); bufferIndex++ )
	{
		const CBufferBindingOGL& binding = source.uniforms[ bufferIndex ];
		const CBufferBindingOGL* existing = FindByName( destination.uniforms, binding.name.ToPtr() );
		if( existing )
		{
			chkRET_X_IF_NOT( existing->slot == binding.slot, ERR_LINKING_FAILED );
		}
		else
		{
			destination.uniforms.Add( binding );
		}
	}
	for( UINT samplerIndex = 0; samplerIndex < source.samplers.Num(); samplerIndex++ )
	{
		const SamplerBindingOGL& binding = source.samplers[ samplerIndex ];
		const SamplerBindingOGL* existing = FindByName( destination.samplers, binding.name.ToPtr() );
		if( existing )
		{
			chkRET_X_IF_NOT( existing->slot == binding.slot, ERR_LINKING_FAILED );
		}
		else
		{
			destination.samplers.Add( binding );
		}
	}
	destination.activeVertexAttributes |= source.activeVertexAttributes;
	return ALL_OK;
}

mxBEGIN_REFLECT_ENUM( ScenePassT )
	mxREFLECT_ENUM_ITEM( Unknown, ScenePass::Unknown ),
	mxREFLECT_ENUM_ITEM( FillBuffers, ScenePass::FillBuffers ),
	mxREFLECT_ENUM_ITEM( Translucent, ScenePass::Translucent ),
	mxREFLECT_ENUM_ITEM( Shadow, ScenePass::Shadow ),
	mxREFLECT_ENUM_ITEM( Fallback, ScenePass::Fallback ),
mxEND_REFLECT_ENUM

mxBEGIN_REFLECT_ENUM( VertexTypeT )
	mxREFLECT_ENUM_ITEM( Static, VertexType::Static ),
	mxREFLECT_ENUM_ITEM( Skinned, VertexType::Skinned ),
	mxREFLECT_ENUM_ITEM( Generic, VertexType::Generic ),
mxEND_REFLECT_ENUM

mxDEFINE_CLASS( RawVertexStream );
mxBEGIN_REFLECTION( RawVertexStream )
	mxMEMBER_FIELD( data ),
mxEND_REFLECTION

mxDEFINE_CLASS( RawVertexData );
mxBEGIN_REFLECTION( RawVertexData )
	mxMEMBER_FIELD( streams ),
	mxMEMBER_FIELD( count ),
	mxMEMBER_FIELD( type ),
mxEND_REFLECTION

mxDEFINE_CLASS( RawIndexData );
mxBEGIN_REFLECTION( RawIndexData )
	mxMEMBER_FIELD( data ),
	mxMEMBER_FIELD( stride ),
mxEND_REFLECTION

mxDEFINE_CLASS( RawMeshPart );
mxBEGIN_REFLECTION( RawMeshPart )
	mxMEMBER_FIELD( baseVertex ),
	mxMEMBER_FIELD( startIndex ),
	mxMEMBER_FIELD( indexCount ),
	mxMEMBER_FIELD( vertexCount ),
mxEND_REFLECTION

mxDEFINE_CLASS(Bone);
mxBEGIN_REFLECTION(Bone)
	mxMEMBER_FIELD(orientation),
	mxMEMBER_FIELD(position),
	mxMEMBER_FIELD(parent),
mxEND_REFLECTION
Bone::Bone()
{
	orientation = Quaternion_Identity();
	position = Float3_Replicate(0.0f);
	parent = -1;
}

mxDEFINE_CLASS(Skeleton);
mxBEGIN_REFLECTION(Skeleton)
	mxMEMBER_FIELD(bones),
	mxMEMBER_FIELD(boneNames),
	mxMEMBER_FIELD(invBindPoses),
mxEND_REFLECTION
Skeleton::Skeleton()
{
}

mxDEFINE_CLASS( RawMeshData );
mxBEGIN_REFLECTION( RawMeshData )
	mxMEMBER_FIELD( vertexData ),
	mxMEMBER_FIELD( indexData ),
	mxMEMBER_FIELD( topology ),
	mxMEMBER_FIELD( skeleton ),
	mxMEMBER_FIELD( bounds ),
	mxMEMBER_FIELD( parts ),
mxEND_REFLECTION
RawMeshData::RawMeshData()
{
	topology = Topology::TriangleList;
	AABB24_Clear(&bounds);
}

/*
	This code is based on information obtained from the following site:
	http://www.pcidatabase.com/
*/
DeviceVendor::Enum DeviceVendor::FourCCToVendorEnum( UINT32 fourCC )
{
	switch ( fourCC ) {
		case 0x3D3D	:		return DeviceVendor::Vendor_3DLABS;
		case 0x1002 :		return DeviceVendor::Vendor_ATI;
		case 0x8086 :		return DeviceVendor::Vendor_Intel;
		case 0x102B :		return DeviceVendor::Vendor_Matrox;
		case 0x10DE	:		return DeviceVendor::Vendor_NVidia;
		case 0x5333	:		return DeviceVendor::Vendor_S3;
		case 0x1039	:		return DeviceVendor::Vendor_SIS;
		default :			return DeviceVendor::Vendor_Unknown;
	}
}

/*
	This code is based on information obtained from the following site:
	http://www.pcidatabase.com/
*/
const char* DeviceVendor::GetVendorString( DeviceVendor::Enum vendorId )
{
	switch ( vendorId ) {
		case DeviceVendor::Vendor_3DLABS :
			return "3Dlabs, Inc. Ltd";

		case DeviceVendor::Vendor_ATI :
			return "ATI Technologies Inc. / Advanced Micro Devices, Inc.";

		case DeviceVendor::Vendor_Intel :
			return "Intel Corporation";

		case DeviceVendor::Vendor_Matrox :
			return "Matrox Electronic Systems Ltd.";

		case DeviceVendor::Vendor_NVidia :
			return "NVIDIA Corporation";

		case DeviceVendor::Vendor_S3 :
			return "S3 Graphics Co., Ltd";

		case DeviceVendor::Vendor_SIS :
			return "SIS";

		case DeviceVendor::Vendor_Unknown :
		default:
			;
	}
	return "Unknown vendor";
}

const char* EImageFileFormat::GetFileExtension( EImageFileFormat::Enum e )
{
	switch( e ) {
	case EImageFileFormat::IFF_BMP :	return ".bmp";
	case EImageFileFormat::IFF_JPG :	return ".jpg";
	case EImageFileFormat::IFF_PNG :	return ".png";
	case EImageFileFormat::IFF_DDS :	return ".dds";
	case EImageFileFormat::IFF_TIFF :	return ".tiff";
	case EImageFileFormat::IFF_GIF :	return ".gif";
	case EImageFileFormat::IFF_WMP :	return ".wmp";
	mxNO_SWITCH_DEFAULT;
	}
	return ".image";
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
