/*
=============================================================================
	File:	mxClass.cpp
	Desc:	Classes for run-time type checking
			and run-time instancing of objects.
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Object/BaseType.h>
#include <Base/Object/TypeRegistry.h>
#include <Base/Object/ClassDescriptor.h>

/*================================
		mxClass
================================*/

// static
mxClass* mxClass::m_head = nil;

mxClass::mxClass(
	const Chars& className,
	const TypeIDArg classGuid,
	const mxClass* const baseClass,
	const SClassDescription& classInfo,
	const mxClassLayout& reflectedMembers
	)
	: mxType( ETypeKind::Type_Class, className, classInfo )
	, m_uid( classGuid )
	, m_base( baseClass )
	, m_members( reflectedMembers )
{
	mxASSERT( m_base != this );	// but parentType can be NULL for root classes

	m_creator = classInfo.creator;
	m_constructor = classInfo.constructor;
	m_destructor = classInfo.destructor;

	// Insert this object into the linked list
	m_next = m_head;
	m_head = this;

	fallbackInstance = nil;
	ass = nil;
	editorInfo = nil;
	allocationGranularity = 1;
}

bool mxClass::IsDerivedFrom( const mxClass& other ) const
{
	for ( const mxClass * current = this; current != nil; current = current->GetParent() )
	{
		if ( current == &other )
		{
			return true;
		}
	}
	return false;
}

bool mxClass::IsDerivedFrom( TypeIDArg typeCode ) const
{
	mxASSERT(typeCode != mxNULL_TYPE_ID);
	for ( const mxClass * current = this; current != nil; current = current->GetParent() )
	{
		if ( current->GetTypeID() == typeCode )
		{
			return true;
		}
	}
	return false;
}

bool mxClass::IsDerivedFrom( PCSTR className ) const
{
	mxASSERT_PTR(className);
	for ( const mxClass * current = this; current != nil; current = current->GetParent() )
	{
		if ( 0 == strcmp( m_name.buffer, className ) )
		{
			return true;
		}
	}
	return false;
}

AObject* mxClass::CreateInstance() const
{
	mxASSERT( this->IsConcrete() );

	F_CreateObject* pCreateFunction = this->GetCreator();
	mxASSERT_PTR( pCreateFunction );

	AObject* pObjectInstance = (AObject*) (*pCreateFunction)();
	mxASSERT_PTR( pObjectInstance );

	return pObjectInstance;
}

bool mxClass::ConstructInPlace( void* o ) const
{
	F_ConstructObject* constructor = this->GetConstructor();
	chkRET_FALSE_IF_NIL(constructor);
	(*constructor)( o );
	return true;
}

bool mxClass::DestroyInstance( void* o ) const
{
	F_DestructObject* destructor = this->GetDestructor();
	chkRET_FALSE_IF_NIL(destructor);
	(*destructor)( o );
	return true;
}

bool mxClass::IsEmpty() const
{
	UINT numFields = 0;
	for ( const mxClass * current = this; current != nil; current = current->GetParent() )
	{
		numFields += current->GetLayout().numFields;
	}
	return numFields == 0;
}

//---------------------------------------------------------------------
//	STypedObject
//---------------------------------------------------------------------
//
STypedObject::STypedObject()
	: o( nil )
	, type( &CStruct::MetaClass() )
{}

//---------------------------------------------------------------------
//	SClassIdType
//---------------------------------------------------------------------
//
SClassIdType SClassIdType::ms_staticInstance;

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
