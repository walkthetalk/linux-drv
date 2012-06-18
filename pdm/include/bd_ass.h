#pragma once

#include <stdio.h>
#include <assert.h>
#include <iostream>
using namespace std;

#include "cp_hdr.h"

/*
 *   the classes *CBdAssParen* and *CBdAssChild* are for build bidirection
 * association between two item, one is parent, another is child, but
 * indeed you don't need differentiate the p and c, you can see them
 * located in the same position.
 *   For use of these two classes, you can provide two instances, one
 * of them is derived from the CBdAssParent, and the other one is derived
 * from CBdAssChild.
 *   NOTE: indeed you can let the two instances all derived from one
 * of the two classes.
 *   after that, you can use the two macro *buildBdAss* and *destroyBdAss*
 * to build or destroy the association between the two instance.
 */
template<typename child_t>
class CBdAssParent
{
public:
	explicit CBdAssParent() :
		m_pChld(NULL)
	{
		// TODO: ADD content
	}

	virtual ~CBdAssParent()
	{
		// TODO: ADD content
	}

	void attach(child_t & child)
	{
		assert(m_pChld == NULL);
		m_pChld = &child;
	}

	void dettach(void)
	{
		assert(m_pChld);
		m_pChld = NULL;
	}
protected:
	child_t * getOppositeEnd(void)
	{
		return m_pChld;
	}
private:
	//FORBID_COPY_CTOR(CBdAssParent);
	FORBID_OP_EQUAL(CBdAssParent);
private:
	child_t * m_pChld;
};

template<typename parent_t>
class CBdAssChild
{
public:
	explicit CBdAssChild() :
		m_pParent(NULL)
	{
		// TODO: ADD content
	}

	virtual ~CBdAssChild()
	{
		// TODO: ADD content
	}

	void attach(parent_t & parent)
	{
		assert(m_pParent == NULL);
		m_pParent = &parent;
	}

	void dettach(void)
	{
		assert(m_pParent);
		m_pParent = NULL;
	}
protected:
	parent_t * getOppositeEnd(void)
	{
		return m_pParent;
	}
private:
	FORBID_COPY_CTOR(CBdAssChild);
	FORBID_OP_EQUAL(CBdAssChild);
private:
	parent_t * m_pParent;
};

/*
 * build the bidirection association between parent and child
 */
template<typename parentType, typename childType>
void buildBdAss(parentType & parent, childType & child)
{
	parent.attach(child);
	child.attach(parent);
}

/*
 * destroy the bidirection association between parent and child
 */
template<typename parentType, typename childType>
void destroyBdAss(parentType & parent, childType & child)
{
	parent.dettach();
	child.dettach();
}

