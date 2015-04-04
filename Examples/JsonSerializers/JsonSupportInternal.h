// JsonSupportCommon.h
// internal header
#pragma once

#include <ExtLibs/jansson/src/jansson.h>

#include <Core/ObjectModel.h>

#include "JsonSupport.h"

//---------------------------------------------------------------------------

extern const char* CHUNK_TAG;
extern const char* HEADER_TAG;

extern const char* NODE_TYPE_TAG;
extern const char* NODE_DATA_TAG;

extern const char* CLASS_NAME_TAG;
extern const char* CLASS_DATA_TAG;
extern const char* BASE_CLASS_TAG;
extern const char* ASSET_GUID_TAG;
extern const char* ASSET_PATH_TAG;
extern const char* ASSET_TYPE_TAG;

extern const char* OBJECT_CLASS_TAG;
extern const char* OBJECT_INDEX_TAG;
extern const char* OBJECT_ARRAY_TAG;
extern const char* OBJECT_ITEMS_TAG;
extern const char* OBJECT_COUNT_TAG;

const int JSON_NULL_POINTER_ID = -1;
const int JSON_FALLBACK_INSTANCE_ID = -2;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

const char* JSON_Type_To_Chars( json_type e );

bool JSON_Get_Integer_Value( const json_t* node, json_int_t &value );
bool JSON_Find_Integer_Value( const json_t* parent, const char* key, json_int_t &value );

template< typename INTEGER >
bool JSON_Read_Integer_Value( const json_t* parent, const char* key, INTEGER &value )
{
	json_int_t  jsonInt;
	mxSTATIC_ASSERT(sizeof(INTEGER) <= sizeof(jsonInt));
	if( !JSON_Find_Integer_Value( parent, key, jsonInt ) ) {
		return false;
	}
	//@todo: range checks
	value = jsonInt;
	return true;
}


bool JSON_Get_Real_Value( const json_t* node, double &value );
bool JSON_Find_Real_Value( const json_t* parent, const char* key, double &value );

template< typename REAL >
bool JSON_Read_Real_Value( const json_t* parent, const char* key, REAL &value )
{
	double  jsonDouble;
	mxSTATIC_ASSERT(sizeof(REAL) <= sizeof(jsonDouble));
	if( !JSON_Find_Real_Value( parent, key, jsonDouble ) ) {
		return false;
	}
	//@todo: range checks
	value = jsonDouble;
	return true;
}


bool JSON_Get_Boolean_Value( const json_t* node, bool &value );
bool JSON_Find_Boolean_Value( const json_t* parent, const char* key, bool &value );


bool JSON_Get_String_Value( const json_t* node, String &value );
bool JSON_Find_String_Value( const json_t* parent, const char* key, String &value );

// file name is used for diagnostics
json_t* JSON_ParseStream( AStreamReader& stream, const char* file = "", int line = 0 );
bool JSON_WriteToStream( const json_t* root, AStreamWriter& stream );


bool JSON_DumpToFile( const json_t* root, const char* file );


//NOTE: pointers are not supported
//NOTE: call json_decref() after using the JSON value
json_t* JSON_EncodeObject( const void* o, const mxType& type );

template< typename TYPE >
json_t* JSON_EncodeObject( const TYPE& o )
{
	return JSON_EncodeObject( &o, T_DeduceTypeInfo< TYPE >() );
}

void JSON_DecodeObject( const json_t* source, const mxType& type, void *o );

template< typename TYPE >
void JSON_DecodeObject( const json_t* source, TYPE &o )
{
	return JSON_DecodeObject( source, T_DeduceTypeInfo< TYPE >(), &o );
}

/*
-----------------------------------------------------------------------------
	JsonEncoder
	this class can be used to encode values to JSON.
	only objects without pointers can be encoded.
-----------------------------------------------------------------------------
*/
class JsonEncoder : public Reflection::AVisitor
{
	json_t *	m_root;

public:
	typedef Reflection::AVisitor Super;

	JsonEncoder();
	//NOTE: doesn't drop references to JSON nodes
	~JsonEncoder();

	json_t* EncodeObject( const void* o, const mxType& type );

	//-- Reflection::AVisitor

	virtual void* Visit_POD( void * o, const mxType& type, void* _userData ) override;
	virtual void* Visit_String( String & s, void* _userData ) override;
	virtual void* Visit_TypeId( SClassId * o, void* _userData ) override;

	virtual void* Visit_Pointer( VoidPointer& p, const mxPointerType& type, void* _userData ) override;
	virtual void* Visit_UserPointer( void * o, const mxUserPointerType& type, void* _userData ) override;
	virtual void* Visit_AssetId( AssetID & o, void* _userData ) override;

	virtual void* Visit_Aggregate( void * o, const mxClass& type, void* _userData ) override;

	virtual void* Visit_Field( void * o, const mxField& field, void* _userData ) override;

	virtual void* Visit_Array( void * arrayObject, const mxArrayType& arrayType, void* _userData ) override;
	virtual void* Visit_Blob( void * blobObject, const mxBlobType& type, void* _userData ) override;

public:
	// this structure is used for returning values from functions
	struct JSON_Result
	{
		json_t *	parent;	//[input] parent value - non-null if in aggregate scope
		json_t *	value;	//[output] child value

	public:
		JSON_Result()
			: parent( nil ), value( nil )
		{}
	};
};

/*
-----------------------------------------------------------------------------
	JsonDecoder

	only objects without pointers are supported.
-----------------------------------------------------------------------------
*/
class JsonDecoder : public Reflection::AVisitor
{
public:
	typedef Reflection::AVisitor Super;

	JsonDecoder();
	~JsonDecoder();

	void DecodeObject( const json_t* source, const mxType& type, void *o );

	//-- Reflection::AVisitor

	virtual void* Visit_POD( void * o, const mxType& type, void* _userData ) override;

	virtual void* Visit_Aggregate( void * o, const mxClass& type, void* _userData ) override;

	virtual void* Visit_Field( void * o, const mxField& field, void* _userData ) override;

	virtual void* Visit_Pointer( VoidPointer& p, const mxPointerType& type, void* _userData ) override;
	virtual void* Visit_UserPointer( void * o, const mxUserPointerType& type, void* _userData ) override;

	virtual void* Visit_AssetId( AssetID & o, void* _userData ) override;

	virtual void* Visit_TypeId( SClassId * o, void* _userData ) override;

	virtual void* Visit_Array( void * arrayObject, const mxArrayType& arrayType, void* _userData ) override;
	virtual void* Visit_Blob( void * blobObject, const mxBlobType& type, void* _userData ) override;

	virtual void* Visit_String( String & s, void* _userData ) override;
};

json_t* AssetId_To_JSON_String( const AssetID& assetId );
AssetID JSON_String_To_AssetId( const json_t* jsonValue );


//PointerInfo
// used for mapping pointer to numerical id during serialization
struct ObjectInfo
{
	void *			o;
	const mxClass*	type;
	UINT			index;
};

typedef TPointerMap< ObjectInfo, mxPointerHasher >	ObjectMap;

typedef TArray< const ObjectList* >					ObjectListsArray;
typedef THashMap< const mxClass*, ObjectListsArray >	ObjectListsByType;
typedef TArray< AObject* >							PolymorphicObjects;

struct ClumpInfo
{
	ObjectMap	objects;	// stored in Clump's Object lists
};

void CollectObjects( const Clump& clump, ClumpInfo &outInfo );

/*
-----------------------------------------------------------------------------
	ClumpWriterJson
-----------------------------------------------------------------------------
*/
class ClumpWriterJson : public AClumpWriter
{
	struct json_t *		m_root;
	struct json_t *		m_curr;

public:
	// need asset database to perform conversions between string and integer resource ids
	ClumpWriterJson();
	~ClumpWriterJson();

	// writes serialized data in text form to stream
	bool DumpToStream( AStreamWriter& stream ) const;

	//--- AClumpWriter
	virtual bool SaveClump( const Clump& clump ) override;

	json_t* GetRoot() { return m_root; }
};

/*
-----------------------------------------------------------------------------
	ClumpLoaderJson
-----------------------------------------------------------------------------
*/
class ClumpLoaderJson : public AClumpLoader
{
	 json_t *		m_root;

public:
	ClumpLoaderJson( json_t* root );
	ClumpLoaderJson( AStreamReader& stream );
	~ClumpLoaderJson();

	//--- AClumpLoader

	//NOTE: doesn't call Clump::LoadAssets()
	virtual bool LoadClump( Clump &clump ) override;

	const json_t* GetRoot() const { return m_root; }
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
