#pragma once

#include <Base/Object/TypeDescriptor.h>
#include <Base/Object/Reflection.h>

/*
-----------------------------------------------------------------------------
	mxFlagsType

	TBits< ENUM, STORAGE > - 8,16,32 bits of named values.
-----------------------------------------------------------------------------
*/
struct mxFlagsType : public mxType
{
	typedef UINT32 Mask;

	struct Member
	{
		const char*	name;	// name of the value in the code
		Mask		mask;
	};
	struct MemberList
	{
		const Member *	array;
		const UINT		count;
	};
	struct Accessor
	{
		virtual Mask Get_Value( const void* flags ) const = 0;
		virtual void Set_Value( void *flags, Mask value ) const = 0;
	};

	const Member*	m_members;
	const UINT		m_numFlags;
	const Accessor&	m_accessor;

public:
	inline mxFlagsType( const Chars& typeName, const MemberList& members, const Accessor& accessor, const STypeDescription& info )
		: mxType( ETypeKind::Type_Flags, typeName, info )
		, m_members( members.array )
		, m_numFlags( members.count )
		, m_accessor( accessor )
	{}

	UINT	GetItemIndexByName( const char* name ) const;
	Mask	GetItemValueByName( const char* name ) const;
};


//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//!=- ENUM - name of enumeration
//!=- STORAGE - type of storage
//!=- FLAGS - type of TBits<ENUM,STORAGE>
//
#define mxDECLARE_FLAGS( ENUM, STORAGE, FLAGS )\
	typedef TBits< ENUM, STORAGE > FLAGS;\
	mxDECLARE_POD_TYPE( FLAGS );\
	extern const mxFlagsType::MemberList& PP_JOIN_TOKEN( Reflect_Flags_, FLAGS )();\
	template<>\
	struct TypeDeducer< FLAGS >\
	{\
		static inline const mxType& GetType()\
		{\
			mxSTATIC_ASSERT( sizeof FLAGS <= sizeof UINT );\
			\
			struct ValueAccessor : public mxFlagsType::Accessor\
			{\
				virtual mxFlagsType::Mask Get_Value( const void* flags ) const override\
				{\
					return *(const FLAGS*) flags;\
				}\
				virtual void Set_Value( void *flags, mxFlagsType::Mask value ) const override\
				{\
					*((FLAGS*) flags) = (ENUM) value;\
				}\
			};\
			static ValueAccessor valueAccessor;\
			static mxFlagsType staticTypeInfo(\
								mxEXTRACT_TYPE_NAME( FLAGS ),\
								PP_JOIN_TOKEN( Reflect_Flags_, FLAGS )(),\
								valueAccessor,\
								STypeDescription::For_Type< FLAGS >()\
							);\
			return staticTypeInfo;\
		}\
		static inline ETypeKind GetTypeKind()\
		{\
			return ETypeKind::Type_Flags;\
		}\
	};

//!--------------------------------------------------------------------------




//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//!=- should be placed into source files
//
#define mxBEGIN_FLAGS( FLAGS )\
	const mxFlagsType::MemberList& PP_JOIN_TOKEN( Reflect_Flags_, FLAGS )() {\
		static mxFlagsType::Member items[] = {\


//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
#define mxREFLECT_BIT( NAME, VALUE )\
	{\
		mxEXTRACT_NAME(NAME).buffer,\
		VALUE\
	}


//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
#define mxEND_FLAGS\
		};\
		static mxFlagsType::MemberList reflectionMetadata = { items, mxCOUNT_OF(items) };\
		return reflectionMetadata;\
	}


//!--------------------------------------------------------------------------


//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
