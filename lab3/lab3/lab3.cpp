#include "stdafx.h"
#include <iostream>
#include <string.h>
using namespace std;

struct Map
{
private:
	bool init;

public:
	int countOfBlocks;
	int* listOfBlocks;

	Map()
	{
		init = false;
	};

	Map(int count)
	{
		countOfBlocks = count;
		if (countOfBlocks > 0) {
			listOfBlocks = new int[count];
			init = true;
		}
	}
	~Map()
	{
	}
	void AddMap(Map other)
	{
		int newCount = this->countOfBlocks + other.countOfBlocks;
		int* newList = new int[newCount];
		for (int i = 0; i < this->countOfBlocks; ++i) {
			newList[i] = this->listOfBlocks[i];
		}
		for (int i = 0; i < other.countOfBlocks; ++i) {
			newList[this->countOfBlocks + i] = other.listOfBlocks[i];
		}
		delete[] listOfBlocks;
		this->listOfBlocks = newList;
		this->countOfBlocks = newCount;
	}
	void Dispose()
	{
		delete[] this->listOfBlocks;
	}
};

struct Descriptor 
{
	char file_type;
	int linkCount;
	int fileSize;
	Map map;
};

struct FileLink 
{
	char fileName[28];
	int descriptorNum;
	FileLink(const char* name, int num) 
	{
		strcpy_s(fileName, 28, name);
		descriptorNum = num;
	}
};

class BlockStorage 
{
private:
	int freeSpace;
	__int32 *bitMap;
	int bitMapSize;
	int descriptorsCount;
	Descriptor *descriptors;
	void* blockStorage;
	int* countableDescriptor;
public:
	const int BlockSize = 128;

	int getBitMapSize() 
	{
		return bitMapSize;
	}

	int getDescriptorsCount() 
	{
		return descriptorsCount;
	}

	__int32* getBitMap() 
	{
		return bitMap;
	}

	Descriptor* getDescriptors() 
	{
		return descriptors;
	}

	void* getBlockStorage() 
	{
		return blockStorage;
	}

	BlockStorage() {};

	BlockStorage(int countOfBlocks)
	{
		bitMapSize = (countOfBlocks - 1) / 32 + 1;
		bitMap = new __int32[bitMapSize];
		for (int i = 0; i < bitMapSize; ++i)
			bitMap[i] = 0;

		descriptorsCount = countOfBlocks / 2;
		countableDescriptor = new int[descriptorsCount];
		descriptors = new Descriptor[descriptorsCount];
		for (int i = 0; i < descriptorsCount; ++i) 
		{
			descriptors[i].file_type = 'e';
			countableDescriptor[i] = -1;
		}

		freeSpace = countOfBlocks;

		blockStorage = malloc(BlockSize * countOfBlocks);
		CreateDescriptor('F', 1);
		//printf("%d", CreateDescriptor('F', 1));
	}

	void cleanResources() 
	{
		delete[] bitMap;
		delete[] descriptors;
		
		free(blockStorage);
	}

	~BlockStorage() 
	{
		cleanResources();
	}

	void loadDataFromFile(FILE* file) 
	{
		fread(&bitMapSize, sizeof(int), 1, file);
		fread(&descriptorsCount, sizeof(int), 1, file);
		bitMap = new __int32[bitMapSize];
		descriptors = new Descriptor[descriptorsCount];
		blockStorage = malloc(BlockSize * descriptorsCount * 2);

		fread(bitMap, sizeof(__int32), bitMapSize, file);
		fread(descriptors, sizeof(Descriptor), descriptorsCount, file);
		fread(blockStorage, sizeof(char), descriptorsCount * 2 * BlockSize, file);

		for (int i = 0; i < descriptorsCount; ++i) 
		{
			if (descriptors[i].file_type != 'e') 
			{
				descriptors[i].map.listOfBlocks = new int[descriptors[i].map.countOfBlocks];
				fread(descriptors[i].map.listOfBlocks, sizeof(int), descriptors[i].map.countOfBlocks, file);
			}
		}

		printf("\nSuccessful loading\n\n");
	}

	void printBitMap() 
	{
		printf("\nStorage filling:\n");
		for (int i = 0; i < bitMapSize; ++i) 
		{
			__int32 mask = 1;
			for (int j = 0; j < 32; ++j) 
			{
				printf("%d", (bitMap[i] & mask) != 0);
				mask = mask << 1;
			}
			printf("\n");
		}
		printf("\n");
	}

	int CreateFolder(const char* name, int father_id = 0) 
	{

		if (descriptors[father_id].file_type != 'F') 
		{
			return -1;
		}

		int newIndex = CreateDescriptor('F', 1);
		if (newIndex == -1)
			return -1;

		AddFileToFolder(newIndex, name, father_id);
		return newIndex;
	}

	int CreateFile(const char* fileName, int fileSize, int father_id = 0) 
	{
		int newIndex = CreateDescriptor('f', fileSize);
		if (newIndex == -1)
		{
			printf("\nCreating failed\n\n");
			return -1;
		}

		AddFileToFolder(newIndex, fileName, father_id);
		printf("\nSuccessful creating\n\n");
		return newIndex;
	}

	void getDescriptorInfo(int descriptorID, int level = 0) 
	{
		if (descriptorID < 0 || descriptorID >= descriptorsCount)
			return;

		Descriptor desc = descriptors[descriptorID];
		printf("\nDescriptor %d info:\n", descriptorID);
		for (int i = 0; i < level; ++i)
			printf("\t");
		printf("File Type: %c, File Size: %d, LinkCount: %d\n", desc.file_type, desc.fileSize, desc.linkCount);
		for (int i = 0; i < level; ++i)
			printf("\t");
		printf("Blocks Count: %d, BlockNums: ", desc.map.countOfBlocks);
		for (int i = 0; i < desc.map.countOfBlocks; ++i) 
		{
			printf("%d ", desc.map.listOfBlocks[i]);
		}
		printf("\n");
	}

	void getFileList(int folder_id = 0) 
	{
		if (folder_id < 0 || folder_id >= descriptorsCount)
			return;

		Descriptor folder = descriptors[folder_id];

		if (folder.file_type != 'F')
			return;

		printf("\nFolder Filling:\n");
		int countOfFileLinksInBlock = BlockSize / sizeof(FileLink);
		int countOfFiles = folder.fileSize;
		for (int i = 0; i < folder.map.countOfBlocks; ++i) 
		{
			int blockNum = folder.map.listOfBlocks[i];
			FileLink* p = (FileLink*)(static_cast<char*>(blockStorage) + blockNum * BlockSize);
			for (int j = 0; j < countOfFileLinksInBlock; ++j) 
			{
				if (countOfFiles == 0)
					break;
				FileLink link = *(p + j);
				printf("FileName %s, Descriptor %d\n", link.fileName, link.descriptorNum);
				--countOfFiles;
			}
		}

		printf("\n");
	}

	int CreateLink(const char* linkName, const char* fileName, int folder_id = 0) 
	{
		if (descriptors[folder_id].file_type != 'F')
		{
			printf("\nCreating link failed\n\n");
			return -1;
		}

		int folder = -1;
		int descriptor_id = findDescriptorByName(fileName, folder);
		AddFileToFolder(descriptor_id, linkName, folder_id);

		descriptors[descriptor_id].linkCount++;
		printf("\nSuccessful creating link\n\n");
		return descriptor_id;
	}

	bool destroyLink(const char* linkName) 
	{
		int folder_id = -1;
		int descriptor_id = findDescriptorByName(linkName, folder_id);

		if (descriptor_id == -1)
		{
			printf("\nDestroyed link failed\n\n");
			return false;
		}

		RemoveFromFolder(folder_id, linkName);
		descriptors[descriptor_id].linkCount--;

		if (descriptors[descriptor_id].linkCount == 0) 
		{
			DeleteDescriptor(descriptor_id);
		}
		printf("\nSuccessful destroyed link\n\n");
		return true;
	}

	int openFile(const char* FileName) 
	{
		int father_id = 0;
		int descriptor_id = findDescriptorByName(FileName, father_id);

		if (descriptor_id == -1)
			return -1;

		return getFreeCountDescriptor(descriptor_id);
	}

	bool closeFile(int countableDescriptor_id) 
	{
		if (countableDescriptor_id < 0 ||
			countableDescriptor_id >= descriptorsCount)
		{
			printf("\nClosing failed\n\n");
			return false;
		}

		countableDescriptor[countableDescriptor_id] = -1;
		printf("\nSuccessful closing\n\n");
		return true;
	}

	void readFromFile(int countableDescriptor_id, int offset, int length) 
	{
		int descriptor_id = countableDescriptor[countableDescriptor_id];
		if (descriptor_id == -1)
			return;

		int blockNumber = offset / BlockSize;
		int blockPlace = offset % BlockSize;

		Descriptor desc = descriptors[descriptor_id];

		while (length > 0) {
			int blockNum = desc.map.listOfBlocks[blockNumber];
			char* p = static_cast<char*>(blockStorage) + blockNum * BlockSize;
			while (length > 0 && blockPlace < BlockSize) 
			{
				printf("%c", *(p + blockPlace));
				blockPlace++;
				length--;
			}

			if (length != 0) 
			{
				blockNumber++;
				blockPlace = 0;
				if (blockNumber >= desc.map.countOfBlocks) 
				{
					break;
				}
			}
		}
		printf("\n");
	}

	void writeToFile(int countableDescriptor_id, int offset, int length, char* ch) 
	{
		int descriptor_id = countableDescriptor[countableDescriptor_id];
		if (descriptor_id == -1)
		{
			printf("\nWriting failed\n\n");
			return;
		}

		int blockNumber = offset / BlockSize;
		int blockPlace = offset % BlockSize;
		int i = 0;

		Descriptor desc = descriptors[descriptor_id];

		while (length > 0) {
			int blockNum = desc.map.listOfBlocks[blockNumber];
			char* p = static_cast<char*>(blockStorage) + blockNum * BlockSize;
			while (length > 0 && blockPlace < BlockSize) 
			{
				if (ch[i] == '\0')
					*(p + blockPlace) = ' ';
				else
				{
					*(p + blockPlace) = ch[i++];
				}
				blockPlace++;
				length--;
			}

			if (length != 0) 
			{
				blockNumber++;
				blockPlace = 0;
				if (blockNumber >= desc.map.countOfBlocks) 
				{
					break;
				}
			}
		}
		printf("\nSuccessful writing\n\n");
	}

	bool truncate(const char* filename, int size) 
	{
		int father_id = 0;
		int descriptor_id = findDescriptorByName(filename, father_id);

		if (descriptor_id == -1)
		{
			printf("\nTruncating failed\n\n");
			return false;
		}

		Descriptor desc = descriptors[descriptor_id];
		int countOfBlocksToChange = abs(((desc.fileSize - 1) / BlockSize) - ((size - 1) / BlockSize));

		if (countOfBlocksToChange == 0) 
		{
			descriptors[descriptor_id].fileSize = size;
			printf("\nSuccessful truncating\n\n");
			return true;
		}

		int diff = desc.fileSize - size;
		if (diff > 0) {
			while (countOfBlocksToChange) 
			{
				int blockNum = desc.map.listOfBlocks[desc.map.countOfBlocks - 1];
				setFree(blockNum);
				desc.map.countOfBlocks--;
				--countOfBlocksToChange;
			}
			desc.fileSize = size;
			descriptors[descriptor_id] = desc;
			printf("\nSuccessful truncating\n\n");
			return true;
		}

		Map newMap = findFreeSpace(countOfBlocksToChange);

		descriptors[descriptor_id].map.AddMap(newMap);
		descriptors[descriptor_id].fileSize = size;
		printf("\nSuccessful truncating\n\n");
		return true;
	}

private:

	int getFreeCountDescriptor(int descriptor_id) 
	{
		for (int i = 0; i < descriptorsCount; ++i) 
		{
			if (countableDescriptor[i] == -1) 
			{
				countableDescriptor[i] = descriptor_id;
				return i;
			}
		}
		return -1;
	}

	bool DeleteDescriptor(int descriptor_id) 
	{
		Descriptor desc = descriptors[descriptor_id];
		for (int i = 0; i < desc.map.countOfBlocks; ++i) 
		{
			setFree(desc.map.listOfBlocks[i]);
		}
		desc.map.Dispose();
		desc.file_type = 'e';
		return true;
	}

	bool RemoveFromFolder(int folder_id, const char* fileName) 
	{
		Descriptor folder = descriptors[folder_id];
		int countOfFileLinksInBlock = BlockSize / sizeof(FileLink);
		int countOfFiles = folder.fileSize;
		for (int i = 0; i < folder.map.countOfBlocks; ++i) 
		{
			int blockNum = folder.map.listOfBlocks[i];
			FileLink* p = (FileLink*)(static_cast<char*>(blockStorage) + blockNum * BlockSize);
			for (int j = 0; j < countOfFileLinksInBlock; ++j) 
			{
				FileLink link = *(p + j);
				--countOfFiles;
				if (strcmp(fileName, link.fileName) == 0) 
				{
					if (countOfFiles) 
					{
						int lastBlockNum = folder.map.listOfBlocks[folder.map.countOfBlocks - 1];
						FileLink* lastP = (FileLink*)(static_cast<char*>(blockStorage) + lastBlockNum * BlockSize);

						lastP += (folder.fileSize - 1) % countOfFileLinksInBlock;

						(p + j)->descriptorNum = lastP->descriptorNum;
						strcpy_s((p + j)->fileName, 28, lastP->fileName);
					}

					folder.fileSize--;
					if (folder.fileSize % countOfFileLinksInBlock == 0
						&& folder.fileSize != 0) 
					{
						int lastBlock = folder.map.listOfBlocks[folder.map.countOfBlocks - 1];
						setFree(lastBlock);
						folder.map.countOfBlocks--;
						descriptors[folder_id] = folder;
						return true;
					}
					descriptors[folder_id] = folder;
					return true;
				}
			}
		}
		return false;
	}

	int findDescriptorByName(const char* fileName, int &folder_id) 
	{
		for (int f = 0; f < descriptorsCount; ++f) 
		{
			if (descriptors[f].file_type != 'F')
				continue;

			Descriptor folder = descriptors[f];
			int countOfFileLinksInBlock = BlockSize / sizeof(FileLink);
			int countOfFiles = folder.fileSize;
			for (int i = 0; i < folder.map.countOfBlocks; ++i) 
			{
				int blockNum = folder.map.listOfBlocks[i];
				FileLink* p = (FileLink*)(static_cast<char*>(blockStorage) + blockNum * BlockSize);
				for (int j = 0; j < countOfFileLinksInBlock; ++j) 
				{
					if (countOfFiles == 0)
						break;
					FileLink link = *(p + j);
					if (strcmp(fileName, link.fileName) == 0) 
					{
						folder_id = f;
						return link.descriptorNum;
					}
					--countOfFiles;
				}
			}
		}
		folder_id = -1;
		return -1;
	}

	bool isBlockFree(int number) 
	{
		int index = number / 32;
		__int32 mask = 1 << (number % 32);
		return ((bitMap[index] & mask) == mask);
	}

	void setFree(int block) 
	{
		int index = block / 32;
		__int32 mask = 1 << (block % 32);
		if ((bitMap[index] & mask) == mask)
			bitMap[index] -= mask;
	}

	void setOccupied(int block) 
	{
		int index = block / 32;
		__int32 mask = 1 << (block % 32);
		bitMap[index] |= mask;
	}

	int getNewDescriptorIndex() 
	{
		for (int i = 0; i < descriptorsCount; ++i) 
		{
			if (descriptors[i].file_type == 'e') 
			{
				return i;
			}
		}
		return -1;
	}

	bool AddFileToFolder(int descriptorNum, const char* fileName, int father_id = 0) 
	{
		Descriptor folder = descriptors[father_id];
		if (folder.file_type != 'F') 
		{
			return false;
		}

		int countOfFileLinksInBlock = BlockSize / sizeof(FileLink);
		FileLink link = FileLink(fileName, descriptorNum);

		if (folder.fileSize == 0
			|| folder.fileSize % countOfFileLinksInBlock != 0) 
		{
			int lastIndex = folder.map.countOfBlocks;
			int blockNum = folder.map.listOfBlocks[lastIndex - 1];
			FileLink* p = (FileLink*)((void*)(static_cast<char*>(blockStorage) + blockNum * BlockSize)) + folder.fileSize % countOfFileLinksInBlock;
			p->descriptorNum = link.descriptorNum;
			strcpy_s(p->fileName, 28, link.fileName);

			folder.fileSize++;
			descriptors[father_id] = folder;
			return true;
		}
		Map additionalMap = findFreeSpace(1); 
		int blockNum = additionalMap.listOfBlocks[0];

		FileLink* p = (FileLink*)((void*)(static_cast<char*>(blockStorage) + blockNum * BlockSize));
		p->descriptorNum = link.descriptorNum;
		strcpy_s(p->fileName, 28, link.fileName);

		folder.fileSize++;
		folder.map.AddMap(additionalMap);
		descriptors[father_id] = folder;

		return true;
	}

	int CreateDescriptor(char fileType, int fileSize) 
	{
		int newIndex = getNewDescriptorIndex();
		if (newIndex == -1) 
		{
			return -1;
		}

		int countOfBlocks = (fileSize - 1) / BlockSize + 1;
		Descriptor descriptor = Descriptor();
		descriptor.fileSize = fileType == 'F' ? 0 : fileSize;
		descriptor.linkCount = 1;
		descriptor.file_type = fileType;
		descriptor.map = findFreeSpace(countOfBlocks);

		descriptors[newIndex] = descriptor;

		return newIndex;
	}

	Map findFreeSpace(int count) 
	{
		if (count > freeSpace)
			return Map(-1);

		int countOfFreeInLine = 0;
		Map newMap = Map(count);
		freeSpace -= count;


		for (int i = 0; i < bitMapSize; ++i) 
		{
			__int32 mask = 1;
			for (int j = 0; j < 32; ++j) 
			{
				if (!(bitMap[i] & mask)) 
				{
					++countOfFreeInLine;
					if (countOfFreeInLine == count) 
					{
						int start = 32 * i + j + 1 - countOfFreeInLine;

						for (int k = 0; k < count; ++k) 
						{
							setOccupied(start + k);
							newMap.listOfBlocks[k] = start + k;
						}
						return newMap;
					}
				}
				else 
				{
					countOfFreeInLine = 0;
				}
				mask = mask << 1;
			}
		}

		countOfFreeInLine = 0;
		for (int i = 0; i < bitMapSize; ++i) 
		{
			__int32 mask = 1;
			for (int j = 0; j < 32; ++j) {
				if (!(bitMap[i] & mask)) {
					int position = i * 32 + j;
					setOccupied(position);
					newMap.listOfBlocks[countOfFreeInLine] = position;
				}
				mask = mask << 1;
			}
		}
		return newMap;
	}
};

class Console 
{
private:
	BlockStorage * storage;

public:
	Console(int countOFBlocks)
	{
		storage = new BlockStorage(countOFBlocks);
	};

	~Console() 
	{
		delete storage;
	}

	void run() 
	{
		printCommandList();
		while (true) 
		{
			printf("Enter Command:\n");
			char command[256];
			gets_s(command, 256);

			char* str = command;
			char* context;
			char* context2;
			char* p = strtok_s(str, " ", &context);
			if (p != NULL) 
			{
				if (!strcmp(p, "mount")) 
				{
					mount();
				}
				else if (!strcmp(p, "unmount")) 
				{
					unmount();
				}
				else if (!strcmp(p, "filestat")) 
				{
					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}

					int descriptor_id = strtol(p, &context2, 10);
					storage->getDescriptorInfo(descriptor_id);
				}
				else if (!strcmp(p, "Is")) 
				{
					p = strtok_s(NULL, " ", &context);

					if (p == NULL) 
					{
						storage->getFileList();
					}
					else 
					{
						int folder_id = strtol(p, &context2, 10);
						storage->getFileList(folder_id);
					}

				}
				else if (!strcmp(p, "create")) 
				{
					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}

					char name[28];
					strcpy_s(name, p);

					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}

					int size = strtol(p, &context2, 10);

					p = strtok_s(NULL, " ", &context);
					if (p != NULL) 
					{
						int descriptor_id = strtol(p, &context2, 10);
						storage->CreateFile(name, size, descriptor_id);
					}
					else 
					{
						storage->CreateFile(name, size);
					}
				}
				else if (!strcmp(p, "open")) 
				{
					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}

					printf("Descriptor of file '%s' is %d\n",p ,storage->openFile(p));
				}
				else if (!strcmp(p, "close")) 
				{
					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}

					int countableDescriptor_id = strtol(p, &context2, 10);
					storage->closeFile(countableDescriptor_id);
				}
				else if (!strcmp(p, "read")) 
				{
					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}

					int countableDescriptor_id = strtol(p, &context2, 10);

					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}

					int offset = strtol(p, &context2, 10);

					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}
					int size = strtol(p, &context2, 10);

					storage->readFromFile(countableDescriptor_id, offset, size);
				}
				else if (!strcmp(p, "write")) 
				{
					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}
					int countableDescriptor_id = strtol(p, &context2, 10);

					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}
					int offset = strtol(p, &context2, 10);

					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}
					int size = strtol(p, &context2, 10);

					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}

					storage->writeToFile(countableDescriptor_id, offset, size, p);
				}
				else if (!strcmp(p, "link")) 
				{
					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}
					char name[28];
					strcpy_s(name, p);

					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}
					char filename[28];
					strcpy_s(filename, p);

					p = strtok_s(NULL, " ", &context);
					if (p != NULL) 
					{
						int descriptor_id = strtol(p, &context2, 10);
						storage->CreateLink(name, filename, descriptor_id);
					}
					else 
					{
						storage->CreateLink(name, filename);
					}
				}
				else if (!strcmp(p, "unlink")) 
				{
					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}
					storage->destroyLink(p);
				}
				else if (!strcmp(p, "truncate")) 
				{
					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}
					char name[28];
					strcpy_s(name, p);

					p = strtok_s(NULL, " ", &context);
					if (p == NULL)
					{
						printf("\nNot enough arguments\n\n");
						continue;
					}
					int size = strtol(p, &context2, 10);

					storage->truncate(name, size);
				}
				else if (!strcmp(p, "exit")) 
				{
					break;
				}
			}
		}
	}

private:
	void printCommandList() {
		printf("Run Command: \n");
		printf("\tmount\n");
		printf("\tunmount\n");
		printf("\tfilestat (id)\n");
		printf("\tIs\n");
		printf("\tcreate (filename) (size)\n");
		printf("\topen (filename)\n");
		printf("\tclose (fd)\n");
		printf("\tread (fd) (offset) (length)\n");
		printf("\twrite (fd) (offset) (length) (string)\n");
		printf("\tlink (linkname) (filename)\n");
		printf("\tunlink (filename)\n");
		printf("\ttruncate (filename) (size)\n");
		printf("\texit\n");
	}

	void unmount() {
		FILE *output = NULL;
		

		fopen_s(&output, "output.bin", "wb");
		if (output == NULL) {
			printf("\nError opening file\n\n");
			return;
		}

		int bmSize = storage->getBitMapSize();
		int dsCount = storage->getDescriptorsCount();

		fwrite(&bmSize, sizeof(int), 1, output);
		fwrite(&dsCount, sizeof(int), 1, output);
		fwrite(storage->getBitMap(), sizeof(__int32), bmSize, output);
		fwrite(storage->getDescriptors(), sizeof(Descriptor), dsCount, output);
		fwrite(storage->getBlockStorage(), sizeof(char), dsCount * 2 * storage->BlockSize, output);
		for (int i = 0; i < dsCount; ++i) {
			if (storage->getDescriptors()[i].file_type != 'e') {
				fwrite(storage->getDescriptors()[i].map.listOfBlocks, sizeof(int), storage->getDescriptors()[i].map.countOfBlocks, output);
			}
		}
		fclose(output);
		printf("\nSuccessful saving\n\n");
	}

	void mount() {
		FILE *output = NULL;

		fopen_s(&output, "output.bin", "rb");
		if (output == NULL) {
			printf("\nError opening file\n\n");
			return;
		}
		storage->cleanResources();

		storage->loadDataFromFile(output);

		fclose(output);
	}
};

int main()
{
	Console console = Console(64);
	console.run();
	system("pause");
	return 0;
}
