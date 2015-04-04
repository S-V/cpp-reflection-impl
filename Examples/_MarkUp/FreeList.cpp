/*
=============================================================================
	File:	FreeListAllocator.cpp
	Desc:
=============================================================================
*/
#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Memory/FreeList/FreeList.h>

/*
--------------------------------------------------------------
	FixedListAllocator
--------------------------------------------------------------
*/
mxDEFINE_CLASS( FixedListAllocator );
mxBEGIN_REFLECTION( FixedListAllocator )
	mxMEMBER_FIELD( m_firstFree ),
	mxMEMBER_FIELD( m_start ),
	mxMEMBER_FIELD( m_size ),
	//mxMEMBER_FIELD( m_end ),
mxEND_REFLECTION

FixedListAllocator::FixedListAllocator()
{
	this->Clear();
}

FixedListAllocator::~FixedListAllocator()
{
}

void FixedListAllocator::Initialize( void* memory, int itemSize, int maxItems )
{
	mxASSERT_PTR(memory);
	mxASSERT(itemSize > 0);
	mxASSERT(maxItems > 0);
	//mxASSERT(m_start == NULL);

	m_firstFree = memory;
	m_start = (BYTE*)memory;
	m_size = maxItems * itemSize;
	//m_end = m_start + maxItems * itemSize;
	//m_num = maxItems;

	void* p = m_start;
	int count = maxItems;

	while( --count )
	{
		void* next = mxAddByteOffset( p, itemSize );
		*(void**)p = next;
		p = next;
	}
	*(void**)p = NULL;
}

void FixedListAllocator::Clear()
{
	m_firstFree = NULL;
	m_start = NULL;
	m_size = 0;
	//m_end = NULL;
	//m_num = 0;
}

void* FixedListAllocator::Allocate()
{
	if( !m_firstFree ) {
		return NULL;
	}
	void* result = m_firstFree;
	m_firstFree = *(void**)m_firstFree;
	return result;
}

bool FixedListAllocator::Deallocate( void* item )
{
	mxASSERT(this->HasAddress(item));
	*(void**)item = m_firstFree;
	m_firstFree = item;
	return true;
}

// frees the given element and inserts it into the sorted free list;
// use this if the free list should be sorted by increasing addresses
void FixedListAllocator::ReleaseSorted( void* item )
{
	// iterate over the dead elements in the free list and find the correct position for insertion

	void* previous = NULL;
	void* current = m_firstFree;

	while( current != NULL )
	{
		mxASSERT(current != item);

		if( current > item ) {
			break;
		}

		previous = current;
		current = *(void**)current;
	}

	if( previous != NULL )
	{
		*(void**)item = *(void**)previous;	// item->next = previous->next
		*(void**)previous = item;			// previous->next = item
	}
	else
	{
		*(void**)item = m_firstFree;
		m_firstFree = item;
	}

#if MX_DEBUG
	for( void *previous = NULL, *current = m_firstFree; current != NULL; current = *(void**)current )
	{
		mxASSERT2(current > previous, "The free list should be sorted by increasing object addresses!");
		previous = current;
	}
#endif
}

bool FixedListAllocator::ItemInFreeList( const void* item ) const
{
	const void* current = m_firstFree;
	while(PtrToBool(current))
	{
		mxASSERT(this->HasAddress(current));
		if( current == item )
		{
			return true;	// the item has been marked as free
		}
		current = *(void**) current;
	}
	return false;
}

namespace FreeListUtil
{
	void AddItemsToFreeList( void** freeListHead, UINT itemSize, void* itemsArray, UINT numItems )
	{
		mxASSERT_PTR(freeListHead);
		mxASSERT(itemSize > 0);
		mxASSERT_PTR(itemsArray);
		mxASSERT(numItems > 0);

		void* p = itemsArray;
		while( --numItems )
		{
			void* next = mxAddByteOffset( p, itemSize );
			*(void**)p = next;
			p = next;
		}
		// set the last item's next pointer to m_firstFree
		*(void**)p = *freeListHead;
		// set m_firstFree to the first item
		*freeListHead = itemsArray;
	}
}//namespace FreeListUtil

/*
--------------------------------------------------------------
	FreeListAllocator
--------------------------------------------------------------
*/
enum { MIN_ITEM_SIZE = sizeof(void*) + sizeof(UINT32) };
enum { MIN_ALIGNMENT = sizeof(void*) };

FreeListAllocator::FreeListAllocator()
{
	m_firstFree = NULL;
	m_stride = 0;
	m_blockSize = 0;
	m_alignment = EFFICIENT_ALIGNMENT;
	m_allocatedBlocks = NULL;
}

FreeListAllocator::~FreeListAllocator()
{
	this->ReleaseMemory();
}

// if 'blockSize' is 0, then we cannot dynamically allocate new memory blocks
bool FreeListAllocator::Initialize( UINT itemSize, UINT blockSize )
{
	chkRET_FALSE_IF_NOT( itemSize >= MIN_ITEM_SIZE );
	chkRET_FALSE_IF_NOT( m_allocatedBlocks == NULL );

	//m_stride = AlignUp( itemSize, m_alignment );
	m_stride = itemSize;

	m_blockSize = blockSize;

	return true;
}

bool FreeListAllocator::ReleaseMemory()
{
	SMemoryBlock* current = m_allocatedBlocks;
	while(PtrToBool(current))
	{
		SMemoryBlock* next = current->_next;
		mxFree(current);
		current = next;
	}
	m_allocatedBlocks = NULL;
	m_alignment = 0;
	m_blockSize = 0;
	m_stride = 0;
	m_firstFree = NULL;

	return true;
}

UINT FreeListAllocator::GetAlignedItemSize() const
{
	//alignedItemSize = AlignUp( itemSize, m_alignment );
	return m_stride;
}

void* FreeListAllocator::AllocateItem()
{
	// if no free items available...
	if(!PtrToBool(m_firstFree))
	{
		// ...allocate a new memory block and add its items to the free list
		void* newItemsArray = this->AllocateNewBlock( m_blockSize );
		this->AddItemsToFreeList( newItemsArray, m_blockSize );
	}

	mxASSERT_PTR(m_firstFree);

	void* result = m_firstFree;

	m_firstFree = *(void**)m_firstFree;

	if(MX_DEBUG)
	{
		memset(result, mxDBG_UNINITIALIZED_MEMORY_TAG, m_stride);
	}

	return result;
}

bool FreeListAllocator::ReleaseItem( void* item )
{
	chkRET_FALSE_IF_NIL(item);
	mxASSERT(this->HasItem(item));

	if(MX_DEBUG)
	{
		memset(item, mxDBG_UNINITIALIZED_MEMORY_TAG, m_stride);
	}

	*(void**)item = m_firstFree;
	m_firstFree = item;

	return true;
}

// grabs a new memory block of the given size
//NOTE: doesn't add all its items to the free list
//
void* FreeListAllocator::AllocateNewBlock( UINT numItems )
{
	chkRET_NIL_IF_NOT(numItems > 0);

	//DBGOUT("FreeList: allocating a new block (%u elements)\n", numItems);

	const UINT blockHeaderSize = AlignUp( sizeof SMemoryBlock, m_alignment );
	const UINT blockDataSize = m_stride * numItems;

	void* newBlockStorage = mxAlloc( blockHeaderSize + blockDataSize );
	chkRET_NIL_IF_NIL(newBlockStorage);

	void* blockDataPtr = mxAddByteOffset( newBlockStorage, blockHeaderSize );

	SMemoryBlock* newBlock = new (newBlockStorage) SMemoryBlock();

	newBlock->m_start = (BYTE*)blockDataPtr;
	newBlock->m_count = numItems;
	newBlock->PrependSelfToList( &m_allocatedBlocks );

	return blockDataPtr;
}

bool FreeListAllocator::AddItemsToFreeList( void* items, UINT count )
{
	chkRET_FALSE_IF_NIL(items);
	chkRET_FALSE_IF_NOT(count > 0);

	FreeListUtil::AddItemsToFreeList( &m_firstFree, m_stride, items, count );

	return true;
}

// returns true if the given item is currently free
bool FreeListAllocator::ItemInFreeList( const void* item ) const
{
	const void* current = m_firstFree;
	while(PtrToBool(current))
	{
		if( current == item )
		{
			return true;
		}
		current = *(void**) current;
	}
	return false;
}

// returns true if the given item was allocated from my memory blocks
bool FreeListAllocator::HasItem( const void* item ) const
{
	chkRET_FALSE_IF_NIL(item);
	SMemoryBlock* current = m_allocatedBlocks;
	while(PtrToBool(current))
	{
		if( this->HasAddress(current, item) )
		{
			const UINT32 byteOffset = mxGetByteOffset32( current->m_start, item );
			mxASSERT( byteOffset % m_stride == 0 );
			return true;
		}
		current = current->_next;
	}
	return false;
}

bool FreeListAllocator::HasAddress( const void* ptr ) const
{
	SMemoryBlock* current = m_allocatedBlocks;
	while(PtrToBool(current))
	{
		if( this->HasAddress( current, ptr ) )
		{
			return true;
		}
		current = current->_next;
	}
	return false;
}

bool FreeListAllocator::HasAddress( SMemoryBlock* block, const void* ptr ) const
{
	const BYTE* start = block->m_start;
	const BYTE* end = mxAddByteOffset( block->m_start, block->m_count * m_stride );
	const BYTE* bytePtr = (const BYTE*)ptr;
	return (bytePtr >= start) && (bytePtr < end);
}

FreeListAllocator::SMemoryBlock::Head FreeListAllocator::GetAllocatedBlocksList() const
{
	return m_allocatedBlocks;
}

UINT FreeListAllocator::CalculateAllocatedMemorySize() const
{
	UINT result = 0;

	const UINT blockHeaderSize = AlignUp( sizeof SMemoryBlock, m_alignment );

	SMemoryBlock* current = m_allocatedBlocks;
	while(PtrToBool(current))
	{
		const UINT blockDataSize = current->m_count * m_stride;

		result += blockHeaderSize + blockDataSize;

		current = current->_next;
	}
	return result;
}

void FreeListAllocator::SetGranularity( UINT newBlockSize )
{
	chkRET_IF_NOT( newBlockSize > 0 );
	m_blockSize = newBlockSize;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
