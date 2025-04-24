#include <iostream>
#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"

/*
//STAGE 1

int main(int argc, char *argv[]) {
  //Initialize the Run Copy of Disk
  Disk disk_run;
  // StaticBuffer buffer;
  // OpenRelTable cache;


  unsigned char buffer[BLOCK_SIZE];
  Disk::readBlock(buffer, 7000);
  char message[] = "hello";
  memcpy(buffer + 20, message, 6);
  Disk::writeBlock(buffer, 7000);

  unsigned char buffer2[BLOCK_SIZE];
  char message2[6];
  Disk::readBlock(buffer2, 7000);
  memcpy(message2, buffer2 + 20, 6);
  std::cout << message2 << std::endl;;
  
  
  //STAGE 1 Exercise ->
  
  
  unsigned char buffer3[BLOCK_SIZE];
  Disk::readBlock(buffer3, 0);
  char mess[BLOCK_SIZE];
  memcpy(mess, buffer3, BLOCK_SIZE);
  for (int i=0;i<BLOCK_SIZE;i++) {
        std :: cout << (int)(unsigned char)mess[i] << " ";
  }
  std::cout << std::endl;
  
  
  
  //return FrontendInterface::handleFrontend(argc, argv);
  return 0;
}*/


/////////////////////////////////////////////////////////////////////////////
/*
//STAGE 2
int main(int argc, char *argv[]) {
  Disk disk_run;

  // create objects for the relation catalog and attribute catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // (we will implement these functions later)
  relCatBuffer.getHeader(&relCatHeader);
  attrCatBuffer.getHeader(&attrCatHeader);

  for (int i=0; i<relCatHeader.numEntries; i++) {

    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

    relCatBuffer.getRecord(relCatRecord, i);

    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);
    
    int currentBlock = ATTRCAT_BLOCK;

    //EX 1
    while (currentBlock != -1) {
      RecBuffer attrCatBuffer(currentBlock);
      HeadInfo attrCatHeader;
      attrCatBuffer.getHeader(&attrCatHeader);

      for (int j=0; j<attrCatHeader.numEntries; j++) {
        // declare attrCatRecord and load the attribute catalog entry into it
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBuffer.getRecord(attrCatRecord,j);
        
        //if(strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Class")==0){
          //strncpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Batch", sizeof(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal) - 1);
          //attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal[sizeof(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal) - 1] = '\0';
        //}
        if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,relCatRecord[RELCAT_REL_NAME_INDEX].sVal)==0) {
        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
          printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
        }
      }
    currentBlock = attrCatHeader.rblock;
    }
    printf("\n");
  }
  return 0;
}
*/
////////////////////////////////////////////
/*
//STAGE 3
int main(int argc, char *argv[]) {

  Disk disk_run;
  StaticBuffer buffer;

  // create objects for the relation catalog and attribute catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // (we will implement these functions later)
  relCatBuffer.getHeader(&relCatHeader);
  attrCatBuffer.getHeader(&attrCatHeader);

  for (int i=0; i<relCatHeader.numEntries; i++) {

    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

    relCatBuffer.getRecord(relCatRecord, i);

    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);
    
    int currentBlock = ATTRCAT_BLOCK;

    //EX 1
    while (currentBlock != -1) {
      RecBuffer attrCatBuffer(currentBlock);
      HeadInfo attrCatHeader;
      attrCatBuffer.getHeader(&attrCatHeader);

      for (int j=0; j<attrCatHeader.numEntries; j++) {
        // declare attrCatRecord and load the attribute catalog entry into it
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBuffer.getRecord(attrCatRecord,j);
        
        if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,relCatRecord[RELCAT_REL_NAME_INDEX].sVal)==0) {
        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
          printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
        }
      }
    currentBlock = attrCatHeader.rblock;
    }
    printf("\n");
  }
  return 0;
}
*/

/////////////////////////////////////////////
//STAGE 3 PT.2
/*
int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;

  
   for (int i = RELCAT_RELID; i <= ATTRCAT_RELID
   //+1
   ;i++){
        int relId = i;
        RelCatEntry* relCatBuf = new RelCatEntry();
        RelCacheTable::getRelCatEntry(relId, relCatBuf);
        std::cout << "Relation: " << relCatBuf->relName << std::endl;

        for (int j = 0; j < relCatBuf->numAttrs; j++){
            AttrCatEntry* attrCatBuf = new AttrCatEntry(); 
            AttrCacheTable::getAttrCatEntry(relId, j, attrCatBuf);
            const char* attrType = attrCatBuf->attrType == 0 ? "NUM" : "STR";
            std::cout << "  " << attrCatBuf->attrName << ": " << attrType << std::endl;

            delete attrCatBuf;
        }

        delete relCatBuf;
    }
    return 0;
}
*/
///////////////////////////////////////////
//STAGE 4
int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;

  return FrontendInterface::handleFrontend(argc, argv);
}


