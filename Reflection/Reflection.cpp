/*
=============================================================================
	File:	Reflection.cpp
	Desc:	Reflection - having access at runtime
			to information about the C++ classes in your program.
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Object/BaseType.h>
#include <Base/Object/Reflection.h>
#include <Base/Text/String.h>

#define MX_DEBUG_REFLECTION		(0)

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//static
mxClassLayout mxClassLayout::dummy = { nil, 0 };

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace Reflection
{

/*
-----------------------------------------------------------------------------
	AVisitor
-----------------------------------------------------------------------------
*/
void* AVisitor::Visit_Field( void * _o, const mxField& _field, void* _userData )
{
	return Walker::Visit( _o, _field.type, this, _userData );
}

void* AVisitor::Visit_Aggregate( void * _o, const mxClass& _type, void* _userData )
{
	return Walker::VisitAggregate( _o, _type, this, _userData );
}

void* AVisitor::Visit_Array( void * _array, const mxArray& _type, void* _userData )
{
	return Walker::VisitArray( _array, _type, this, _userData );
}

void* AVisitor::Visit_Blob( void * _blob, const mxBlobType& _type, void* _userData )
{
	Unimplemented;
	return _userData;
}

//=====================================================================

void* Walker::Visit( void * _object, const mxType& _type, AVisitor* _visitor, void *_userData )
{
	mxASSERT_PTR(_object);

	switch( _type.m_kind )
	{
	case ETypeKind::Type_Integer :
	case ETypeKind::Type_Float :
	case ETypeKind::Type_Bool :
	case ETypeKind::Type_Enum :
	case ETypeKind::Type_Flags :
		{
			return _visitor->Visit_POD( _object, _type, _userData );
		}
		break;

	case ETypeKind::Type_String :
		{
			String & rString = TPODCast< String >::GetNonConst( _object );
			return _visitor->Visit_String( rString, _userData );
		}
		break;

	case ETypeKind::Type_Class :
		{
			const mxClass& classType = _type.UpCast< mxClass >();
			return _visitor->Visit_Aggregate( _object, classType, _userData );
		}
		break;

	case ETypeKind::Type_Pointer :
		{
			const mxPointerType& pointerType = _type.UpCast< mxPointerType >();
			VoidPointer& pointer = *static_cast< VoidPointer* >( _object );
			return _visitor->Visit_Pointer( pointer, pointerType, _userData );
		}
		break;

	case ETypeKind::Type_AssetId :
		{
			AssetID& assetId = *static_cast< AssetID* >( _object );
			return _visitor->Visit_AssetId( assetId, _userData );
		}
		break;

	case ETypeKind::Type_ClassId :
		{
			SClassId * classId = static_cast< SClassId* >( _object );
			return _visitor->Visit_TypeId( classId, _userData );
		}
		break;

	case ETypeKind::Type_UserData :
		{
			const mxUserPointerType& userPointerType = _type.UpCast< mxUserPointerType >();
			return _visitor->Visit_UserPointer( _object, userPointerType, _userData );
		}
		break;

	case ETypeKind::Type_Blob :
		{
			const mxBlobType& blobType = _type.UpCast< mxBlobType >();
			return _visitor->Visit_Blob( _object, blobType, _userData );
		}
		break;

	case ETypeKind::Type_Array :
		{
			const mxArray& arrayType = _type.UpCast< mxArray >();
			return _visitor->Visit_Array( _object, arrayType, _userData );
		}
		break;

		mxNO_SWITCH_DEFAULT;
	}//switch

	return nil;
}

void* Walker::VisitArray( void * _array, const mxArray& _type, AVisitor* _visitor, void* _userData )
{
	mxASSERT_PTR(_array);
	const UINT numObjects = _type.Generic_Get_Count( _array );
	const void* arrayBase = _type.Generic_Get_Data( _array );

	const mxType& itemType = _type.m_itemType;
	const UINT32 itemStride = _type.m_itemSize;

#if MX_DEBUG_REFLECTION
	DBGOUT("[Reflector]: Visit array of '%s' (%u items)\n", itemType.m_name, numObjects);
#endif // MX_DEBUG_REFLECTION

#if MX_DEBUG
	enum { DBG_MAX_ARRAY_CONTENTS_SIZE = 4*mxMEGABYTE };
	const UINT arrayContentsSize = itemStride * numObjects;
	mxASSERT( arrayContentsSize < DBG_MAX_ARRAY_CONTENTS_SIZE );
#endif // MX_DEBUG

	for( UINT iObject = 0; iObject < numObjects; iObject++ )
	{
		const MetaOffset itemOffset = iObject * itemStride;
		void* itemPtr = mxAddByteOffset( c_cast(void*)arrayBase, itemOffset );

		Visit( itemPtr, itemType, _visitor, _userData );
	}
	return _userData;
}

void* Walker::VisitAggregate( void * _struct, const mxClass& _type, AVisitor* _visitor, void *_userData )
{
	mxASSERT_PTR(_struct);
#if MX_DEBUG_REFLECTION
	DBGOUT("[Reflector]: Visit object of class '%s'\n", type.m_name);
#endif // MX_DEBUG_REFLECTION

	// First recursively visit the parent classes.
	const mxClass* parentType = _type.GetParent();
	while( parentType != nil )
	{
		VisitStructFields( _struct, *parentType, _visitor, _userData );
		parentType = parentType->GetParent();
	}

	// Now visit the members of this class.
	VisitStructFields( _struct, _type, _visitor, _userData );

	return _userData;
}

void* Walker::VisitStructFields( void * _struct, const mxClass& _type, AVisitor* _visitor, void* _userData )
{
	const mxClassLayout& layout = _type.GetLayout();
	for( UINT fieldIndex = 0 ; fieldIndex < layout.numFields; fieldIndex++ )
	{
		const mxField& field = layout.fields[ fieldIndex ];

#if MX_DEBUG_REFLECTION
		DBGOUT("[Reflector]: Visit field '%s' of type '%s' in '%s' at offset '%u'\n",
			field.name, field.type.m_name.ToPtr(), _type.m_name.ToPtr(), field.offset);
#endif // MX_DEBUG_REFLECTION

		void* memberVarPtr = mxAddByteOffset( _struct, field.offset );

		_visitor->Visit_Field( memberVarPtr, field, _userData );
	}
	return _userData;
}

static void ValidatePointer( void* ptr )
{
	if( ptr != nil )
	{
		mxASSERT(mxIsValidHeapPointer(ptr));
		//mxASSERT(_CrtIsValidHeapPointer(ptr));
		// try to read a value from the pointer
		void* value = *(void**)ptr;
		(void)value;
	}
}

void* PointerChecker::Visit_Array( void * arrayObject, const mxArray& arrayType, void* _userData )
{
	ValidatePointer(arrayType.Generic_Get_Data(arrayObject));
	return _userData;
}

void* PointerChecker::Visit_Pointer( VoidPointer& p, const mxPointerType& type, void* _userData )
{
	ValidatePointer(p.o);
	return _userData;
}

void ValidatePointers( const void* o, const mxClass& type )
{
	PointerChecker	pointerChecker;
	Walker::Visit( const_cast<void*>(o), type, &pointerChecker, nil );
}

void CheckAllPointersAreInRange( const void* o, const mxClass& type, const void* start, UINT32 size )
{
	class PointerRangeValidator : public Reflection::AVisitor
	{
		const void *	m_start;
		const UINT		m_size;

	public:
		PointerRangeValidator( const void* start, UINT size )
			: m_start( start ), m_size( size )
		{}
		virtual void* Visit_String( String & s, void* _userData ) override
		{
			if( s.NonEmpty() ) {
				this->ValidatePointer( s.GetBufferAddress() );
			}
			return _userData;
		}
		virtual void* Visit_Array( void * arrayObject, const mxArray& arrayType, void* _userData ) override
		{
			if( arrayType.IsDynamic() )
			{
				const UINT32 capacity = arrayType.Generic_Get_Capacity( arrayObject );
				if( capacity > 0 )
				{
					const void* arrayBase = arrayType.Generic_Get_Data( arrayObject );
					this->ValidatePointer( arrayBase );
				}
			}
			return Reflection::AVisitor::Visit_Array( arrayObject, arrayType, _userData );
		}
		virtual void* Visit_Pointer( VoidPointer& p, const mxPointerType& type, void* _userData ) override
		{
			if( p.o != nil )
			{
				this->ValidatePointer( p.o );
			}
			return _userData;
		}

	private:
		void ValidatePointer( const void* pointer )
		{
			if( !mxPointerInRange( pointer, m_start, m_size ) )
			{
				ptERROR("Invalid pointer\n");
			}
		}
	};
	PointerRangeValidator	pointerChecker( start, size );
	Reflection::Walker::Visit( const_cast<void*>(o), type, &pointerChecker, nil );
}

//=====================================================================

void Walker2::Visit( void * _memory, const mxType& _type, AVisitor2* _visitor, const AVisitor2::Context& _context )
{
	//if( _visitor->Visit( _memory, _type, _context ) )
	{
		switch( _type.m_kind )
		{
		case ETypeKind::Type_Integer :
		case ETypeKind::Type_Float :
		case ETypeKind::Type_Bool :
		case ETypeKind::Type_Enum :
		case ETypeKind::Type_Flags :
			{
				return _visitor->Visit_POD( _memory, _type, _context );
			}
			break;

		case ETypeKind::Type_String :
			{
				String & stringReference = TPODCast< String >::GetNonConst( _memory );
				return _visitor->Visit_String( stringReference, _context );
			}
			break;

		case ETypeKind::Type_Class :
			{
				const mxClass& classType = _type.UpCast< mxClass >();
				return VisitAggregate( _memory, classType, _visitor, _context );
			}
			break;

		case ETypeKind::Type_Pointer :
			{
				const mxPointerType& pointerType = _type.UpCast< mxPointerType >();
				VoidPointer& pointerReference = *static_cast< VoidPointer* >( _memory );
				return _visitor->Visit_Pointer( pointerReference, pointerType, _context );
			}
			break;

		case ETypeKind::Type_AssetId :
			{
				AssetID& assetId = *static_cast< AssetID* >( _memory );
				return _visitor->Visit_AssetId( assetId, _context );
			}
			break;

		case ETypeKind::Type_ClassId :
			{
				SClassId * classId = static_cast< SClassId* >( _memory );
				return _visitor->Visit_TypeId( classId, _context );
			}
			break;

		case ETypeKind::Type_UserData :
			{
				const mxUserPointerType& userPointerType = _type.UpCast< mxUserPointerType >();
				return _visitor->Visit_UserPointer( _memory, userPointerType, _context );
			}
			break;

		case ETypeKind::Type_Blob :
			{
				Unimplemented;
			}
			break;

		case ETypeKind::Type_Array :
			{
				const mxArray& arrayType = _type.UpCast< mxArray >();
				return VisitArray( _memory, arrayType, _visitor, _context );
			}
			break;

			mxNO_SWITCH_DEFAULT;
		}//switch
	}
}
void Walker2::Visit( void * _memory, const mxType& _type, AVisitor2* _visitor, void *_userData )
{
	mxASSERT_PTR(_memory);

	AVisitor2::Context	context;
	context.userData = _userData;

	Visit(_memory,_type,_visitor, context);
}

void Walker2::VisitArray( void * _array, const mxArray& _type, AVisitor2* _visitor, const AVisitor2::Context& _context )
{
	mxASSERT_PTR(_array);
	if( _visitor->Visit_Array(_array,_type,_context) )
	{
		const UINT numObjects = _type.Generic_Get_Count( _array );
		const void* arrayBase = _type.Generic_Get_Data( _array );

		const mxType& itemType = _type.m_itemType;
		const UINT32 itemStride = _type.m_itemSize;

#if MX_DEBUG_REFLECTION
		DBGOUT("[Reflector]: Visit array of '%s' (%u items)\n", itemType.m_name, numObjects);
#endif // MX_DEBUG_REFLECTION

#if MX_DEBUG
		enum { DBG_MAX_ARRAY_CONTENTS_SIZE = 4*mxMEGABYTE };
		const UINT arrayContentsSize = itemStride * numObjects;
		mxASSERT( arrayContentsSize < DBG_MAX_ARRAY_CONTENTS_SIZE );
#endif // MX_DEBUG

		AVisitor2::Context	itemContext( _context.depth + 1 );
		itemContext.parent = &_context;
		itemContext.userData = _context.userData;

		for( UINT32 iObject = 0; iObject < numObjects; iObject++ )
		{
			const MetaOffset itemOffset = iObject * itemStride;
			void* itemPtr = mxAddByteOffset( c_cast(void*)arrayBase, itemOffset );

			//_visitor->Visit_Array_Item( itemPtr, itemType, iObject, itemContext );

			Visit( itemPtr, itemType, _visitor, itemContext );
		}
	}
}

void Walker2::VisitAggregate( void * _struct, const mxClass& _type, AVisitor2* _visitor, const AVisitor2::Context& _context )
{
	mxASSERT_PTR(_struct);
#if MX_DEBUG_REFLECTION
	DBGOUT("[Reflector]: Visit object of class '%s'\n", type.m_name);
#endif // MX_DEBUG_REFLECTION

	if( _visitor->Visit_Class(_struct,_type,_context) )
	{
		// First recursively visit the parent classes.
		const mxClass* parentType = _type.GetParent();
		while( parentType != nil )
		{
			VisitStructFields( _struct, *parentType, _visitor, _context );
			parentType = parentType->GetParent();
		}

		// Now visit the members of this class.
		VisitStructFields( _struct, _type, _visitor, _context );
	}
}

void Walker2::VisitStructFields( void * _struct, const mxClass& _type, AVisitor2* _visitor, const AVisitor2::Context& _context )
{
	const mxClassLayout& layout = _type.GetLayout();
	for( UINT fieldIndex = 0 ; fieldIndex < layout.numFields; fieldIndex++ )
	{
		const mxField& field = layout.fields[ fieldIndex ];
		
		AVisitor2::Context	fieldContext( _context.depth + 1 );
		fieldContext.parent = &_context;
		fieldContext.member = &field;
		fieldContext.userData = _context.userData;

#if MX_DEBUG_REFLECTION
		DBGOUT("[Reflector]: Visit field '%s' of type '%s' in '%s' at offset '%u'\n",
			field.name, field.type.m_name.ToPtr(), _type.m_name.ToPtr(), field.offset);
#endif // MX_DEBUG_REFLECTION

		void* memberVarPtr = mxAddByteOffset( _struct, field.offset );

		if( _visitor->Visit_Field( memberVarPtr, field, fieldContext ) )
		{
			Visit( memberVarPtr, field.type, _visitor, fieldContext );
		}
	}
}

bool TellNotToFreeMemory::Visit_Array( void * _array, const mxArray& _type, const Context& _context )
{
	_type.SetDontFreeMemory( _array );
	return true;
}
void TellNotToFreeMemory::Visit_String( String & _string, const Context& _context )
{
	_string.DoNotFreeMemory();
}

void MarkMemoryAsExternallyAllocated( void* _memory, const mxClass& _type )
{
	// Tell the objects not to deallocated the buffer memory.
	// (As a side effect it also warms up the data cache.)
	TellNotToFreeMemory		markMemoryAsExternallyAllocated;
	Reflection::Walker2::Visit( _memory, _type, &markMemoryAsExternallyAllocated );
}

}//namespace Reflection

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
