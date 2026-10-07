#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#define GENESIS_DUNE_MOUSE
#include "../genesis_recompiler/dune_audio.h"
int main(void){
 assert(dune_audio_volume(NULL)==100 && dune_audio_volume("")==100);
 assert(dune_audio_volume("50")==50 && dune_audio_volume("200")==200);
 assert(dune_audio_volume("-1")==100 && dune_audio_volume("oops")==100);
 assert(dune_audio_scale(1000,100)==4000 && dune_audio_scale(-1000,100)==-4000);
 assert(dune_audio_scale(1000,50)==2000 && dune_audio_scale(32767,0)==0);
 for(int volume=0;volume<=200;volume+=10){
  int previous=-32768;
  for(int sample=-32768;sample<=32767;++sample){
   int value=dune_audio_scale((int16_t)sample,volume);
   assert(value>=previous && value>=-32767 && value<=32767);previous=value;
  }
 }
 return 0;
}
