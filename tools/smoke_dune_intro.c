/* Cold boot with no pad/mouse input: do not skip the intro through Start. */
#define GENESIS_NO_MAIN
#include "../build/dune.c"
static CPU c;
int main(int argc,char **argv){
 unsigned long frames=argc>1?strtoul(argv[1],NULL,10):7200;
 unsigned intro_calls=0;
 c.rom=rom_data;c.rom_size=sizeof rom_data;c.sr=0x2700;
 c.audio_mode=argc>2 && !strcmp(argv[2],"on")?AUDIO_ON:AUDIO_MUTE;
 if(c.audio_mode==AUDIO_ON && !audio_init(&c,NULL))return 3;
 c.io_tx[0]=c.io_tx[1]=0xff;c.io_tx[2]=0xfb;
 c.a[7]=read_mem(&c,0,4);c.ssp=c.a[7];c.pc=read_mem(&c,4,4)&0xffffff;
 while(!c.fault && c.vdp.frames<frames){
  if(c.pc==0x41978)intro_calls++;
  machine_step(&c);
 }
 unsigned lit=0;
 for(unsigned n=0;n<c.vdp.frame_width*c.vdp.frame_height*3;n++)lit+=c.vdp.frame[n]!=0;
 fprintf(stderr,"pc=%06x frames=%llu intro_calls=%u lit=%u fault=%s\n",c.pc,(unsigned long long)c.vdp.frames,intro_calls,lit,c.reason?c.reason:"none");
 FILE *out=fopen("build/intro.ppm","wb");if(!out)return 3;
 fprintf(out,"P6\n%u %u\n255\n",c.vdp.frame_width,c.vdp.frame_height);
 fwrite(c.vdp.frame,3,c.vdp.frame_width*c.vdp.frame_height,out);fclose(out);
 audio_finish(&c);
 return c.fault?1:!intro_calls?7:0;
}
