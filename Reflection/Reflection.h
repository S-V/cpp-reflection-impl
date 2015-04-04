/*
=============================================================================
	File:	Reflection.h
	Desc:	Contains code for defining structure layouts
			and inspecting them at run-time.

			Reflection - having access at runtime
			to information about the C++ classes in your program.
			Reflection is used mainly for serialization.

	Note:	This has been written in haste and is by no means complete.

	Note:	base classes are assumed to start at offset 0 (see mxFIELD_SUPER)

	ToDo:	don't leave string names in release,
			they should only be used in editor mode
=============================================================================
*/
#pragma once

#include <Base/Object/TypeDescriptor.h>

// this is defined in the core engine module
struct AssetID;

enum EFieldFlags
{
	// The field won't be initialized with default values (e.g. fallback resources).
	Field_NoDefaultInit = BIT(0),

	// (mainly for text-based formats such as JSON/SON/XML)
	Field_NoSerialize	= BIT(1),

	// Potential field flags, see:
	// http://www.altdevblogaday.com/2012/01/03/reflection-in-c-part-2-the-simple-implementation-of-splinter-cell/
	/*
	// Is this a transient field, ignored during serialisation?
	F_Transient = 0x02,

	// Is this a network transient field, ignored during network serialisation?
	// A good example for this use-case is a texture type which contains a description
	// and its data. For disk serialisation you want to save everything, for network
	// serialisation you don't really want to send over all the texture data.
	F_NetworkTransient = 0x04,

	// Can this field be edited by tools?
	F_ReadOnly = 0x08,

	// Is this a simple type that can be serialised in terms of a memcpy?
	// Examples include int, float, any vector types or larger types that you're not
	// worried about versioning.
	F_SimpleType = 0x10,

	// Set if the field owns the memory it points to.
	// Any loading code must allocate it before populating it with data.
	F_OwningPointer = 0x20
	*/

	//REMOVE THIS:
	// Create a save file dialog for editing this field
	// (Only for string fields)
	//Field_EditAsSaveFile	= BIT(1),

	Field_DefaultFlags = 0,
};

typedef UINT32 FieldFlags;

/*
-----------------------------------------------------------------------------
	mxField

	structure field definition
	,used for reflecting contained, nested objects

	@todo: don't leave name string in release exe
	@todo: each one of offset/flags could fit in 16 bits
-----------------------------------------------------------------------------
*/
struct mxField
{
	const mxType &		type;	// type of the field
	const char *		name;	// name of the variable in the code
	const MetaOffset	offset;	// byte offset in the structure (relative to the immediately enclosing type)
	FieldFlags			flags;	// combination of EFieldFlags::Field_* bits
};

/*
-----------------------------------------------------------------------------
	Metadata

	reflection metadata for describing the contents of a structure
-----------------------------------------------------------------------------
*/
struct mxClassLayout
{
	const mxField *	fields;		// array of fields in the structure
	const UINT		numFields;	// number of fields in the structure

	// time stamp for version tracking
	// metadata is implemented in source files ->
	// time stamp changes when file is recompiled
	//STimeStamp	timeStamp;
public:
	static mxClassLayout	dummy;	// empty, null instance
};

//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//	mxNO_REFLECTED_MEMBERS - must be included in the declaration of a class.
//---------------------------------------------------------------------------
//
#define mxNO_REFLECTED_MEMBERS\
	public:\
		static mxClassLayout& StaticLayout() { return mxClassLayout::dummy; }\


//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//	mxDECLARE_REFLECTION - must be included in the declaration of a class.
//---------------------------------------------------------------------------
//
#define mxDECLARE_REFLECTION\
	public:\
		static mxClassLayout& StaticLayout();\


//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//	mxBEGIN_REFLECTION( CLASS ) - should be placed in source file.
//---------------------------------------------------------------------------
//
#define mxBEGIN_REFLECTION( CLASS )\
	mxClassLayout& CLASS::StaticLayout() {\
		typedef CLASS OuterType;\
		static mxField fields[] = {\

//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
#define mxMEMBER_FIELD( VAR )\
	{\
		T_DeduceTypeInfo( ((OuterType*)0)->VAR ),\
		mxEXTRACT_NAME( VAR ).buffer,\
		OFFSET_OF( OuterType, VAR ),\
		Field_DefaultFlags\
	}

//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// allows the programmer to specify her own type
// 'TYPE' must be (const mxType&)
//
#define mxMEMBER_FIELD_OF_TYPE( VAR, TYPE )\
	{\
		TYPE,\
		mxEXTRACT_NAME( VAR ).buffer,\
		OFFSET_OF( OuterType, VAR ),\
		Field_DefaultFlags\
	}

//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
#if MX_DEVELOPER

	// allow empty reflection metadata descriptions in debug mode
	#define mxEND_REFLECTION\
				{ T_DeduceTypeInfo< INT >(), nil, 0, 0 }\
			};\
			static mxClassLayout reflectedMembers = { fields, mxCOUNT_OF(fields)-1 };\
			return reflectedMembers;\
		}

#else

	// saves a little bit of memory (size of pointer) in release mode
	#define mxEND_REFLECTION\
			};\
			static mxClassLayout reflectionMetadata = { fields, mxCOUNT_OF(fields) };\
			return reflectionMetadata;\
		}

#endif

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// A visitor interface for serializable fields defined in a class

namespace Reflection
{

mxREFACTOR("bogus names, inconvenient function signatures");

/*
-----------------------------------------------------------------------------
	AVisitor
	low-level interface for iterating over object fields via reflection

	NOTE: its functions accept some user-defined data and return user data.
-----------------------------------------------------------------------------
*/
mxDEPRECATED
class AVisitor
{
	friend class Walker;

protected:
	// this function is called to visit a class member field
	// (e.g. this is used by buffer/JSON/XML serializers).
	// it returns a pointer to some user-defined data.
	// NOTE: don't forget to call the parent's function if needed
	virtual void* Visit_Field( void * _o, const mxField& _field, void* _userData );

	// structures
	// NOTE: don't forget to call the parent's function if needed
	virtual void* Visit_Aggregate( void * _o, const mxClass& _type, void* _userData );

	// arrays
	// NOTE: don't forget to call the parent's function if needed
	virtual void* Visit_Array( void * _array, const mxArray& _type, void* _userData );
	virtual void* Visit_Blob( void * _blob, const mxBlobType& _type, void* _userData );


	// built-in types (int, float, bool), enums, bitmasks
	virtual void* Visit_POD( void * _o, const mxType& _type, void* _userData )
	{
		return _userData;
	}
	virtual void* Visit_TypeId( SClassId * _o, void* _userData )
	{
		return _userData;
	}
	virtual void* Visit_String( String & _string, void* _userData )
	{
		return _userData;
	}
	virtual void* Visit_AssetId( AssetID & _assetId, void* _userData )
	{
		return _userData;
	}

	// references
	virtual void* Visit_Pointer( VoidPointer& _pointer, const mxPointerType& _type, void* _userData )
	{
		return _userData;
	}
	virtual void* Visit_UserPointer( void * _o, const mxUserPointerType& _type, void* _userData )
	{
		return _userData;
	}

protected:
	virtual ~AVisitor() {}
};

// Object/Field iterator
mxDEPRECATED
struct Walker {
	static void* Visit( void * _object, const mxType& _type, AVisitor* _visitor, void *_userData = nil );
	static void* VisitArray( void * _array, const mxArray& _type, AVisitor* _visitor, void* _userData = nil );
	static void* VisitAggregate( void * _struct, const mxClass& _type, AVisitor* _visitor, void *_userData = nil );
	static void* VisitStructFields( void * _struct, const mxClass& _type, AVisitor* _visitor, void* _userData = nil );
};

class PointerChecker : public Reflection::AVisitor
{
public:
	virtual void* Visit_Array( void * arrayObject, const mxArray& arrayType, void* _userData ) override;
	virtual void* Visit_Pointer( VoidPointer& p, const mxPointerType& type, void* _userData ) override;
};

void ValidatePointers( const void* o, const mxClass& type );

void CheckAllPointersAreInRange( const void* o, const mxClass& type, const void* start, UINT32 size );


class CountReferences : public Reflection::AVisitor
{
	void *	m_pointer;
	UINT	m_numRefs;
public:
	inline CountReferences( void* o )
	{
		m_pointer = o;
		m_numRefs = 0;
	}
	inline UINT GetNumReferences() const
	{
		return m_numRefs;
	}
protected:
	virtual void* Visit_Pointer( VoidPointer& p, const mxPointerType& type, void* _userData ) override
	{
		if( p.o == m_pointer ) {
			++m_numRefs;
		}
		return _userData;
	}
};


class AVisitor2
{
	friend class Walker2;

public:
	// for passing parameters between callbacks
	struct Context
	{
		const Context *	parent;
		const mxField *	member;	// for class members
		//UINT32			index;	// for arrays only
		const char *	userName;
		mutable void *	userData;
		const int		depth;	// for debug print
	public:
		Context( int _depth = 0 )
			: depth( _depth )
		{
			parent = NULL;
			member = NULL;
			//index = ~0;
			userName = "?";
			userData = NULL;
		}
		const char* GetMemberName() const {
			return member ? member->name : userName;
		}
		const char* GetMemberType() const {
			return member ? member->type.m_name.buffer : userName;
		}
	};
protected:
	// return false to skip further processing
	//virtual bool Visit( void * _memory, const mxType& _type, const Context& _context )
	//{ return true; }

	// this function is called to visit a class member field
	// (e.g. this is used by buffer/JSON/XML serializers).
	// return false to skip further processing
	virtual bool Visit_Field( void * _memory, const mxField& _field, const Context& _context )
	{ return true; }

	//this adds too much overhead:
	//virtual void Visit_Array_Item( void * _pointer, const mxType& _type, UINT32 _index, const Context& _context ) {}

	// structures
	virtual bool Visit_Class( void * _object, const mxClass& _type, const Context& _context )
	{ return true; }

	// arrays
	// return false to skip processing the array elements
	virtual bool Visit_Array( void * _array, const mxArray& _type, const Context& _context ) {return true;}

	// 'Leaf' types:

	// built-in types (int, float, bool), enums, bitmasks
	virtual void Visit_POD( void * _memory, const mxType& _type, const Context& _context ) {}
	virtual void Visit_String( String & _string, const Context& _context ) {}
	virtual void Visit_TypeId( SClassId * _class, const Context& _context ) {}
	virtual void Visit_AssetId( AssetID & _assetId, const Context& _context ) {}

	// references
	virtual void Visit_Pointer( VoidPointer & _pointer, const mxPointerType& _type, const Context& _context ) {}
	virtual void Visit_UserPointer( void * _pointer, const mxUserPointerType& _type, const Context& _context ) {}

protected:
	virtual ~AVisitor2() {}
};

// Object/Field iterator
struct Walker2 {
	static void Visit( void * _memory, const mxType& _type, AVisitor2* _visitor, void *_userData = nil );
	static void VisitArray( void * _array, const mxArray& _type, AVisitor2* _visitor, void* _userData = nil );
	static void VisitAggregate( void * _struct, const mxClass& _type, AVisitor2* _visitor, void *_userData = nil );
	static void VisitStructFields( void * _struct, const mxClass& _type, AVisitor2* _visitor, void* _userData = nil );

	static void Visit( void * _memory, const mxType& _type, AVisitor2* _visitor, const AVisitor2::Context& _context );
	static void VisitArray( void * _array, const mxArray& _type, AVisitor2* _visitor, const AVisitor2::Context& _context );
	static void VisitAggregate( void * _struct, const mxClass& _type, AVisitor2* _visitor, const AVisitor2::Context& _context );
	static void VisitStructFields( void * _struct, const mxClass& _type, AVisitor2* _visitor, const AVisitor2::Context& _context );
};

struct TellNotToFreeMemory : public Reflection::AVisitor2
{
	typedef Reflection::AVisitor Super;
	//-- Reflection::AVisitor
	virtual bool Visit_Array( void * _array, const mxArray& _type, const Context& _context ) override;
	virtual void Visit_String( String & _string, const Context& _context ) override;
};

void MarkMemoryAsExternallyAllocated( void* _memory, const mxClass& _type );

bool HasCrossPlatformLayout( const mxClass& _type );

bool ObjectsAreEqual( const mxClass& _type1, const void* _o1, const mxClass& _type2, const void* _o2 );

}//namespace Reflection


//!--------------------------------------------------------------------------

template< typename TYPE >
struct TPODCast
{
	static mxFORCEINLINE void Set( void* const base, const MetaOffset offset, const TYPE& newValue )
	{
		TYPE & o = *(TYPE*) ( (BYTE*)base + offset );
		o = newValue;
	}
	static mxFORCEINLINE const TYPE& Get( const void*const base, const MetaOffset offset )
	{
		return *(TYPE*) ( (BYTE*)base + offset );
	}
	static mxFORCEINLINE TYPE& GetNonConst( void *const base, const MetaOffset offset )
	{
		return *(TYPE*) ( (BYTE*)base + offset );
	}
	static mxFORCEINLINE const TYPE& GetConst( const void*const objAddr )
	{
		return *(const TYPE*) objAddr;
	}
	static mxFORCEINLINE TYPE& GetNonConst( void*const objAddr )
	{
		return *(TYPE*) objAddr;
	}
};

inline INT64 GetInteger( const void* _pointer, const int _byteWidth )
{
	if( _byteWidth == 1 ) {
		return *(INT8*) _pointer;
	}
	else if( _byteWidth == 2 ) {
		return *(INT16*) _pointer;
	}
	else if( _byteWidth == 4 ) {
		return *(INT32*) _pointer;
	}
	else if( _byteWidth == 8 ) {
		return *(INT64*) _pointer;
	}
	else {
		mxUNREACHABLE;
		return 0;
	}	
}

inline double GetDouble( const void* _pointer, const int _byteWidth )
{
	if( _byteWidth == 4 ) {
		return *(float*) _pointer;
	}
	else if( _byteWidth == 8 ) {
		return *(double*) _pointer;
	}
	else {
		mxUNREACHABLE;
		return 0;
	}
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
