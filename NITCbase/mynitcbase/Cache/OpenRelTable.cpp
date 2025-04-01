#include "OpenRelTable.h"
#include <iostream>
#include <cstring>

int test2 = 0;

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
    //STAGE 5
    OpenRelTable::tableMetaInfo[i].free = true;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT
  
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

  struct RelCacheEntry attrCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &attrCacheEntry.relCatEntry);
  attrCacheEntry.recId.block = RELCAT_BLOCK;
  attrCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrCacheEntry;

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]


  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  //attrCatBlock.getRecord(attrCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

  // iterate through all the attributes of the relation catalog and create a linked
  // list of AttrCacheEntry (slots 0 to 5)
  // for each of the entries, set
  //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  //    attrCacheEntry.recId.slot = i   (0 to 5)
  //    and attrCacheEntry.next appropriately
  // NOTE: allocate each entry dynamically using malloc
  
  AttrCacheEntry* AttrCachelistSupport=nullptr;
  AttrCacheEntry* AttrCacheListTail=nullptr;
  
  for(int i=0;i<6;i++){
    attrCatBlock.getRecord(attrCatRecord,i);
    AttrCacheEntry* entry = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &entry->attrCatEntry);
    entry->recId.block = ATTRCAT_BLOCK;
    entry->recId.slot = i;
    entry->next = nullptr;
    
    if(AttrCacheListTail) AttrCacheListTail->next=entry;
    else AttrCachelistSupport=entry;
    AttrCacheListTail=entry;
  }


  // set the next field in the last entry to nullptr

  AttrCacheTable::attrCache[RELCAT_RELID] = AttrCachelistSupport;

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately
  AttrCachelistSupport=nullptr;
  AttrCacheListTail=nullptr;
  
  for(int i=6;i<12;i++){
    attrCatBlock.getRecord(attrCatRecord,i);
    AttrCacheEntry* entry = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &entry->attrCatEntry);
    entry->recId.block = ATTRCAT_BLOCK;
    entry->recId.slot = i;
    entry->next = nullptr;
    
    if(AttrCacheListTail) AttrCacheListTail->next=entry;
    else AttrCachelistSupport=entry;
    AttrCacheListTail=entry;
  }

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
  AttrCacheTable::attrCache[ATTRCAT_RELID] = AttrCachelistSupport;
  
  /************ Setting up tableMetaInfo entries ************/

  // in the tableMetaInfo array
  //   set free = false for RELCAT_RELID and ATTRCAT_RELID
  //   set relname for RELCAT_RELID and ATTRCAT_RELID
  tableMetaInfo[RELCAT_RELID].free = false;
  tableMetaInfo[ATTRCAT_RELID].free = false;
  strcpy(tableMetaInfo[RELCAT_RELID].relName, RELCAT_RELNAME);
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName, ATTRCAT_RELNAME);
}

OpenRelTable::~OpenRelTable() {
  // close all open relations (from rel-id = 2 onwards. Why?)
  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }

  /**** Closing the catalog relations in the relation cache ****/
  //releasing the relation cache entry of the attribute catalog
  if (RelCacheTable::relCache[ATTRCAT_RELID]->dirty)
	{
    /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
    // declaring an object of RecBuffer class to write back to the buffer
		RelCatEntry relCatBuffer;
		RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &relCatBuffer);

		Attribute relCatRecord [RELCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&relCatBuffer, relCatRecord);

		RecId recId = RelCacheTable::relCache[ATTRCAT_RELID]->recId;
    
    RecBuffer relCatBlock(recId.block);
		// Write back to the buffer using relCatBlock.setRecord() with recId.slot
    relCatBlock.setRecord(relCatRecord, recId.slot);
  }
	// free the memory dynamically allocated to this RelCacheEntry
  free(RelCacheTable::relCache[ATTRCAT_RELID]);

	//releasing the relation cache entry of the relation catalog
  if (RelCacheTable::relCache[RELCAT_RELID]->dirty)
	{
    /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
    // declaring an object of RecBuffer class to write back to the buffer
		RelCatEntry relCatBuffer;
		RelCacheTable::getRelCatEntry(RELCAT_RELID, &relCatBuffer);

		Attribute relCatRecord [RELCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&relCatBuffer, relCatRecord);

		RecId recId = RelCacheTable::relCache[RELCAT_RELID]->recId;

    RecBuffer relCatBlock(recId.block);
    // Write back to the buffer using relCatBlock.setRecord() with recId.slot
		relCatBlock.setRecord(relCatRecord, recId.slot);
  }
  // free the memory dynamically allocated for this RelCacheEntry
	free(RelCacheTable::relCache[RELCAT_RELID]);

  // free the memory allocated for the attribute cache entries of the
  // relation catalog and the attribute catalog
	for (int relId = ATTRCAT_RELID; relId >= RELCAT_RELID; relId--)
	{
		AttrCacheEntry *head = AttrCacheTable::attrCache[relId], *next = nullptr;
		while(head != nullptr)
		{
			next = head->next;

			free (head);
			head = next;
		}
	}
}

//STAGE 4
/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.

int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {
  if (strcmp(relName, RELCAT_RELNAME)==0){
    return RELCAT_RELID;
  }
  if (strcmp(relName,ATTRCAT_RELNAME)==0){
    return ATTRCAT_RELID;
  }

  return E_RELNOTOPEN;
}
*/

//STAGE 5
int OpenRelTable::getFreeOpenRelTableEntry() {

  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
    for (int i = 0; i < MAX_OPEN; ++i) {
      if(tableMetaInfo[i].free){
        return i;
      }
    }

  // if found return the relation id, else return E_CACHEFULL.
  return E_CACHEFULL;
}

int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  /* traverse through the tableMetaInfo array,
    find the entry in the Open Relation Table corresponding to relName.*/
    for (int i = 0; i < MAX_OPEN; ++i) {
      if(strcmp(tableMetaInfo[i].relName, relName)==0 && tableMetaInfo[i].free == false){
        return i;
      }
    }

  // if found return the relation id, else indicate that the relation do not
  // have an entry in the Open Relation Table.
  return E_RELNOTOPEN;
}

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
  int val = getRelId(relName);
  if(val!=E_RELNOTOPEN){
    // (checked using OpenRelTable::getRelId())

    // return that relation id;
    return val;
  }

  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */
  int relId = OpenRelTable::getFreeOpenRelTableEntry();
  if (relId==E_CACHEFULL){
    return E_CACHEFULL;
  }

  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/
  RelCacheTable::resetSearchIndex(RELCAT_RELID);
  
  Attribute attrVal; 
  strcpy(attrVal.sVal, relName);
  
  RecId relcatRecId;
  relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, attrVal, EQ, &test2);


  if (relcatRecId == (RecId){-1, -1}) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */
 RecBuffer relBuf(relcatRecId.block);
 Attribute record[RELCAT_NO_ATTRS];
 relBuf.getRecord(record, relcatRecId.slot);

 RelCacheEntry* relCacheBuffer = (RelCacheEntry*) malloc (sizeof(RelCacheEntry));

 RelCacheTable::recordToRelCatEntry(record,&(relCacheBuffer->relCatEntry));
 relCacheBuffer->recId.block=relcatRecId.block;
 relCacheBuffer->recId.slot=relcatRecId.slot;
 RelCacheTable::relCache[relId]=relCacheBuffer;

  /****** Setting up Attribute Cache entry for the relation ******/

  // let listHead be used to hold the head of the linked list of attrCache entries.
  AttrCacheEntry* listSupport = nullptr;
  AttrCacheEntry* listHead = nullptr;
  int noOfAttrs=RelCacheTable::relCache[relId]->relCatEntry.numAttrs;

  listHead = listSupport = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));

  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/

  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  for(int i = 0; i < noOfAttrs; i++)
  {
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      RecId attrcatRecId;
      attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, RELCAT_ATTR_RELNAME, attrVal, EQ, &test2);
      
      /* read the record entry corresponding to attrcatRecId and create an
      Attribute Cache entry on it using RecBuffer::getRecord() and
      AttrCacheTable::recordToAttrCatEntry().
      update the recId field of this Attribute Cache entry to attrcatRecId.
      add the Attribute Cache entry to the linked list of listHead .*/
      // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
      RecBuffer attrCatBlock(attrcatRecId.block);
      attrCatBlock.getRecord(attrCatRecord, attrcatRecId.slot);

      AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &listSupport->attrCatEntry);

      listSupport->recId = attrcatRecId;
      
      if (i < noOfAttrs - 1) {
        listSupport->next = (AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));
        listSupport = listSupport->next;
    } else {
        listSupport->next = nullptr;
    }
  }

  // set the relIdth entry of the AttrCacheTable to listHead.
  AttrCacheTable::attrCache[relId] = listHead;

  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.
  tableMetaInfo[relId].free = false;
	strcpy(tableMetaInfo[relId].relName, relName);

  return relId;
}

int OpenRelTable::closeRel(int relId) {
  if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) {
    return E_NOTPERMITTED;
  }

  if (relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  if (tableMetaInfo[relId].free) {
    return E_RELNOTOPEN;
  }

  if (RelCacheTable::relCache[relId]->dirty == true) {
		/* Get the Relation Catalog entry from RelCacheTable::relCache
		Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
		Attribute relCatBuffer [RELCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&(RelCacheTable::relCache[relId]->relCatEntry), relCatBuffer);

		// declaring an object of RecBuffer class to write back to the buffer
		RecId recId = RelCacheTable::relCache[relId]->recId;
		RecBuffer relCatBlock(recId.block);

		// Write back to the buffer using relCatBlock.setRecord() with recId.slot
		relCatBlock.setRecord(relCatBuffer, RelCacheTable::relCache[relId]->recId.slot);
	}

  // Free the memory allocated in the relation cache
  if(RelCacheTable::relCache[relId] != nullptr) free(RelCacheTable::relCache[relId]);

  // Free the memory allocated in the attribute cache (linked list)
  if(AttrCacheTable::attrCache[relId] != nullptr){
    AttrCacheEntry* head = AttrCacheTable::attrCache[relId];
    while (head != nullptr) {
      AttrCacheEntry* next = head->next;
      free(head);  // Free the current entry
      head = next; // Move to the next entry
    }
  }

  // Update `tableMetaInfo` to set `relId` as a free slot
  tableMetaInfo[relId].free = true;
  RelCacheTable::relCache[relId] = nullptr;
  AttrCacheTable::attrCache[relId] = nullptr;

  return SUCCESS;
}
