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

int BlockAccess::insert(int relId, Attribute *record) {
    // get the relation catalog entry from relation cache
    // ( use RelCacheTable::getRelCatEntry() of Cache Layer)
    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(relId, &relCatEntry);

    int blockNum = relCatEntry.firstBlk /* first record block of the relation (from the rel-cat entry)*/;

    // rec_id will be used to store where the new record will be inserted
    RecId rec_id = {-1, -1};

    int numOfSlots = relCatEntry.numSlotsPerBlk /* number of slots per record block */;
    int numOfAttributes = relCatEntry.numAttrs /* number of attributes of the relation */;

    int prevBlockNum = -1 /* block number of the last element in the linked list = -1 */;

    /*
        Traversing the linked list of existing record blocks of the relation
        until a free slot is found OR
        until the end of the list is reached
    */
    while (blockNum != -1) {
        // create a RecBuffer object for blockNum (using appropriate constructor!)
        RecBuffer recBuf(blockNum);

        // get header of block(blockNum) using RecBuffer::getHeader() function
        HeadInfo head;
        recBuf.getHeader(&head);

        // get slot map of block(blockNum) using RecBuffer::getSlotMap() function
        unsigned char slotMap[head.numSlots];
        recBuf.getSlotMap(slotMap);

        // search for free slot in the block 'blockNum' and store it's rec-id in rec_id
        // (Free slot can be found by iterating over the slot map of the block)
        /* slot map stores SLOT_UNOCCUPIED if slot is free and
           SLOT_OCCUPIED if slot is occupied) */
        for(int i=0;i<head.numSlots;i++){
            /* if a free slot is found, set rec_id and discontinue the traversal
           of the linked list of record blocks (break from the loop) */
            if(slotMap[i] == SLOT_UNOCCUPIED){
                rec_id = RecId{blockNum, i};
                break;
            }
        }
        if(!(rec_id == (RecId){-1,-1})) break;

        /* otherwise, continue to check the next block by updating the
           block numbers as follows:
              update prevBlockNum = blockNum
              update blockNum = header.rblock (next element in the linked
                                               list of record blocks)
        */
       prevBlockNum = blockNum;
       blockNum = head.rblock;
    }

    //  if no free slot is found in existing record blocks (rec_id = {-1, -1})
    if(rec_id == RecId{-1,-1}){
        // if relation is RELCAT, do not allocate any more blocks
        //     return E_MAXRELATIONS;
        if(relId == RELCAT_RELID) return E_MAXRELATIONS;

        // Otherwise,
        // get a new record block (using the appropriate RecBuffer constructor!)
        // get the block number of the newly allocated block
        // (use BlockBuffer::getBlockNum() function)
        // let ret be the return value of getBlockNum() function call
        RecBuffer recBuf;
        blockNum = recBuf.getBlockNum();
        if (blockNum == E_DISKFULL) {
            return E_DISKFULL;
        }

        // Assign rec_id.block = new block number(i.e. ret) and rec_id.slot = 0
        rec_id = RecId{blockNum,0};
        /*
            set the header of the new record block such that it links with
            existing record blocks of the relation
            set the block's header as follows:
            blockType: REC, pblock: -1
            lblock
                  = -1 (if linked list of existing record blocks was empty
                         i.e this is the first insertion into the relation)
                  = prevBlockNum (otherwise),
            rblock: -1, numEntries: 0,
            numSlots: numOfSlots, numAttrs: numOfAttributes
            (use BlockBuffer::setHeader() function)
        */
       HeadInfo head;
       head.blockType = REC;
       head.lblock = prevBlockNum;
       head.rblock = head.pblock = -1;
       head.numAttrs = numOfAttributes;
       head.numSlots = numOfSlots;
       head.numEntries = 0;

       recBuf.setHeader(&head);

        /*
            set block's slot map with all slots marked as free
            (i.e. store SLOT_UNOCCUPIED for all the entries)
            (use RecBuffer::setSlotMap() function)
        */
       unsigned char slotMap[numOfSlots];
       for(int i=0;i<numOfSlots;i++){
        slotMap[i] = SLOT_UNOCCUPIED;
       }
       recBuf.setSlotMap(slotMap);

        // if prevBlockNum != -1
        if(prevBlockNum != -1){
            // create a RecBuffer object for prevBlockNum
            // get the header of the block prevBlockNum and
            // update the rblock field of the header to the new block
            // number i.e. rec_id.block
            // (use BlockBuffer::setHeader() function)
            RecBuffer prevBuf(prevBlockNum);
            HeadInfo prevHead;
            prevBuf.getHeader(&prevHead);
            prevHead.rblock = blockNum;
            prevBuf.setHeader(&prevHead);
        }
        // else
        else{
            // update first block field in the relation catalog entry to the
            // new block (using RelCacheTable::setRelCatEntry() function)
            relCatEntry.firstBlk = blockNum;
            RelCacheTable::setRelCatEntry(relId,&relCatEntry);
        }

        // update last block field in the relation catalog entry to the
        // new block (using RelCacheTable::setRelCatEntry() function)
        relCatEntry.lastBlk = blockNum;
        RelCacheTable::setRelCatEntry(relId,&relCatEntry);
    }

    // create a RecBuffer object for rec_id.block
    // insert the record into rec_id'th slot using RecBuffer.setRecord())
    RecBuffer blockBuf(rec_id.block);
    blockBuf.setRecord(record, rec_id.slot);

    /* update the slot map of the block by marking entry of the slot to
       which record was inserted as occupied) */
    // (ie store SLOT_OCCUPIED in free_slot'th entry of slot map)
    // (use RecBuffer::getSlotMap() and RecBuffer::setSlotMap() functions)
    unsigned char slotMap[numOfSlots];
    blockBuf.getSlotMap(slotMap);

    slotMap[rec_id.slot] = SLOT_OCCUPIED;
    blockBuf.setSlotMap(slotMap);

    // increment the numEntries field in the header of the block to
    // which record was inserted
    // (use BlockBuffer::getHeader() and BlockBuffer::setHeader() functions)
    HeadInfo blockHead;
    blockBuf.getHeader(&blockHead);

    blockHead.numEntries++;
    blockBuf.setHeader(&blockHead);

    // Increment the number of records field in the relation cache entry for
    // the relation. (use RelCacheTable::setRelCatEntry function)
    relCatEntry.numRecs++;
    RelCacheTable::setRelCatEntry(relId,&relCatEntry);
    
    /* B+ Tree Insertions */
    // (the following section is only relevant once indexing has been implemented)

    int flag = SUCCESS;
    // Iterate over all the attributes of the relation
    // (let attrOffset be iterator ranging from 0 to numOfAttributes-1)
	for (int attrindex = 0; attrindex < numOfAttributes; attrindex++)
    {
        // get the attribute catalog entry for the attribute from the attribute cache
        // (use AttrCacheTable::getAttrCatEntry() with args relId and attrOffset)
		AttrCatEntry attrCatEntryBuffer;
		AttrCacheTable::getAttrCatEntry(relId, attrindex, &attrCatEntryBuffer);

        // get the root block field from the attribute catalog entry
		int rootBlock = attrCatEntryBuffer.rootBlock;

        // if index exists for the attribute(i.e. rootBlock != -1)
		if (rootBlock != -1)
        {
            /* insert the new record into the attribute's bplus tree using
             BPlusTree::bPlusInsert()*/
            int ret = BPlusTree::bPlusInsert(relId, attrCatEntryBuffer.attrName, record[attrindex], rec_id);

            if (ret == E_DISKFULL) {
                //(index for this attribute has been destroyed)
                // flag = E_INDEX_BLOCKS_RELEASED
                flag = E_INDEX_BLOCKS_RELEASED;
            }
        }
    }

    return flag;
}

int BlockAccess::search(int relId, Attribute *record, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
    // Declare a variable called recid to store the searched record
    RecId recId;

    /* get the attribute catalog entry from the attribute cache corresponding
    to the relation with Id=relid and with attribute_name=attrName  */
    // if this call returns an error, return the appropriate error code
    AttrCatEntry attrCatEntry;
	int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);
    if (ret != SUCCESS) return ret;

    // get rootBlock from the attribute catalog entry
    int rootBlock = attrCatEntry.rootBlock;
    /* if Index does not exist for the attribute (check rootBlock == -1) */ 
    if (rootBlock == -1){
        /* search for the record id (recid) corresponding to the attribute with
           attribute name attrName, with value attrval and satisfying the
           condition op using linearSearch()
        */
       recId = linearSearch(relId, attrName, attrVal, op);
    }

    /* else */ 
    else{
        // (index exists for the attribute)
        /* search for the record id (recid) correspoding to the attribute with
        attribute name attrName and with value attrval and satisfying the
        condition op using BPlusTree::bPlusSearch() */
        recId = BPlusTree::bPlusSearch(relId, attrName, attrVal, op);
    }


    // if there's no record satisfying the given condition (recId = {-1, -1})
    //     return E_NOTFOUND;
    if (recId == RecId{-1, -1}) return E_NOTFOUND;

    /* Copy the record with record id (recId) to the record buffer (record).
       For this, instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */
   RecBuffer recordBuffer(recId.block);
   recordBuffer.getRecord(record, recId.slot);
   
   return SUCCESS;
}

/*
NOTE: the caller is expected to allocate space for the argument `record` based
      on the size of the relation. This function will only copy the result of
      the projection onto the array pointed to by the argument.
*/
int BlockAccess::project(int relId, Attribute *record) {
    // get the previous search index of the relation relId from the relation
    // cache (use RelCacheTable::getSearchIndex() function)
    RecId prevRecId;
	RelCacheTable::getSearchIndex(relId, &prevRecId);

    // declare block and slot which will be used to store the record id of the
    // slot we need to check.
    int block, slot;

    /* if the current search index record is invalid(i.e. = {-1, -1})
       (this only happens when the caller reset the search index)
    */
    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        // (new project operation. start from beginning)

        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)

        // block = first record block of the relation
        // slot = 0
        RelCatEntry relCatEntryBuffer;
		RelCacheTable::getRelCatEntry(relId, &relCatEntryBuffer);

		block = relCatEntryBuffer.firstBlk;
        slot = 0;
    }
    else
    {
        // (a project/search operation is already in progress)

        // block = previous search index's block
        // slot = previous search index's slot + 1
        block = prevRecId.block;
        slot = prevRecId.slot + 1;
    }


    // The following code finds the next record of the relation
    /* Start from the record id (block, slot) and iterate over the remaining
       records of the relation */
    while (block != -1)
    {
        // create a RecBuffer object for block (using appropriate constructor!)
        RecBuffer buffer (block);

        // get header of the block using RecBuffer::getHeader() function
        // get slot map of the block using RecBuffer::getSlotMap() function
        HeadInfo header;
		buffer.getHeader(&header);

		unsigned char slotmap [header.numSlots];
		buffer.getSlotMap(slotmap);

        if(slot >= header.numSlots /* slot >= the number of slots per block*/)
        {
            // (no more slots in this block)
            // update block = right block of block
            // update slot = 0
            // (NOTE: if this is the last block, rblock would be -1. this would
            //        set block = -1 and fail the loop condition )
            block = header.rblock;
            slot = 0;
        }
        else if (slotmap[slot] == SLOT_UNOCCUPIED /* slot is free */)
        { // (i.e slot-th entry in slotMap contains SLOT_UNOCCUPIED)

            // increment slot
            slot++;
        }
        else {
            // (the next occupied slot / record has been found)
            break;
        }
    }

    if (block == -1){
        // (a record was not found. all records exhausted)
        return E_NOTFOUND;
    }

    // declare nextRecId to store the RecId of the record found
    RecId nextRecId{block, slot};

    // set the search index to nextRecId using RelCacheTable::setSearchIndex
    RelCacheTable::setSearchIndex(relId, &nextRecId);

    /* Copy the record with record id (nextRecId) to the record buffer (record)
       For this Instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */
    RecBuffer recordBuffer (block);
    recordBuffer.getRecord(record, slot);

    return SUCCESS;
}

int BlockAccess::deleteRelation(char relName[ATTR_SIZE]) {
    // if the relation to delete is either Relation Catalog or Attribute Catalog,
    //     return E_NOTPERMITTED
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)
    if (strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0)
    return E_NOTPERMITTED;

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameAttr; // (stores relName as type union Attribute)
    // assign relNameAttr.sVal = relName
    strcpy((char*)relNameAttr.sVal,(const char*)relName);

    //  linearSearch on the relation catalog for RelName = relNameAttr
    RecId relCatRecId = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, relNameAttr ,EQ);

    // if the relation does not exist (linearSearch returned {-1, -1})
    //     return E_RELNOTEXIST
    if (relCatRecId == RecId{-1, -1}) return E_RELNOTEXIST;

    Attribute relCatEntryRecord[RELCAT_NO_ATTRS];
    /* store the relation catalog record corresponding to the relation in
       relCatEntryRecord using RecBuffer.getRecord */
       RecBuffer relCatBlockBuffer (relCatRecId.block);
       relCatBlockBuffer.getRecord(relCatEntryRecord, relCatRecId.slot);

    /* get the first record block of the relation (firstBlock) using the
       relation catalog entry record */
    /* get the number of attributes corresponding to the relation (numAttrs)
       using the relation catalog entry record */
    int firstBlk = relCatEntryRecord[RELCAT_FIRST_BLOCK_INDEX].nVal;
    int numAttrs = relCatEntryRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal;

    /*
     Delete all the record blocks of the relation
    */
    // for each record block of the relation:
    //     get block header using BlockBuffer.getHeader
    //     get the next block from the header (rblock)
    //     release the block using BlockBuffer.releaseBlock
    //
    //     Hint: to know if we reached the end, check if nextBlock = -1
    int currentBlkNum = firstBlk;
    while (currentBlkNum != -1) {
		RecBuffer currentBlkBuffer(currentBlkNum);

		HeadInfo currentBlkHeader;
		currentBlkBuffer.getHeader(&currentBlkHeader);

		currentBlkNum = currentBlkHeader.rblock;

		currentBlkBuffer.releaseBlock();
	}

    /***
        Deleting attribute catalog entries corresponding the relation and index
        blocks corresponding to the relation with relName on its attributes
    ***/

    // reset the searchIndex of the attribute catalog
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    int numberOfAttributesDeleted = 0;

    while(true) {
        RecId attrCatRecId;
        // attrCatRecId = linearSearch on attribute catalog for RelName = relNameAttr
        attrCatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, RELCAT_ATTR_RELNAME, relNameAttr, EQ);

        // if no more attributes to iterate over (attrCatRecId == {-1, -1})
        //     break;
        if (attrCatRecId == RecId{-1, -1}) break;
        numberOfAttributesDeleted++;

        // create a RecBuffer for attrCatRecId.block
        // get the header of the block
        // get the record corresponding to attrCatRecId.slot
        RecBuffer attrCatBlockBuffer (attrCatRecId.block);

		HeadInfo attrCatHeader;
		attrCatBlockBuffer.getHeader(&attrCatHeader);

		Attribute attrCatRecord [ATTRCAT_NO_ATTRS];
		attrCatBlockBuffer.getRecord(attrCatRecord, attrCatRecId.slot);

        // declare variable rootBlock which will be used to store the root
        // block field from the attribute catalog record.
        int rootBlock = attrCatRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal /* get root block from the record */;
        // (This will be used later to delete any indexes if it exists)

        // Update the Slotmap for the block by setting the slot as SLOT_UNOCCUPIED
        // Hint: use RecBuffer.getSlotMap and RecBuffer.setSlotMap
        unsigned char slotmap [attrCatHeader.numSlots];
		attrCatBlockBuffer.getSlotMap(slotmap);

		slotmap[attrCatRecId.slot] = SLOT_UNOCCUPIED;
		attrCatBlockBuffer.setSlotMap(slotmap);

        /* Decrement the numEntries in the header of the block corresponding to
           the attribute catalog entry and then set back the header
           using RecBuffer.setHeader */
        attrCatHeader.numEntries--;
		attrCatBlockBuffer.setHeader(&attrCatHeader);

        /* If number of entries become 0, releaseBlock is called after fixing
           the linked list.
        */
        if (attrCatHeader.numEntries == 0 /*   header.numEntries == 0  */) {
            /* Standard Linked List Delete for a Block
               Get the header of the left block and set it's rblock to this
               block's rblock
            */

            // create a RecBuffer for lblock and call appropriate methods
            RecBuffer prevBlock (attrCatHeader.lblock);
			
			HeadInfo leftHeader;
			prevBlock.getHeader(&leftHeader);

			leftHeader.rblock = attrCatHeader.rblock;
			prevBlock.setHeader(&leftHeader);

            if (attrCatHeader.rblock != INVALID_BLOCKNUM /* header.rblock != -1 */) {
                /* Get the header of the right block and set it's lblock to
                   this block's lblock */
                // create a RecBuffer for rblock and call appropriate methods
                RecBuffer nextBlock (attrCatHeader.rblock);
				
				HeadInfo rightHeader;
				nextBlock.getHeader(&rightHeader);

				rightHeader.lblock = attrCatHeader.lblock;
				nextBlock.setHeader(&rightHeader);

            } else {
                // (the block being released is the "Last Block" of the relation.)
                /* update the Relation Catalog entry's LastBlock field for this
                   relation with the block number of the previous block. */
                RelCatEntry relCatEntryBuffer;
                RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);

                relCatEntryBuffer.lastBlk = attrCatHeader.lblock;
                RelCacheTable::setRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);
            }

            // (Since the attribute catalog will never be empty(why?), we do not
            //  need to handle the case of the linked list becoming empty - i.e
            //  every block of the attribute catalog gets released.)

            // call releaseBlock()
            attrCatBlockBuffer.releaseBlock();

			RecId nextSearchIndex = {attrCatHeader.rblock, 0};
			RelCacheTable::setSearchIndex(ATTRCAT_RELID, &nextSearchIndex);
        }

        // (the following part is only relevant once indexing has been implemented)
        // if index exists for the attribute (rootBlock != -1), call bplus destroy
        if (rootBlock != -1) {
            // delete the bplus tree rooted at rootBlock using BPlusTree::bPlusDestroy()
            BPlusTree::bPlusDestroy(rootBlock);
        }
    }

    /*** Delete the entry corresponding to the relation from relation catalog ***/
    // Fetch the header of Relcat block
    HeadInfo relCatHeader;
	relCatBlockBuffer.getHeader(&relCatHeader);

    /* Decrement the numEntries in the header of the block corresponding to the
       relation catalog entry and set it back */
    relCatHeader.numEntries--;
	relCatBlockBuffer.setHeader(&relCatHeader);

    /* Get the slotmap in relation catalog, update it by marking the slot as
       free(SLOT_UNOCCUPIED) and set it back. */
    unsigned char slotmap [relCatHeader.numSlots];
    relCatBlockBuffer.getSlotMap(slotmap);

    slotmap[relCatRecId.slot] = SLOT_UNOCCUPIED;
    relCatBlockBuffer.setSlotMap(slotmap);

    /*** Updating the Relation Cache Table ***/
    /** Update relation catalog record entry (number of records in relation
        catalog is decreased by 1) **/
    // Get the entry corresponding to relation catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)
    RelCatEntry relCatEntryBuffer;
	RelCacheTable::getRelCatEntry(RELCAT_RELID, &relCatEntryBuffer);
    relCatEntryBuffer.numRecs--;
	RelCacheTable::setRelCatEntry(RELCAT_RELID, &relCatEntryBuffer);

    /** Update attribute catalog entry (number of records in attribute catalog
        is decreased by numberOfAttributesDeleted) **/
    // i.e., #Records = #Records - numberOfAttributesDeleted

    // Get the entry corresponding to attribute catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)
    RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);
	relCatEntryBuffer.numRecs -= numberOfAttributesDeleted;
	RelCacheTable::setRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);

    return SUCCESS;
}