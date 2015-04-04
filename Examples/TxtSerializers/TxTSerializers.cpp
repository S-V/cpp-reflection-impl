/*
=============================================================================
=============================================================================
*/
#include <Base/Base.h>
#include <Base/Object/Reflection.h>
#include <Base/Template/Containers/HashMap/TPointerMap.h>
#include <Core/Core.h>
#include <Core/Asset.h>
#include <Core/ObjectModel.h>
#include <Core/Util/ScopedTimer.h>
#include <TxTSupport/TxTCommon.h>
#include <TxTSupport/TxTReader.h>
#include <TxTSupport/TxTWriter.h>
#include <TxTSupport/TxTSerializers.h>

#define ENABLE_ASSET_LOADING	(0)

namespace SON
{

const char* CHUNK_TAG = "_CHUNK";
const char* HEADER_TAG = "_HEAD";

const char* NODE_TYPE_TAG = "_TYPE";
const char* NODE_DATA_TAG = "_DATA";
#if 0
	const char* CLASS_NAME_TAG = "_CLASS";
	const char* CLASS_DATA_TAG = "_VALUE";
	const char* BASE_CLASS_TAG = "_SUPER";
#else
	const char* CLASS_NAME_TAG = "_TYPE";
	const char* CLASS_DATA_TAG = "_DATA";
	const char* BASE_CLASS_TAG = "_BASE";
#endif
const char* ASSET_GUID_TAG = "_ASSET_GUID";
const char* ASSET_PATH_TAG = "_ASSET_PATH";
const char* ASSET_TYPE_TAG = "_ASSET_TYPE";

//const char* OBJECT_CLASS_TAG = "_CLASS";
//const char* OBJECT_INDEX_TAG = "_INDEX";
//const char* OBJECT_ARRAY_TAG = "_ARRAY";
//const char* OBJECT_ITEMS_TAG = "_ITEMS";
//const char* OBJECT_COUNT_TAG = "_COUNT";

const char* OBJECT_CLASS_TAG = "_TYPE";
const char* OBJECT_INDEX_TAG = "_ID";
const char* OBJECT_ITEMS_TAG = "_DATA";
const char* OBJECT_COUNT_TAG = "_SIZE";

const int NULL_POINTER_TAG = -1;
const int FALLBACK_INSTANCE_TAG = -2;

static inline void PutInteger( const Node* _source, const UINT _byteWidth, void *_pointer )
{
	const double value = AsDouble( _source );
	if( _byteWidth == 1 ) {
		*(INT8*)_pointer = value;
	}
	else if( _byteWidth == 2 ) {
		*(INT16*)_pointer = value;
	}
	else if( _byteWidth == 4 ) {
		*(INT32*)_pointer = value;
	}
	else if( _byteWidth == 8 ) {
		*(INT64*)_pointer = value;
	}
	else {
		mxUNREACHABLE;
	}
}

static inline void PutFloat( const Node* _source, const UINT _byteWidth, void *_pointer )
{
	const double value = AsDouble( _source );
	if( _byteWidth == 4 ) {
		*(float*)_pointer = value;
	}
	else if( _byteWidth == 8 ) {
		*(double*)_pointer = value;
	}
	else {
		mxUNREACHABLE;
	}
}

static void SON_To_Flags( const Node* _sourceNode, const mxFlagsType& _type, void *_flags )
{
	mxASSERT(_sourceNode->tag.type == TypeTag_List);

	mxFlagsType::Mask integerValue = 0;

	const Node* flagValue = _sourceNode->value.l.kids;
	while( flagValue )
	{
		const char* valueName = SON::AsString( flagValue );
		const mxFlagsType::Mask mask = _type.GetItemValueByName( valueName );
		integerValue |= mask;
		flagValue = flagValue->next;
	}

	_type.m_accessor.Set_Value( _flags, integerValue );
}

static Node* Flags_To_SON( const void* _flags, const mxFlagsType& _type, Allocator & _allocator )
{
	const UINT currVal = _type.m_accessor.Get_Value( _flags );
	const UINT numBits = _type.m_numFlags;
	Node* listNode = NewList( _allocator );
	for( UINT i = 0; i < numBits; i++ )
	{
		const mxFlagsType::Member& flag = _type.m_members[ i ];
		const bool flagIsSet = ( currVal & flag.mask );
		if( flagIsSet )
		{
			Node* flagValue = NewString( flag.name, _allocator );
			AddChild( listNode, flagValue );
		}
	}
	return listNode;
}

static Node* AssetId_To_SON( const AssetID& _o, Allocator & _allocator )
{
	return NewString( _o.d.ToPtr(), _o.d.Length(), _allocator );
}

static AssetID SON_To_AssetId( const Node* _sourceNode )
{
	mxASSERT_PTR(_sourceNode);
	mxASSERT(_sourceNode->tag.type == TypeTag_String);
	const char* assetPath = AsString(_sourceNode);
	if( strcmp(assetPath, "") == 0 ) {
		return AssetId_GetNull();
	} else {
		AssetID assetId;
		assetId.d = NameID( assetPath );
		mxASSERT( AssetId_IsValid( assetId ) );
		return assetId;
	}
}

class Decoder : public Reflection::AVisitor
{
public:
	typedef Reflection::AVisitor Super;

	//-- Reflection::AVisitor
	virtual void* Visit_Field( void * _o, const mxField& _field, void* _userData ) override
	{
		const SON::Node* parentNode = static_cast< Node* >( _userData );
		const SON::Node* fieldValue = FindValue( parentNode, _field.name );
		if( fieldValue ) {
			return Super::Visit_Field( _o, _field, (void*)fieldValue );
		} else {
			//ptWARN("Missing struct field: '%s' in '%s'\n", _field.name,parentNode->name);
			return nil;
		}
	}
	virtual void* Visit_POD( void * _o, const mxType& _type, void* _userData ) override
	{
		const Node* sourceNode = static_cast< Node* >( _userData );

		switch( _type.m_kind )
		{
		case ETypeKind::Type_Integer :
			PutInteger( sourceNode, _type.m_size, _o );
			break;

		case ETypeKind::Type_Float :
			PutFloat( sourceNode, _type.m_size, _o );
			break;

		case ETypeKind::Type_Bool :
			{
				const bool value = AsBoolean( sourceNode );
				TPODCast< bool >::GetNonConst( _o ) = value;
			}			
			break;

		case ETypeKind::Type_Enum :
			{
				const mxEnumType& enumType = _type.UpCast< mxEnumType >();
				const char* sEnumValue = AsString( sourceNode );
				const UINT nEnumValue = enumType.GetValueByString(sEnumValue);
				enumType.m_accessor.Set_Value( _o, nEnumValue );
			}
			break;

		case ETypeKind::Type_Flags :
			{
				const mxFlagsType& flagsType = _type.UpCast< mxFlagsType >();
				SON_To_Flags( sourceNode, flagsType, _o );
			}
			break;

			mxNO_SWITCH_DEFAULT;
		}
		return _userData;
	}
	virtual void* Visit_TypeId( SClassId * _o, void* _userData ) override
	{
		const Node* sourceNode = static_cast< Node* >( _userData );
		mxASSERT(sourceNode->tag.type == TypeTag_String);
		const char* className = sourceNode->value.s.start;
		_o->type = TypeRegistry::Get().FindClassByName( className );
		return _userData;
	}
	virtual void* Visit_String( String & _string, void* _userData ) override
	{
		const Node* sourceNode = static_cast< Node* >( _userData );
		mxASSERT(sourceNode->tag.type == TypeTag_String);
		const char* data = sourceNode->value.s.start;
		const UINT32 len = sourceNode->value.s.length;
		Str::CopyS( _string, data, len );
		return _userData;
	}
	virtual void* Visit_AssetId( AssetID & _assetId, void* _userData ) override
	{
		const Node* sourceNode = static_cast< Node* >( _userData );
		mxASSERT(sourceNode->tag.type == TypeTag_String);
		const char* assetId = sourceNode->value.s.start;
		_assetId.d = NameID( assetId );
		return _userData;
	}
	virtual void* Visit_Pointer( VoidPointer& p, const mxPointerType& _type, void* _userData ) override
	{
		Unimplemented;
		return _userData;
	}
	virtual void* Visit_UserPointer( void * _o, const mxUserPointerType& _type, void* _userData ) override
	{
		Unimplemented;
		return _userData;
	}
	//virtual void* Visit_Aggregate( void * _o, const mxClass& _type, void* _userData ) override
	//{
	//	const Node* sourceNode = static_cast< Node* >( _userData );
	//	mxUNUSED(sourceNode);
	//	return Super::Visit_Aggregate( _o, _type, _userData );
	//}
	virtual void* Visit_Array( void * _array, const mxArray& _type, void* _userData ) override
	{
		const Node* sourceNode = static_cast< Node* >( _userData );
		mxASSERT(sourceNode->tag.type == TypeTag_Array);

		const UINT32 arraySize = sourceNode->value.a.size;
		_type.Generic_Set_Count( _array, arraySize );

		const void* arrayBase = _type.Generic_Get_Data( _array );

		const mxType& itemType = _type.m_itemType;
		const UINT32 itemSize = _type.m_itemSize;

		for( UINT itemIndex = 0; itemIndex < arraySize; itemIndex++ )
		{
			const Node* itemValue = sourceNode->value.a.kids + itemIndex;

			const MetaOffset itemOffset = itemIndex * itemSize;
			void* itemPointer = mxAddByteOffset( c_cast(void*)arrayBase, itemOffset );

			Reflection::Walker::Visit( itemPointer, itemType, this, (void*)itemValue );
		}
		return _userData;
	}
	virtual void* Visit_Blob( void * _blob, const mxBlobType& _type, void* _userData ) override
	{
		Unimplemented;
		return _userData;
	}
};

ERet LoadFromBuffer(
	char* _text, int _size,
	void *_o, const mxType& _type,
	const char* _file, int _line
)
{
	SON::Parser		parser;
	parser.buffer = _text;
	parser.length = _size;
	parser.line = _line;
	parser.file = _file;

	SON::Allocator	allocator;
	SON::Node* root = SON::ParseBuffer(parser, allocator);
	chkRET_X_IF_NIL(root, ERR_FAILED_TO_PARSE_DATA);
	chkRET_X_IF_NOT(parser.errorCode == 0, ERR_FAILED_TO_PARSE_DATA);

	SON::Decoder	decoder;

	Reflection::Walker::Visit( _o, _type, &decoder, root );

	return ALL_OK;
}

ERet LoadFromStream(
	AStreamReader& _stream,
	void *_o, const mxType& _type,
	const char* _file, int _line
)
{
	const size_t size = _stream.GetSize();

	ScopedStackAlloc	tempAlloc( gCore.frameAlloc );
	char* buffer = (char*) tempAlloc.Alloc( size );
	mxDO(_stream.Read( buffer, size ));

	mxDO(LoadFromBuffer(buffer, size, _o, _type, _file, _line));

	return ALL_OK;
}

class Encoder : public Reflection::AVisitor {
protected:
	SON::Allocator &	m_allocator;
public:
	typedef Reflection::AVisitor Super;

	Encoder( SON::Allocator & allocator )
		: m_allocator( allocator )
	{
	}
	//-- Reflection::AVisitor
	virtual void* Visit_Field( void * _o, const mxField& _field, void* _userData ) override
	{
		SON::Node* parentNode = static_cast< Node* >( _userData );
		SON::Node* fieldValue = static_cast< Node* >( Super::Visit_Field( _o, _field, _userData ) );
		if( fieldValue ) {
			SON::AddChild( parentNode, _field.name, fieldValue );
		} else {
			ptWARN("SON: Failed to convert field: '%s'\n", _field.name);
		}
		return fieldValue;
	}
	virtual void* Visit_POD( void * _o, const mxType& _type, void* _userData ) override
	{
		switch( _type.m_kind )
		{
		case ETypeKind::Type_Integer :
			{
				const INT64 value = GetInteger( _o, _type.m_size );
				return NewNumber( value, m_allocator );
			}
			break;

		case ETypeKind::Type_Float :
			{
				const double value = GetDouble( _o, _type.m_size );
				return NewNumber( value, m_allocator );
			}
			break;

		case ETypeKind::Type_Bool :
			{
				const bool value = TPODCast< bool >::GetConst( _o );
				return NewBoolean( value, m_allocator );
			}
			break;

		case ETypeKind::Type_Enum :
			{
				const mxEnumType& enumType = _type.UpCast< mxEnumType >();
				const UINT32 enumValue = enumType.m_accessor.Get_Value( _o );
				const char* valueName = enumType.GetStringByValue(enumValue);
				return NewString( valueName, m_allocator );
			}
			break;

		case ETypeKind::Type_Flags :
			{
				const mxFlagsType& flagsType = _type.UpCast< mxFlagsType >();
				return Flags_To_SON( _o, flagsType, m_allocator );
			}
			break;

			mxNO_SWITCH_DEFAULT;
		}
		return nil;
	}
	virtual void* Visit_TypeId( SClassId * _o, void* _userData ) override
	{
		const mxType& type = *_o->type;
		return NewString( type.m_name.buffer, type.m_name.length, m_allocator );
	}
	virtual void* Visit_String( String & _string, void* _userData ) override
	{
		return NewString( _string.SafeGetPtr(), _string.Length(), m_allocator );
	}
	virtual void* Visit_AssetId( AssetID & _assetId, void* _userData ) override
	{
		return AssetId_To_SON( _assetId, m_allocator );
	}
	virtual void* Visit_Pointer( VoidPointer& p, const mxPointerType& _type, void* _userData ) override
	{
		if( p.o != NULL )
		{
			Unimplemented;
			//NOTE: we must allocate strings dynamically:
			//String256 temp;
			//Str::SPrintF( temp, "0x%p", p.o );
			//return NewString( temp.ToPtr(), temp.Length(), m_allocator );
			return NewNumber( (size_t)p.o, m_allocator );
		}
		else
		{
			return NewNumber( 0, m_allocator );
		}		
	}
	virtual void* Visit_UserPointer( void * _o, const mxUserPointerType& _type, void* _userData ) override
	{
		Unimplemented;
		return _userData;
	}
	virtual void* Visit_Aggregate( void * _o, const mxClass& _type, void* _userData ) override
	{
		Node* objectNode = NewObject( m_allocator );
		Super::Visit_Aggregate( _o, _type, objectNode );
		return objectNode;
	}
	virtual void* Visit_Array( void * _array, const mxArray& _type, void* _userData ) override
	{
#if 1
		const UINT32 arraySize = _type.Generic_Get_Count( _array );
		const void* arrayBase = _type.Generic_Get_Data( _array );
		const mxType& itemType = _type.m_itemType;
		const UINT32 itemSize = _type.m_itemSize;

		Node* arrayNode = NewArray( arraySize, m_allocator );
		arrayNode->value.a.size = arraySize;

		for( UINT itemIndex = 0; itemIndex < arraySize; itemIndex++ )
		{
			const MetaOffset itemOffset = itemIndex * itemSize;
			const void* itemData = mxAddByteOffset( c_cast(void*)arrayBase, itemOffset );

			Node* itemValue = arrayNode->value.a.kids + itemIndex;
			// release the node so that it will be allocated first next time
			m_allocator.ReleaseNode(itemValue);

			Node* returnedValue = (Node*) Reflection::Walker::Visit( (void*)itemData, itemType, this, arrayNode );
			mxASSERT(itemValue == returnedValue);
		}

		return arrayNode;
#else
		const UINT32 arraySize = _type.Generic_Get_Count( _array );
		const void* arrayBase = _type.Generic_Get_Data( _array );
		const mxType& itemType = _type.m_itemType;
		const UINT32 itemSize = _type.m_itemSize;

		Node* listNode = NewList( m_allocator );

		for( UINT itemIndex = 0; itemIndex < arraySize; itemIndex++ )
		{
			const MetaOffset itemOffset = itemIndex * itemSize;
			const void* itemData = mxAddByteOffset( c_cast(void*)arrayBase, itemOffset );

			Node* itemValue = (Node*) Reflection::Walker::Visit( (void*)itemData, itemType, this, listNode );
			AddChild( listNode, itemValue );
		}

		return listNode;
#endif
	}
	virtual void* Visit_Blob( void * _blob, const mxBlobType& _type, void* _userData ) override
	{
		Unimplemented;
		return _userData;
	}
};

ERet Decode( const Node* _root, const mxType& _type, void *_o )
{
	mxASSERT_PTR(_root);
	SON::Decoder	decoder;
	Reflection::Walker::Visit( _o, _type, &decoder, (void*)_root );
	return ALL_OK;
}

Node* Encode( const void* _o, const mxType& _type, Allocator & _allocator )
{
//	ScopedTimer		timer( "SON::Encode" );
	SON::Encoder	encoder( _allocator );
	SON::Node *		root = (Node*) Reflection::Walker::Visit( (void*)_o, _type, &encoder );
	return root;
}

ERet SaveToStream(
				  const void* _o, const mxType& _type,
				  AStreamWriter &_stream
				  )
{
	Allocator	allocator;

	SON::Node *	root = Encode(_o, _type, allocator);

	chkRET_X_IF_NIL(root, ERR_UNKNOWN_ERROR);

	mxDO(SON::WriteToStream(root, _stream));

	return ALL_OK;
}

//PointerInfo
// used for mapping pointers to numerical ids during serialization
struct ObjectInfo
{
	void *			o;		// pointer to the object data stored in the clump
	const mxClass *	type;
	UINT32			index;	// unique (sequential) id of this object
};
typedef TPointerMap< ObjectInfo, mxPointerHasher >	ObjectMap;

typedef TArray< const ObjectList* >						ObjectListsArray;
typedef THashMap< const mxClass*, ObjectListsArray >	ObjectListsByType;
typedef TArray< AObject* >								PolymorphicObjects;

static void CollectObjects( const Clump& _clump, ObjectMap &_map )
{
	_map.Empty();

	THashMap< TypeID, UINT32 >	instanceCount;

	ObjectList::Iterator it( _clump.GetObjectLists() );
	while( it.IsValid() )
	{
		ObjectList& objectList = it.Value();
		if( objectList.Num() > 0 )
		{
			const TypeID typeId = objectList.GetType().GetTypeID();

			UINT* pCount = instanceCount.Find( typeId );
			if( pCount == nil ) {
				pCount = &instanceCount.Set( typeId, 0 );
			}

			{
				ObjectList::IteratorBase itemIterator( objectList );
				while( itemIterator.IsValid() )
				{
					void* o = itemIterator.ToVoidPtr();

					ObjectInfo	objectInfo;
					{
						objectInfo.o = o;
						objectInfo.type = &objectList.GetType();
						objectInfo.index = *pCount;
					}

					_map.Set( o, objectInfo );

					(*pCount)++;

					itemIterator.MoveToNext();
				}
			}
		}
		it.MoveToNext();
	}
}

static void CollectObjectLists( const Clump& _clump, ObjectListsByType &_objectLists )
{
	ObjectList::Iterator it( _clump.GetObjectLists() );
	while( it.IsValid() )
	{
		const ObjectList& objectList = it.Value();
		const mxClass* type = &objectList.GetType();

		ObjectListsArray* listsArray = _objectLists.Find( type );
		if( listsArray == nil ) {
			listsArray = &_objectLists.Set( type, ObjectListsArray() );
		}
		listsArray->Add( &objectList );

		it.MoveToNext();
	}
}

// if this is a pointer to some object inside the _clump, then write its index;
// if this is a pointer to an external asset, then write the asset id;
static Node* InternalPointerToJsonValue(
										CStruct* _pointerTarget,
										const mxClass& _pointee,
										const Clump& _clump,
										const ObjectMap& _map,
										Allocator & _allocator
										)
{
	mxASSERT_PTR(_pointerTarget);

	// If the pointer points at some object inside the _clump, then serialize the pointer as an object index.

	const ObjectInfo* objectInfo = _map.Find( _pointerTarget );
	if(PtrToBool( objectInfo ))
	{
		mxASSERT( objectInfo->type->IsDerivedFrom( _pointee ) );
		mxASSERT(objectInfo->index != INDEX_NONE);

		const mxClass& pointeeType = *objectInfo->type;

		Node* pointerNode = NewObject(_allocator);

		Node *	pointeeTypeNode = NewString( pointeeType.m_name.buffer, pointeeType.m_name.length, _allocator );
		Node *	objectIndexNode = NewNumber( objectInfo->index, _allocator );

		AddChild( pointerNode, OBJECT_CLASS_TAG, pointeeTypeNode );
		AddChild( pointerNode, OBJECT_INDEX_TAG, objectIndexNode );

		return pointerNode;
	}

	// Pointer was not found in the given _clump.

#if ENABLE_ASSET_LOADING
	// See if it's a pointer to an asset instance
	// (maybe, pointing to an object from another _clump).
	// If so, serialize the pointer as an external reference.

	const AssetKey* assetReference = Assets::FindByPointer( _pointerTarget );
	if( assetReference != nil )
	{
		if( AssetId_IsNull( assetReference->id ) )
		{
			// this is a reference to the fallback instance
			//mxASSERT( _pointerTarget == _pointee.m_fallback );
		}
		// serialize the pointer as an asset id
		return AssetId_To_SON( assetReference->id, _allocator );
	}
#endif // ENABLE_ASSET_LOADING

	if( _pointerTarget == _pointee.fallbackInstance )
	{
		return NewNumber( FALLBACK_INSTANCE_TAG, _allocator );
	}

	ptWARN("TextClumpWriter: couldn't serialize pointer of type '%s' (target= 0x%x)\n",
		_pointee.GetTypeName(), _pointerTarget);

	mxUNREACHABLE;

	return NewNumber( FALLBACK_INSTANCE_TAG, _allocator );
}

static Node* EncodeObjectStoredInClump(
									   const void* _o,
									   const mxType& _type,
									   const Clump& _clump,
									   const ObjectMap& _objectMap,
									   SON::Allocator & _allocator
									   )
{
	class ObjectEncoder : public SON::Encoder
	{
		const Clump &		m_clump;
		const ObjectMap &	m_objectMap;
	public:
		ObjectEncoder( const Clump& _clump, const ObjectMap& _objectMap, SON::Allocator & _allocator )
			: m_clump( _clump ), m_objectMap( _objectMap ), SON::Encoder( _allocator )
		{}
		virtual void* Visit_Pointer( VoidPointer& p, const mxPointerType& type, void* _userData ) override
		{
			if( !type.pointee.IsClass() ) {
				ptERROR("JSON serializer : Only pointers to CStruct-derived objects are supported!");
				return nil;
			}
			CStruct* target = (CStruct*) p.o;
			if( nil == target ) {
				return NewNumber( FALLBACK_INSTANCE_TAG, m_allocator );
			}
			const mxClass& pointeeBaseClass = type.pointee.UpCast< mxClass >();
			return InternalPointerToJsonValue( target, pointeeBaseClass, m_clump, m_objectMap, m_allocator );
		}
	};
	ObjectEncoder	encoder( _clump, _objectMap, _allocator );
	return (Node*) Reflection::Walker::Visit( (void*)_o, _type, &encoder );
}

ERet SaveClump(
			   const Clump& _clump,
			   AStreamWriter &_stream,
			   const SaveOptions& _options
			   )
{
	// Collect all pointers and object memory blocks.
	ObjectMap	objectMap;
	CollectObjects( _clump, objectMap );

	// Gather all object lists sorted by their type.
	ObjectListsByType	objectLists;
	CollectObjectLists( _clump, objectLists );

	Allocator	allocator;

	Node* root = NewObject(allocator);

	// serialize object lists
	{
		Node *	objectListsNode = NewList(allocator);

		ObjectListsByType::ConstIterator	it( objectLists );
		while( it.IsValid() )
		{
			const ObjectListsArray& objectLists = it.Value();
			const mxClass& type = *it.Key();
			UINT numItemsOfThisType = 0;
			UINT maxItemsOfThisType = 0;
			for( UINT iObjectList = 0; iObjectList < objectLists.Num(); iObjectList++ )
			{
				const ObjectList* objectList = objectLists[ iObjectList ];
				numItemsOfThisType += objectList->Num();
				maxItemsOfThisType += objectList->Capacity();
			}
			if( numItemsOfThisType > 0 )
			{
				Node *	objectListNode = NewObject(allocator);
				Node *	classNameNode = NewString( type.m_name.buffer, type.m_name.length, allocator );
				Node *	instanceCountNode = NewNumber( numItemsOfThisType, allocator );
				Node *	objectListDataNode = NewList(allocator);
				for( UINT i = 0; i < objectLists.Num(); i++ )
				{
					const ObjectList& objectList = *objectLists[ i ];
					ObjectList::IteratorBase	itemIterator( objectList );
					while( itemIterator.IsValid() )
					{
						void* o = itemIterator.ToVoidPtr();
						Node* objectNode = EncodeObjectStoredInClump(o, type, _clump, objectMap, allocator);
						if( objectNode ) {
							mxOPTIMIZE("remove O(N^2)");
							AppendChild( objectListDataNode, objectNode );
						}
						itemIterator.MoveToNext();
					}
				}
				AddChild( objectListNode, OBJECT_ITEMS_TAG, objectListDataNode );
				AddChild( objectListsNode, objectListNode );
				AddChild( objectListNode, OBJECT_COUNT_TAG, instanceCountNode );				
				AddChild( objectListNode, OBJECT_CLASS_TAG, classNameNode );				
			}
			it.MoveToNext();
		}
		AddChild( root, NODE_DATA_TAG, objectListsNode );
	}

	// write header
	{
		Node* header = Encode( PtSessionInfo::CURRENT, allocator );
		AddChild( root, HEADER_TAG, header );
	}

	mxDO(SON::WriteToStream(root, _stream));

	return ALL_OK;
}

ERet SaveClumpToFile( const Clump& _clump, const char* _file )
{
	FileWriter	stream(_file);
	mxDO(SON::SaveClump(_clump, stream));
	return ALL_OK;
}

// populate the empty object list with parsed data
static void LoadObjectList( const Node* _objectListNode, ObjectList &_objectList, Clump & _clump )
{
	mxASSERT(_objectListNode->tag.type == TypeTag_List);

	class ObjectDecoder : public SON::Decoder
	{
		Clump &	m_clump;

	public:
		ObjectDecoder( Clump & _clump )
			: m_clump( _clump )
		{}
		virtual void* Visit_Pointer( VoidPointer& _pointer, const mxPointerType& _type, void* _userData ) override
		{
			if( !_type.pointee.IsClass() )
			{
				ptERROR("JSON deserializer : Only pointers to CStruct-derived objects are supported!");
				return nil;
			}
			const mxClass& pointeeBaseClass = _type.pointee.UpCast< mxClass >();
			const Node*	sourceNode = static_cast< Node* >( _userData );
			mxASSERT_PTR(sourceNode);

			// check if this is a null pointer or a pointer to the fallback instance
			if( sourceNode->tag.type == TypeTag_Number )
			{
				const int integerValue = (int) AsDouble(sourceNode);
				if( integerValue == NULL_POINTER_TAG ) {
					_pointer.o = nil;
				} else {
					mxASSERT(integerValue == FALLBACK_INSTANCE_TAG);
					_pointer.o = pointeeBaseClass.fallbackInstance;
				}
			}
			// check if this is an object id
			else if( sourceNode->tag.type == TypeTag_Object )
			{
				const char* typeName = AsString( FindValue(sourceNode, OBJECT_CLASS_TAG) );
				const mxClass* pointeeClass = TypeRegistry::Get().FindClassByName( typeName );
				mxASSERT_PTR(pointeeClass);
				// NOTE: serialized clumps have unique object lists (1-1 correspondence between class and object list)
				ObjectList* objectList = FindFirstObjectListOfType( m_clump, *pointeeClass );
				mxASSERT_PTR(objectList);
				if( objectList ) {
					const UINT32 objectIndex = (UINT32) AsDouble( FindValue(sourceNode, OBJECT_INDEX_TAG) );
					_pointer.o = objectList->GetItemAtIndex( objectIndex );
				}
			}
#if ENABLE_ASSET_LOADING
			// check if this is an asset id
			else if( sourceNode->tag.type == TypeTag_String )
			{
				const AssetID assetId = SON_To_AssetId( sourceNode );
				const AssetType assetType = &pointeeBaseClass;
				_pointer.o = Assets::FindInstance(AssetKey( assetId, assetType ));
				if( !_pointer.o ) {
					_pointer.o = assetType->fallbackInstance;
				}			
			}
#endif // ENABLE_ASSET_LOADING
			else {
				ptERROR("JSON loader: couldn't resolve pointer of _type '%s'\n", _type.GetTypeName());
			}
			return _userData;
		}
		virtual void* Visit_AssetId( AssetID & o, void* _userData ) override
		{
			const Node*	sourceNode = static_cast< Node* >( _userData );
			mxASSERT_PTR(sourceNode);
			o = SON_To_AssetId( sourceNode );
			return _userData;
		}
	};

	const mxClass& objectType = _objectList.GetType();

	const Node* objectNode = _objectListNode->value.l.kids;
	while( objectNode )
	{
		CStruct* o = _objectList.Allocate();

		// call the default constructor
		objectType.ConstructInPlace( o );

		// deserialize the object
		ObjectDecoder	decoder( _clump );
		Reflection::Walker::Visit( o, objectType, &decoder, (void*)objectNode );

		objectNode = objectNode->next;
	}
}

ERet LoadClump(
		AStreamReader &_stream, Clump &_clump,
		const char* _file, int _line
	)
{
	chkRET_X_IF_NOT( _clump.IsEmpty(), ERR_INVALID_PARAMETER );

	ByteBuffer	fileData;
	mxDO(Util_LoadStreamToBlob(_stream, fileData));

	SON::Parser		parser;
	parser.buffer = fileData.ToPtr();
	parser.length = fileData.GetDataSize();
	parser.file = _file;
	parser.line = _line;

	SON::Allocator	allocator;
	SON::Node* root = SON::ParseBuffer(parser, allocator);
	chkRET_X_IF_NIL(root, ERR_FAILED_TO_PARSE_DATA);

	return LoadClump(root, _clump);
}

ERet LoadClump( const SON::Node* root, Clump &_clump )
{
	// read headerNode
	{
		const Node* headerNode = FindValue( root, HEADER_TAG );

		PtSessionInfo	deserializedSessionInfo;
		mxDO(SON::Decode( headerNode, deserializedSessionInfo ));

		if( !PtSessionInfo::AreCompatible( PtSessionInfo::CURRENT, deserializedSessionInfo ) )
		{
			ptERROR("Failed to load clump: incompatible header\n");
			return ERR_INCOMPATIBLE_VERSION;
		}
	}

	// load object lists
	// 1) create object lists, initialize objects with default constructors
	// 2) patch pointers, resolve cross references
	// 3) resolve references to external assets, finalize asset instances
	{
		const Node*	objectListsNode = FindValue( root, NODE_DATA_TAG );
		mxASSERT(objectListsNode->tag.type == TypeTag_List);

		const UINT numObjectLists = objectListsNode->value.l.size;

		struct ObjectListInfo
		{
			const Node *	sourceNode;	// OBJECT_ARRAY_TAG
			ObjectList *	objectList;
		};
		ObjectListInfo				localStorage[64];
		TArray< ObjectListInfo >	objectListsInfos(localStorage, mxCOUNT_OF(localStorage));

		objectListsInfos.Reserve(numObjectLists);

		const Node* objectListNode = objectListsNode->value.l.kids;
		while( objectListNode )
		{
			const char* className = AsString( FindValue( objectListNode, OBJECT_CLASS_TAG ) );
			mxASSERT_PTR(className);

			const mxClass* classInfo = TypeRegistry::Get().FindClassByName(className);
			mxASSERT_PTR(classInfo);

			const UINT32 maxItemCount = (UINT32) AsDouble( FindValue( objectListNode, OBJECT_COUNT_TAG ) );
			mxASSERT(maxItemCount > 0);

			const Node* objectDataNode = FindValue( objectListNode, OBJECT_ITEMS_TAG );
			mxASSERT_PTR(objectDataNode);

			ObjectList* newObjectList = _clump.CreateObjectList( *classInfo, maxItemCount );

			ObjectListInfo& newObjectListInfo = objectListsInfos.Add();
			newObjectListInfo.sourceNode = objectDataNode;
			newObjectListInfo.objectList = newObjectList;

			objectListNode = objectListNode->next;
		}

		// deserialize POD objects (stored in ObjectLists inside Clumps)
		for( UINT iObjectList = 0; iObjectList < numObjectLists; iObjectList++ )
		{
			const ObjectListInfo& objectListInfo = objectListsInfos[ iObjectList ];
			LoadObjectList( objectListInfo.sourceNode, *objectListInfo.objectList, _clump );
		}
	}
	return ALL_OK;
}

ERet LoadClumpFromFile( const char* _file, Clump &_clump )
{
	FileReader	reader;
	mxDO(reader.Open(_file));
	return LoadClump(reader, _clump);
}

}//namespace SON

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
