#include "StaticBuffer.h"
#include <iostream>

// the declarations for this class can be found at "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer() {

  // initialise all blocks as free
  for (int bufferIndex = 0; bufferIndex<BUFFER_CAPACITY; bufferIndex++) {
    metainfo[bufferIndex].free = true;
    metainfo[bufferIndex].dirty = false;
    metainfo[bufferIndex].timeStamp = -1;
    metainfo[bufferIndex].blockNum = -1;
  }
}

/*
At this stage, we are not writing back from the buffer to the disk since we are
not modifying the buffer. So, we will define an empty destructor for now. In
subsequent stages, we will implement the write-back functionality here.
*/

// write back all modified blocks on system exit
StaticBuffer::~StaticBuffer() {
  /*iterate through all the buffer blocks,
    write back blocks with metainfo as free=false,dirty=true
    using Disk::writeBlock()
    */
  for (int bufferIndex = 0; bufferIndex<BUFFER_CAPACITY; bufferIndex++) {
    if (metainfo[bufferIndex].free == false && metainfo[bufferIndex].dirty == true){
      Disk::writeBlock(blocks[bufferIndex],metainfo[bufferIndex].blockNum);
    }
  }
}

int StaticBuffer::getFreeBuffer(int blockNum) {
  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  for(int i=0;i<BUFFER_CAPACITY;i++){
    if(!metainfo[i].free){
      metainfo[i].timeStamp++;
    }
  }

  int bufferNum=-1;

  // iterate through all the blocks in the StaticBuffer
  // find the first free block in the buffer (check metainfo)
  // assign allocatedBuffer = index of the free block
  for(int i=0;i<BUFFER_CAPACITY;i++){
    if(metainfo[i].free){
      bufferNum=i;
      break;
    }
  }

  // if a free buffer is not available,
  //     find the buffer with the largest timestamp
  //     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
  //     set bufferNum = index of this buffer

  // update the metaInfo entry corresponding to bufferNum with
  // free:false, dirty:false, blockNum:the input block number, timeStamp:0.

  // return the bufferNum.
  if(bufferNum==-1){
    int max=0;
    for(int i=0;i<BUFFER_CAPACITY;i++){
      if(metainfo[i].timeStamp>max){
        max=metainfo[i].timeStamp;
        if(metainfo[i].dirty){
          Disk::writeBlock(blocks[i],metainfo[i].blockNum);
        } 
        bufferNum=i;
      }
    }
  }


  metainfo[bufferNum].free = false;
  metainfo[bufferNum].blockNum = blockNum;
  metainfo[bufferNum].timeStamp = 0;

  return bufferNum;
}

/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::getBufferNum(int blockNum) {
  // Check if blockNum is valid (between zero and DISK_BLOCKS)
  // and return E_OUTOFBOUND if not valid.
  if (blockNum < 0 || blockNum >= DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  // find and return the bufferIndex which corresponds to blockNum (check metainfo)

  for(int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++){
    if(metainfo[bufferIndex].blockNum==blockNum){
      return bufferIndex;
    }
  }

  // if block is not in the buffer
  return E_BLOCKNOTINBUFFER;
}

int StaticBuffer::setDirtyBit(int blockNum){
  // find the buffer index corresponding to the block using getBufferNum().
  int bufferNum = getBufferNum(blockNum);
  // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
  //     return E_BLOCKNOTINBUFFER
  if(bufferNum == E_BLOCKNOTINBUFFER){
    return E_BLOCKNOTINBUFFER;
  }

  // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
  //     return E_OUTOFBOUND
  if(bufferNum == E_OUTOFBOUND){
    return E_OUTOFBOUND;
  }

  // else
  //     (the bufferNum is valid)
  //     set the dirty bit of that buffer to true in metainfo
  metainfo[bufferNum].dirty = true;

  // return SUCCESS
  return SUCCESS;
}
