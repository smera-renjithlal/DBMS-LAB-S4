#include "BlockBuffer.h"
#include <iostream>
#include <cstdlib>
#include <cstring>

// the declarations for these functions can be found in "BlockBuffer.h"

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

// load the block header into the argument pointer
int BlockBuffer::getHeader(struct HeadInfo *head) {
  unsigned char buffer[BLOCK_SIZE];

  // read the block at this.blockNum into the buffer
  Disk::readBlock(buffer,this->blockNum);

  // populate the numEntries, numAttrs and numSlots fields in *head
  memcpy(&head->numSlots, buffer + 24, 4);
  memcpy(&head->numEntries, buffer+16, 4);
  memcpy(&head->numAttrs, buffer+20, 4);
  memcpy(&head->rblock, buffer+12, 4);
  memcpy(&head->lblock, buffer+8, 4);

  return SUCCESS;
}

// load the record at slotNum into the argument pointer
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  struct HeadInfo head;

  // get the header using this.getHeader() function
  this->getHeader(&head);

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  // read the block at this.blockNum into a buffer
  unsigned char buffer[BLOCK_SIZE];
  Disk::readBlock(buffer,this->blockNum);

  /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
     - each record will have size attrCount * ATTR_SIZE
     - slotMap will be of size slotCount
  */
  int recordSize = attrCount * ATTR_SIZE;
  int slotMapSize = head.numSlots;
  int recordOffset = HEADER_SIZE + slotMapSize + (recordSize * slotNum);
  unsigned char *slotPointer = buffer + recordOffset;

  // load the record into the rec data structure
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
}

//STAGE 4
/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char** bufferPtr){
  static unsigned char buffer[BLOCK_SIZE];

  int ret = Disk::readBlock(buffer, this->blockNum);
  if (ret != SUCCESS) {
    return ret;
  }
  *bufferPtr = buffer;

  return SUCCESS;
} */


int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;
  // get the header of the block using getHeader() function
  this->getHeader(&head);

  int slotCount = head.numSlots;

  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
  memcpy(slotMap, slotMapInBuffer, slotCount);

  return SUCCESS;
}

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

    int diff=0;
    if (attrType == STRING) diff = strcmp(attr1.sVal, attr2.sVal);
    else diff = (attr1.nVal > attr2.nVal) ? 1 : (attr1.nVal < attr2.nVal) ? -1 : 0;

    
    if (diff > 0) return 1;
    if (diff < 0) return -1;
    return 0;
    
}

BlockBuffer::BlockBuffer(int blockNum) {
  // initialise this.blockNum with the argument
  this->blockNum=blockNum;
}

//STAGE 6
/* NOTE: This function will NOT check if the block has been initialised as a
   record or an index block. It will copy whatever content is there in that
   disk block to the buffer.
   Also ensure that all the methods accessing and updating the block's data
   should call the loadBlockAndGetBufferPtr() function before the access or
   update is done. This is because the block might not be present in the
   buffer due to LRU buffer replacement. So, it will need to be bought back
   to the buffer before any operations can be done.
 */
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char ** buffPtr) {
  /* check whether the block is already present in the buffer
     using StaticBuffer.getBufferNum() */
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
  if (bufferNum == E_OUTOFBOUND) return E_OUTOFBOUND;

  // if present (!=E_BLOCKNOTINBUFFER),
      // set the timestamp of the corresponding buffer to 0 and increment the
      // timestamps of all other occupied buffers in BufferMetaInfo.
  if(bufferNum != E_BLOCKNOTINBUFFER){
    StaticBuffer::metainfo[bufferNum].timeStamp = 0;
    for(int i=0;i<BUFFER_CAPACITY;i++){
      if(i!=bufferNum && StaticBuffer::metainfo[i].free==false){
        StaticBuffer::metainfo[i].timeStamp++;
      }
    }
  }

  // else
      // get a free buffer using StaticBuffer.getFreeBuffer()

      // if the call returns E_OUTOFBOUND, return E_OUTOFBOUND here as
      // the blockNum is invalid

      // Read the block into the free buffer using readBlock()
  else{
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
    if(bufferNum==E_OUTOFBOUND) return E_OUTOFBOUND;
    Disk::readBlock(StaticBuffer::blocks[bufferNum],this->blockNum);
  }

  // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
  // return SUCCESS;
  *buffPtr = StaticBuffer::blocks[bufferNum];
  Disk::writeBlock(*buffPtr, this->blockNum);
  return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block
     using loadBlockAndGetBufferPtr(&bufferPtr). */
  int startAdd = loadBlockAndGetBufferPtr(&bufferPtr);

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
      // return the value returned by the call.
  if(startAdd != SUCCESS) return startAdd;

  /* get the header of the block using the getHeader() function */
  HeadInfo head;
  getHeader(&head);

  // get number of attributes in the block.
  int noOfAttrs = head.numAttrs;

  // get the number of slots in the block.
  int noOfSlots = head.numSlots;

  // if input slotNum is not in the permitted range return E_OUTOFBOUND.
  if(slotNum >= noOfSlots) return E_OUTOFBOUND;

  /* offset bufferPtr to point to the beginning of the record at required
     slot. the block contains the header, the slotmap, followed by all
     the records. so, for example,
     record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
     copy the record from `rec` to buffer using memcpy
     (hint: a record will be of size ATTR_SIZE * numAttrs)
  */
  unsigned char* slotter = bufferPtr + HEADER_SIZE + noOfSlots + (slotNum * ATTR_SIZE * noOfAttrs);
  memcpy(slotter, rec, (ATTR_SIZE * noOfAttrs));

  // update dirty bit using setDirtyBit()
  int flag = StaticBuffer::setDirtyBit(this->blockNum);

  /* (the above function call should not fail since the block is already
     in buffer and the blockNum is valid. If the call does fail, there
     exists some other issue in the code) */
  if(flag != SUCCESS){
    printf("error detected in code\n");
    return flag;
  }

  // return SUCCESS
  return SUCCESS;
}
