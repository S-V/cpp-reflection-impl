#pragma once

#include <Utility/TxTSupport/TxTCommon.h>

class Clump;

namespace SON
{
	struct SaveOptions {
		bool	wrapRootInBraces;
	public:
		SaveOptions() {
			wrapRootInBraces = false;
		}
	};

	// NOTE: only objects without pointers can be (de)serialized.

	ERet LoadFromBuffer(
		char* _text, int _size,
		void *_o, const mxType& _type,
		const char* _file = "", int _line = 1
	);

	ERet LoadFromStream(
		AStreamReader& _stream,
		void *_o, const mxType& _type,
		const char* _file = "", int _line = 1
	);

	template< typename TYPE >
	ERet Load( AStreamReader& _stream, TYPE &_o )
	{
		return LoadFromStream( _stream, &_o, mxTYPE_OF(_o) );
	}

	template< typename TYPE >
	ERet LoadFromFile( const char* _file, TYPE &_o )
	{
		FileReader	stream;
		mxTRY(stream.Open( _file, FileRead_NoErrors ));
		mxTRY(LoadFromStream( stream, &_o, mxTYPE_OF(_o), _file ));
		return ALL_OK;
	}

	ERet Decode( const Node* _root, const mxType& _type, void *_o );

	template< typename TYPE >
	ERet Decode( const Node* _root, TYPE &_o )
	{
		return Decode( _root, mxTYPE_OF(_o), &_o );
	}

	Node* Encode( const void* _o, const mxType& _type, Allocator & _allocator );

	template< typename TYPE >
	Node* Encode( const TYPE& _o, Allocator & _allocator )
	{
		return Encode( &_o, mxTYPE_OF(_o), _allocator );
	}

	ERet SaveToStream(
		const void* _o, const mxType& _type,
		AStreamWriter &_stream
	);

	template< typename TYPE >
	ERet Save( const TYPE& _o, AStreamWriter &_stream )
	{
		return SaveToStream( &_o, mxTYPE_OF(_o), _stream );
	}

	template< typename TYPE >
	ERet SaveToFile( const TYPE& _o, const char* _file )
	{
		FileWriter	stream;
		mxTRY(stream.Open( _file, FileWrite_NoErrors ));
		mxTRY(SaveToStream( &_o, mxTYPE_OF(_o), stream ));
		return ALL_OK;
	}

	ERet SaveClump(
		const Clump& _clump,
		AStreamWriter &_stream,
		const SaveOptions& _options = SaveOptions()
	);

	ERet LoadClump(
		AStreamReader &_stream, Clump &_clump,
		const char* _file = "", int _line = 1
	);

	ERet LoadClump( const SON::Node* root, Clump &_clump );

	ERet SaveClumpToFile( const Clump& _clump, const char* _file );
	ERet LoadClumpFromFile( const char* _file, Clump &_clump );

}//namespace SON

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
