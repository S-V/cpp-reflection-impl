/*
=============================================================================
	File:	Serialization.h
	Desc:	Binary serialization of arbitrary types via reflection.
=============================================================================
*/
#pragma once

#include <Core/ObjectModel.h>
#include <Core/Asset.h>

namespace Serialization
{

#pragma pack (push,1)
	struct ImageHeader
	{
		PtSessionInfo	session;	// 8 platform/engine info
		TypeID			classId;	// 4 type of stored object
		UINT32			payload;	// 4 size of stored data
	};
	ASSERT_SIZEOF(ImageHeader, 16);

	struct BinaryHeader
	{
		PtSessionInfo	session;	// 8 platform/engine info
		TypeID			classId;	// 4 type of stored object
		UINT32			_unused;	// 4 padding to align data to 16 bytes
	};
	ASSERT_SIZEOF(BinaryHeader, 16);
#pragma pack (pop)

	//
	// Memory image dump based on reflection metadata:
	// serializes into native memory layout for in-place loading (LIP).
	// NOTE: only POD types are supported (constructors are not called).
	// NOTE: pointers to external memory blocks are not supported!
	//

	ERet SaveImage( const void* o, const mxClass& type, AStreamWriter& stream );
	ERet SaveImage( const Clump& clump, AStreamWriter& stream );

	ERet LoadImage( AStreamReader& stream, const mxClass& type, ByteArrayT &buffer );

	mxDEPRECATED
	// assumes that the buffer starts with an ImageHeader
	ERet LoadInPlace( const mxClass& type, void* buffer, UINT32 length, void *&o );

	// parses the stream and loads the object data into the user-supplied buffer
	ERet LoadInPlace(
		const mxClass& type, const ImageHeader& header,
		void * buffer, UINT32 length,
		AStreamReader& stream
	);

	// assumes that the buffer starts with an ImageHeader
	//NOTE: this function must be thread-safe, it's often called in the background loading thread
	//ERet FixupBufferWithHeader( void* buffer, UINT32 length );

	template< typename CLASS >
	ERet LoadInPlace( void* buffer, UINT32 length, CLASS *&o ) {
		void* voidPtr = NULL;
		mxDO(LoadInPlace( CLASS::MetaClass(), buffer, length, voidPtr ));
		o = static_cast< CLASS* >( voidPtr );
		return ALL_OK;
	}

	template< typename CLASS >
	ERet SaveImage( const CLASS& o, AStreamWriter& stream ) {
		return SaveImage( &o, CLASS::MetaClass(), stream );
	}

	//
	// Automatic binary serialization (via reflection):
	// serializes to a compact binary format.
	// NOTE: only internal pointers to objects (structs or classes) are supported.
	//

	ERet SaveBinary( const void* o, const mxClass& _type, AStreamWriter &stream );
	ERet LoadBinary( AStreamReader& stream, const mxClass& _type, void *o );

	template< typename CLASS >
	ERet SaveBinary( const CLASS& o, AStreamWriter& stream ) {
		return SaveBinary( &o, CLASS::MetaClass(), stream );
	}

	ERet SaveBinaryToFile( const void* o, const mxClass& type, const char* file );
	ERet LoadBinaryFromFile( const char* file, const mxClass& type, void *o );

	ERet SaveClumpImage( const Clump& _clump, AStreamWriter &_stream );
	ERet LoadClumpImage( AStreamReader& _stream, UINT32 _payload, void *_buffer );

	ERet SaveClumpBinary( const Clump& _clump, AStreamWriter &_stream );
	ERet LoadClumpBinary( AStreamReader& _stream, Clump& _clump );

}//namespace Serialization

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
