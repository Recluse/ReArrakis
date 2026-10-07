#define GENESIS_NO_MAIN
#include "../build/dune.c"
static CPU c;
#if defined(DUNE_MOUSE_SMOKE) || defined(DUNE_BASE_SMOKE)
static DuneMouse mouse={.enabled=1};
static uint32_t selected_unit,unit_position;
#endif
int main(int argc,char **argv){
 unsigned long frames=argc>1?strtoul(argv[1],NULL,10):3600;
 c.rom=rom_data;c.rom_size=sizeof rom_data;c.sr=0x2700;c.audio_mode=argc>2?(!strcmp(argv[2],"on")?AUDIO_ON:AUDIO_MUTE):AUDIO_STUB;
 if(c.audio_mode==AUDIO_ON && !audio_init(&c,"build/smoke-audio.wav"))return 3;
 c.io_tx[0]=c.io_tx[1]=0xff;c.io_tx[2]=0xfb;
 c.a[7]=read_mem(&c,0,4);c.ssp=c.a[7];c.pc=read_mem(&c,4,4)&0xffffff;
 while(!c.fault && c.vdp.frames<frames){
  unsigned f=c.vdp.frames;
  c.pad_buttons[0]= f>=3350?0: f>600 && f%180<8 ? PAD_START : f>900 && f%180>=90 && f%180<98 ? PAD_C : 0;
#ifdef DUNE_MOUSE_SMOKE
  if(f==4944 && !mouse.clicks && !mouse.count)
   dune_mouse_click(&mouse,&c,72,130,DUNE_CLICK_SELECT);
  if(f==4964 && mouse.clicks==1 && !mouse.count){
   selected_unit=dune_long(&c,0xc25c);
   if(selected_unit<0xff0000 || selected_unit>0xffffb0)return 4;
   unit_position=dune_long(&c,(selected_unit&65535)+10);
   dune_mouse_click(&mouse,&c,160,176,DUNE_CLICK_ORDER);
  }
  dune_mouse_observe(&mouse,&c);
#elif defined(DUNE_BASE_SMOKE)
  if((f==4944 && !mouse.clicks && !mouse.count) ||
     (f==4964 && mouse.clicks==1 && !mouse.count))
   dune_mouse_click(&mouse,&c,135,108,DUNE_CLICK_SELECT);
  dune_mouse_observe(&mouse,&c);
#endif
  machine_step(&c);
 }
 fprintf(stderr,"pc=%06x frames=%llu steps=%llu fault=%s\n",c.pc,(unsigned long long)c.vdp.frames,(unsigned long long)c.steps,c.reason?c.reason:"none");
 FILE *out=fopen("build/gameplay.ppm","wb");fprintf(out,"P6\n%u %u\n255\n",c.vdp.frame_width,c.vdp.frame_height);fwrite(c.vdp.frame,3,c.vdp.frame_width*c.vdp.frame_height,out);fclose(out);
#ifdef DUNE_MOUSE_SMOKE
 fprintf(stderr,"mouse clicks=%llu unit=%06x position=%08x -> %08x\n",
  (unsigned long long)mouse.clicks,selected_unit,unit_position,
  selected_unit?dune_long(&c,(selected_unit&65535)+10):0);
 if(mouse.clicks!=2 || !selected_unit ||
    unit_position==dune_long(&c,(selected_unit&65535)+10))return 5;
#endif
#ifdef DUNE_BASE_SMOKE
 fprintf(stderr,"base clicks=%llu page=%02x table=%06x\n",
  (unsigned long long)mouse.clicks,c.ram[0xc618],dune_long(&c,0xc61c));
 if(mouse.clicks!=2 || c.ram[0xc618]!=0x13)return 6;
#endif
 audio_finish(&c);
 return c.fault?1:0;
}
