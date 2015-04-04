/*
=============================================================================
	File:	JsonSupport.cpp
	Desc:	text serializers
	ToDo:	refactor; add range checks
=============================================================================
*/
#include <Base/Base.h>

#include "JsonSupport.h"
#include "JsonSupportInternal.h"

#if 0
/* swap 32 bits integers */
#define swap_uint32(x)	((((x) & 0x000000FFU) << 24) | \
						(((x) & 0x0000FF00U) << 8)  | \
						(((x) & 0x00FF0000U) >> 8)  | \
						(((x) & 0xFF000000U) >> 24))

static TLocalString<8> FileChunkType_To_String( FileChunkType chunkId )
{
	// convert to human-readable fourCC string
	// e.g. 'TIDE' => 'EDIT'
#if (mxCPU_ENDIANNESS == mxCPU_ENDIAN_LITTLE)
	const UINT32 fourCC = swap_uint32( chunkId );
#else
	const UINT32 fourCC = chunkId;
#endif

	char asString[ sizeof(fourCC) + 1 ];
	memcpy( asString, &fourCC, sizeof(fourCC) );
	asString[ sizeof(fourCC) ] = '\0';

	return TLocalString<8>( asString );
}
#endif


namespace JSON
{
	Scope::Scope()
	{
	}
	Scope::~Scope()
	{
		if( root ) {
			json_decref(root);
			root = NULL;
		}
	}

	ERet WriteToStream( const void* o, const mxType& type, AStreamWriter &stream )
	{
		Scope	scope;
		scope.root = JSON_EncodeObject( o, type );
		chkRET_X_IF_NIL(scope.root, ERR_UNKNOWN_ERROR);
		chkRET_X_IF_NOT(JSON_WriteToStream( scope.root, stream ), ERR_FAILED_TO_WRITE_FILE);
		return IM_OK;
	}
	ERet LoadFromStream( AStreamReader& stream, void *o, const mxType& type, const char* name, int line )
	{
		Scope	scope;
		scope.root = JSON_ParseStream( stream, name, line );
		chkRET_X_IF_NIL(scope.root, ERR_FAILED_TO_PARSE_DATA);
		JSON_DecodeObject( scope.root, type, o );
		return IM_OK;
	}

	ERet SaveClumpToFile( const Clump& clump, const char* file )
	{
		ClumpWriterJson	writer;
		if( !writer.SaveClump(clump) ) {
			return ERR_FAILED_TO_PARSE_DATA;
		}

		FileWriter	stream;
		mxDO(stream.Open(file, FileWrite_NoErrors));

		if( !writer.DumpToStream(stream) ) {
			return ERR_FAILED_TO_WRITE_FILE;
		}

		return IM_OK;
	}
	ERet LoadClumpFromFile( const char* file, Clump & clump )
	{
		FileReader	stream;
		mxDO(stream.Open(file, FileRead_NoErrors));

		ClumpLoaderJson	loader( stream );
		chkRET_X_IF_NOT(loader.LoadClump( clump ), ERR_FAILED_TO_PARSE_DATA);

		return IM_OK;
	}
}//namespace JSON



/*
-----------------------------------------------------------------------------
	JsonWriter
-----------------------------------------------------------------------------
*/
JsonWriter::JsonWriter()
{
	m_root = json_object();
	mxASSERT_PTR(m_root);
	m_curr = m_root;
}

JsonWriter::~JsonWriter()
{
	if( m_root != nil )
	{
		json_decref( m_root );
		m_root = nil;
	}
	m_curr = nil;
}

bool JsonWriter::DumpToStream( AStreamWriter& stream ) const
{
	return JSON_WriteToStream( m_root, stream );
}

void JsonWriter::Enter_Scope( const char* name )
{
	json_t *	newValue = json_object();
	{
		Scope	newItem;
		newItem.name.Copy(Chars(name));
		newItem.node = m_curr;

		m_scopeStack.Add( newItem );
	}
	m_curr = newValue;
}

void JsonWriter::Leave_Scope()
{
	Scope& topItem = m_scopeStack.GetLast();

	json_object_set_new( topItem.node, topItem.name.ToPtr(), m_curr );

	m_curr = topItem.node;

	m_scopeStack.PopLast();
}

bool JsonWriter::Serialize_Bool( const char* name, bool & rValue )
{
	json_t *	booleanValue = json_boolean( rValue );
	json_object_set_new( m_curr, name, booleanValue );
	return true;
}

bool JsonWriter::Serialize_Int8( const char* name, INT8 & rValue )
{
	json_t *	intValue = json_integer( rValue );
	json_object_set_new( m_curr, name, intValue );
	return true;
}

bool JsonWriter::Serialize_Uint8( const char* name, UINT8 & rValue )
{
	json_t *	newNode = json_integer( rValue );
	json_object_set_new( m_curr, name, newNode );
	return true;
}

bool JsonWriter::Serialize_Int16( const char* name, INT16 & rValue )
{
	json_t *	newNode = json_integer( rValue );
	json_object_set_new( m_curr, name, newNode );
	return true;
}

bool JsonWriter::Serialize_Uint16( const char* name, UINT16 & rValue )
{
	json_t *	newNode = json_integer( rValue );
	json_object_set_new( m_curr, name, newNode );
	return true;
}

bool JsonWriter::Serialize_Int32( const char* name, INT32 & rValue )
{
	json_t *	newNode = json_integer( rValue );
	json_object_set_new( m_curr, name, newNode );
	return true;
}

bool JsonWriter::Serialize_Uint32( const char* name, UINT32 & rValue )
{
	json_t *	newNode = json_integer( rValue );
	json_object_set_new( m_curr, name, newNode );
	return true;
}

bool JsonWriter::Serialize_Int64( const char* name, INT64 & rValue )
{
	//Signed64_Union	union64;
	//union64.v = rValue;

	//Json::Value	int64Value;
	//{
	//	int64Value["Low_Part"] = union64.lo;
	//	int64Value["High_Part"] = union64.hi;
	//}
	//m_curr[ name ] = int64Value;
	UNDONE;
	return true;
}

bool JsonWriter::Serialize_Uint64( const char* name, UINT64 & rValue )
{
	//Unsigned64_Union	union64;
	//union64.v = rValue;

	//Json::Value	uint64Value;
	//{
	//	uint64Value["Low_Part"] = union64.lo;
	//	uint64Value["High_Part"] = union64.hi;
	//}
	//m_curr[ name ] = uint64Value;
	UNDONE;
	return true;
}

bool JsonWriter::Serialize_Float32( const char* name, FLOAT & rValue )
{
	json_t *	newNode = json_real( rValue );
	json_object_set_new( m_curr, name, newNode );
	return true;
}

bool JsonWriter::Serialize_Float64( const char* name, DOUBLE & rValue )
{
	json_t *	newNode = json_real( rValue );
	json_object_set_new( m_curr, name, newNode );
	return true;
}

bool JsonWriter::Serialize_String( const char* name, String & rValue )
{
	json_t *	newNode = json_string( rValue.ToPtr() );
	json_object_set_new( m_curr, name, newNode );
	return true;
}

bool JsonWriter::Serialize_StringList( const char* name, StringListT & rValue )
{
	json_t *	stringArrayValue = json_array();

	const UINT numItems = rValue.Num();
	for( UINT i = 0; i < numItems; i++ )
	{
		const String& o = rValue[ i ];

		json_t *	itemValue = json_string( o.ToPtr() );
		json_array_append_new( stringArrayValue, itemValue );
	}

	json_object_set_new( m_curr, name, stringArrayValue );

	return true;
}


/*
-----------------------------------------------------------------------------
	JsonReader
-----------------------------------------------------------------------------
*/
JsonReader::JsonReader( AStreamReader& stream )
{
	m_root = JSON_ParseStream( stream );
	m_curr = m_root;
}

JsonReader::~JsonReader()
{
	if( m_root != nil )
	{
		json_decref( m_root );
		m_root = nil;
	}
	m_curr = nil;
}

void JsonReader::Enter_Scope( const char* name )
{
	UNDONE;
	//json_t* scopeValue = json_object_get( m_curr, name );

	//if( !scopeValue ) {
	//	ptWARN("JsonReader: null scope: '%s'\n", name);
	//	scopeValue = json_object();
	//}
	//mxASSERT_PTR(scopeValue);
	//{
	//	Scope &	newScope = m_scopeStack.Add();
	//	newScope.node = m_curr;
	//	newScope.name = name;
	//}
	//m_curr = scopeValue;
}

void JsonReader::Leave_Scope()
{
	UNDONE;
	//Scope &	topScope = m_scopeStack.GetLast();
	//m_curr = topScope.node;
	//m_scopeStack.PopLast();
}

bool JsonReader::Serialize_Bool( const char* name, bool & rValue )
{
	return JSON_Find_Boolean_Value( m_curr, name, rValue );
}

bool JsonReader::Serialize_Int8( const char* name, INT8 & rValue )
{
	return JSON_Read_Integer_Value( m_curr, name, rValue );
}

bool JsonReader::Serialize_Uint8( const char* name, UINT8 & rValue )
{
	return JSON_Read_Integer_Value( m_curr, name, rValue );
}

bool JsonReader::Serialize_Int16( const char* name, INT16 & rValue )
{
	return JSON_Read_Integer_Value( m_curr, name, rValue );
}

bool JsonReader::Serialize_Uint16( const char* name, UINT16 & rValue )
{
	return JSON_Read_Integer_Value( m_curr, name, rValue );
}

bool JsonReader::Serialize_Int32( const char* name, INT32 & rValue )
{
	return JSON_Read_Integer_Value( m_curr, name, rValue );
}

bool JsonReader::Serialize_Uint32( const char* name, UINT32 & rValue )
{
	return JSON_Read_Integer_Value( m_curr, name, rValue );
}

bool JsonReader::Serialize_Int64( const char* name, INT64 & rValue )
{
	UNDONE;
	return false;
	//const Json::Value	int64Value = m_curr[ name ];

	//if( !int64Value.isNull() )
	//{
	//	mxASSERT( int64Value.isObject() );

	//	Signed64_Union	union64;

	//	this->Serialize_Int32( "Low_Part", union64.lo );
	//	this->Serialize_Int32( "High_Part", union64.hi );

	//	rValue = union64.v;
	//	return true;
	//}
	//else
	//{
	//	ptWARN("Read_Int64( '%s' ) = null\n", name);
	//	return false;
	//}
}

bool JsonReader::Serialize_Uint64( const char* name, UINT64 & rValue )
{
		UNDONE;
	return false;
	//const Json::Value	uint64Value = m_curr[ name ];

	//if( !uint64Value.isNull() )
	//{
	//	mxASSERT( uint64Value.isObject() );

	//	Unsigned64_Union	union64;

	//	this->Serialize_Uint32( "Low_Part", union64.lo );
	//	this->Serialize_Uint32( "High_Part", union64.hi );

	//	rValue = union64.v;
	//	return true;
	//}
	//else
	//{
	//	ptWARN("Read_UInt64( '%s' ) = null\n", name);
	//	return false;
	//}
}

bool JsonReader::Serialize_Float32( const char* name, FLOAT & rValue )
{
	return JSON_Read_Real_Value( m_curr, name, rValue );
}

bool JsonReader::Serialize_Float64( const char* name, DOUBLE & rValue )
{
	return JSON_Read_Real_Value( m_curr, name, rValue );
}

bool JsonReader::Serialize_String( const char* name, String & rValue )
{
	return JSON_Find_String_Value( m_curr, name, rValue );
}

bool JsonReader::Serialize_StringList( const char* name, StringListT & rValue )
{
	json_t* node = json_object_get( m_curr, name );
	if( nil == node ) {
		ptWARN("Expected an array of strings named '%s', but got 'NULL'\n", name);
		return false;
	}
	if( !json_is_array( node) )
	{
		ptWARN("Expected an array of strings named '%s', but got '%s'\n",
			name, JSON_Type_To_Chars(node->type));
		return false;
	}

	const size_t count = json_array_size( node );
	rValue.SetNum( count );

	for( UINT i = 0; i < count; i++ )
	{
		json_t* stringValue = json_array_get( node, i );
		JSON_Get_String_Value( stringValue, rValue[ i ] );
	}

	return true;
}
#if 0

/*
-----------------------------------------------------------------------------
	JsonObjectWriter
-----------------------------------------------------------------------------
*/
JsonObjectWriter::JsonObjectWriter()
{
	m_root = nil;
}

JsonObjectWriter::~JsonObjectWriter()
{
	if( m_root != nil )
	{
		json_decref( m_root );
		m_root = nil;
	}
}

bool JsonObjectWriter::DumpToStream( AStreamWriter& stream ) const
{
	chkRET_FALSE_IF_NOT( JSON_WriteToStream( m_root, stream ) );
	return true;
}

void JsonObjectWriter::Serialize( const void* o, const mxType& typeInfo )
{
	chkRET_IF_NIL( o );
	mxASSERT(m_root == nil);
	m_root = JSON_EncodeObject( o, typeInfo );
}

/*
-----------------------------------------------------------------------------
	JsonObjectReader
-----------------------------------------------------------------------------
*/
JsonObjectReader::JsonObjectReader( AStreamReader& stream )
{
	m_root = JSON_ParseStream( stream );
}

JsonObjectReader::~JsonObjectReader()
{
	if( m_root != nil )
	{
		json_decref( m_root );
		m_root = nil;
	}
}

void JsonObjectReader::Deserialize( void * o, const mxType& typeInfo )
{
	chkRET_IF_NIL( o );
	chkRET_IF_NIL( m_root );

	JSON_DecodeObject( m_root, typeInfo, o );
}

/*
-----------------------------------------------------------------------------
	JsonChunkedFileWriter
-----------------------------------------------------------------------------
*/
JsonChunkedFileWriter::JsonChunkedFileWriter()
{
	m_root = json_object();
	m_currChunk = nil;
}

JsonChunkedFileWriter::~JsonChunkedFileWriter()
{
	if( m_root != nil )
	{
		json_decref( m_root );
		m_root = nil;
	}
}

bool JsonChunkedFileWriter::DumpToStream( AStreamWriter& stream ) const
{
	chkRET_FALSE_IF_NIL( m_root );
	chkRET_FALSE_IF_NOT( JSON_WriteToStream( m_root, stream ) );
	return true;
}

bool JsonChunkedFileWriter::BeginChunk( FileChunkType chunkId )
{
	mxASSERT_PTR(m_root);

	mxASSERT( m_currChunk == nil );
	m_currChunk = json_object();
	mxASSERT_PTR(m_currChunk);

	json_object_set_new( m_root, FileChunkType_To_String(chunkId).ToPtr(), m_currChunk );

	return true;
}

void JsonChunkedFileWriter::Serialize( const void* o, const mxType& typeInfo )
{
	mxASSERT_PTR(m_currChunk);

	json_t* jsonValue = JSON_EncodeObject( o, typeInfo );

	//json_object_set_new( m_currChunk, NODE_TYPE_TAG, json_string(typeInfo.m_name) );
	json_object_set_new( m_currChunk, NODE_DATA_TAG, jsonValue );

	m_currChunk = nil;
}

bool JsonChunkedFileWriter::SaveClump( const Clump& clump )
{
	mxASSERT_PTR(m_currChunk);

	ClumpWriterJson	writer;
	chkRET_FALSE_IF_NOT(writer.SaveClump( clump ));

	json_t* jsonValue = writer.GetRoot();

	//json_object_set_new( m_currChunk, NODE_TYPE_TAG, json_string("Clump") );
	json_object_set_new( m_currChunk, NODE_DATA_TAG, jsonValue );

	m_currChunk = nil;

	return true;
}

/*
-----------------------------------------------------------------------------
	JsonChunkedFileReader
-----------------------------------------------------------------------------
*/
JsonChunkedFileReader::JsonChunkedFileReader( AStreamReader& stream )
{
	m_root = JSON_ParseStream( stream );
	m_currChunk = nil;
}

JsonChunkedFileReader::~JsonChunkedFileReader()
{
	if( m_root != nil )
	{
		json_decref( m_root );
		m_root = nil;
	}
}

bool JsonChunkedFileReader::GoToChunk( FileChunkType chunkId )
{
	chkRET_FALSE_IF_NIL(m_root);

	mxASSERT( m_currChunk == nil );
	m_currChunk = json_object_get( m_root, FileChunkType_To_String(chunkId).ToPtr() );
	chkRET_FALSE_IF_NIL(m_currChunk);

	return true;
}

void JsonChunkedFileReader::Deserialize( void * o, const mxType& typeInfo )
{
	mxASSERT_PTR(m_currChunk);

	//const json_t *	objectType = json_object_get( m_currChunk, NODE_TYPE_TAG );
	//mxASSERT(mxStrEquAnsi( json_string_value(objectType), typeInfo.m_name ));

	const json_t *	objectValue = json_object_get( m_currChunk, NODE_DATA_TAG );
	JSON_DecodeObject( objectValue, typeInfo, o );

	m_currChunk = nil;
}

bool JsonChunkedFileReader::LoadClump( Clump &clump )
{
	mxASSERT_PTR(m_currChunk);

	//json_t *	clumpType = json_object_get( m_currChunk, NODE_TYPE_TAG );
	//mxASSERT(mxStrEquAnsi( json_string_value(clumpType), "Clump" ));

	json_t *	clumpValue = json_object_get( m_currChunk, NODE_DATA_TAG );
	mxASSERT_PTR(clumpValue);

	ClumpLoaderJson	reader( clumpValue );
	chkRET_FALSE_IF_NOT(reader.LoadClump( clump ));

	m_currChunk = nil;
	return true;
}
#endif

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
