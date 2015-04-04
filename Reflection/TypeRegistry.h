/*
=============================================================================
	File:	TypeRegistry.h
	Desc:	TypeRegistry - central object type database,
			also serves as an object factory - creates objects by a class name.
			TypeRegistry is a singleton.
			Classes register themselves in the factory
			through the macros DECLARE_CLASS and DEFINE_CLASS.
=============================================================================
*/
#pragma once

#include <Base/Template/Containers/HashMap/THashMap.h>
#include <Base/Template/Containers/HashMap/TStringMap.h>

#include <Base/Object/ClassDescriptor.h>

// Forward declarations.
class String;
class AObject;

//
//	TypeRegistry
//	
//	This class maintains a map of
/// class names and GUIDs to factory functions.
//
class TypeRegistry : SingleInstance< TypeRegistry > {
public:
	// singleton access
	// (we cannot use the Singleton template because ctor shouldn't be accessible to Singleton).

	static TypeRegistry& Get();

	// must be called after all type infos have been statically initialized
	static void Initialize();

	// NOTE: this must be called at the end of the main function to delete the instance of TypeRegistry.
	static void Destroy();

	static bool IsInitialized();

public:
	TypeRegistry();
	~TypeRegistry();

	bool	ClassExists( TypeIDArg typeCode ) const;

	const mxClass* FindClassByGuid( TypeIDArg typeCode ) const;
	const mxClass* FindClassByName( const char* className ) const;

	AObject* CreateInstance( TypeIDArg typeCode ) const;

	void EnumerateDescendants( const mxClass& baseClass, TArray<const mxClass*> &OutClasses );
	void EnumerateConcreteDescendants( const mxClass& baseClass, TArray<const mxClass*> &OutClasses );

	typedef void F_ClassIterator( mxClass & type, void* userData );

	void ForAllClasses( F_ClassIterator* visitor, void* userData );

private:
	THashMap< TypeID, const mxClass* >	m_typesByGuid;	// for fast lookup by TypeID code
	// TODO: fast string dictionary, binary search
	THashMap< String, const mxClass* >		m_typesByName;	// for fast lookup by class name (and for detecting duplicate names)
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
