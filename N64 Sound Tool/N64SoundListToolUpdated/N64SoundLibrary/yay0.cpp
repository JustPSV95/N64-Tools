#include "stdafx.h"
//version 1.0 (20050701)
//by thakis
#include "YAY0.h"
#include <cstdio>
#include <cstdlib>

using namespace std;

typedef unsigned char u8;
typedef unsigned char u16;
typedef unsigned int u32;

u32 toDWORD(u32 d)
{
  u8 w1 = d & 0xFF;
  u8 w2 = (d >> 8) & 0xFF;
  u8 w3 = (d >> 16) & 0xFF;
  u8 w4 = d >> 24;
  return (w1 << 24) | (w2 << 16) | (w3 << 8) | w4;
}

struct Ret
{
  int srcPos, dstPos;
};

Ret decodeYay0(u8* codes, u8* counts, u8* srcData, u8* dst, int uncompressedSize)
{
  Ret r = { 0, 0 };
  //int srcPlace = 0, dstPlace = 0; //current read/write positions
  
  u32 codePos = 0;
  u32 countPos = 0;

  u32 validBitCount = 0; //number of valid bits left in "code" byte
  u8 currCodeByte;
  while(r.dstPos < uncompressedSize)
  {
    //read new "code" byte if the current one is used up
    if(validBitCount == 0)
    {
      currCodeByte = codes[codePos];
      ++codePos;
      validBitCount = 8;
    }

    if((currCodeByte & 0x80) != 0)
    {
      //straight copy
      dst[r.dstPos] = srcData[r.srcPos];
      r.dstPos++;
      r.srcPos++;
      //if(r.srcPos >= srcSize)
      //  return r;
    }
    else
    {
      //RLE part
      u8 byte1 = counts[countPos];
      u8 byte2 = counts[countPos + 1];
      countPos += 2;
      //if(r.srcPos >= srcSize)
      //  return r;
      
      u32 dist = ((byte1 & 0xF) << 8) | byte2;
      u32 copySource = r.dstPos - (dist + 1);

      u32 numBytes = byte1 >> 4;
      if(numBytes == 0)
      {
        numBytes = srcData[r.srcPos] + 0x12;
        r.srcPos++;
        //if(r.srcPos >= srcSize)
        //  return r;
      }
      else
        numBytes += 2;

      //copy run
      for(int i = 0; i < numBytes; ++i)
      {
        dst[r.dstPos] = dst[copySource];
        copySource++;
        r.dstPos++;
      }
    }
    
    //use next bit from "code" byte
    currCodeByte <<= 1;
    validBitCount-=1;    
  }

  return r;
}


int YAY0::decodeAll(unsigned char * src, unsigned char* result, int& fileSizeCompressed)
{
  if(strncmp((char*)src, "Yay0", 4) != 0)
    return 0;

  u32 decodedSize = toDWORD(*(u32*)(src + 4));
  u32 countOffset = toDWORD(*(u32*)(src + 8));
  u32 dataOffset = toDWORD(*(u32*)(src + 12));

  Ret r = decodeYay0(src + 16, src + countOffset, 
    src + dataOffset, result, decodedSize);

  return decodedSize;
}
