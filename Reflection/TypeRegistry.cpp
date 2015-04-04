/*
=============================================================================
	File:	TypeRegistry.cpp
	Desc:	Object factory for run-time class instancing and type information.
	ToDo:	check inheritance depth and warn about pathologic cases?
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Object/BaseType.h>
#include <Base/Object/TypeRegistry.h>

/*
-----------------------------------------------------------------------------
	TypeRegistry
-----------------------------------------------------------------------------
*/
namespace {
	static TPtr< TypeRegistry >	gPtr;	//	Singleton.
}

//
//	TypeRegistry::GetInstance
//
TypeRegistry& TypeRegistry::Get()
{
	return *gPtr;
}

bool TypeRegistry::IsInitialized()
{
	return gPtr != nil;
}

void TypeRegistry::Initialize()
{
	mxASSERT(gPtr == nil);

	if ( gPtr == nil )
	{
		gPtr.ConstructInPlace();

		// Calculate number of types.
		int	numTypes = 0;
		{
			mxClass* curr = mxClass::m_head;

			while( PtrToBool(curr) )
			{
				mxClass* next = curr->m_next;

				++numTypes;

				curr = next;
			}
		}

		DBGOUT("TypeRegistry::Initialize(): %d types", numTypes);

		gPtr->m_typesByGuid.Reserve( numTypes );
		gPtr->m_typesByName.Reserve( numTypes );

		// Register types.
		{
			mxClass* current = mxClass::m_head;
			UINT classIndex = 0;

			while( PtrToBool(current) )
			{
				mxClass* next = current->m_next;

				const TypeID classGuid = current->GetTypeID();

				DBGOUT("Register class %s (ID=%u, GUID=0x%x)",
					current->GetTypeName(), classIndex, (UINT)classGuid );

				const mxClass* existing = gPtr->m_typesByGuid.FindRef( classGuid );
				if( existing != nil ) {
					ptFATAL("class name collision: '%s' and '%s'\n", current->GetTypeName(), existing->GetTypeName());
				}
				gPtr->m_typesByGuid.Set( classGuid, current );

				mxASSERT( ! gPtr->FindClassByName( current->m_name.buffer ) );

				String	classNameStr;
				classNameStr.SetReference(current->m_name);

				gPtr->m_typesByName.Set( classNameStr, current );

				current = next;

				classIndex++;
			}
		}
	}
}

void TypeRegistry::Destroy()
{
	if ( nil != gPtr )
	{
		gPtr.Destruct();
	}
}

TypeRegistry::TypeRegistry()
	: m_typesByGuid( _NoInit )
	, m_typesByName( _NoInit )
{
}

TypeRegistry::~TypeRegistry()
{
}

bool TypeRegistry::ClassExists( TypeIDArg typeCode ) const
{
	return m_typesByGuid.Contains( typeCode );
}

const mxClass* TypeRegistry::FindClassByGuid( TypeIDArg typeCode ) const
{
	const mxClass* typeInfo = m_typesByGuid.FindRef( typeCode );
	mxASSERT_PTR(typeInfo);
	return typeInfo;
}

const mxClass* TypeRegistry::FindClassByName( const char* className ) const
{
	String	classNameStr;
	classNameStr.SetReference(Chars(className));

	const mxClass* typeInfo = m_typesByName.FindRef( classNameStr );
	return typeInfo;
}

AObject* TypeRegistry::CreateInstance( TypeIDArg typeCode ) const
{
	AObject* pObjectInstance = ObjectUtil::Create_Object_Instance( typeCode );
	mxASSERT_PTR( pObjectInstance );
	return pObjectInstance;
}

void TypeRegistry::EnumerateDescendants( const mxClass& baseClass, TArray<const mxClass*> &OutClasses )
{
	mxClass* curr = mxClass::m_head;

	while( PtrToBool(curr) )
	{
		mxClass* next = curr->m_next;

		if( curr->IsDerivedFrom( baseClass ) )
		{
			OutClasses.Add( curr );
		}

		curr = next;
	}
}

void TypeRegistry::EnumerateConcreteDescendants( const mxClass& baseClass, TArray<const mxClass*> &OutClasses )
{
	mxClass* curr = mxClass::m_head;

	while( PtrToBool(curr) )
	{
		mxClass* next = curr->m_next;

		if( curr->IsDerivedFrom( baseClass ) && curr->IsConcrete() )
		{
			OutClasses.Add( curr );
		}

		curr = next;		
	}
}

void TypeRegistry::ForAllClasses( F_ClassIterator* visitor, void* userData )
{
	chkRET_IF_NIL(visitor);

	mxClass* curr = mxClass::m_head;

	while( PtrToBool(curr) )
	{
		mxClass* next = curr->m_next;

		(*visitor)( *curr, userData );

		curr = next;		
	}
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
