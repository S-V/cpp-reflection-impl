#pragma once

#include <Base/Object/TypeDescriptor.h>
#include <Base/Object/Reflection.h>

/*
-----------------------------------------------------------------------------
	mxEnumType
-----------------------------------------------------------------------------
*/
struct mxEnumType : public mxType
{
	struct Member
	{
		const char*	name;	// name of the value in the code
		UINT32		value;
	};
	struct MemberList
	{
		const Member *	items;
		const UINT32	count;
	public:
		UINT GetItemIndexByValue( const UINT nEnumValue ) const;
		UINT GetItemIndexByString( const char* name ) const;	// NOTE: very slow!
	};
	struct Accessor
	{
		virtual UINT Get_Value( const void* pEnum ) const = 0;
		virtual void Set_Value( void *pEnum, UINT value ) const = 0;
	};

	const MemberList	m_members;
	const Accessor&		m_accessor;

public:
	inline mxEnumType( const Chars& typeName, const MemberList& members, const Accessor& accessor, const STypeDescription& info )
		: mxType( ETypeKind::Type_Enum, typeName, info )
		, m_members( members )
		, m_accessor( accessor )
	{}

	UINT GetItemIndexByValue( const UINT nEnumValue ) const;
	UINT GetItemIndexByString( const char* sEnumValue ) const;	// NOTE: very slow!

	const char* GetStringByValue( const UINT nEnumValue ) const;
	UINT GetValueByString( const char* sEnumValue ) const;
};

template< typename ENUM >	// some enum or TEnum< some enum >
inline const mxEnumType::MemberList GetEnumMembers()
{
	Unknown_Enum_Type;
};

#define mxINTERNAL_GENERATE_ENUM_TYPE( ENUM, TYPEDEF )\
	inline const mxEnumType& PP_JOIN_TOKEN( GetTypeOf_, TYPEDEF )()\
	{\
		extern const mxEnumType::MemberList PP_JOIN_TOKEN(TYPEDEF,_members)();\
		struct EnumAccessor : public mxEnumType::Accessor\
		{\
			virtual UINT Get_Value( const void* pEnum ) const override\
			{\
				return *(const TYPEDEF*) pEnum;\
			}\
			virtual void Set_Value( void *pEnum, UINT value ) const override\
			{\
				*((TYPEDEF*) pEnum) = (ENUM) value;\
			}\
		};\
		static EnumAccessor enumAccessor;\
		static mxEnumType enumTypeInfo(\
							mxEXTRACT_TYPE_NAME(ENUM),\
							PP_JOIN_TOKEN(TYPEDEF,_members)(),\
							enumAccessor,\
							STypeDescription::For_Type< TYPEDEF >()\
						);\
		return enumTypeInfo;\
	}\
	template<>\
	struct TypeDeducer< TYPEDEF >\
	{\
		mxSTATIC_ASSERT( sizeof(TYPEDEF) <= sizeof(UINT32) );\
		static inline const mxType& GetType()\
		{\
			return PP_JOIN_TOKEN( GetTypeOf_, TYPEDEF )();\
		}\
		static inline ETypeKind GetTypeKind()\
		{\
			return ETypeKind::Type_Enum;\
		}\
	};



//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//!=- should be placed into header files
//!=- ENUM - name of enumeration
//!=- STORAGE - type of storage
//!=- TYPEDEF - type of TEnum<ENUM,STORAGE>
//
#define mxDECLARE_ENUM( ENUM, STORAGE, TYPEDEF )\
	typedef TEnum< enum ENUM, STORAGE > TYPEDEF;\
	mxINTERNAL_GENERATE_ENUM_TYPE( ENUM, TYPEDEF );\
	template<>\
	inline const mxEnumType::MemberList GetEnumMembers< ENUM >() {\
		extern const mxEnumType::MemberList PP_JOIN_TOKEN(TYPEDEF,_members)();\
		return PP_JOIN_TOKEN(TYPEDEF,_members)();\
	}\
	template<>\
	inline const mxEnumType::MemberList GetEnumMembers< TYPEDEF >() {\
		extern const mxEnumType::MemberList PP_JOIN_TOKEN(TYPEDEF,_members)();\
		return PP_JOIN_TOKEN(TYPEDEF,_members)();\
	}\

//!--------------------------------------------------------------------------

#define mxGET_ENUM_TYPE( ENUM_NAME )	PP_JOIN_TOKEN( GetTypeOf_, ENUM_NAME )()


//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//!=- should be placed into source files
//
#define mxBEGIN_REFLECT_ENUM( TYPEDEF )\
	const mxEnumType::MemberList PP_JOIN_TOKEN(TYPEDEF,_members)() {\
		static mxEnumType::Member items[] = {\

//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
#define mxREFLECT_ENUM_ITEM( NAME, VALUE )\
		{\
			mxEXTRACT_NAME(NAME).buffer,\
			VALUE\
		}

//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
#define mxREFLECT_ENUM_ITEM1( X )\
		{\
			mxEXTRACT_NAME( X ).buffer,\
			X\
		}

//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
#define mxEND_REFLECT_ENUM\
		};\
		const mxEnumType::MemberList result = { items, mxCOUNT_OF(items) };\
		return result;\
	}

//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//!=- should be placed into header files
//!=- ENUM - name of enumeration
//!=- STORAGE - type of storage
//!=- TYPEDEF - type of TEnum<ENUM,STORAGE>
//
#define mxBEGIN_ENUM( ENUM, STORAGE, TYPEDEF )\
	mxDECLARE_ENUM( ENUM, STORAGE, TYPEDEF )
//!--------------------------------------------------------------------------




template< typename ENUM, typename VALUE >
bool GetEnumFromString( VALUE &result, const char* name )
{
	const mxEnumType::MemberList enumMembers = GetEnumMembers< ENUM >();
	const UINT enumItemIndex = enumMembers.GetItemIndexByString( name );
	if( enumItemIndex == INDEX_NONE ) {
		ptERROR("Unknown enum value: '%s'\n", name);
		return false;
	}
	result = (ENUM) enumMembers.items[ enumItemIndex ].value;
	return true;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
