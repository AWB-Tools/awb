// ==================================================
//Copyright (C) 2003-2006 Intel Corporation
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
  * @file  DBListenerDef.h
  */

#ifndef _DRALDB_DBLISTENERDEF_H
#define _DRALDB_DBLISTENERDEF_H

// QT Library
#include <qstring.h>
#include <qptrlist.h>
#include <qvaluevector.h>
#include <qintdict.h>

// local
#include "asim/draldb_syntax.h"
#include "asim/DRALTag.h"

/** @typedef SOVTagValueStruct
  * @brief
  * Holds the content of a set of values. 64 bits in 32 bits
  * machines.
  */
typedef struct
{
    UINT32   nval;         // 32 bits
    SOVList* sovl;         // 32 bits on 32-bits @-BUS machines (x86 & co)
} SOVTagValueStruct;

/** @typedef union TagValueUnion
  * @brief
  * Holds the content of a tag value : int, string index map or
  * set of values. 64 bits in 32 bits machines.
  */
typedef union
{
    UINT64            value;
    SOVTagValueStruct sovData;
} TagValueUnion;

/**
  * @brief
  * Class used to represent a tag set list.
  *
  * @description
  * This class is used to hold the set of tags information such
  * the type of tag and it's value is stored in
  * this class.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class LSetTagListNode
{
    public:

        /**
         * Default string tag constructor.
         *
         * @return the new object.
         */
        LSetTagListNode (UINT32 cyc,INT32 p_tagid,QString p_str)
        {
            commonInit(cyc,p_tagid);
            isString = 1;
            str = p_str;
        }

        /**
         * Default integer tag constructor.
         *
         * @return the new object.
         */
        LSetTagListNode (UINT32 cyc,INT32 p_tagid,UINT64 p_value)
        {
            commonInit(cyc,p_tagid);
            data.value = p_value;
        }

        /**
         * Default set of values tag constructor.
         *
         * @return the new object.
         */
        LSetTagListNode (UINT32 cyc,INT32 p_tagid, UINT32 p_nval,
                        UINT64* p_value_array)
        {
            commonInit(cyc,p_tagid);

            data.sovData.nval = p_nval;
            data.sovData.sovl = new SOVList(p_nval);
            Q_ASSERT(data.sovData.sovl!=NULL);
            for (UINT32 i=0; i<p_nval;i++) data.sovData.sovl->append(p_value_array[i]);
            isSOV = 1;
        }

        /**
         * Deletes the set of value data if was allocated.
         *
         * @return destroys the object.
         */
        ~LSetTagListNode()
        {
            if (isSOV) delete data.sovData.sovl;
        }

    public:
        // this is performance critical so we make them public.
        TagValueUnion data;
        UINT32        cycle;
        UINT16        tagid;
        QString       str;   // I cannot put objects in an union...

        UBYTE    isMutable : 1;
        UBYTE    used      : 1;
        UBYTE    isSOV     : 1;
        UBYTE    isString  : 1;

    private:
        /**
         * Common init of all the creators of this class.
         *
         * @return void.
         */
        inline void commonInit(UINT32 cyc,UINT16 p_tagid)
        {
            cycle = cyc;
            isMutable = 0;
            used = 0;
            tagid = p_tagid;
            isSOV = 0;
            isString=0;
        }
};

/** @typedef NewTagList
  * @brief
  * Hols a tag list of an item. Maximum 256k tag values per item.
  */
typedef AEVector<LSetTagListNode*,1024,256> NewTagList;

/**
  * @brief
  * Holds a move item information.
  *
  * @description
  * All the information related to a move item is stored in this
  * class : cycle, edge and pos.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class LMoveItemListNode
{
    public:
        /**
         * Default move item constructor.
         *
         * @return the new object.
         */
        LMoveItemListNode()
        {
            cycle=0;
            edgeid=0;
            pos=0;
        }

        /**
         * Parametrized move item constructor
         *
         * @return the new object.
         */
        LMoveItemListNode(UINT32 _cycle, UINT16 _edgeid, UINT16 _pos )
        {
            cycle = _cycle;
            edgeid=_edgeid;
            pos=_pos;
        };

    public:
        // this is performance critical so we make them public.
        UINT32 cycle;
        UINT16 edgeid;
        UINT16 pos;
};

/** @typdef MoveItemList
  * @brief
  * List of move items. Maximum of 16k movements per item.
  */
typedef AEVector<LMoveItemListNode,128,128> MoveItemList;

/**
  * @brief
  * Holds information of an enter / exit node.
  *
  * @description
  * The track id of the destination node is kept in this class
  * to allow fast access when processed. The cycle when the move
  * is done is saved too.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class LEENodeListNode
{
    public:
        /**
         * Default exit enter node creator.
         *
         * @return the new object.
         */
        LEENodeListNode()
        {
            cycle=0;
            track_id = -1;
        }

        /**
         * Parametrized enter exit node creator.
         *
         * @return the new object.
         */
        LEENodeListNode (UINT32 cyc, INT32 p_track_id)
        {
            //item_id  = p_item_id;
            cycle    = cyc;
            track_id = p_track_id;
        };

    public: // this is performance critical so we make them public
        UINT32   cycle;
        INT32    track_id;
};

/** @typdef MoveItemList
  * @brief
  * List of enter/exit nodes.
  */
typedef FValueVector<LEENodeListNode> EENodeList;

/**
  * @brief
  * This class has all the information of an item.
  *
  * @description
  * This class is used to hold the information of an item until
  * it is deleted. This data includes the item tags, the movements
  * and the enter and exit nodes.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class LNewItemListNode
{
    public:
        /**
         * Default creator of an alive item.
         *
         * @return the new object.
         */
        LNewItemListNode(UINT32 cyc, UINT32 iid)
        {
            myTags = NULL; // lazy init
            myMovs = NULL;
            myEnterNodes = NULL;
            myExitNodes = NULL;
            item_id = iid;
            cycle = cyc;
        }

        /**
         * All the lists that were allocated are erased.
         *
         * @return destroys the object.
         */
        ~LNewItemListNode()
        {
            if (myMovs!=NULL)
            {
                delete myMovs;
            }
            if (myEnterNodes!=NULL)
            {
                delete myEnterNodes;
            }
            if (myExitNodes!=NULL)
            {
                delete myExitNodes;
            }
            if (myTags!=NULL)
            {
                // delete all the LSetTagListNode elements
                for (int i=0;i<=myTags->getMaxUsedIdx();i++)
                {
                    delete (*myTags)[i];
                }
                delete myTags;
            }
        }

    public:
        /**
         * Looks if the item has tags.
         *
         * @return if the item has tags.
         */
        bool hasTags()
        {
            return myTags !=NULL;
        }

        /**
         * Looks if the item has movements.
         *
         * @return if the item has movements.
         */
        bool
        hasMovs()
        {
            return myMovs !=NULL;
        }

        /**
         * Looks if the item has enter nodes.
         *
         * @return if the item has enter nodes.
         */
        bool
        hasEnterNodes()
        {
            return myEnterNodes !=NULL;
        }

        /**
         * Looks if the item has exit nodes.
         *
         * @return if the item has exit nodes.
         */
        bool
        hasExitNodes()
        {
            return myExitNodes !=NULL;
        }

        /**
         * Returns the tag list of the item. If it doesn't exists
         * then is created.
         *
         * @return the tags list.
         */
        NewTagList*
        getMyTags()
        {
            if (myTags==NULL)
            {
                myTags = new NewTagList();
                //myTags->setAutoDelete(true);
            }
            return myTags;
        }

        /**
         * Returns the movement list of the item. If it doesn't exists
         * then is created.
         *
         * @return the movement list.
         */
        MoveItemList*
        getMyMovs()
        {
            if (myMovs==NULL)
            {
                myMovs = new MoveItemList();
            }
            return myMovs;
        }

        /**
         * Returns the enter node list of the item. If it doesn't exists
         * then is created.
         *
         * @return the enter node list.
         */
        EENodeList*
        getMyEnterNodes()
        {
            if (myEnterNodes==NULL)
            {
                myEnterNodes = new EENodeList(16); // 64 before
            }
            return myEnterNodes;
        }

        /**
         * Returns the exit node list of the item. If it doesn't exists
         * then is created.
         *
         * @return the exit node list.
         */
        EENodeList*
        getMyExitNodes()
        {
            if (myExitNodes==NULL)
            {
                myExitNodes = new EENodeList(16); // 64 before
            }
            return myExitNodes;
        }

    public:
        // this is performance critical so we make them public
        UINT32   item_id;
        UINT32   cycle;

    protected:
        NewTagList*   myTags;
        MoveItemList* myMovs;
        EENodeList*   myExitNodes;
        EENodeList*   myEnterNodes;

    private:
        static UINT64 _tagListCnt;
        static UINT64 _tagListAcum;
};

/** @typdef NewItemList
  * @brief
  * A list of items.
  */
typedef QIntDict<LNewItemListNode> NewItemList;

class AuxItemListNode
{
    public:
       AuxItemListNode() { cycle =0; item_id=0; }
       AuxItemListNode(UINT32 cyc, UINT32 iid)
       {
           item_id = iid;
           cycle = cyc;
       }

    public: // this is performance critical so we make them public
       UINT32   item_id;
       UINT32   cycle;
};

class AuxItemList : public QPtrList<AuxItemListNode>
{
   public:
        AuxItemList() : QPtrList<AuxItemListNode>()
        {setAutoDelete(true);}

    protected:
        inline int compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 );
};

int
AuxItemList::compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 )
{
    AuxItemListNode* node1 = (AuxItemListNode*)item1;
    AuxItemListNode* node2 = (AuxItemListNode*)item2;
    return node1->cycle - node2->cycle;
}

#endif

