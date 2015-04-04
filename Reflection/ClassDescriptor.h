/*
=============================================================================
	File:	ClassDescriptor.h
	Desc:	Class reflection metadata descriptor.
	ToDo:	strip class names from the executable in release version
=============================================================================
*/
#pragma once

#include <Base/Object/TypeDescriptor.h>
#include <Base/Object/Reflection.h>

/*
=============================================================================
	Object factory
=============================================================================
*/

// function prototypes (used for creating object instances by type information)

// allocates memory and calls constructor
typedef void* F_CreateObject();
// constructs the object in-place (used to init vtables)
typedef void F_ConstructObject( void* objMem );
//F_DestroyObject
typedef void F_DestructObject( void* objMem );

//
// Object factory function generators,
// wrapped in the common 'friend' class.
// this is wrapped in a struct rather than a namespace
// to allow access to private/protected class members
// (via 'friend' keyword).
//
struct TypeHelper
{
	template< typename TYPE >
	static inline void* CreateObjectTemplate()
	{
		return new TYPE();
	}
	template< typename TYPE >
	static inline void ConstructObjectTemplate( void* objMem )
	{
		::new (objMem) TYPE();
	}
	template< typename TYPE >
	static inline void DestructObjectTemplate( void* objMem )
	{
		((TYPE*)objMem)->TYPE::~TYPE();
	}
};

//
//	contains some common parameters for initializing mxClass structure
//
struct SClassDescription : public STypeDescription
{
	F_CreateObject *	creator;	// 'create' function, null for abstract classes
	F_ConstructObject *	constructor;// 'create in place' function, null for abstract classes
	F_DestructObject *	destructor;

public:
	inline SClassDescription()
	{
		creator = nil;
		constructor = nil;
		destructor = nil;
	}
	inline SClassDescription(ENoInit)
		: STypeDescription(_NoInit)
	{}
	template< typename CLASS >
	static SClassDescription For_Class_With_Default_Ctor()
	{
		SClassDescription	classInfo(_NoInit);
		classInfo.Collect_Common_Properties_For_Class< CLASS >();

		classInfo.alignment = mxALIGNMENT( CLASS );

		classInfo.creator = TypeHelper::CreateObjectTemplate< CLASS >;
		classInfo.constructor = TypeHelper::ConstructObjectTemplate< CLASS >;
		classInfo.destructor = TypeHelper::DestructObjectTemplate< CLASS >;

		return classInfo;
	}
	template< typename CLASS >
	static SClassDescription For_Class_With_No_Default_Ctor()
	{
		SClassDescription	classInfo(_NoInit);
		classInfo.Collect_Common_Properties_For_Class< CLASS >();

		classInfo.alignment = mxALIGNMENT( CLASS );

		classInfo.creator = nil;
		classInfo.constructor = nil;
		classInfo.destructor = TypeHelper::DestructObjectTemplate< CLASS >;

		return classInfo;
	}
	template< typename CLASS >
	static SClassDescription For_Abstract_Class()
	{
		SClassDescription	classInfo(_NoInit);
		classInfo.Collect_Common_Properties_For_Class< CLASS >();

		classInfo.alignment = 0;

		classInfo.creator = nil;
		classInfo.constructor = nil;
		classInfo.destructor = TypeHelper::DestructObjectTemplate< CLASS >;

		return classInfo;
	}
private:
	template< typename CLASS >
	inline void Collect_Common_Properties_For_Class()
	{
		this->size = sizeof CLASS;
	}
};

/*
-----------------------------------------------------------------------------
	mxClass

	this is a base class for providing information about C++ classes

	NOTE: only single inheritance class hierarchies are supported!
-----------------------------------------------------------------------------
*/
class mxClass : public mxType
{
public:
	const char*	GetTypeName() const;	// GetTypeName, because GetClassName is defined in Windows headers.
	TypeIDArg	GetTypeID() const;

	// Returns the size of a single instance of the class, in bytes.
	size_t		GetInstanceSize() const;

	const mxClassLayout& GetLayout() const;

	const mxClass *	GetParent() const;

	bool	IsDerivedFrom( const mxClass& other ) const;
	bool	IsDerivedFrom( TypeIDArg typeCode ) const;
	bool	IsDerivedFrom( const char* className ) const;

	bool	operator == ( const mxClass& other ) const;
	bool	operator != ( const mxClass& other ) const;

	// Returns 'true' if this type inherits from the given type.
	template< class CLASS >	// where CLASS : AObject
	inline bool IsA() const
	{
		return this->IsDerivedFrom( CLASS::MetaClass() );
	}

	// Returns 'true' if this type is the same as the given type.
	template< class CLASS >	// where CLASS : AObject
	inline bool Is() const
	{
		return this == &CLASS::MetaClass();
	}

	bool IsAbstract() const;
	bool IsConcrete() const;

	F_CreateObject *	GetCreator() const;
	F_ConstructObject *	GetConstructor() const;
	F_DestructObject *	GetDestructor() const;

	AObject* CreateInstance() const;
	bool ConstructInPlace( void* o ) const;
	bool DestroyInstance( void* o ) const;

	bool IsEmpty() const;

public_internal:

	// these constructors are wrapped in macros
	mxClass(
		const Chars& className,
		const TypeIDArg classGuid,
		const mxClass* const baseClass,
		const SClassDescription& classInfo,
		const mxClassLayout& reflectedMembers
		);

private:
	const TypeID			m_uid;	// unique type identifier

	const mxClass * const	m_base;	// base class of this class

	const mxClassLayout &	m_members;	// reflected members of this class (not including inherited members)

//#if MX_USE_FAST_RTTI
//	TypeID	m_dynamicId;	// dynamic id of this class
//	TypeID	m_lastChild;	// dynamic id of the last descendant of this class
//#endif // MX_USE_FAST_RTTI

	F_CreateObject *	m_creator;
	F_ConstructObject *	m_constructor;
	F_DestructObject *	m_destructor;

private:
	friend class TypeRegistry;

	// these are used for building the linked list of registered classes
	// during static initialization

	mxClass *		m_next;

	// this is the head of the singly linked list of all class descriptors
	static mxClass *	m_head;

public:
	void *	fallbackInstance;	// [optional] e.g. a pointer to the fallback asset instance
	class AssetCallbacks *	ass;	// callbacks for asset loading (project-specific)

	const class EdClassInfo *	editorInfo;	// additional metadata (loaded from external files)

	UINT32		allocationGranularity;	// new objects in clumps should be allocated in batches

private:
	NO_COPY_CONSTRUCTOR( mxClass );
	NO_ASSIGNMENT( mxClass );
};

#include <Base/Object/ClassDescriptor.inl>

/*
=============================================================================
	
	Macros for declaring and implementing type information

=============================================================================
*/


//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// NOTE: 'virtual' is omitted intentionally in order to avoid vtbl placement in 'non-virtual' classes.
//
#define mxDECLARE_CLASS_COMMON( CLASS, BASE_KLASS )\
	mxUTIL_CHECK_BASE_CLASS( CLASS, BASE_KLASS );\
	private:\
		static mxClass ms_staticTypeInfo;\
		friend class TypeHelper;\
	public:\
		mxFORCEINLINE static mxClass& MetaClass() { return ms_staticTypeInfo; };\
		mxFORCEINLINE /*virtual*/ mxClass& rttiGetClass() const { return ms_staticTypeInfo; }\
		typedef CLASS ThisType;\
		typedef BASE_KLASS Super;\



//== MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//	This macro must be included in the definition of any subclass of AObject (usually in header files).
//	This must be used on single inheritance concrete classes only!
//---------------------------------------------------------------------------
//
#define mxDECLARE_CLASS( CLASS, BASE_KLASS )\
	mxDECLARE_CLASS_COMMON( CLASS, BASE_KLASS );\



//== MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//	This macro must be included in the definition of any abstract subclass of AObject (usually in header files).
//	This must be used on single inheritance abstract classes only!
//---------------------------------------------------------------------------
//
#define mxDECLARE_ABSTRACT_CLASS( CLASS, BASE_KLASS )\
	mxDECLARE_CLASS_COMMON( CLASS, BASE_KLASS );\
	NO_ASSIGNMENT(CLASS);\
	mxUTIL_CHECK_ABSTRACT_CLASS( CLASS );



//== MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//	mxDEFINE_CLASS must be included in the implementation of any subclass of AObject (usually in source files).
//---------------------------------------------------------------------------
//
#define mxDEFINE_CLASS( CLASS )\
	mxClass	CLASS::ms_staticTypeInfo(\
					mxEXTRACT_TYPE_NAME( CLASS ), mxEXTRACT_TYPE_GUID( CLASS ),\
					&T_DeduceClass< Super >(),\
					SClassDescription::For_Class_With_Default_Ctor< CLASS >(),\
					CLASS::StaticLayout()\
					);


//== MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//	mxDEFINE_CLASS_NO_DEFAULT_CTOR must be included in the implementation of any subclass of AObject (usually in source files).
//---------------------------------------------------------------------------
//
#define mxDEFINE_CLASS_NO_DEFAULT_CTOR( CLASS )\
	mxClass	CLASS::ms_staticTypeInfo(\
					mxEXTRACT_TYPE_NAME( CLASS ), mxEXTRACT_TYPE_GUID( CLASS ),\
					&Super::MetaClass(),\
					SClassDescription::For_Class_With_No_Default_Ctor< CLASS >(),\
					CLASS::StaticLayout()\
					);

//== MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//	mxDEFINE_ABSTRACT_CLASS must be included in the implementation of any abstract subclass of AObject (usually in source files).
//---------------------------------------------------------------------------
//
#define mxDEFINE_ABSTRACT_CLASS( CLASS )\
	mxClass	CLASS::ms_staticTypeInfo(\
					mxEXTRACT_TYPE_NAME( CLASS ), mxEXTRACT_TYPE_GUID( CLASS ),\
					&Super::MetaClass(),\
					SClassDescription::For_Abstract_Class< CLASS >(),\
					CLASS::StaticLayout()\
					);


//---------------------------------------------------------------------------
// Helper macros used for type checking
//---------------------------------------------------------------------------

//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// compile-time inheritance test for catching common typing errors
//
#define mxUTIL_CHECK_BASE_CLASS( CLASS, BASE_KLASS )\
	private:\
		static void PP_JOIN_TOKEN( CLASS, __SimpleInheritanceCheck )()\
		{\
			BASE_KLASS* base;\
			CLASS* derived = static_cast<CLASS*>( base );\
			(void)derived;\
		}\
	public:


//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// all abstract classes must be derived from AObject
//
#define mxUTIL_CHECK_ABSTRACT_CLASS( CLASS )\
	private:\
		static void PP_JOIN_TOKEN( Abstract_Class_,\
						PP_JOIN_TOKEN( CLASS, _Must_Derive_From_AObject )\
					)()\
		{\
			AObject* base;\
			CLASS* derived = static_cast<CLASS*>( base );\
			(void)derived;\
		}\
	public:


//---------------------------------------------------------------------
//	STypedObject
//---------------------------------------------------------------------
//
struct STypedObject
{
	CStruct *		o;
	const mxClass *	type;

public:
	STypedObject();

	bool IsValid() const
	{
		return o != nil;
	}
};

//---------------------------------------------------------------------
//	SClassIdType
//---------------------------------------------------------------------
//
struct SClassId
{
	const mxClass* type;
};
struct SClassIdType : public mxType
{
	inline SClassIdType()
		: mxType( Type_ClassId, mxEXTRACT_TYPE_NAME(SClassId), STypeDescription::For_Type< SClassId >() )
	{}

	static SClassIdType ms_staticInstance;
};

template<>
struct TypeDeducer< const mxClass* >
{
	static inline ETypeKind GetTypeKind()	{ return ETypeKind::Type_ClassId; }
	static inline const mxType& GetType()	{ return SClassIdType::ms_staticInstance; }
};
template<>
struct TypeDeducer< mxClass* >
{
	static inline ETypeKind GetTypeKind()	{ return ETypeKind::Type_ClassId; }
	static inline const mxType& GetType()	{ return SClassIdType::ms_staticInstance; }
};
template<>
struct TypeDeducer< TPtr< const mxClass > >
{
	static inline ETypeKind GetTypeKind()	{ return ETypeKind::Type_ClassId; }
	static inline const mxType& GetType()	{ return SClassIdType::ms_staticInstance; }
};
template<>
struct TypeDeducer< TPtr< mxClass > >
{
	static inline ETypeKind GetTypeKind()	{ return ETypeKind::Type_ClassId; }
	static inline const mxType& GetType()	{ return SClassIdType::ms_staticInstance; }
};

// K usually has a member function:
// void SetObject( void* o, const mxClass& type )
// 
template< class T, class K >
void TSetObject( K & k, T & t )
{
	k.SetObject( &t, T_DeduceClass< T >() );
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
