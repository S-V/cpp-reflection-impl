#include <Base/Base.h>
#include "JsonSupportInternal.h"

static const bool bEnableJsonComments = true;

const char* CHUNK_TAG = "$CHUNK";
const char* HEADER_TAG = "$HEAD";

const char* NODE_TYPE_TAG = "$TYPE";
const char* NODE_DATA_TAG = "$DATA";
#if 0
	const char* CLASS_NAME_TAG = "$CLASS";
	const char* CLASS_DATA_TAG = "$VALUE";
	const char* BASE_CLASS_TAG = "$SUPER";
#else
	const char* CLASS_NAME_TAG = "$TYPE";
	const char* CLASS_DATA_TAG = "$DATA";
	const char* BASE_CLASS_TAG = "$BASE";
#endif
const char* ASSET_GUID_TAG = "$ASSET_GUID";
const char* ASSET_PATH_TAG = "$ASSET_PATH";
const char* ASSET_TYPE_TAG = "$ASSET_TYPE";

const char* OBJECT_CLASS_TAG = "$CLASS";
const char* OBJECT_INDEX_TAG = "$INDEX";
const char* OBJECT_ARRAY_TAG = "$ARRAY";
const char* OBJECT_ITEMS_TAG = "$ITEMS";
const char* OBJECT_COUNT_TAG = "$COUNT";

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

const char* JSON_Type_To_Chars( json_type e )
{
	switch( e )
	{
	case JSON_FALSE :	return "false";
	case JSON_TRUE :	return "true";
	case JSON_NULL :	return "NULL";
	case JSON_INTEGER :	return "Integer Number";
	case JSON_REAL :	return "Real Number";
	case JSON_STRING :	return "String";
	case JSON_ARRAY :	return "Array";
	case JSON_OBJECT :	return "Object";
		mxNO_SWITCH_DEFAULT;
	}
	return "Unknown";
}

bool JSON_Get_Integer_Value( const json_t* node, json_int_t &value )
{
	chkRET_FALSE_IF_NIL(node);
	if( !json_is_integer( node ) ) {
		ptWARN("Expected an integer, but got '%s'\n", JSON_Type_To_Chars(node->type));
		return false;
	}
	value = json_integer_value( node );
	return true;
}

bool JSON_Find_Integer_Value( const json_t* parent, const char* key, json_int_t &value )
{
	const json_t* node = json_object_get( parent, key );
	if( node == nil ) {
		ptWARN("Expected an integer named '%s', but got NULL\n", key);
		return false;
	}
	return JSON_Get_Integer_Value( node, value );
}

bool JSON_Get_Real_Value( const json_t* node, double &value )
{
	chkRET_FALSE_IF_NIL(node);
	if( !json_is_real( node ) ) {
		ptWARN("Expected a real value, but got '%s'\n", JSON_Type_To_Chars(node->type));
		return false;
	}
	value = json_real_value( node );
	return true;
}

bool JSON_Find_Real_Value( const json_t* parent, const char* key, double &value )
{
	const json_t* node = json_object_get( parent, key );
	if( node == nil ) {
		ptWARN("Expected a real value named '%s', but got NULL\n", key);
		return false;
	}
	return JSON_Get_Real_Value( node, value );
}

bool JSON_Get_Boolean_Value( const json_t* node, bool &value )
{
	chkRET_FALSE_IF_NIL(node);
	if( !json_is_boolean( node ) ) {
		ptWARN("Expected a boolean, but got '%s'\n", JSON_Type_To_Chars(node->type));
		return false;
	}
	value = json_is_true( node );
	return true;
}

bool JSON_Find_Boolean_Value( const json_t* parent, const char* key, bool &value )
{
	const json_t* node = json_object_get( parent, key );
	if( node == nil ) {
		ptWARN("Expected a boolean named '%s', but got NULL\n", key);
		return false;
	}
	return JSON_Get_Boolean_Value( node, value );
}

bool JSON_Get_String_Value( const json_t* node, String &value )
{
	chkRET_FALSE_IF_NIL(node);
	if( !json_is_string( node ) ) {
		ptWARN("Expected a string, but got '%s'\n", JSON_Type_To_Chars(node->type));
		return false;
	}
	value.Copy(Chars(json_string_value( node )));
	return true;
}

bool JSON_Find_String_Value( const json_t* parent, const char* key, String &value )
{
	const json_t* node = json_object_get( parent, key );
	if( node == nil ) {
		ptWARN("Expected a string named '%s', but got NULL\n", key);
		return false;
	}
	return JSON_Get_String_Value( node, value );
}

json_t* Flags_To_JSON( const mxFlagsType& flagsType, const void* flagsObject )
{
	json_t* flagsNode = json_array();

	const UINT currVal = flagsType.m_accessor.Get_Value( flagsObject );
	const UINT numBits = flagsType.m_numFlags;
	for( UINT i = 0; i < numBits; i++ )
	{
		const mxFlagsType::Member& flag = flagsType.m_members[ i ];
		if( currVal & flag.mask ) {
			json_array_append_new( flagsNode, json_string(flag.name) );
		}
	}

	return flagsNode;
}

void JSON_To_Flags( const json_t* jsonValue, const mxFlagsType& flagsType, void* flagsObject )
{
	mxASSERT(json_is_array(jsonValue));
	const UINT numFlags = json_array_size( jsonValue );
	mxFlagsType::Mask integerValue = 0;
	for( UINT i = 0; i < numFlags; i++ )
	{
		const json_t* flagValue = json_array_get( jsonValue, i );
		const mxFlagsType::Mask mask = flagsType.GetItemValueByName( json_string_value(flagValue) );
		integerValue |= mask;
	}
	flagsType.m_accessor.Set_Value( flagsObject, integerValue );
}

json_t* JSON_ParseStream( AStreamReader& stream, const char* file, int line )
{
	json_error_t error;

	ByteBuffer	fileData;
	if(mxSUCCEDED(Util_LoadStreamToBlob( stream, fileData )))
	{
		const UINT fileSize = fileData.GetDataSize();
		fileData.Reserve( fileSize + 1 );
		fileData[ fileSize ] = '\0';

		const char* start = c_cast(const char*) fileData.ToPtr();
		//const char* end = start + fileSize;

		const size_t flags = 0;

		json_t* root = json_loads( start, flags, &error );
		if( !root ) {
			ptERROR("%s(%d,%d): parse error: %s", file, error.line+line, error.column, error.text );
			return nil;
		}
		return root;
	}
	return nil;
}

extern "C" void jsonp_free(void *ptr);

bool JSON_WriteToStream( const json_t* root, AStreamWriter& stream )
{
	chkRET_FALSE_IF_NIL( root );

	const size_t flags = JSON_INDENT(4)
		| JSON_ENSURE_ASCII
		//| JSON_SORT_KEYS
		| JSON_PRESERVE_ORDER
		;

	char * text = json_dumps( root, flags );
	chkRET_FALSE_IF_NIL( text );

	stream.Write( text, strlen(text) );

	if( text != nil ) {
		jsonp_free( text );
	}

	return true;
}

bool JSON_DumpToFile( const json_t* root, const char* file )
{
	const size_t flags = JSON_INDENT(4)
		| JSON_ENSURE_ASCII
		| JSON_SORT_KEYS
		;
	return json_dump_file( root, file, flags );
}


/*
-----------------------------------------------------------------------------
	JsonEncoder
-----------------------------------------------------------------------------
*/
JsonEncoder::JsonEncoder()
{
	m_root = nil;
}

JsonEncoder::~JsonEncoder()
{
	if( m_root != nil )
	{
		m_root = nil;
	}
}

json_t* JsonEncoder::EncodeObject( const void* o, const mxType& type )
{
	mxASSERT(m_root == nil);

#if 0
	mxASSERT2(type.IsClass(), "Only aggregates are supported!");
	const mxClass& classInfo = type.UpCast< mxClass >();

	JSON_Result	result;
	this->Visit_Aggregate( c_cast(void*)o, classInfo, &result );
#else
	JSON_Result	result;
	Reflection::Walker::Visit( (void*)o, type, this, &result );
#endif

	m_root = result.value;

	return m_root;
}

template< typename INTEGER >
static
json_t* JSON_Encode_Integer( const INTEGER value )
{
	mxSTATIC_ASSERT( sizeof(value) <= sizeof(json_int_t) );
	return json_integer( value );
}

static
json_t* JSON_Encode_Integer( const void* mem, const UINT size )
{
	if( size == 1 ) {
		return JSON_Encode_Integer( *(UINT8*)mem );
	}
	if( size == 2 ) {
		return JSON_Encode_Integer( *(UINT16*)mem );
	}
	if( size == 4 ) {
		return JSON_Encode_Integer( *(UINT32*)mem );
	}
	mxUNREACHABLE;
	return nil;
}


/* Work around nonstandard isnan() and isinf() implementations */
#ifndef isnan
static inline int isnan(double x) { return x != x; }
#endif
#ifndef isinf
static inline int isinf(double x) { return !isnan(x) && isnan(x - x); }
#endif

template< typename FLOAT_TYPE >
static
json_t* JSON_Encode_Real( const FLOAT_TYPE value )
{
	mxSTATIC_ASSERT( sizeof(value) <= sizeof(double) );
	return json_real( value );
}

inline
json_t* JSON_Encode_Real( const void* mem, const UINT size )
{
	double value;

	if( size == 4 ) {
		value = *(FLOAT*)mem;
	}
	else if( size == 8 ) {
		value = *(DOUBLE*)mem;
	}
	else {
		mxUNREACHABLE;
		return nil;
	}
    if( isnan(value) || isinf(value) )
	{
		mxASSERT(false && "Invalid floating-point value!");
		value = 0.0f;
	}
	return JSON_Encode_Real( value );
}

void* JsonEncoder::Visit_POD( void * o, const mxType& type, void* _userData )
{
	mxASSERT(ETypeKind_Is_Bitwise_Serializable(type.m_kind));

	JSON_Result* result = (JSON_Result*)_userData;

	const ETypeKind typeKind = type.m_kind;
	const MetaSize typeSize = type.m_size;

	json_t* jsonValue = nil;

	switch( typeKind )
	{
	case ETypeKind::Type_Integer :
		jsonValue = JSON_Encode_Integer( o, typeSize );
		break;

	case ETypeKind::Type_Float :
		jsonValue = JSON_Encode_Real( o, typeSize );
		break;

	case ETypeKind::Type_Bool :
		{
			const bool booleanValue = TPODCast< bool >::GetConst( o );
			jsonValue = json_boolean( booleanValue );
		}
		break;

	case ETypeKind::Type_Enum :
		{
			const mxEnumType& enumInfo = type.UpCast< mxEnumType >();
			const UINT enumValue = enumInfo.m_accessor.Get_Value( o );
			jsonValue = json_string( enumInfo.GetStringByInteger(enumValue) );
		}
		break;

	case ETypeKind::Type_Flags :
		{
			const mxFlagsType& flagsType = type.UpCast< mxFlagsType >();
			jsonValue = Flags_To_JSON( flagsType, o );
		}
		break;

		mxNO_SWITCH_DEFAULT;
	}

	mxASSERT_PTR(jsonValue);

	result->value = jsonValue;

	return nil;
}

void* JsonEncoder::Visit_Aggregate( void * o, const mxClass& type, void* _userData )
{
	JSON_Result *	result = (JSON_Result*) _userData;

	// create a new JSON object value
	json_t* objectValue = json_object();
	mxASSERT_PTR(objectValue);

	// write object type
	if(0)
	{
		json_t* typeTag = json_string( type.m_name.buffer );
		json_object_set_new( objectValue, CLASS_NAME_TAG, typeTag );
	}

	// write object data
	{
		JSON_Result	objectData;
		objectData.parent = objectValue;

		Super::Visit_Aggregate( o, type, &objectData );
	}

	result->value = objectValue;

	return nil;
}

void* JsonEncoder::Visit_Field( void * o, const mxField& field, void* _userData )
{
	JSON_Result *	context = (JSON_Result*) _userData;
	mxASSERT_PTR(context->parent);

	Super::Visit_Field( o, field, context );
	mxASSERT_PTR(context->value);

	json_object_set_new( context->parent, field.name, context->value );
	return nil;
}

void* JsonEncoder::Visit_Pointer( VoidPointer& p, const mxPointerType& type, void* _userData )
{
	mxDBG_UNREACHABLE;
	return nil;
}

void* JsonEncoder::Visit_UserPointer( void * o, const mxUserPointerType& type, void* _userData )
{
	JSON_Result *	result = (JSON_Result*) _userData;
	result->value = json_string( type.GetPersistentStringId( o ) );
	return nil;
}

void* JsonEncoder::Visit_AssetId( AssetID & o, void* _userData )
{
	JSON_Result *	result = (JSON_Result*) _userData;
	result->value = json_string( AssetId_ToChars( o ) );
	return nil;
}

void* JsonEncoder::Visit_TypeId( SClassId * o, void* _userData )
{
	mxASSERT_PTR(o);
	JSON_Result *	result = (JSON_Result*) _userData;
	const mxClass* type = o->type;
	if( type ) {
		result->value = json_string( type->GetTypeName() );
	} else {
		result->value = json_string( "NULL" );
	}
	return nil;
}

void* JsonEncoder::Visit_Array( void * arrayObject, const mxArrayType& arrayType, void* _userData )
{
	const UINT numObjects = arrayType.Generic_Get_Count( arrayObject );
	const void* arrayBase = arrayType.Generic_Get_Data( arrayObject );

	const mxType& itemType = arrayType.m_itemType;
	const UINT itemSize = itemType.m_size;

	json_t *	arrayValue = json_array();

	for( UINT iObject = 0; iObject < numObjects; iObject++ )
	{
		const UINT itemOffset = iObject * itemSize;
		void* itemData = mxAddByteOffset( c_cast(void*)arrayBase, itemOffset );

		JSON_Result	result;
		Reflection::Walker::Visit( itemData, itemType, this, &result );
		mxASSERT_PTR(result.value);

		json_array_append_new( arrayValue, result.value );
	}

	JSON_Result *	result = (JSON_Result*) _userData;
	result->value = arrayValue;
	return nil;
}

void* JsonEncoder::Visit_Blob( void * blobObject, const mxBlobType& type, void* _userData )
{
	void* memoryBlock = type.GetBufferPointer( blobObject );
	const mxClass& layout = type.GetBufferLayout( blobObject );
	this->Visit_Aggregate( memoryBlock, layout, _userData );
	return nil;
}

void* JsonEncoder::Visit_String( String & s, void* _userData )
{
	JSON_Result *	result = (JSON_Result*) _userData;
	if( s.IsEmpty() ) {
		result->value = json_string( "" );
	} else {
		result->value = json_string( s.ToPtr() );
	}
	return nil;
}

/*
-----------------------------------------------------------------------------
	JsonDecoder
-----------------------------------------------------------------------------
*/
JsonDecoder::JsonDecoder()
{
}

JsonDecoder::~JsonDecoder()
{
}

void JsonDecoder::DecodeObject( const json_t* source, const mxType& type, void *o )
{
	mxASSERT_PTR(source);
	mxASSERT_PTR(o);
#if 0
	mxASSERT2(type.IsClass(), "Only aggregates are supported!");
	const mxClass& classInfo = type.UpCast< mxClass >();
	this->Visit_Aggregate( o, classInfo, c_cast(void*)source );
#else
	Reflection::Walker::Visit( o, type, this, (void*)source );
#endif
}

template< typename INTEGER >
static
void JSON_Decode_Integer( const json_t* source, INTEGER *value )
{
	mxASSERT(json_is_integer(source));
	const json_int_t jsonInt = json_integer_value( source );
	mxSTATIC_ASSERT( sizeof(value) <= sizeof(jsonInt) );
	*value = jsonInt;
}

static
void JSON_Decode_Integer( const json_t* source, void *mem, const UINT size )
{
	if( size == 1 ) {
		return JSON_Decode_Integer( source, (UINT8*)mem );
	}
	if( size == 2 ) {
		return JSON_Decode_Integer( source, (UINT16*)mem );
	}
	if( size == 4 ) {
		return JSON_Decode_Integer( source, (UINT32*)mem );
	}
	mxUNREACHABLE;
}

template< typename FLOAT_TYPE >
static
void JSON_Decode_Real( const json_t* source, FLOAT_TYPE *value )
{
	const double jsonReal = json_real_value( source );
	mxSTATIC_ASSERT( sizeof(value) <= sizeof(jsonReal) );
	*value = jsonReal;
}

inline
void JSON_Decode_Real( const json_t* source, void *mem, const UINT size )
{
	if( size == 4 ) {
		return JSON_Decode_Real( source, (FLOAT*)mem );
	}
	if( size == 8 ) {
		return JSON_Decode_Real( source, (DOUBLE*)mem );
	}
	mxUNREACHABLE;
}

void* JsonDecoder::Visit_POD( void * o, const mxType& type, void* _userData )
{
	mxASSERT(ETypeKind_Is_Bitwise_Serializable(type.m_kind));

	const json_t* jsonValue = (json_t*)_userData;
	mxASSERT_PTR(jsonValue);

	const ETypeKind typeKind = type.m_kind;
	const MetaSize typeSize = type.m_size;

	switch( typeKind )
	{
	case ETypeKind::Type_Integer :
		JSON_Decode_Integer( jsonValue, o, typeSize );
		break;

	case ETypeKind::Type_Float :
		JSON_Decode_Real( jsonValue, o, typeSize );
		break;

	case ETypeKind::Type_Bool :
		{
			mxASSERT(json_is_boolean(jsonValue));
			const bool booleanValue = json_is_true( jsonValue );
			TPODCast< bool >::GetNonConst( o ) = booleanValue;
		}
		break;

	case ETypeKind::Type_Enum :
		{
			const mxEnumType& enumInfo = type.UpCast< mxEnumType >();
			const char* sEnumValue = json_string_value( jsonValue );
			const UINT nEnumValue = enumInfo.GetIntegerByString(sEnumValue);
			enumInfo.m_accessor.Set_Value( o, nEnumValue );
		}
		break;

	case ETypeKind::Type_Flags :
		{
			const mxFlagsType& flagsType = type.UpCast< mxFlagsType >();
			JSON_To_Flags( jsonValue, flagsType, o );
		}
		break;

		mxNO_SWITCH_DEFAULT;
	}
	return nil;
}

void* JsonDecoder::Visit_Aggregate( void * o, const mxClass& type, void* _userData )
{
	const json_t *	objectValue = (json_t*) _userData;

	// read object type
	if(0)
	{
		const json_t* typeTag = json_object_get( objectValue, CLASS_NAME_TAG );
		mxASSERT(!strcmp( json_string_value(typeTag), type.m_name.buffer ));
	}

	// read object data
	{
		Super::Visit_Aggregate( o, type, (void*)objectValue );
	}
	return nil;
}

void* JsonDecoder::Visit_Field( void * o, const mxField& field, void* _userData )
{
	const json_t *	objectValue = (json_t*) _userData;
	mxASSERT_PTR(objectValue);

	mxASSERT_PTR(field.name);
	const json_t *	fieldValue = json_object_get( objectValue, field.name );
	if( fieldValue != nil )
	{
		Super::Visit_Field( o, field, (void*)fieldValue );
	}
	else
	{
		ptWARN("Missing struct field: '%s'\n", field.name);
	}
	return nil;
}

void* JsonDecoder::Visit_Pointer( VoidPointer& p, const mxPointerType& type, void* _userData )
{
	mxDBG_UNREACHABLE;
	return nil;
}

void* JsonDecoder::Visit_UserPointer( void * o, const mxUserPointerType& type, void* _userData )
{
	const json_t* stringNode = c_cast(json_t*) _userData;
	type.SetFromStringId( o, json_string_value( stringNode ) );
	return nil;
}

void* JsonDecoder::Visit_AssetId( AssetID & o, void* _userData )
{
	const json_t* stringNode = c_cast(json_t*) _userData;
	const char* stringValue = json_string_value( stringNode );
	if( strcmp(stringValue,"NULL") != 0 ) {
		o.d = mxName(stringValue);
	} else {
		o.d = mxName();
	}
	return nil;
}

void* JsonDecoder::Visit_TypeId( SClassId * o, void* _userData )
{
	mxASSERT_PTR(o);
	const json_t *	objectValue = (json_t*) _userData;
	mxASSERT_PTR(objectValue);

	const char* typeName = json_string_value( objectValue );
	if( strcmp(typeName, "NULL") != 0 ) {
		o->type = TypeRegistry::Get().FindClassByName( typeName );
	} else {
		o->type = nil;
	}
	return nil;
}

void* JsonDecoder::Visit_Array( void * arrayObject, const mxArrayType& arrayType, void* _userData )
{
	const json_t *	arrayValue = (json_t*) _userData;
	mxASSERT_PTR(arrayValue);
	mxASSERT(json_is_array(arrayValue));

	const UINT numObjects = json_array_size( arrayValue );
	arrayType.Generic_Set_Count( arrayObject, numObjects );

	const void* arrayBase = arrayType.Generic_Get_Data( arrayObject );

	const mxType& itemType = arrayType.m_itemType;
	const UINT itemSize = itemType.m_size;

	for( UINT iObject = 0; iObject < numObjects; iObject++ )
	{
		const UINT itemOffset = iObject * itemSize;
		void* itemData = mxAddByteOffset( (void*)arrayBase, itemOffset );

		const json_t *	itemValue = json_array_get( arrayValue, iObject );

		Reflection::Walker::Visit( itemData, itemType, this, (void*)itemValue );
	}
	return nil;
}

void* JsonDecoder::Visit_Blob( void * blobObject, const mxBlobType& type, void* _userData )
{
	void* memoryBlock = type.GetBufferPointer( blobObject );
	const mxClass& layout = type.GetBufferLayout( blobObject );
	this->Visit_Aggregate( memoryBlock, layout, _userData );
	return nil;
}

void* JsonDecoder::Visit_String( String & s, void* _userData )
{
	const json_t *	jsonValue = (json_t*) _userData;
	JSON_Get_String_Value( jsonValue, s );
	return nil;
}

json_t* JSON_EncodeObject( const void* o, const mxType& type )
{
	JsonEncoder	jsonWriter;
	json_t* value = jsonWriter.EncodeObject( o, type );
	return value;
}

void JSON_DecodeObject( const json_t* source, const mxType& type, void *o )
{
	mxASSERT_PTR(source);
	JsonDecoder	jsonReader;
	jsonReader.DecodeObject( source, type, o );
}

json_t* AssetId_To_JSON_String( const AssetID& assetId )
{
	if( AssetId_IsValid( assetId ) ) {
		return json_string( assetId.d.ToPtr() );
	} else {
		return json_string( "" );
	}
}

AssetID JSON_String_To_AssetId( const json_t* jsonValue )
{
	mxASSERT_PTR(jsonValue);
	mxASSERT( json_is_string(jsonValue) );

	const char* assetPath = json_string_value(jsonValue);
	if( strcmp(assetPath, "") == 0 ) {
		return AssetId_GetNull();
	} else {
		AssetID assetId;
		assetId.d = mxName( assetPath );
		mxASSERT( AssetId_IsValid( assetId ) );
		return assetId;
	}
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
