#include "BITMAPDecoder.h"

BITMAPDecoder::BITMAPDecoder(){
  BITMAPDecoder:initialize();
}
BITMAPDecoder::~BITMAPDecoder(){
}
void BITMAPDecoder::initialize(){
  bitmapData = false;
}
bool BITMAPDecoder::checkFile( File fp ){
  struct BITMAPHEADER {
    // BITMAPFILEHEADER
    short  bfType;
    long   bfSize;
    short  bfReserved1;
    short  bfReserved2;
    long   bfOffBits;
    // BITMAPINFOHEADER
    long   biSize;
    long   biWidth;
    long   biHeight;
    short  biPlanes;
    short  biBitCount;
    long   biCompression;
    long   biSizeImage;
    long   biXPelsPerMeter;
    long   biYPelsPerMeter;
    long   biClrUsed;
    long   biClrImportant;
  }header;
  BITMAPDecoder:initialize();
  int fileLength = fp.size();
  if( fileLength<sizeof(header) ){
    Serial.print("file length is short!=");
    Serial.println( fileLength );
    return false;
  }
  fp.seek( 0 );
  // CPU毎に命令が対応していない可能性があるのでバラバラに読み込んでいる
  // sizeof(long)==4 / sizeof(short)==2 で返るのが前提です。
  fp.readBytes( (char*)&header.bfType,          sizeof(header.bfType) );
  fp.readBytes( (char*)&header.bfSize,          sizeof(header.bfSize) );
  fp.readBytes( (char*)&header.bfReserved1,     sizeof(header.bfReserved1) );
  fp.readBytes( (char*)&header.bfReserved2,     sizeof(header.bfReserved2) );
  fp.readBytes( (char*)&header.bfOffBits,       sizeof(header.bfOffBits) );
  fp.readBytes( (char*)&header.biSize,          sizeof(header.biSize) );
  fp.readBytes( (char*)&header.biWidth,         sizeof(header.biWidth) );
  fp.readBytes( (char*)&header.biHeight,        sizeof(header.biHeight) );
  fp.readBytes( (char*)&header.biPlanes,        sizeof(header.biPlanes) );
  fp.readBytes( (char*)&header.biBitCount,      sizeof(header.biBitCount) );
  fp.readBytes( (char*)&header.biCompression,   sizeof(header.biCompression) );
  fp.readBytes( (char*)&header.biSizeImage,     sizeof(header.biSizeImage) );
  fp.readBytes( (char*)&header.biXPelsPerMeter, sizeof(header.biXPelsPerMeter) );
  fp.readBytes( (char*)&header.biYPelsPerMeter, sizeof(header.biYPelsPerMeter) );
  fp.readBytes( (char*)&header.biClrUsed,       sizeof(header.biClrUsed) );
  fp.readBytes( (char*)&header.biClrImportant,  sizeof(header.biClrImportant) );
  bfOffBits = header.bfOffBits;
  biHeight = header.biHeight;
  biWidth = header.biWidth;
  biBitCount = header.biBitCount;
  if( header.bfType!=0x4D42 ){
    Serial.println("bfType Error");
    return false;
  }
  int dataSize;
  switch( header.biBitCount ){
    case 1:      dataSize = (header.biWidth + 7) / 8;       break;
    case 4:      dataSize = (header.biWidth + 1) / 2;       break;
    case 8:      dataSize = header.biWidth;                 break;
    case 24:     dataSize = header.biWidth * 3;             break;
  }
  oneLineBytes = ((dataSize + ((dataSize%4)==0?0:4))/4)*4;
  bitmapData = true;
#ifdef DEBUG
  Serial.print("Header Size    =");     Serial.println( sizeof(header) );
  Serial.print("bfSize         =");     Serial.println( header.bfSize );
  Serial.print("bfOffBits      =");     Serial.println( header.bfOffBits );
  Serial.print("biSize         =");     Serial.println( header.biSize );
  Serial.print("biWidth        =");     Serial.println( header.biWidth );
  Serial.print("biHeight       =");     Serial.println( header.biHeight );
  Serial.print("biPlanes       =");     Serial.println( header.biPlanes );   // ここは１固定らしい
  Serial.print("biBitCount     =");     Serial.print( header.biBitCount );
  switch( header.biBitCount ){
    case 1:      Serial.println(":monotone");      if( header.biClrUsed==0 ) header.biClrUsed=  2; break;
    case 4:      Serial.println(":16color");       if( header.biClrUsed==0 ) header.biClrUsed= 16; break;
    case 8:      Serial.println(":256color");      if( header.biClrUsed==0 ) header.biClrUsed=256; break;
    case 24:     Serial.println(":24bit color");   break;
    default:     Serial.println(":no support!!");  break;
  }
  Serial.print("biCompression  =");
  switch(header.biCompression){
    case 0:/*BI_RGB:*/ Serial.println("BI_RGB");       break;
    default:           Serial.println("no support!!"); return false;
  }
  Serial.print("biSizeImage    =");     Serial.println( header.biSizeImage );
  Serial.print("biXPelsPerMeter=");     Serial.println( header.biXPelsPerMeter );
  Serial.print("biYPelsPerMeter=");     Serial.println( header.biYPelsPerMeter );
  Serial.print("biClrUsed      =");     Serial.println( header.biClrUsed );
  Serial.print("biClrImportant =");     Serial.println( header.biClrImportant );
  for( int i=0 ; i<header.biClrUsed ; i++ ){
    PIXEL p = readPalette( fp, i );
    Serial.print("color[");  Serial.print(i);
    Serial.print("]=B:");    Serial.print( p.b );
    Serial.print("  G:");    Serial.print( p.g );
    Serial.print("  R:");    Serial.print( p.r );
    Serial.print("  ?:");    Serial.println( p.a );
  }
  Serial.print("OneLineBytes   =");     Serial.println( oneLineBytes );
#endif
  return true;
}

PIXEL BITMAPDecoder::readPalette( File fp, int table ){
  fp.seek( 54 + 4 * table );    // 54:palette position
PIXEL p;
  p.b = fp.read();  p.g = fp.read();  p.r = fp.read();  p.a = fp.read();
  return p;
}
// カラーパレットに従ってデコード
PIXEL BITMAPDecoder::readPixel( File fp, int X, int Y )
{
  int table;
  switch( biBitCount ){
  case  1: table = readPixelMonotone(fp, X, Y ); break;
  case  4: table = readPixel4bit(fp, X, Y );     break;
  case  8: table = readPixel8bit(fp, X, Y );     break;
  case 24: return readPixel24bit(fp, X, Y );
  }
  return readPalette( fp, table );
}
// monotone
int BITMAPDecoder::readPixelMonotone( File fp, int X, int Y ){
int maskBit = (0x80>>(X%8));  
  fp.seek( bfOffBits + (biHeight - (Y + 1))*oneLineBytes + X / 8 );
  return (((fp.read()&maskBit)==0 )?0:1);
}
// 4bit color
int BITMAPDecoder::readPixel4bit( File fp, int X, int Y ){
  fp.seek( bfOffBits + (biHeight - (Y + 1))*oneLineBytes + X / 2 );
  int data = fp.read();
  if( ((X % 2)==0) ) return data/0x10;
  else               return data&0x0f;
}
// 8bit color
int BITMAPDecoder::readPixel8bit( File fp, int X, int Y ){
  fp.seek( bfOffBits + (biHeight - (Y + 1))*oneLineBytes + X );
  return fp.read();
}
// 24bit color
PIXEL BITMAPDecoder::readPixel24bit( File fp, int X, int Y ){
  fp.seek( bfOffBits + (biHeight - (Y + 1))*oneLineBytes + X * 3 );
  PIXEL p;  p.a = 0;  p.b = fp.read();  p.g = fp.read();  p.r = fp.read();
  return p;
}