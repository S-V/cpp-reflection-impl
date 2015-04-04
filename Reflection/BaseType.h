/*
=============================================================================
	File:	BaseType.h
	Desc:	Base class for many objects, root of our class hierarchy.
			It's quite lightweight (only vtbl overhead).
=============================================================================
*/
#pragma once

#include <Base/Object/TypeRegistry.h>
#include <Base/Object/Reflection.h>

class mxArchive;
struct SDbgDumpContext;

/*
-----------------------------------------------------------------------------
	CStruct
	base class for C-compatible structs (ideally, bitwise copyable, POD types),
	i.e. derived classes MUST *NOT* have virtual functions and pointers to self
-----------------------------------------------------------------------------
*/
struct CStruct
{
	inline ~CStruct() {}

public:	// Reflection
	static mxClass & MetaClass() { return ms_staticTypeInfo; };
	inline mxClass& rttiGetClass() const { return ms_staticTypeInfo; }

protected:
	inline CStruct() {}

private:
	static mxClass	ms_staticTypeInfo;
};

/*
-----------------------------------------------------------------------------
	AObject

	abstract base class for most polymorphic classes in our class hierarchy,
	provides fast RTTI.

	NOTE: if this class is used as a base class when using multiple inheritance,
	this class should go first (to ensure the vtable is placed at the beginning).

	NOTE: this class must be abstract!
	NOTE: this class must not define any member fields!
-----------------------------------------------------------------------------
*/
class AObject : public CStruct {
public:
	// Static Reflection
	static mxClass & MetaClass();

	//
	//	Run-Time Type Information (RTTI)
	//
	virtual mxClass& rttiGetClass() const;

	// These functions are provided for convenience.

	const char *	rttiGetTypeName() const;
	const TypeID	rttiGetTypeID() const;

	// Returns 'true' if this type inherits from the given type.
	bool	IsA( const mxClass& type ) const;
	bool	IsA( TypeIDArg typeCode ) const;

	// Returns 'true' if this type is the same as the given type.
	bool	IsInstanceOf( const mxClass& type ) const;
	bool	IsInstanceOf( TypeIDArg typeCode ) const;

	// Returns 'true' if this type inherits from the given type.
	template< class CLASS >
	inline bool IsA() const
	{
		return this->IsA( CLASS::MetaClass() );
	}

	// Returns 'true' if this type is the same as the given type.
	template< class CLASS >
	inline bool Is() const
	{
		return IsInstanceOf( CLASS::MetaClass() );
	}

public:	// Serialization / Streaming

	// serialize persistent data (including pointers to other objects)
	// in binary form (MFC-style).
	// NOTE: extremely brittle, use with caution!
	//
	virtual void Serialize( mxArchive & s ) {}

	// complete loading (e.g. fixup asset references)
	// this function is called immediately after deserialization to complete the object's initialization
	virtual void PostLoad() {}

	// perform minor optimizations before saving
	//virtual void PreSave( const SPreSaveContext& saveArgs ) {}

public:	// Cloning (virtual constructor, deep copying)
	//virtual AObject* Clone() { return nil; }

public:	// Testing & Debugging

	// returns the memory address of the object
	const void* GetAddress() const { return this; }

	virtual void DbgAssertValid() {}
	virtual void DbgDumpContents( const SDbgDumpContext& dc ) {}

public:
	virtual	~AObject() = 0;

protected:
	AObject() {}

private:	NO_COMPARES(AObject);

	static mxClass	ms_staticTypeInfo;
};

#include <Base/Object/BaseType.inl>

//
//	DynamicCast< T > - safe (checked) cast, returns a nil pointer on failure
//
template< class TypeTo >
mxINLINE
TypeTo* DynamicCast( AObject* pObject )
{
	mxASSERT_PTR( pObject );
	return pObject->IsA( TypeTo::MetaClass() ) ?
		static_cast< TypeTo* >( pObject ) : nil;
}

//
//	ConstCast< T >
//
template< class TypeTo, class TypeFrom > // where TypeTo : AObject and TypeFrom : AObject
mxINLINE
const TypeTo* ConstCast( const TypeFrom* pObject )
{
	mxASSERT_PTR( pObject );
	return pObject->IsA( TypeTo::MetaClass() ) ?
		static_cast< const TypeTo* >( pObject ) : nil;
}

//
//	UpCast< T > - unsafe cast, raises an error on failure,
//	assumes that you know what you're doing.
//
template< class TypeTo >
mxINLINE
TypeTo* UpCast( AObject* pObject )
{
	mxASSERT_PTR( pObject );
	mxASSERT( pObject->IsA( TypeTo::MetaClass() ) );
	return static_cast< TypeTo* >( pObject );
}

template< class TypeTo, class TypeFrom > // where TypeTo : AObject and TypeFrom : AObject
mxINLINE
TypeTo* SafeCast( TypeFrom* pObject )
{
	if( pObject != nil )
	{
		return pObject->IsA( TypeTo::MetaClass() ) ?
			static_cast< TypeTo* >( pObject ) : nil;
	}
	return nil;
}


namespace ObjectUtil
{
	inline bool Class_Has_Fields( const mxClass& classInfo )
	{
		return (classInfo.GetLayout().numFields > 0)
			// the root classes don't have any serializable state
			&& (classInfo != CStruct::MetaClass())
			&& (classInfo != AObject::MetaClass())
			;
	}
	inline bool Serializable_Class( const mxClass& classInfo )
	{
		return Class_Has_Fields(classInfo);
	}

	AObject* Create_Object_Instance( const mxClass& classInfo );
	AObject* Create_Object_Instance( TypeIDArg classGuid );

	template< class CLASS >
	AObject* Create_Object_Instance()
	{
		AObject* pNewInstance = Create_Object_Instance( CLASS::MetaClass() );
		return UpCast< CLASS >( pNewInstance );
	}

}//namespace ObjectUtil




//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// NOTE: can only be used on classes deriving from AObject
//
#define mxDECLARE_SERIALIZABLE( CLASS )\
	friend mxArchive& operator && ( mxArchive & archive, CLASS *& o )\
	{\
		archive.SerializePointer( *(AObject**)&o );\
		return archive;\
	}\
	friend mxArchive& operator && ( mxArchive & archive, TPtr<CLASS> & o )\
	{\
		archive.SerializePointer( *(AObject**)&o.Ptr );\
		return archive;\
	}

/*
	friend mxArchive& operator && ( mxArchive & archive, TRefPtr<CLASS> & o )\
	{\
		if( archive.IsWriting() )\
		{\
			AObject* basePtr = o;\
			archive.SerializePointer( basePtr );\
		}\
		else\
		{\
			AObject* basePtr;\
			archive.SerializePointer( basePtr );\
			o = UpCast<CLASS>( basePtr );\
		}\
		return archive;\
	}
*/

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
