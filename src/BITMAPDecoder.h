#ifndef BITMAPDECODER_H
#define BITMAPDECODER_H

#include <FS.h>

typedef struct PIXEL{
  unsigned char a;
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

class BITMAPDecoder{
private:
  bool bitmapData;
  int oneLineBytes;
  int bfOffBits, biWidth, biHeight, biBitCount;
public:
  BITMAPDecoder();
  void initialize( void );
  bool checkFile( File fp );
  PIXEL readPalette( File fp, int table );
  PIXEL readPixel( File fp , int x, int y);
  int readPixelMonotone( File fp, int X, int Y );
  int readPixel4bit( File fp, int X, int Y );
  int readPixel8bit( File fp, int X, int Y );
  PIXEL readPixel24bit( File fp, int X, int Y );
  int width(){    return biWidth;  };
  int height(){   return biHeight; };
  int bitCount(){ return biBitCount; };
  ~BITMAPDecoder();
};
#endif