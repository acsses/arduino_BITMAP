#include <SPIFFS.h>
#include <FS.h>
#include <BITMAPDecoder.h>
BITMAPDecoder bitmap = BITMAPDecoder();

//
void setup() {
  Serial.begin(115200);
  while(!Serial);
  if ( SPIFFS.begin() )
    Serial.println("SPIFFS opened!");
  File fp;
  fp = SPIFFS.open( "/sample.bmp", "r" );
  bitmap.checkFile( fp );
  for( int Y=0 ; Y<bitmap.height() ; Y++ ){
    for( int X=0 ; X<bitmap.width() ; X++ ){
      PIXEL p = bitmap.readPixel( fp, X, Y );
      if( p.g>128 )  Serial.print("■");
      else           Serial.print("　");
    }
    Serial.println("");
  }
  fp.close();
}
void loop() {
}
