// copyright (c) 2007 magnus auvinen, see licence.txt for more info
#include <base/system.h>
#include <engine/storage.h>
#include "datafile.h"
#include "engine.h"
#include <zlib.h>

static const int DEBUG=0;

struct CDatafileItemType
{
	int m_Type;
	int m_Start;
	int m_Num;
} ;

struct CDatafileItem
{
	int m_TypeAndId;
	int m_Size;
};

struct CDatafileHeader
{
	char m_aId[4];
	int m_Version;
	int m_Size;
	int m_Swaplen;
	int m_NumItemTypes;
	int m_NumItems;
	int m_NumRawData;
	int m_ItemSize;
	int m_DataSize;
};

struct CDatafileData
{
	int m_NumItemTypes;
	int m_NumItems;
	int m_NumRawData;
	int m_ItemSize;
	int m_DataSize;
	char m_aStart[4];
};

struct CDatafileInfo
{
	CDatafileItemType *m_pItemTypes;
	int *m_pItemOffsets;
	int *m_pDataOffsets;
	int *m_pDataSizes;

	char *m_pItemStart;
	char *m_pDataStart;
};

struct CDatafile
{
	IOHANDLE m_File;
	unsigned m_Crc;
	CDatafileInfo m_Info;
	CDatafileHeader m_Header;
	int m_DataStartOffset;
	char **m_ppDataPtrs;
	char *m_pData;
};

bool CDataFileReader::Open(class IStorage *pStorage, const char *pFilename)
{
	dbg_msg("datafile", "loading. filename='%s'", pFilename);

	IOHANDLE File = pStorage->OpenFile(pFilename, IOFLAG_READ);
	if(!File)
	{
		dbg_msg("datafile", "could not open '%s'", pFilename);
		return false;
	}	
	
	
	// take the CRC of the file and store it
	unsigned Crc = 0;
	{
		enum
		{
			BUFFER_SIZE = 64*1024
		};
		
		unsigned char aBuffer[BUFFER_SIZE];
		
		while(1)
		{
			unsigned Bytes = io_read(File, aBuffer, BUFFER_SIZE);
			if(Bytes <= 0)
				break;
			Crc = crc32(Crc, aBuffer, Bytes); // ignore_convention
		}
		
		io_seek(File, 0, IOSEEK_START);
	}
	
	
	// TODO: change this header
	CDatafileHeader Header;
	io_read(File, &Header, sizeof(Header));
	if(Header.m_aId[0] != 'A' || Header.m_aId[1] != 'T' || Header.m_aId[2] != 'A' || Header.m_aId[3] != 'D')
	{
		if(Header.m_aId[0] != 'D' || Header.m_aId[1] != 'A' || Header.m_aId[2] != 'T' || Header.m_aId[3] != 'A')
		{
			dbg_msg("datafile", "wrong signature. %x %x %x %x", Header.m_aId[0], Header.m_aId[1], Header.m_aId[2], Header.m_aId[3]);
			return 0;
		}
	}

#if defined(CONF_ARCH_ENDIAN_BIG)
	swap_endian(&Header, sizeof(int), sizeof(Header)/sizeof(int));	
#endif
	if(Header.m_Version != 3 && Header.m_Version != 4)
	{
		dbg_msg("datafile", "wrong version. version=%x", Header.m_Version);
		return 0;
	}
	
	// read in the rest except the data
	unsigned Size = 0;
	Size += Header.m_NumItemTypes*sizeof(CDatafileItemType);
	Size += (Header.m_NumItems+Header.m_NumRawData)*sizeof(int);
	if(Header.m_Version == 4)
		Size += Header.m_NumRawData*sizeof(int); // v4 has uncompressed data sizes aswell
	Size += Header.m_ItemSize;
	
	unsigned AllocSize = Size;
	AllocSize += sizeof(CDatafile); // add space for info structure
	AllocSize += Header.m_NumRawData*sizeof(void*); // add space for data pointers

	m_pDataFile = (CDatafile*)mem_alloc(AllocSize, 1);
	m_pDataFile->m_Header = Header;
	m_pDataFile->m_DataStartOffset = sizeof(CDatafileHeader) + Size;
	m_pDataFile->m_ppDataPtrs = (char**)(m_pDataFile+1);
	m_pDataFile->m_pData = (char *)(m_pDataFile+1)+Header.m_NumRawData*sizeof(char *);
	m_pDataFile->m_File = File;
	m_pDataFile->m_Crc = Crc;
	
	// clear the data pointers
	mem_zero(m_pDataFile->m_ppDataPtrs, Header.m_NumRawData*sizeof(void*));
	
	// read types, offsets, sizes and item data
	unsigned ReadSize = io_read(File, m_pDataFile->m_pData, Size);
	if(ReadSize != Size)
	{
		mem_free(m_pDataFile);
		m_pDataFile = 0;
		dbg_msg("datafile", "couldn't load the whole thing, wanted=%d got=%d", Size, ReadSize);
		return false;
	}

#if defined(CONF_ARCH_ENDIAN_BIG)
	swap_endian(m_pDataFile->m_pData, sizeof(int), Header.m_Swaplen / sizeof(int));
#endif

	//if(DEBUG)
	{
		dbg_msg("datafile", "allocsize=%d", AllocSize);
		dbg_msg("datafile", "readsize=%d", ReadSize);
		dbg_msg("datafile", "swaplen=%d", Header.m_Swaplen);
		dbg_msg("datafile", "item_size=%d", m_pDataFile->m_Header.m_ItemSize);
	}
	
	m_pDataFile->m_Info.m_pItemTypes = (CDatafileItemType *)m_pDataFile->m_pData;
	m_pDataFile->m_Info.m_pItemOffsets = (int *)&m_pDataFile->m_Info.m_pItemTypes[m_pDataFile->m_Header.m_NumItemTypes];
	m_pDataFile->m_Info.m_pDataOffsets = (int *)&m_pDataFile->m_Info.m_pItemOffsets[m_pDataFile->m_Header.m_NumItems];
	m_pDataFile->m_Info.m_pDataSizes = (int *)&m_pDataFile->m_Info.m_pDataOffsets[m_pDataFile->m_Header.m_NumRawData];
	
	if(Header.m_Version == 4)
		m_pDataFile->m_Info.m_pItemStart = (char *)&m_pDataFile->m_Info.m_pDataSizes[m_pDataFile->m_Header.m_NumRawData];
	else
		m_pDataFile->m_Info.m_pItemStart = (char *)&m_pDataFile->m_Info.m_pDataOffsets[m_pDataFile->m_Header.m_NumRawData];
	m_pDataFile->m_Info.m_pDataStart = m_pDataFile->m_Info.m_pItemStart + m_pDataFile->m_Header.m_ItemSize;

	dbg_msg("datafile", "loading done. datafile='%s'", pFilename);

	if(DEBUG)
	{
		/*
		for(int i = 0; i < m_pDataFile->data.num_raw_data; i++)
		{
			void *p = datafile_get_data(df, i);
			dbg_msg("datafile", "%d %d", (int)((char*)p - (char*)(&m_pDataFile->data)), size);
		}
			
		for(int i = 0; i < datafile_num_items(df); i++)
		{
			int type, id;
			void *data = datafile_get_item(df, i, &type, &id);
			dbg_msg("map", "\t%d: type=%x id=%x p=%p offset=%d", i, type, id, data, m_pDataFile->info.item_offsets[i]);
			int *idata = (int*)data;
			for(int k = 0; k < 3; k++)
				dbg_msg("datafile", "\t\t%d=%d (%x)", k, idata[k], idata[k]);
		}

		for(int i = 0; i < m_pDataFile->data.num_m_aItemTypes; i++)
		{
			dbg_msg("map", "\t%d: type=%x start=%d num=%d", i,
				m_pDataFile->info.m_aItemTypes[i].type,
				m_pDataFile->info.m_aItemTypes[i].start,
				m_pDataFile->info.m_aItemTypes[i].num);
			for(int k = 0; k < m_pDataFile->info.m_aItemTypes[i].num; k++)
			{
				int type, id;
				datafile_get_item(df, m_pDataFile->info.m_aItemTypes[i].start+k, &type, &id);
				if(type != m_pDataFile->info.m_aItemTypes[i].type)
					dbg_msg("map", "\tERROR");
			}
		}
		*/
	}
		
	return true;
}

int CDataFileReader::NumData()
{
	if(!m_pDataFile) { return 0; }
	return m_pDataFile->m_Header.m_NumRawData;
}

// always returns the size in the file
int CDataFileReader::GetDataSize(int Index)
{
	if(!m_pDataFile) { return 0; }
	
	if(Index == m_pDataFile->m_Header.m_NumRawData-1)
		return m_pDataFile->m_Header.m_DataSize-m_pDataFile->m_Info.m_pDataOffsets[Index];
	return  m_pDataFile->m_Info.m_pDataOffsets[Index+1]-m_pDataFile->m_Info.m_pDataOffsets[Index];
}

void *CDataFileReader::GetDataImpl(int Index, int Swap)
{
	if(!m_pDataFile) { return 0; }
	
	// load it if needed
	if(!m_pDataFile->m_ppDataPtrs[Index])
	{
		// fetch the data size
		int DataSize = GetDataSize(Index);
		int SwapSize = DataSize;
		
		if(m_pDataFile->m_Header.m_Version == 4)
		{
			// v4 has compressed data
			void *pTemp = (char *)mem_alloc(DataSize, 1);
			unsigned long UncompressedSize = m_pDataFile->m_Info.m_pDataSizes[Index];
			unsigned long s;

			dbg_msg("datafile", "loading data index=%d size=%d uncompressed=%d", Index, DataSize, UncompressedSize);
			m_pDataFile->m_ppDataPtrs[Index] = (char *)mem_alloc(UncompressedSize, 1);
			
			// read the compressed data
			io_seek(m_pDataFile->m_File, m_pDataFile->m_DataStartOffset+m_pDataFile->m_Info.m_pDataOffsets[Index], IOSEEK_START);
			io_read(m_pDataFile->m_File, pTemp, DataSize);

			// decompress the data, TODO: check for errors
			s = UncompressedSize;
			uncompress((Bytef*)m_pDataFile->m_ppDataPtrs[Index], &s, (Bytef*)pTemp, DataSize); // ignore_convention
			SwapSize = s;

			// clean up the temporary buffers
			mem_free(pTemp);
		}
		else
		{
			// load the data
			dbg_msg("datafile", "loading data index=%d size=%d", Index, DataSize);
			m_pDataFile->m_ppDataPtrs[Index] = (char *)mem_alloc(DataSize, 1);
			io_seek(m_pDataFile->m_File, m_pDataFile->m_DataStartOffset+m_pDataFile->m_Info.m_pDataOffsets[Index], IOSEEK_START);
			io_read(m_pDataFile->m_File, m_pDataFile->m_ppDataPtrs[Index], DataSize);
		}

#if defined(CONF_ARCH_ENDIAN_BIG)
		if(Swap && SwapSize)
			swap_endian(m_pDataFile->m_ppDataPtrs[Index], sizeof(int), SwapSize/sizeof(int));
#endif
	}
	
	return m_pDataFile->m_ppDataPtrs[Index];
}

void *CDataFileReader::GetData(int Index)
{
	return GetDataImpl(Index, 0);
}

void *CDataFileReader::GetDataSwapped(int Index)
{
	return GetDataImpl(Index, 1);
}

void CDataFileReader::UnloadData(int Index)
{
	if(Index < 0)
		return;
		
	//
	mem_free(m_pDataFile->m_ppDataPtrs[Index]);
	m_pDataFile->m_ppDataPtrs[Index] = 0x0;
}

int CDataFileReader::GetItemSize(int Index)
{
	if(!m_pDataFile) { return 0; }
	if(Index == m_pDataFile->m_Header.m_NumItems-1)
		return m_pDataFile->m_Header.m_ItemSize-m_pDataFile->m_Info.m_pItemOffsets[Index];
	return  m_pDataFile->m_Info.m_pItemOffsets[Index+1]-m_pDataFile->m_Info.m_pItemOffsets[Index];
}

void *CDataFileReader::GetItem(int Index, int *pType, int *pId)
{
	if(!m_pDataFile) { if(pType) *pType = 0; if(pId) *pId = 0; return 0; }
	
	CDatafileItem *i = (CDatafileItem *)(m_pDataFile->m_Info.m_pItemStart+m_pDataFile->m_Info.m_pItemOffsets[Index]);
	if(pType)
		*pType = (i->m_TypeAndId>>16)&0xffff; // remove sign extention
	if(pId)
		*pId = i->m_TypeAndId&0xffff;
	return (void *)(i+1);
}

void CDataFileReader::GetType(int Type, int *pStart, int *pNum)
{
	*pStart = 0;
	*pNum = 0;

	if(!m_pDataFile)
		return;
	
	for(int i = 0; i < m_pDataFile->m_Header.m_NumItemTypes; i++)
	{
		if(m_pDataFile->m_Info.m_pItemTypes[i].m_Type == Type)
		{
			*pStart = m_pDataFile->m_Info.m_pItemTypes[i].m_Start;
			*pNum = m_pDataFile->m_Info.m_pItemTypes[i].m_Num;
			return;
		}
	}
}

void *CDataFileReader::FindItem(int Type, int Id)
{
	if(!m_pDataFile) return 0;
	
	int Start, Num;
	GetType(Type, &Start, &Num);
	for(int i = 0; i < Num; i++)
	{
		int ItemId;
		void *pItem = GetItem(Start+i,0, &ItemId);
		if(Id == ItemId)
			return pItem;
	}
	return 0;
}

int CDataFileReader::NumItems()
{
	if(!m_pDataFile) return 0;
	return m_pDataFile->m_Header.m_NumItems;
}

bool CDataFileReader::Close()
{
	if(!m_pDataFile)
		return true;
	
	// free the data that is loaded
	int i;
	for(i = 0; i < m_pDataFile->m_Header.m_NumRawData; i++)
		mem_free(m_pDataFile->m_ppDataPtrs[i]);
	
	io_close(m_pDataFile->m_File);
	mem_free(m_pDataFile);
	m_pDataFile = 0;
	return true;
}

unsigned CDataFileReader::Crc()
{
	if(!m_pDataFile) return -1;
	return m_pDataFile->m_Crc;
}

bool CDataFileWriter::Open(class IStorage *pStorage, const char *pFilename)
{
	int i;
	//DATAFILE_OUT *df = (DATAFILE_OUT*)mem_alloc(sizeof(DATAFILE_OUT), 1);
	m_File = pStorage->OpenFile(pFilename, IOFLAG_WRITE);
	if(!m_File)
		return false;
	
	m_NumItems = 0;
	m_NumDatas = 0;
	m_NumItemTypes = 0;
	mem_zero(&m_aItemTypes, sizeof(m_aItemTypes));

	for(i = 0; i < 0xffff; i++)
	{
		m_aItemTypes[i].m_First = -1;
		m_aItemTypes[i].m_Last = -1;
	}
	
	return true;
}

int CDataFileWriter::AddItem(int Type, int Id, int Size, void *pData)
{
	m_aItems[m_NumItems].m_Type = Type;
	m_aItems[m_NumItems].m_Id = Id;
	m_aItems[m_NumItems].m_Size = Size;
	
	/*
	dbg_msg("datafile", "added item type=%d id=%d size=%d", type, id, size);
	int i;
	for(i = 0; i < size/4; i++)
		dbg_msg("datafile", "\t%d: %08x %d", i, ((int*)data)[i], ((int*)data)[i]);
	*/
	
	// copy data
	m_aItems[m_NumItems].m_pData = mem_alloc(Size, 1);
	mem_copy(m_aItems[m_NumItems].m_pData, pData, Size);

	if(!m_aItemTypes[Type].m_Num) // count item types
		m_NumItemTypes++;

	// link
	m_aItems[m_NumItems].m_Prev = m_aItemTypes[Type].m_Last;
	m_aItems[m_NumItems].m_Next = -1;
	
	if(m_aItemTypes[Type].m_Last != -1)
		m_aItems[m_aItemTypes[Type].m_Last].m_Next = m_NumItems;
	m_aItemTypes[Type].m_Last = m_NumItems;
	
	if(m_aItemTypes[Type].m_First == -1)
		m_aItemTypes[Type].m_First = m_NumItems;
	
	m_aItemTypes[Type].m_Num++;
		
	m_NumItems++;
	return m_NumItems-1;
}

int CDataFileWriter::AddData(int Size, void *pData)
{
	CDataInfo *pInfo = &m_aDatas[m_NumDatas];
	unsigned long s = compressBound(Size);
	void *pCompData = mem_alloc(s, 1); // temporary buffer that we use duing compression

	int Result = compress((Bytef*)pCompData, &s, (Bytef*)pData, Size); // ignore_convention
	if(Result != Z_OK)
	{
		dbg_msg("datafile", "compression error %d", Result);
		dbg_assert(0, "zlib error");
	}
		
	pInfo->m_UncompressedSize = Size;
	pInfo->m_CompressedSize = (int)s;
	pInfo->m_pCompressedData = mem_alloc(pInfo->m_CompressedSize, 1);
	mem_copy(pInfo->m_pCompressedData, pCompData, pInfo->m_CompressedSize);
	mem_free(pCompData);

	m_NumDatas++;
	return m_NumDatas-1;
}

int CDataFileWriter::AddDataSwapped(int Size, void *pData)
{
#if defined(CONF_ARCH_ENDIAN_BIG)
	void *pSwapped = mem_alloc(Size, 1); // temporary buffer that we use duing compression
	int Index;
	mem_copy(pSwapped, pData, Size);
	swap_endian(&pSwapped, sizeof(int), Size/sizeof(int));
	Index = AddData(Size, pSwapped);
	mem_free(pSwapped);
	return Index;
#else
	return AddData(Size, pData);
#endif
}


int CDataFileWriter::Finish()
{
	int ItemSize = 0;
	int TypesSize, HeaderSize, OffsetSize, FileSize, SwapSize;
	int DataSize = 0;
	CDatafileHeader Header;

	// we should now write this file!
	if(DEBUG)
		dbg_msg("datafile", "writing");

	// calculate sizes
	for(int i = 0; i < m_NumItems; i++)
	{
		if(DEBUG)
			dbg_msg("datafile", "item=%d size=%d (%d)", i, m_aItems[i].m_Size, m_aItems[i].m_Size+sizeof(CDatafileItem));
		ItemSize += m_aItems[i].m_Size + sizeof(CDatafileItem);
	}
	
	
	for(int i = 0; i < m_NumDatas; i++)
		DataSize += m_aDatas[i].m_CompressedSize;
	
	// calculate the complete size
	TypesSize = m_NumItemTypes*sizeof(CDatafileItemType);
	HeaderSize = sizeof(CDatafileHeader);
	OffsetSize = m_NumItems*sizeof(int) + m_NumDatas*sizeof(int);
	FileSize = HeaderSize + TypesSize + OffsetSize + ItemSize + DataSize;
	SwapSize = FileSize - DataSize;
	
	(void)SwapSize;
	
	if(DEBUG)
		dbg_msg("datafile", "num_m_aItemTypes=%d TypesSize=%d m_aItemsize=%d DataSize=%d", m_NumItemTypes, TypesSize, ItemSize, DataSize);
	
	// construct Header
	{
		Header.m_aId[0] = 'D';
		Header.m_aId[1] = 'A';
		Header.m_aId[2] = 'T';
		Header.m_aId[3] = 'A';
		Header.m_Version = 4;
		Header.m_Size = FileSize - 16;
		Header.m_Swaplen = SwapSize - 16;
		Header.m_NumItemTypes = m_NumItemTypes;
		Header.m_NumItems = m_NumItems;
		Header.m_NumRawData = m_NumDatas;
		Header.m_ItemSize = ItemSize;
		Header.m_DataSize = DataSize;
		
		// TODO: apply swapping
		// write Header
		if(DEBUG)
			dbg_msg("datafile", "HeaderSize=%d", sizeof(Header));
		io_write(m_File, &Header, sizeof(Header));
	}
	
	// write types
	for(int i = 0, Count = 0; i < 0xffff; i++)
	{
		if(m_aItemTypes[i].m_Num)
		{
			// write info
			CDatafileItemType Info;
			Info.m_Type = i;
			Info.m_Start = Count;
			Info.m_Num = m_aItemTypes[i].m_Num;
			if(DEBUG)
				dbg_msg("datafile", "writing type=%x start=%d num=%d", Info.m_Type, Info.m_Start, Info.m_Num);
			io_write(m_File, &Info, sizeof(Info));
			Count += m_aItemTypes[i].m_Num;
		}
	}
	
	// write item offsets
	for(int i = 0, Offset = 0; i < 0xffff; i++)
	{
		if(m_aItemTypes[i].m_Num)
		{
			// write all m_aItems in of this type
			int k = m_aItemTypes[i].m_First;
			while(k != -1)
			{
				if(DEBUG)
					dbg_msg("datafile", "writing item offset num=%d offset=%d", k, Offset);
				io_write(m_File, &Offset, sizeof(Offset));
				Offset += m_aItems[k].m_Size + sizeof(CDatafileItem);
				
				// next
				k = m_aItems[k].m_Next;
			}
		}
	}
	
	// write data offsets
	for(int i = 0, Offset = 0; i < m_NumDatas; i++)
	{
		if(DEBUG)
			dbg_msg("datafile", "writing data offset num=%d offset=%d", i, Offset);
		io_write(m_File, &Offset, sizeof(Offset));
		Offset += m_aDatas[i].m_CompressedSize;
	}

	// write data uncompressed sizes
	for(int i = 0; i < m_NumDatas; i++)
	{
		/*
		if(DEBUG)
			dbg_msg("datafile", "writing data offset num=%d offset=%d", i, offset);
		*/
		io_write(m_File, &m_aDatas[i].m_UncompressedSize, sizeof(int));
	}
	
	// write m_aItems
	for(int i = 0; i < 0xffff; i++)
	{
		if(m_aItemTypes[i].m_Num)
		{
			// write all m_aItems in of this type
			int k = m_aItemTypes[i].m_First;
			while(k != -1)
			{
				CDatafileItem Item;
				Item.m_TypeAndId = (i<<16)|m_aItems[k].m_Id;
				Item.m_Size = m_aItems[k].m_Size;
				if(DEBUG)
					dbg_msg("datafile", "writing item type=%x idx=%d id=%d size=%d", i, k, m_aItems[k].m_Id, m_aItems[k].m_Size);
				
				io_write(m_File, &Item, sizeof(Item));
				io_write(m_File, m_aItems[k].m_pData, m_aItems[k].m_Size);
				
				// next
				k = m_aItems[k].m_Next;
			}
		}
	}
	
	// write data
	for(int i = 0; i < m_NumDatas; i++)
	{
		if(DEBUG)
			dbg_msg("datafile", "writing data id=%d size=%d", i, m_aDatas[i].m_CompressedSize);
		io_write(m_File, m_aDatas[i].m_pCompressedData, m_aDatas[i].m_CompressedSize);
	}

	// free data
	for(int i = 0; i < m_NumItems; i++)
		mem_free(m_aItems[i].m_pData);

	
	io_close(m_File);
	
	if(DEBUG)
		dbg_msg("datafile", "done");
	return 0;
}