/*
=============================================================================
	File:	TFreeList.h
	Desc:
=============================================================================
*/
#pragma once

#include <Base/Template/Containers/LinkedList/TSinglyLinkedList.h>

/*
--------------------------------------------------------------
	FixedListAllocator

	Fast allocator for fixed size memory blocks
	based on a non-growing free list.
	NOTE: Doesn't own used memory.
	NOTE: No destructor / constructor is called for the type !
--------------------------------------------------------------
*/
struct FixedListAllocator : CStruct
{
	void *	m_firstFree;// pointer to the first free element
	BYTE *	m_start;	// pointer to the start of the memory block
	UINT32	m_size;		// total size of the memory block
	//BYTE *	m_end;		// pointer to the end of the memory block
	//UINT	m_num;		// number of allocated elements in the memory block

public:
	mxDECLARE_CLASS(FixedListAllocator,CStruct);
	mxDECLARE_REFLECTION;
	FixedListAllocator();
	~FixedListAllocator();

	void Initialize( void* memory, int itemSize, int maxItems );
	void Clear();

	void* Allocate();
	bool Deallocate( void* item );
	void ReleaseSorted( void* item );

	bool ItemInFreeList( const void* item ) const;
	bool HasAddress( const void* ptr ) const;

	void* GetBuffer();
	void* GetFirstFree();
	void** GetFirstFreePtr();
};

inline bool FixedListAllocator::HasAddress( const void* ptr ) const
{
	mxASSERT_PTR(ptr);
	return mxPointerInRange( ptr, m_start, m_size );
	//return mxPointerInRange( ptr, m_start, m_end );
}

inline void* FixedListAllocator::GetBuffer()
{
	return m_start;
}

inline void* FixedListAllocator::GetFirstFree()
{
	return m_firstFree;
}

inline void** FixedListAllocator::GetFirstFreePtr()
{
	return &m_firstFree;
}

/*
--------------------------------------------------------------
	TStaticFreeList< TYPE, MAX_COUNT >
	non-growing free list allocator
--------------------------------------------------------------
*/
template< typename TYPE, const UINT MAX_COUNT >
class TStaticFreeList
{
	FixedListAllocator	m_allocator;
	UINT32				m_allocated;
	TYPE				m_storage[ MAX_COUNT ];

public:
	TStaticFreeList()
	{
		m_allocator.Initialize( m_storage, sizeof(TYPE), MAX_COUNT );
		m_allocated = 0;
	}
	~TStaticFreeList()
	{
		mxASSERT(m_allocated == 0);
	}
	TYPE* Allocate()
	{
		TYPE* o = (TYPE*)m_allocator.Allocate();
		if( o ) {
			m_allocated++;
			new(o) TYPE();
		}
		return o;
	}
	void Deallocate( TYPE* o )
	{
		o->~TYPE();
		m_allocator.Deallocate( o );
		m_allocated--;
	}
	UINT32 NumAllocated() const
	{
		return m_allocated;
	}
};

union UListHead
{
	UListHead *	next;	// pointer to next element in a list

#if MX_DEBUG
	// for inspecting memory in debugger
	char	dbg[64];
#endif // MX_DEBUG

private:
	UListHead() {}
};

namespace FreeListUtil
{
	void AddItemsToFreeList( void** freeListHead, UINT itemSize, void* itemsArray, UINT numItems );

}//namespace FreeListUtil

/*
--------------------------------------------------------------
	FreeListAllocator
	Dynamically growing free list allocator.
	No destructor / constructor is called for the type !
--------------------------------------------------------------
*/
class FreeListAllocator
{
	struct SMemoryBlock : public TSinglyLinkedList< SMemoryBlock >
	{
		BYTE *	m_start;	// pointer to the array of stored elements
		UINT32	m_count;	// number of elements in this memory block
	};

	void *					m_firstFree;// pointer to the first free element
	UINT16					m_stride;	// aligned size of a single element, in bytes
	UINT16					m_blockSize;	// allocation granularity (minimum allocation block size)
	UINT16					m_alignment;	// alignment of each element, in bytes
	UINT16					m_unusedPadding;
	SMemoryBlock::Head		m_allocatedBlocks;	// linked list of allocated blocks (size of each == m_blockSize)

public:
	FreeListAllocator();
	~FreeListAllocator();

	bool Initialize( UINT itemSize, UINT blockSize );
	bool ReleaseMemory();

	UINT GetAlignedItemSize() const;

	void* AllocateItem();
	bool ReleaseItem( void* item );

	void* AllocateNewBlock( UINT numItems );

	bool AddItemsToFreeList( void* items, UINT count );

	bool ItemInFreeList( const void* item ) const;
	bool HasItem( const void* item ) const;
	bool HasAddress( const void* ptr ) const;
	bool HasAddress( SMemoryBlock* block, const void* ptr ) const;

	SMemoryBlock::Head GetAllocatedBlocksList() const;
	UINT CalculateAllocatedMemorySize() const;

	void SetGranularity( UINT newBlockSize );

public:
	template< typename TYPE >
	TYPE* Create()
	{
		mxASSERT(m_stride >= sizeof TYPE);
		void* storage = this->AllocateItem();
		chkRET_NIL_IF_NIL(storage);
		return new(storage) TYPE();
	}

	template< typename TYPE >
	bool Destroy( TYPE * item )
	{
		mxASSERT_PTR(item);
		mxASSERT(this->HasItem(item));
		item->~TYPE();
		return this->ReleaseItem( item );
	}

private:PREVENT_COPY(FreeListAllocator);
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
