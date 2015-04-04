/*
=============================================================================
	File:	ManualSerialization.h
	Desc:	Manual serialization framework, main header file.
	ToDo:	refactor chunked file serializers
=============================================================================
*/
#pragma once

//#include <Base/Template/Containers/HashMap/THashMap.h>
#include <Base/Object/TypeDescriptor.h>
#include <Base/Object/ClassDescriptor.h>
#include <Base/Object/BaseType.h>

class Clump;

//!=- BUILD CONFIG -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
#define MX_DEBUG_SERIALIZATION	(MX_DEBUG)

/*
=======================================================================

	Manual binary serialization.

=======================================================================
*/

/*
-----------------------------------------------------------------------------
	DataSaver

	Template-based binary serializer for saving POD types.
-----------------------------------------------------------------------------
*/
struct DataWriter
{
	typedef DataWriter THIS_TYPE;

	mxFORCEINLINE bool IsReading() const { return false; }
	mxFORCEINLINE bool IsWriting() const { return !IsReading(); }

	DataWriter( AStreamWriter& writer )
		: stream( writer )
	{
	}

	template< UINT SIZE >
	THIS_TYPE& operator & ( TLocalString<SIZE> & o )
	{
		stream << o;
		return *this;
	}

	THIS_TYPE& operator & ( String & o )
	{
		stream << o;
		return *this;
	}

	template< typename TYPE >
	THIS_TYPE& operator & ( TArray<TYPE> & o )
	{
		const UINT32 num = o.Num();
		const size_t dataSize = o.GetDataSize();

		stream << num;

		if( TypeTrait< TYPE >::IsPlainOldDataType )
		{
			if( dataSize > 0 )
			{
				stream.Write( o.ToPtr(), dataSize );
			}
		}
		else
		{
			for( UINT i = 0; i < num; i++ )
			{
				*this & o[i];
			}
		}

		return *this;
	}

public:

	template< typename TYPE >
	inline THIS_TYPE & SerializeChunk( const TYPE& value )
	{
		stream.Write( &value, sizeof(TYPE) );
		return *this;
	}

	inline THIS_TYPE & SerializeMemory( const void* ptr, const size_t numBytes )
	{
		stream.Write( ptr, numBytes );
		return *this;
	}

	template< typename TYPE >
	inline THIS_TYPE & SerializeViaStream( const TYPE& value )
	{
		stream << value;
		return *this;
	}

	inline AStreamWriter& GetStream() { return stream; }

	template< typename TYPE >
	inline void RelocatePointer( TYPE*& ptr )
	{
		mxUNREACHABLE;
	}

protected:
	AStreamWriter &	stream;
};

/*
-----------------------------------------------------------------------------
	DataLoader

	Template-based binary deserializer of POD objects.
-----------------------------------------------------------------------------
*/
struct DataReader
{
	typedef DataReader THIS_TYPE;

	mxFORCEINLINE bool IsReading() const { return true; }
	mxFORCEINLINE bool IsWriting() const { return !IsReading(); }


	DataReader( AStreamReader& reader )
		: stream( reader )
	{
	}

	template< UINT SIZE >
	THIS_TYPE& operator & ( TLocalString<SIZE> & o )
	{
		stream >> o;
		return *this;
	}

	THIS_TYPE& operator & ( String & o )
	{
		stream >> o;
		return *this;
	}

	template< typename TYPE >
	THIS_TYPE& operator & ( TArray<TYPE> & o )
	{
		UINT32 num;
		stream >> num;

		o.SetNum( num );

		const size_t dataSize = num * sizeof TYPE;

		if( TypeTrait< TYPE >::IsPlainOldDataType )
		{
			if( dataSize > 0 )
			{
				stream.Read( o.ToPtr(), dataSize );
			}
		}
		else
		{
			for( UINT i = 0; i < num; i++ )
			{
				*this & o[i];
			}
		}

		return *this;
	}


public:

	template< typename TYPE >
	inline THIS_TYPE & SerializeChunk( TYPE &value )
	{
		stream.Read( &value, sizeof(TYPE) );
		return *this;
	}

	inline THIS_TYPE & SerializeMemory( void *ptr, const size_t numBytes )
	{
		stream.Read( ptr, numBytes );
		return *this;
	}

	template< typename TYPE >
	inline THIS_TYPE & SerializeViaStream( TYPE &value )
	{
		stream >> value;
		return *this;
	}

	inline AStreamReader& GetStream() { return stream; }

	template< typename TYPE >
	inline void RelocatePointer( TYPE*& ptr )
	{
		mxUNREACHABLE;
	}

protected:
	AStreamReader &	stream;
};

// (de-)serializes from/to archive via templated data serializers
//
template< typename T >
inline mxArchive& Serialize_ArcViaBin( mxArchive& archive, T & o )
{
	if( AStreamWriter* stream = archive.IsWriter() )
	{
		DataWriter	writer( *stream );
		writer & o;
		return archive;
	}
	if( AStreamReader* stream = archive.IsReader() )
	{
		DataReader	reader( *stream );
		reader & o;
		return archive;
	}
	return archive;
}

/*
-----------------------------------------------------------------------------
	ArchivePODWriter
-----------------------------------------------------------------------------
*/
struct ArchivePODWriter : public mxArchive
{
public:
	ArchivePODWriter( AStreamWriter& stream );
	~ArchivePODWriter();

	virtual void SerializeMemory( void* ptr, size_t size ) override;

	inline AStreamWriter& GetStream() {return m_stream;}
	virtual AStreamWriter* IsWriter() override {return &m_stream;}

protected:
	AStreamWriter &	m_stream;
};

/*
-----------------------------------------------------------------------------
	ArchivePODReader
-----------------------------------------------------------------------------
*/
struct ArchivePODReader : public mxArchive
{
public:
	ArchivePODReader( AStreamReader& stream );
	~ArchivePODReader();

	virtual void SerializeMemory( void* ptr, size_t size ) override;

	inline AStreamReader& GetStream() {return m_stream;}
	virtual AStreamReader* IsReader() override {return &m_stream;}

protected:
	AStreamReader &	m_stream;
};

/*
=======================================================================

	Binary serialization of non-POD types.

	NOTE: only classes derived from AObject are supported.

	If you have object references in your graph,
	the objects will only be serialized
	on the first time that they are encountered,
	therefore the same relationships will return from serialization
	as existed when the object was persisted.
	The serializer keeps a note of the objects that it is persisting
	and stores references to previously seen items,
	so everything is the same when you rehydrate the graph.

=======================================================================
*/

// unique object id (used for serialization)
// NOTE: zero object id is reserved for null pointers
typedef UINT32 ObjectUid;

/*
-----------------------------------------------------------------------------
	ArchiveWriter
-----------------------------------------------------------------------------
*/
class ArchiveWriter : public ArchivePODWriter
{
public:
	typedef ArchivePODWriter Super;

	ArchiveWriter( AStreamWriter& stream );
	~ArchiveWriter();

	virtual void SerializePointer( AObject *& o ) override;
	virtual void InsertRootObject( AObject* o ) override;

public:
	ObjectUid InsertObject( AObject* o );

private:
	// so that objects get serialized only once
	THashMap< const AObject*, ObjectUid >	m_registered;
};


/*
-----------------------------------------------------------------------------
	ArchiveReader
-----------------------------------------------------------------------------
*/
class ArchiveReader : public ArchivePODReader
{
public:
	typedef ArchivePODReader Super;

	ArchiveReader( AStreamReader& stream );
	~ArchiveReader();

	virtual void SerializePointer( AObject *& o ) override;
	virtual void InsertRootObject( AObject* o ) override;

public:
	ObjectUid InsertObject( AObject* o );

private:
	// so that objects get deserialized only once
	THashMap< ObjectUid, AObject* >	m_loaded;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// parameters passed to the object after its deserialization
//struct SPostLoadContext
//{
//};

/*
-----------------------------------------------------------------------------
	SObjectPtr

	wrapper around pointer to a polymorphic object
-----------------------------------------------------------------------------
*/
struct SObjectPtr : public CStruct
{
	TPtr< AObject >		o;

public:
	mxDECLARE_CLASS( SObjectPtr, CStruct );
	mxDECLARE_REFLECTION;
};

/*
-----------------------------------------------------------------------------
	AObjectWriter
-----------------------------------------------------------------------------
*/
class AObjectWriter
{
public:
	template< typename CLASS >
	void SaveObject( const CLASS& o )
	{
		this->Serialize( &o, T_DeduceTypeInfo< CLASS >() );
	}

	virtual void Serialize( const void* o, const mxType& typeInfo ) = 0;

	virtual ~AObjectWriter() {}
};

/*
-----------------------------------------------------------------------------
	AObjectReader

	object loader
-----------------------------------------------------------------------------
*/
class AObjectReader
{
public:
	template< typename CLASS >
	void LoadObject( CLASS & o )
	{
		this->Deserialize( &o, T_DeduceTypeInfo< CLASS >() );
	}

	// NOTE: assumes that the memory for the object is already allocated
	virtual void Deserialize( void * o, const mxType& typeInfo ) = 0;

	virtual ~AObjectReader() {}
};

/*
-----------------------------------------------------------------------------
	ASerializer
	abstract high-level serializer
-----------------------------------------------------------------------------
*/
class ASerializer
{
public:
	// serialize a pointer to an object
	//virtual void ProcessPointer( CStruct *& o, const mxType& typeInfo ) = 0;

protected:
	virtual ~ASerializer() {}
};

/*
-----------------------------------------------------------------------------
	ATextSerializer
-----------------------------------------------------------------------------
*/
class ATextSerializer : public ASerializer
{
public:
	virtual void Enter_Scope( const char* name ) = 0;
	virtual void Leave_Scope() = 0;

	virtual bool Serialize_Bool( const char* name, bool & rValue ) = 0;
	virtual bool Serialize_Int8( const char* name, INT8 & rValue ) = 0;
	virtual bool Serialize_Uint8( const char* name, UINT8 & rValue ) = 0;
	virtual bool Serialize_Int16( const char* name, INT16 & rValue ) = 0;
	virtual bool Serialize_Uint16( const char* name, UINT16 & rValue ) = 0;
	virtual bool Serialize_Int32( const char* name, INT32 & rValue ) = 0;
	virtual bool Serialize_Uint32( const char* name, UINT32 & rValue ) = 0;
	virtual bool Serialize_Int64( const char* name, INT64 & rValue ) = 0;
	virtual bool Serialize_Uint64( const char* name, UINT64 & rValue ) = 0;
	virtual bool Serialize_Float32( const char* name, FLOAT & rValue ) = 0;
	virtual bool Serialize_Float64( const char* name, DOUBLE & rValue ) = 0;
	//virtual void Serialize_Buffer( void* pBuffer, size elementSize, size count ) = 0;
	virtual bool Serialize_String( const char* name, String & rValue ) = 0;
	virtual bool Serialize_StringList( const char* name, StringListT & rValue ) = 0;

	virtual bool IsLoading() const { return false; }
	virtual bool IsStoring() const { return false; }

protected:
	virtual ~ATextSerializer() {}
};

class TextSerializationScope
{
	ATextSerializer &	m_serializer;
public:
	TextSerializationScope( ATextSerializer& serializer, const char* scopeName )
		: m_serializer( serializer )
	{
		m_serializer.Enter_Scope( scopeName );
	}
	~TextSerializationScope()
	{
		m_serializer.Leave_Scope();
	}
};

/*
-----------------------------------------------------------------------------
	AClumpWriter

	clump serializer
-----------------------------------------------------------------------------
*/
class AClumpWriter
{
public:
	virtual bool SaveClump( const Clump& clump ) = 0;

protected:
	virtual ~AClumpWriter() {}
};

/*
-----------------------------------------------------------------------------
	AClumpLoader

	clump loader
-----------------------------------------------------------------------------
*/
class AClumpLoader
{
public:
	virtual bool LoadClump( Clump &clump ) = 0;

protected:
	virtual ~AClumpLoader() {}
};

/*
=======================================================================

	Chunked file support.

	File is composed of one or several chunks,
	each chunk has an associated tag for identifying its contents.

	See:
	http://en.wikipedia.org/wiki/File_format#Chunk-based_formats
	http://en.wikipedia.org/wiki/Interchange_File_Format
=======================================================================
*/

typedef UINT32 FileChunkHandle;

static const FileChunkHandle NULL_CHUNK_HANDLE = ~0UL;

// FileChunkType is a tag describing the type of a chunk.
// It is constructed as a sequence of four 8-bit characters
// giving them a descriptive ID if viewed in a hex editor (or Notepad). 
// 4 bytes: an ASCII identifier for this chunk
// (examples are "fmt " and "data"; note the space in "fmt ").
// See: http://en.wikipedia.org/wiki/FourCC
//
typedef UINT32 FileChunkType;

static const FileChunkType HEADER_CHUNK_TYPE = 'HEAD';
static const FileChunkType UNKNOWN_CHUNK_TYPE = 'UNKN';
static const FileChunkType LEVEL_CHUNK_TYPE = 'GAME';
static const FileChunkType EDITOR_CHUNK_TYPE = 'EDIT';


/*
-----------------------------------------------------------------------------
	AChunkWriter
-----------------------------------------------------------------------------
*/
class AChunkWriter : public AObjectWriter, public AClumpWriter
{
public:
	virtual bool BeginChunk( FileChunkType chunkId ) = 0;

protected:
	virtual ~AChunkWriter() {}
};

/*
-----------------------------------------------------------------------------
	AChunkReader
-----------------------------------------------------------------------------
*/
class AChunkReader : public AObjectReader, public AClumpLoader
{
public:
	// functions to locate the chunk withing the stream
	virtual bool GoToChunk( FileChunkType chunkId ) = 0;

protected:
	virtual ~AChunkReader() {}
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
