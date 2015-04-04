/*
=============================================================================
	File:	JsonSupport.h
	Desc:	JSON (de)serializers
=============================================================================
*/
#pragma once

#include <Base/Object/ManualSerialization.h>
#include <Base/Template/Containers/HashMap/TPointerMap.h>
#include <Core/ObjectModel.h>

#if MX_AUTOLINK
#pragma comment( lib, "JsonSupport.lib" )
#endif //MX_AUTOLINK

// we use Jansson
struct json_t;

namespace JSON
{
	ERet WriteToStream( const void* o, const mxType& type, AStreamWriter &stream );
	ERet LoadFromStream( AStreamReader& stream, void *o, const mxType& type, const char* name = "", int line = 1 );

	//@todo: error checking
	template< typename TYPE >
	ERet SaveObject( const TYPE& o, AStreamWriter& stream )
	{
		JsonObjectWriter	serializer;
		serializer.SaveObject( o );
		chkRET_X_IF_NOT(serializer.DumpToStream( stream ), ERR_UNKNOWN_ERROR);
		return ALL_OK;
	}

	template< typename TYPE >
	ERet LoadObject( AStreamReader& stream, TYPE &o )
	{
		JsonObjectReader	deserializer( stream );
		deserializer.LoadObject( o );
		return ALL_OK;
	}

	template< typename TYPE >
	ERet SaveObjectToFile( const TYPE& o, const char* file )
	{
		FileWriter	stream;
		mxDO(stream.Open(file, FileWrite_NoErrors));
		mxDO(WriteToStream( &o, mxTYPE_OF(o), stream ));
		return ALL_OK;
	}

	template< typename TYPE >
	ERet LoadObjectFromFile( const char* file, TYPE &o )
	{
		FileReader	stream;
		mxDO(stream.Open(file, FileRead_NoErrors));
		mxDO(LoadFromStream( stream, &o, mxTYPE_OF(o), file ));
		return ALL_OK;
	}

	ERet SaveClumpToFile( const Clump& clump, const char* file );

	//NOTE: doesn't clear the given clump first!
	ERet LoadClumpFromFile( const char* file, Clump & clump );

	// automatically decrements reference count upon exit
	struct Scope {
		json_t *	root;
	public:
		Scope();
		~Scope();
	};

}//namespace JSON


/*
-----------------------------------------------------------------------------
	JsonWriter
-----------------------------------------------------------------------------
*/
class JsonWriter : public ATextSerializer
{
public:
	JsonWriter();
	~JsonWriter();

	// writes serialized data in text form to stream
	bool DumpToStream( AStreamWriter& stream ) const;

	//=-- ATextSerializer

	virtual void Enter_Scope( const char* name ) override;
	virtual void Leave_Scope() override;

	virtual bool Serialize_Bool( const char* name, bool & rValue ) override;
	virtual bool Serialize_Int8( const char* name, INT8 & rValue ) override;
	virtual bool Serialize_Uint8( const char* name, UINT8 & rValue ) override;
	virtual bool Serialize_Int16( const char* name, INT16 & rValue ) override;
	virtual bool Serialize_Uint16( const char* name, UINT16 & rValue ) override;
	virtual bool Serialize_Int32( const char* name, INT32 & rValue ) override;
	virtual bool Serialize_Uint32( const char* name, UINT32 & rValue ) override;
	virtual bool Serialize_Int64( const char* name, INT64 & rValue ) override;
	virtual bool Serialize_Uint64( const char* name, UINT64 & rValue ) override;
	virtual bool Serialize_Float32( const char* name, FLOAT & rValue ) override;
	virtual bool Serialize_Float64( const char* name, DOUBLE & rValue ) override;
	virtual bool Serialize_String( const char* name, String & rValue ) override;
	virtual bool Serialize_StringList( const char* name, StringListT & rValue ) override;

	virtual bool IsLoading() const { return false; }
	virtual bool IsStoring() const { return true; }

private:
	struct Scope
	{
		struct json_t *	node;
		String			name;	// scope name
	};

private:
	struct json_t *		m_root;
	struct json_t *		m_curr;
	TArray< Scope >		m_scopeStack;
};

/*
-----------------------------------------------------------------------------
	JsonReader
-----------------------------------------------------------------------------
*/
class JsonReader : public ATextSerializer
{
public:
	JsonReader( AStreamReader& stream );
	~JsonReader();

	//=-- ATextSerializer

	virtual void Enter_Scope( const char* name ) override;
	virtual void Leave_Scope() override;

	virtual bool Serialize_Bool( const char* name, bool & rValue ) override;
	virtual bool Serialize_Int8( const char* name, INT8 & rValue ) override;
	virtual bool Serialize_Uint8( const char* name, UINT8 & rValue ) override;
	virtual bool Serialize_Int16( const char* name, INT16 & rValue ) override;
	virtual bool Serialize_Uint16( const char* name, UINT16 & rValue ) override;
	virtual bool Serialize_Int32( const char* name, INT32 & rValue ) override;
	virtual bool Serialize_Uint32( const char* name, UINT32 & rValue ) override;
	virtual bool Serialize_Int64( const char* name, INT64 & rValue ) override;
	virtual bool Serialize_Uint64( const char* name, UINT64 & rValue ) override;
	virtual bool Serialize_Float32( const char* name, FLOAT & rValue ) override;
	virtual bool Serialize_Float64( const char* name, DOUBLE & rValue ) override;
	virtual bool Serialize_String( const char* name, String & rValue ) override;
	virtual bool Serialize_StringList( const char* name, StringListT & rValue ) override;

	virtual bool IsLoading() const { return true; }
	virtual bool IsStoring() const { return false; }

private:
	struct Scope
	{
		struct json_t *	node;
		String			name;	// scope name
	};

protected:
	struct json_t *		m_root;
	struct json_t *		m_curr;
	TArray< Scope >		m_scopeStack;
};

#if 0
/*
-----------------------------------------------------------------------------
	JsonObjectWriter
-----------------------------------------------------------------------------
*/
class JsonObjectWriter : public AObjectWriter
{
	struct json_t *		m_root;

public:
	JsonObjectWriter();
	~JsonObjectWriter();

	// writes serialized data in text form to stream
	bool DumpToStream( AStreamWriter& stream ) const;

	//=-- AObjectWriter

	virtual void Serialize( const void* o, const mxType& typeInfo ) override;

	json_t* GetRoot() { return m_root; }

	bool IsEmpty() const { return m_root == nil; }
};

/*
-----------------------------------------------------------------------------
	JsonObjectReader
-----------------------------------------------------------------------------
*/
class JsonObjectReader : public AObjectReader
{
	json_t *		m_root;

public:
	// need asset database to perform conversions between string and integer resource ids
	JsonObjectReader( AStreamReader& stream );
	~JsonObjectReader();

	//=-- AObjectReader
	virtual void Deserialize( void * o, const mxType& typeInfo ) override;
};

/*
-----------------------------------------------------------------------------
	JsonChunkedFileWriter
-----------------------------------------------------------------------------
*/
class JsonChunkedFileWriter : public AChunkWriter
{
	json_t *		m_root;
	json_t *		m_currChunk;

public:
	JsonChunkedFileWriter();
	~JsonChunkedFileWriter();

	// writes serialized data in text form to stream
	bool DumpToStream( AStreamWriter& stream ) const;

	//-- AChunkWriter
	virtual bool BeginChunk( FileChunkType chunkId ) override;

	//-- AObjectWriter
	virtual void Serialize( const void* o, const mxType& typeInfo ) override;

	//-- AClumpWriter
	virtual bool SaveClump( const Clump& clump ) override;

	const json_t* GetRoot() const { return m_root; }
};

/*
-----------------------------------------------------------------------------
	JsonChunkedFileReader
-----------------------------------------------------------------------------
*/
class JsonChunkedFileReader : public AChunkReader
{
	json_t *		m_root;
	const json_t *	m_currChunk;

public:
	// need asset database to perform conversions between string and integer resource ids
	JsonChunkedFileReader( AStreamReader& stream );
	~JsonChunkedFileReader();

	//-- AChunkWriter
	virtual bool GoToChunk( FileChunkType chunkId ) override;

	//-- AObjectReader
	virtual void Deserialize( void * o, const mxType& typeInfo ) override;

	//-- AClumpLoader
	virtual bool LoadClump( Clump &clump ) override;

	const json_t* GetRoot() const { return m_root; }

	bool IsValid() const { return m_root != nil; }
};
#endif

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
