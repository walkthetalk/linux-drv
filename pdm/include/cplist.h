#pragma once

#include <stddef.h>

#include "cp_hdr.h"

/*
 * the header of double linked list, it only have two member variable,
 * one pointed to the next node, and the other pointed to the previous node.
 * for use the double linked list, you need define your node deriving
 * from *CListHeader*, and the head of whole list is *CListHead<node_t>*,
 * NOTE: the head is one node located in the list.
 */
class CListHeader
{
public:
	explicit CListHeader() :
		m_pNext(this),
		m_pPrev(this)
	{
		// TODO: add
	}
#if 0
	explicit CListHeader(CListHeader * pInst) :
		m_pNext(pInst),
		m_pPrev(pInst)
	{
		// TODO: add
	}
#endif
	virtual ~CListHeader()
	{
		// TODO: add
	}

	CListHeader * next(void)
	{
		assert(m_pNext);
		return m_pNext;
	}

	CListHeader * prev(void)
	{
		assert(m_pPrev);
		return m_pPrev;
	}
protected:
	void del()
	{
		assert(m_pPrev);
		assert(m_pNext);
		sIntDel(m_pPrev, m_pNext);
		m_pNext = NULL;
		m_pPrev = NULL;
	}
protected:
	static void sIntDel(CListHeader * pPrev, CListHeader * pNext)
	{
		assert(pPrev);
		assert(pNext);
		pNext->m_pPrev = pPrev;
		pPrev->m_pNext = pNext;
	}
private:
	//FORBID_COPY_CTOR(CListHeader);
	FORBID_OP_EQUAL(CListHeader);
public:
	CListHeader * m_pNext;
	CListHeader * m_pPrev;
};

/*
 * the head of double linked list, you can traverse all the nodes in it.
 */
template<typename subClass_t>
class CListHead : public CListHeader
{
public:
	explicit CListHead() :
		CListHeader()
	{
		// TODO: add
	}

	virtual ~CListHead()
	{
		// TODO: add
	}

public:
	/*
	 * I don't think you need use this function, and it is only used
	 * by macros for traversing whole list.
	 */
	CListHeader * getThis(void)
	{
		return this;
	}
public:
	bool isEmpty() const
	{
		return (m_pNext == this);
	}
private:
	void add(subClass_t * pNew)
	{
		sIntAdd(pNew, this, this->m_pNext);
	}
public:
	void addTail(subClass_t * pNew)
	{
		sIntAdd(pNew, this->m_pPrev, this);
	}

	/*
	 * get the first/last entry of the list
	 * NOTE: I know this sentence is no use, indeed you can know it
	 * once you know the function name:-)
	 * NOTE: if the list is empty, it will return NULL.
	 */
	subClass_t * getFirstEntry(void)
	{
		if (isEmpty())
		{
			return NULL;
		}

		return (subClass_t *)m_pNext;
	}
	subClass_t * getLastEntry(void)
	{
		if (isEmpty())
		{
			return NULL;
		}

		return (subClass_t *)m_pPrev;
	}

	/*
	 * move a new node from original list to *this* list.
	 */
	void accept(subClass_t * pNew)
	{
		assert(pNew);
		sIntDel(pNew->m_pPrev, pNew->m_pNext);
		addTail(pNew);
	}
private:
	static void sIntAdd(
		CListHeader *pNew,
		CListHeader *pPrev,
		CListHeader *pNext)
	{
		pNext->m_pPrev = pNew;
		pNew->m_pNext = pNext;
		pNew->m_pPrev = pPrev;
		pPrev->m_pNext = pNew;
	}
private:
	FORBID_COPY_CTOR(CListHead);
	FORBID_OP_EQUAL(CListHead);
};

/*
 * traverse a double linked list (forwarding).
 * NOTE: you are forbid to modify this list in the *for* section. if
 * you want, pls. use next macro *listForEachSafe*
 */
#define listForEach(pInst, head) \
	for (CListHeader *pInst = head.next();	\
		pInst != head.getThis(); pInst = pInst->next())

/*
 * traverse a double linked list (forwarding) safely.
 * NOTE: you can modify the *pInst* in *for* section, e.g. delete it
 * from the list.
 */
#define listForEachSafe(pInst, head)	\
	for (CListHeader *pInst = head.next(), *pN = pInst->next();	\
		pInst != head.getThis(); pInst = pN, pN = pN->next())

