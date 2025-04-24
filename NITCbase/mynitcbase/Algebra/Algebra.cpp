#include "Algebra.h"
#include <iostream>
#include <cstring>


// will return if a string can be parsed as a floating point number
bool isNumber(char *str) {
  int len;
  float ignore;
  /*
    sscanf returns the number of elements read, so if there is no float matching
    the first %f, ret will be 0, else it'll be 1

    %n gets the number of characters read. this scanf sequence will read the
    first float ignoring all the whitespace before and after. and the number of
    characters read that far will be stored in len. if len == strlen(str), then
    the string only contains a float with/without whitespace. else, there's other
    characters.
  */
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}

/* used to select all the records that satisfy a condition.
the arguments of the function are
- srcRel - the source relation we want to select from
- targetRel - the relation we want to select into. (ignore for now)
- attr - the attribute that the condition is checking
- op - the operator of the condition
- strVal - the value that we want to compare against (represented as a string)
*/
int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]) {
  //get relid of source relation in cache
  int srcRelId = OpenRelTable::getRelId(srcRel);
  if (srcRelId == E_RELNOTOPEN) {  
    return E_RELNOTOPEN;
  }

  // get the attribute catalog entry for attr, using AttrCacheTable::getAttrcatEntry()
  //    return E_ATTRNOTEXIST if it returns the error
  AttrCatEntry attrCatEntry;
  int ret = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);
  if (ret == E_ATTRNOTEXIST) return E_ATTRNOTEXIST;
  
  /*** Convert strVal (string) to an attribute of data type NUMBER or STRING ***/
  //this is what we'll be comparing the attribute of records with
  int type = attrCatEntry.attrType;
  Attribute attrVal;
  if (type == NUMBER) {
    if (isNumber(strVal)) {
      attrVal.nVal = atof(strVal);
    }
    else {
      return E_ATTRTYPEMISMATCH;
    }
  }
  else if (type == STRING) {
    strcpy(attrVal.sVal, strVal);
  }

  /*** Selecting records from the source relation ***/

  // Before calling the search function, reset the search to start from the first hit
  // using RelCacheTable::resetSearchIndex()
  RelCacheTable::resetSearchIndex(srcRelId);

  /************************
  The following code prints the contents of a relation directly to the output
  console. Direct console output is not permitted by the actual the NITCbase
  specification and the output can only be inserted into a new relation. We will
  be modifying it in the later stages to match the specification.
  ************************/

  RelCatEntry relCatEntry;
  RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);
  //prints the relation name and attr names using getattrcatentry(using attr offset)
  printf("|");
  for (int i = 0; i < relCatEntry.numAttrs; ++i) {
    AttrCatEntry attrCatEntry;
    // get attrCatEntry at offset i using AttrCacheTable::getAttrCatEntry()
    AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);

    printf(" %s |", attrCatEntry.attrName);
  }
  printf("\n");

  //looks for and prints all other records satisfying the select condition in the relation using BlockAccess::linearSearch()
  while (true) {
    static RecId searchRes = {-1, -1};
    searchRes = BlockAccess::linearSearch(srcRelId, attr, attrVal, op);
    if (searchRes.block == -1 && searchRes.slot == -1) {
        break;  // No more matching records, exit loop
    }

    printf("|");

    // Retrieve the full record
    RecBuffer recBuf(searchRes.block);

    HeadInfo head;
    recBuf.getHeader(&head);
    Attribute record[head.numAttrs];
    recBuf.getRecord(record, searchRes.slot);  // Fetch full record

    // Print all attributes
    for (int i = 0; i < head.numAttrs; ++i) {
        AttrCatEntry attrCatEntry;
        int res = AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);
        
        if (res == SUCCESS) {
            if (attrCatEntry.attrType == NUMBER) {
                printf("%d |", (int)record[i].nVal);
            } else {
                printf(" %s |", record[i].sVal);
            }
        }
    }
    printf("\n");
}


  return SUCCESS;
}
