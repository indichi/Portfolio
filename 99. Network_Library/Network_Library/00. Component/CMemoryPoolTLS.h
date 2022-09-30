#pragma once

#include <unordered_map>

#include "CChunk.h"
#include "CLFMemoryPool.h"
#include "CChunkMemoryPool.h"

using namespace procademy;

template <typename T>
class CMemoryPoolTLS
{
public:
    CMemoryPoolTLS();
    ~CMemoryPoolTLS();

    T* Alloc();
    bool Free(volatile T* data);
    int GetAllocChunkSize() { return m_MainPool->GetCapacity(); }
private:
    CLFMemoryPool<CChunk<T>>*                           m_MainPool;
    DWORD                                               m_dwTLSindex;
};

template<typename T>
CMemoryPoolTLS<T>::CMemoryPoolTLS()
    : m_MainPool(new CLFMemoryPool<CChunk<T>>(false))
{
    m_dwTLSindex = TlsAlloc();
    if (m_dwTLSindex == TLS_OUT_OF_INDEXES)
        exit(-1);
}

template<typename T>
CMemoryPoolTLS<T>::~CMemoryPoolTLS()
{
    delete m_MainPool;
}

template<typename T>
T* CMemoryPoolTLS<T>::Alloc()
{
    CChunkMemoryPool<T>* chunkPool = (CChunkMemoryPool<T>*)TlsGetValue(m_dwTLSindex);

    // �������� ���� -> ������ ù ��� -> map�� ThreadID, �޸�Ǯ ��� �� Alloc
    if (chunkPool == nullptr)
    {
        chunkPool = new CChunkMemoryPool<T>(m_MainPool);
        if (!TlsSetValue(m_dwTLSindex, chunkPool))
            CCrashDump::Crash();
    }

    return chunkPool->Alloc();
}

template<typename T>
bool CMemoryPoolTLS<T>::Free(volatile T* data)
{
    // ���� ���� chunk ã��
    struct CChunk<T>::st_DATA* stData = (struct CChunk<T>::st_DATA*)((char*)data - sizeof(CChunk<T>*));
    CChunk<T>* chunk = stData->_pThis;

    // Free Count�� ������Ű�� �� free �ƴ��� Ȯ�� -> �� ������ main pool�� chunk ��ȯ
    if (InterlockedIncrement(&chunk->_dwFreeCount) == dfCHUNK_NODE_COUNT)
    {
        chunk->_dwFreeCount = 0;
        chunk->_dwIndex = 0;
        m_MainPool->Free(chunk);
    }

    return true;
}