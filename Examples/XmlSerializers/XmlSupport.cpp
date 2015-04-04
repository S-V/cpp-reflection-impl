#include <Base/Base.h>
#include "pugixml.hpp"
#include "XmlSupport.h"

namespace XML
{

static UINT32 GetInteger( const void* pointer, const int byteWidth )
{
	if( byteWidth == 1 ) {
		return *(UINT8*) pointer;
	}
	else if( byteWidth == 2 ) {
		return *(UINT16*) pointer;
	}
	else if( byteWidth == 4 ) {
		return *(UINT32*) pointer;
	}
	else {
		mxUNREACHABLE;
		return 0;
	}	
}

static void PutInteger( const UINT32 value, void *pointer, const int byteWidth )
{
	if( byteWidth == 1 ) {
		*(UINT8*) pointer = value;
	}
	else if( byteWidth == 2 ) {
		*(UINT16*) pointer = value;
	}
	else if( byteWidth == 4 ) {
		*(UINT32*) pointer = value;
	}
	else {
		mxUNREACHABLE;
	}
}

static double GetDouble( const void* pointer, const int byteWidth )
{
	if( byteWidth == 4 ) {
		return *(float*) pointer;
	}
	else if( byteWidth == 8 ) {
		return *(double*) pointer;
	}
	else {
		mxUNREACHABLE;
		return 0;
	}
}

static void PutDouble( const double value, const void* pointer, const int byteWidth )
{
	if( byteWidth == 4 ) {
		*(float*) pointer = value;
	}
	else if( byteWidth == 8 ) {
		*(double*) pointer = value;
	}
	else {
		mxUNREACHABLE;
	}
}

static void Flags_To_XML( const void* o, const mxFlagsType& type, pugi::xml_node &node )
{
	const UINT currVal = type.m_accessor.Get_Value( o );
	const UINT numBits = type.m_numFlags;
	for( UINT i = 0; i < numBits; i++ )
	{
		const mxFlagsType::Member& flag = type.m_members[ i ];
		const bool flagValue = ( currVal & flag.mask );

		node.append_attribute(flag.name).set_value(flagValue);
	}
}

static void XML_To_Flags( const pugi::xml_node& node, const mxFlagsType& type, void *o )
{
	pugi::xml_node child = node.first_child();
	const UINT numBits = type.m_numFlags;
	UINT bitMask = 0;
	for( UINT i = 0; i < numBits; i++ )
	{
		const mxFlagsType::Member& flag = type.m_members[ i ];

		const bool flagValue = child.attribute(flag.name).as_bool();
		if( flagValue ) {
			bitMask |= flag.mask;
		}

		child = child.next_sibling();
	}
	type.m_accessor.Set_Value(o, bitMask);
}

class XmlEncoder : public Reflection::AVisitor {
public:
	typedef Reflection::AVisitor Super;

	//-- Reflection::AVisitor
	virtual void Visit_POD( void * o, const mxType& type, void* _userData ) override
	{
		pugi::xml_node node = *(pugi::xml_node*) _userData;

		const ETypeKind typeKind = type.m_kind;
		const MetaSize typeSize = type.m_size;

		//node.append_attribute("type").set_value(type.GetTypeName());
		//node.append_attribute("kind").set_value(ETypeKind_To_Chars(typeKind));

		switch( typeKind )
		{
		case ETypeKind::Type_Integer :
			{
				pugi::xml_attribute value = node.append_attribute("value");
				value.set_value(GetInteger(o, typeSize));
			}
			break;

		case ETypeKind::Type_Float :
			{
				pugi::xml_attribute value = node.append_attribute("value");
				value.set_value(GetDouble(o, typeSize));
			}
			break;

		case ETypeKind::Type_Bool :
			{
				pugi::xml_attribute value = node.append_attribute("value");
				value.set_value(*(bool*) o);
			}
			break;

		case ETypeKind::Type_Enum :
			{
				pugi::xml_attribute value = node.append_attribute("value");
				const mxEnumType& enumType = type.UpCast< mxEnumType >();
				const UINT32 enumValue = enumType.m_accessor.Get_Value( o );
				value.set_value( enumType.GetStringByInteger(enumValue) );
			}
			break;

		case ETypeKind::Type_Flags :
			{
				const mxFlagsType& flagsType = type.UpCast< mxFlagsType >();
				Flags_To_XML( o, flagsType, node );
			}
			break;

			mxNO_SWITCH_DEFAULT;
		}
	}
	virtual void Visit_String( String & s, void* _userData ) override
	{
		pugi::xml_node node = *(pugi::xml_node*) _userData;
		node.append_attribute("value").set_value(s.ToPtr());
	}
	virtual void Visit_TypeId( SClassId * o, void* _userData ) override
	{
		pugi::xml_node node = *(pugi::xml_node*) _userData;
		node.append_attribute("value").set_value(o->type->GetTypeName());
	}
	virtual void Visit_AssetId( AssetID * o, void* _userData ) override
	{
		pugi::xml_node node = *(pugi::xml_node*) _userData;
		node.append_attribute("value").set_value(o->d.ToPtr());
	}
	virtual void Visit_Pointer( VoidPointer& p, const mxPointerType& type, void* _userData ) override
	{
		Unimplemented;
	}
	virtual void Visit_UserPointer( void * o, const mxUserPointerType& type, void* _userData ) override
	{
		Unimplemented;
	}
	virtual void Visit_Aggregate( void * o, const mxClass& type, void* _userData ) override
	{
		Super::Visit_Aggregate(o, type, _userData);
	}
	virtual void Visit_Field( void * o, const mxField& field, void* _userData ) override
	{
		pugi::xml_node parentNode = *(pugi::xml_node*) _userData;
		pugi::xml_node childNode = parentNode.append_child( field.name );
		Super::Visit_Field( o, field, &childNode );
	}
	virtual void Visit_Array( void * arrayObject, const mxArrayType& arrayType, void* _userData ) override
	{
		pugi::xml_node parentNode = *(pugi::xml_node*) _userData;
		pugi::xml_node arrayNode = parentNode.append_child("ARRAY");

		const UINT arrayCount = arrayType.Generic_Get_Count( arrayObject );
		const void* arrayBase = arrayType.Generic_Get_Data( arrayObject );

		const mxType& itemType = arrayType.m_itemType;
		const UINT itemSize = itemType.m_size;

		//arrayNode.append_attribute("array_type").set_value(arrayType.GetTypeName());
		//arrayNode.append_attribute("item_type").set_value(arrayType.m_itemType.GetTypeName());
		arrayNode.append_attribute("COUNT").set_value(arrayCount);

		for( UINT itemIndex = 0; itemIndex < arrayCount; itemIndex++ )
		{
			const MetaOffset itemOffset = itemIndex * itemSize;
			void* itemData = mxAddByteOffset( c_cast(void*)arrayBase, itemOffset );

			pugi::xml_node childNode = arrayNode.append_child(itemType.GetTypeName());
			this->Visit_Element( itemData, itemType, &childNode );
		}
	}
};

class XmlDecoder : public Reflection::AVisitor {
public:
	typedef Reflection::AVisitor Super;

	//-- Reflection::AVisitor
	virtual void Visit_POD( void * o, const mxType& type, void* _userData ) override
	{
		const pugi::xml_node node = *(pugi::xml_node*) _userData;

		const ETypeKind typeKind = type.m_kind;
		const MetaSize typeSize = type.m_size;

		switch( typeKind )
		{
		case ETypeKind::Type_Integer :
			{
				const pugi::xml_attribute value = node.attribute("value");
				PutInteger(value.as_uint(), o, typeSize);
			}
			break;

		case ETypeKind::Type_Float :
			{
				const pugi::xml_attribute value = node.attribute("value");
				PutDouble(value.as_double(), o, typeSize);
			}
			break;

		case ETypeKind::Type_Bool :
			{
				const pugi::xml_attribute value = node.attribute("value");
				*(bool*) o = value.as_bool();
			}
			break;

		case ETypeKind::Type_Enum :
			{
				pugi::xml_attribute value = node.attribute("value");
				const mxEnumType& enumType = type.UpCast< mxEnumType >();
				const UINT32 enumValue = enumType.GetIntegerByString( value.value() );
				enumType.m_accessor.Set_Value( o, enumValue );
			}
			break;

		case ETypeKind::Type_Flags :
			{
				const mxFlagsType& flagsType = type.UpCast< mxFlagsType >();
				XML_To_Flags( node, flagsType, o );
			}
			break;

			mxNO_SWITCH_DEFAULT;
		}
	}
	virtual void Visit_String( String & s, void* _userData ) override
	{
		const pugi::xml_node node = *(pugi::xml_node*) _userData;
		s.Copy(Chars(node.attribute("value").value()));
	}
	virtual void Visit_TypeId( SClassId * o, void* _userData ) override
	{
		const pugi::xml_node node = *(pugi::xml_node*) _userData;
		o->type = TypeRegistry::Get().FindClassByName( node.attribute("value").value() );
	}
	virtual void Visit_AssetId( AssetID * o, void* _userData ) override
	{
		const pugi::xml_node node = *(pugi::xml_node*) _userData;
		o->d = mxName( node.attribute("value").value() );
	}
	virtual void Visit_Pointer( VoidPointer& p, const mxPointerType& type, void* _userData ) override
	{
		Unimplemented;
	}
	virtual void Visit_UserPointer( void * o, const mxUserPointerType& type, void* _userData ) override
	{
		Unimplemented;
	}
	virtual void Visit_Aggregate( void * o, const mxClass& type, void* _userData ) override
	{
		Super::Visit_Aggregate(o, type, _userData);
	}
	virtual void Visit_Field( void * o, const mxField& field, void* _userData ) override
	{
		pugi::xml_node parentNode = *(pugi::xml_node*) _userData;
		pugi::xml_node childNode = parentNode.child( field.name );
		Super::Visit_Field( o, field, &childNode );
	}
	virtual void Visit_Array( void * arrayObject, const mxArrayType& arrayType, void* _userData ) override
	{
		pugi::xml_node parentNode = *(pugi::xml_node*) _userData;
		//pugi::xml_node arrayNode = parentNode.child("ARRAY");
		pugi::xml_node arrayNode = parentNode.first_child();

		const UINT arrayCount = arrayNode.attribute("COUNT").as_uint();
		arrayType.Generic_Set_Count( arrayObject, arrayCount );

		const void* arrayBase = arrayType.Generic_Get_Data( arrayObject );

		const mxType& itemType = arrayType.m_itemType;
		const UINT itemSize = itemType.m_size;

		pugi::xml_node child = arrayNode.first_child();
		for( UINT itemIndex = 0; itemIndex < arrayCount; itemIndex++ )
		{
			const MetaOffset itemOffset = itemIndex * itemSize;
			void* itemData = mxAddByteOffset( c_cast(void*)arrayBase, itemOffset );

			this->Visit_Element( itemData, itemType, &child );
			child = child.next_sibling();
		}
	}
};

ERet EncodeObject( const void* o, const mxType& type, AStreamWriter &stream )
{
	pugi::xml_document	doc;

	XmlEncoder	encoder;
	encoder.Visit_Element( (void*)o, type, &doc );

	class My_XML_Writer : public pugi::xml_writer
	{
		AStreamWriter &	m_stream;
	public:
		inline My_XML_Writer( AStreamWriter &stream ) : m_stream( stream ) {
		}
		virtual void write(const void* data, size_t size) override {
			m_stream.Write(data, size);
		}
	};
	My_XML_Writer	streamWriter( stream );
	doc.save( streamWriter );

	return ALL_OK;
}

ERet DecodeObject( AStreamReader& reader, const mxType& type, void *o )
{
	ByteBuffer	buffer;
	mxDO(Util_LoadStreamToBlob(reader, buffer));

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_buffer_inplace( buffer.ToPtr(), buffer.GetDataSize() );
	if( !result ) {
		return ERR_FAILED_TO_PARSE_DATA;
	}

	XmlDecoder	decoder;
	decoder.Visit_Element( (void*)o, type, &doc );

	return ALL_OK;
}

}//namespace XML

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
