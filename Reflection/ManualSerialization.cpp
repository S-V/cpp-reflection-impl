/*
=============================================================================
	File:	ManualSerialization.cpp
	Desc:	Manual serialization framework, main source file.
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Object/ManualSerialization.h>


enum { DEFAULT_HASH_TABLE_SIZE = 64 };

/*
-----------------------------------------------------------------------------
	ArchivePODReader
-----------------------------------------------------------------------------
*/
ArchivePODReader::ArchivePODReader( AStreamReader& stream )
	: m_stream( stream )
{
}
//---------------------------------------------------------------------------
ArchivePODReader::~ArchivePODReader()
{
}
//---------------------------------------------------------------------------
void ArchivePODReader::SerializeMemory( void* ptr, size_t size )
{
	m_stream.Read( ptr, size );
}
//---------------------------------------------------------------------------

/*
-----------------------------------------------------------------------------
	ArchiveReader
-----------------------------------------------------------------------------
*/
ArchiveReader::ArchiveReader( AStreamReader& stream )
	: ArchivePODReader( stream )
	, m_loaded( DEFAULT_HASH_TABLE_SIZE )
{
	// Insert a pairing between the null pointer and zero index.
	AObject* nullPointer = nil;
	ObjectUid nullPointerID = 0;

	m_loaded.Set( nullPointerID, nullPointer );

#if 0
	// read header
	mxArchiveHeader	currHeader;

	mxArchiveHeader	readHeader;
	m_stream >> readHeader;

	if( !currHeader.Matches(readHeader) )
	{
		ptERROR("Incompatible archives, version mismatch!\n");
	}
#endif
}
//---------------------------------------------------------------------------
ArchiveReader::~ArchiveReader()
{

}
//---------------------------------------------------------------------------
void ArchiveReader::SerializePointer( AObject *& o )
{
	// read object id
	ObjectUid uniqueId;
	m_stream >> uniqueId;

	AObject** existing = m_loaded.Find( uniqueId );
	if(PtrToBool( existing ))
	{
		//DBGOUT("READ OLD ID : %u\n", uniqueId);

		// this object has already been registered and deserialized
		// NOTE: 'existing' may point to the null pointer.
		o = *existing;
	}
	else
	{
		// this object hasn't yet been deserialized...

		//DBGOUT("READ NEW ID : %u\n", uniqueId);

		// read object header
 
		// Get the factory function for the type of object about to be read.
		const TypeID typeId = ReadTypeID( m_stream );
		mxASSERT(typeId != mxNULL_TYPE_ID);

		//DBGOUT("READ TYPE : %s\n", TypeRegistry::Get().FindClassByGuid(typeId)->GetTypeName());

		const mxClass * typeInfo = TypeRegistry::Get().FindClassByGuid( typeId );
		mxASSERT_PTR(typeInfo);

		// create a valid instance

		o = typeInfo->CreateInstance();
		mxASSERT_PTR(o);
		mxASSERT( typeInfo == &o->rttiGetClass() );

		// and remember it
		m_loaded.Set( uniqueId, o );

		// deserialize the object from stream
		o->Serialize( *this );
	}
}
//---------------------------------------------------------------------------
void ArchiveReader::InsertRootObject( AObject* o )
{
	this->InsertObject( o );
}
//---------------------------------------------------------------------------
ObjectUid ArchiveReader::InsertObject( AObject* o )
{
	mxASSERT_PTR(o);

	const ObjectUid uniqueID = m_loaded.NumEntries();

	m_loaded.Set( uniqueID, o );

	return uniqueID;
}

/*
-----------------------------------------------------------------------------
	ArchivePODWriter
-----------------------------------------------------------------------------
*/
ArchivePODWriter::ArchivePODWriter( AStreamWriter& stream )
	: m_stream( stream )
{
}
//---------------------------------------------------------------------------
ArchivePODWriter::~ArchivePODWriter()
{
}
//---------------------------------------------------------------------------
void ArchivePODWriter::SerializeMemory( void* ptr, size_t size )
{
	m_stream.Write( ptr, size );
}
//---------------------------------------------------------------------------

/*
-----------------------------------------------------------------------------
	ArchiveWriter
-----------------------------------------------------------------------------
*/
ArchiveWriter::ArchiveWriter( AStreamWriter& stream )
	: Super( stream )
	, m_registered( DEFAULT_HASH_TABLE_SIZE )
{
	// Insert a pairing between the null pointer and zero index.
	AObject* nullPointer = nil;
	ObjectUid nullPointerID = 0;

	m_registered.Set( nullPointer, nullPointerID );

#if 0
	// write header
	mxArchiveHeader	header;
	m_stream << header;
#endif
}
//---------------------------------------------------------------------------
ArchiveWriter::~ArchiveWriter()
{

}
//---------------------------------------------------------------------------
void ArchiveWriter::SerializePointer( AObject *& o )
{
	// An object can be serialized only once.
	
	if( ObjectUid* registeredID = m_registered.Find( o ) )
	{
		// this object has already been registered and serialized
		//DBGOUT("WRITE OLD ID : %u\n", *registeredID);

		// write this object's unique id
		m_stream << *registeredID;
	}
	else
	{
		// this object hasn't yet been registered

		// null pointer is already registered
		//mxASSERT_PTR(o);

		// generate a new object id and remember it

		const ObjectUid uniqueID = this->InsertObject( o );

		//DBGOUT("WRITE NEW ID : %u\n", uniqueID);

		// Write the unique identifier for the object to stream.
		// This is used during loading and linking.
		m_stream << uniqueID;

		// Write the type info for factory function lookup during Load.
		WriteTypeID( m_stream, o->rttiGetTypeID() );

		//DBGOUT("WRITE TYPE : %s\n", TypeRegistry::Get().FindClassByGuid(o->rttiGetTypeID())->GetTypeName());

		// write object header
		//o->rttiGetClass().PreSave( m_stream, o );

		// serialize the object to stream
		o->Serialize( *this );
	}

}
//---------------------------------------------------------------------------
void ArchiveWriter::InsertRootObject( AObject* o )
{
	this->InsertObject( o );
}
//---------------------------------------------------------------------------
ObjectUid ArchiveWriter::InsertObject( AObject* o )
{
	mxASSERT_PTR(o);

	const ObjectUid uniqueID = m_registered.NumEntries();

	m_registered.Set( o, uniqueID );

	return uniqueID;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
-----------------------------------------------------------------------------
	SObjectPtr
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS( SObjectPtr );

mxBEGIN_REFLECTION( SObjectPtr )
	mxMEMBER_FIELD( o ),
mxEND_REFLECTION

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
