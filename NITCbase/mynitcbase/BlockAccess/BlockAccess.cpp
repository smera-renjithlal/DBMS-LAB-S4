#include "BlockAccess.h"
#include <iostream>
#include <cstring>

RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op) {
    RecId prevRecId;
    //gets search index from cache(not success if not in cache)
    if (RelCacheTable::getSearchIndex(relId, &prevRecId) != SUCCESS) {
        return RecId{-1, -1};
    }

    int block, slot;
    //if searchid has been reset -> get the block from relcat in relcache[relid] and set slot=0
    if (prevRecId.block == -1 && prevRecId.slot == -1) {
        RelCatEntry relCat;
        if (RelCacheTable::getRelCatEntry(relId, &relCat) != SUCCESS) {
            return RecId{-1, -1};
        }
        block = relCat.firstBlk;
        slot = 0;
    } 
    //if not, get the block from prev recid and update the slot+1
    else {
        block = prevRecId.block;
        slot = prevRecId.slot + 1;
    }

    //if the slots have exceeded the numSlotsPerBlk, set block to next block ie rblock
    while (block != -1) {
        RecBuffer recBuffer(block);
        struct HeadInfo head;
        recBuffer.getHeader(&head);
        unsigned char slotMap[head.numSlots];
        recBuffer.getSlotMap(slotMap);

        if (slot >= head.numSlots) {
            block = head.rblock;
            slot = 0;
            continue;
        }

        //iterate through slots in block and skip unoccupied ones
        while (slot < head.numSlots) {
            if (slotMap[slot] == SLOT_UNOCCUPIED) {
                slot++;
                continue;
            }

            union Attribute record[head.numAttrs];
            recBuffer.getRecord(record, slot);

            AttrCatEntry attrCatEntry;
            if (AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry) != SUCCESS) {
                return RecId{-1, -1};
            }

            //the attr in the record and attrVal to compare it with are compared
            //if it satisfies the operation the searchindex is updated and returned
            int cmpVal = compareAttrs(record[attrCatEntry.offset], attrVal, attrCatEntry.attrType);
            if ((op == NE && cmpVal != 0) ||
                (op == LT && cmpVal < 0) ||
                (op == LE && cmpVal <= 0) ||
                (op == EQ && cmpVal == 0) ||
                (op == GT && cmpVal > 0) ||
                (op == GE && cmpVal >= 0)) {
                RecId foundRec = {block, slot};
                RelCacheTable::setSearchIndex(relId, &foundRec);
                return foundRec;
            }
            slot++;
        }
    }
    return RecId{-1, -1};
}

int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE]){
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */

    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute newRelationName;    // set newRelationName with newName
    strcpy(newRelationName.sVal,newName);

    // search the relation catalog for an entry with "RelName" = newRelationName
    RecId ls = linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, newRelationName,EQ);

    // If relation with name newName already exists (result of linearSearch
    //                                               is not {-1, -1})
    //    return E_RELEXIST;
    if(!(ls == (RecId){-1,-1})) return E_RELEXIST;


    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */

    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute oldRelationName;    // set oldRelationName with oldName
    strcpy(oldRelationName.sVal, oldName);

    // search the relation catalog for an entry with "RelName" = oldRelationName
    // If relation with name oldName does not exist (result of linearSearch is {-1, -1})
    //    return E_RELNOTEXIST;
    ls = linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, oldRelationName,EQ);
    if(ls==(RecId){-1,-1}){
        return E_RELNOTEXIST;
    }

    /* get the relation catalog record of the relation to rename using a RecBuffer
       on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
    */
    /* update the relation name attribute in the record with newName.
       (use RELCAT_REL_NAME_INDEX) */
    // set back the record value using RecBuffer.setRecord
    RecBuffer relBuf(RELCAT_BLOCK);
    Attribute relRecord[RELCAT_NO_ATTRS];
    relBuf.getRecord(relRecord, ls.slot);

    strcpy(relRecord[RELCAT_REL_NAME_INDEX].sVal, newName);
    relBuf.setRecord(relRecord, ls.slot);


    /*
    update all the attribute catalog entries in the attribute catalog corresponding
    to the relation with relation name oldName to the relation name newName
    */

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    //for i = 0 to numberOfAttributes :
    //    linearSearch on the attribute catalog for relName = oldRelationName
    //    get the record using RecBuffer.getRecord
    //
    //    update the relName field in the record to newName
    //    set back the record using RecBuffer.setRecord
    for(int i = 0; i < relRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal; i++){
        ls = linearSearch(ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME, oldRelationName, EQ);
        RecBuffer buf(ls.block);
        Attribute rec[ATTRCAT_NO_ATTRS];
        buf.getRecord(rec, ls.slot);

        strcpy(rec[ATTRCAT_REL_NAME_INDEX].sVal, newName);
        buf.setRecord(rec, ls.slot);

    }

    return SUCCESS;
}

int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute relNameAttr;    // set relNameAttr to relName
    strcpy(relNameAttr.sVal, relName);

    // Search for the relation with name relName in relation catalog using linearSearch()
    // If relation with name relName does not exist (search returns {-1,-1})
    //    return E_RELNOTEXIST;
    RecId ind = linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, relNameAttr, EQ);
    if(ind==(RecId){-1,-1}){
        return E_RELNOTEXIST;
    }

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    
    /* declare variable attrToRenameRecId used to store the attr-cat recId
    of the attribute to rename */
    RecId attrToRenameRecId = {-1, -1};

    /* iterate over all Attribute Catalog Entry record corresponding to the
       relation to find the required attribute */
    while (true) {
        // linear search on the attribute catalog for RelName = relNameAttr
        RecId index = linearSearch(ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME, relNameAttr, EQ);

        // if there are no more attributes left to check (linearSearch returned {-1,-1})
        //     break;
        if(index==(RecId){-1,-1}) break;

        /* Get the record from the attribute catalog using RecBuffer.getRecord
          into attrCatEntryRecord */
        RecBuffer buf(index.block);
        Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];
        buf.getRecord(attrCatEntryRecord, index.slot);

        // if attrCatEntryRecord.attrName = oldName
        //     attrToRenameRecId = block and slot of this record
        if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, oldName)==0){
            attrToRenameRecId = index;
            break;
        }

        // if attrCatEntryRecord.attrName = newName
        //     return E_ATTREXIST;
        if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName)==0){
            return E_ATTREXIST;
        }
    }

    // if attrToRenameRecId == {-1, -1}
    //     return E_ATTRNOTEXIST;
    if(attrToRenameRecId==(RecId){-1,-1}) return E_ATTRNOTEXIST;


    // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
    /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
         attrToRenameRecId.slot */
    //   update the AttrName of the record with newName
    //   set back the record with RecBuffer.setRecord
    RecBuffer renameBuf(attrToRenameRecId.block);
    Attribute renameRec[ATTRCAT_NO_ATTRS];
    renameBuf.getRecord(renameRec, attrToRenameRecId.slot);

    strcpy(renameRec[ATTRCAT_ATTR_NAME_INDEX].sVal, newName);
    renameBuf.setRecord(renameRec, attrToRenameRecId.slot);


    return SUCCESS;
}