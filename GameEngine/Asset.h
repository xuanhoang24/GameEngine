#ifndef ASSET_H
#define ASSET_H

#include "StandardIncludes.h"
#include "ObjectPool.h"

class Asset
{
public:
	//Constructos / Destructors
	Asset();
	virtual ~Asset() {};

	// Accessors
	string GetGUID() { return m_GUID; }
	int GetDataSize() { return m_dataSize; }
	void SetDataSize(int _size) { m_dataSize = _size; }
	byte* GetData() { return m_data; }
	void SetData(byte* _data) { m_data = _data; }

	//Methods
	static ObjectPool<Asset>* Pool;
	void Reset()
	{
		m_GUID = "";
		m_dataSize = 0;
		m_data = nullptr;
	}

private:
	//Members
	string m_GUID;
	int m_dataSize;
	byte* m_data;
};

#endif // ASSET_H