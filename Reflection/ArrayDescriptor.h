// ArrayDescriptor
// 
#pragma once

#include <Base/Object/TypeDescriptor.h>

struct mxArray : public mxType
{
	const mxType &	m_itemType;	// type of stored elements
	const MetaSize	m_itemSize;	// array stride (@todo: remove?)

public:
	inline mxArray( const Chars& typeName, const STypeDescription& typeInfo, const mxType& storedItemType, UINT stride )
		: mxType( ETypeKind::Type_Array, typeName, typeInfo )
		, m_itemType( storedItemType )
		, m_itemSize( stride )
	{
	}

	// returns false if this is a static (local, in-place) array
	virtual bool IsDynamic() const = 0;

	// get address of the array pointer (which points at the allocated block of memory)
	virtual void* Get_Array_Pointer_Address( const void* pArrayObject ) const = 0;

	virtual UINT Generic_Get_Count( const void* pArrayObject ) const = 0;
	virtual bool Generic_Set_Count( void* pArrayObject, UINT newNum ) const = 0;

	virtual UINT Generic_Get_Capacity( const void* pArrayObject ) const = 0;
	virtual bool Generic_Set_Capacity( void* pArrayObject, UINT newNum ) const = 0;

	virtual void* Generic_Get_Data( void* pArrayObject ) const = 0;
	virtual const void* Generic_Get_Data( const void* pArrayObject ) const = 0;

	// this function is called after in-place loading
	virtual void SetDontFreeMemory( void* pArrayObject ) const = 0;
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
