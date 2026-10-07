/* Native encoded-reference dispatch and full lookups on a loaded mission. */
#define GENESIS_NO_MAIN
#include "../build/dune.c"
static CPU c,baseline;
static int call(unsigned entry,unsigned ref){
 c=baseline;c.pc=entry;c.sr=0x2700;c.a[7]=0xfff000;
 write_mem(&c,c.a[7],4,0xdead00);write_mem(&c,c.a[7]+4,2,ref);
 unsigned n=0;
 while(!c.fault && c.pc!=0xdead00 && n++<10000)translated_step(&c);
 if(c.fault || c.pc!=0xdead00){fprintf(stderr,"FAIL entry=%06x ref=%04x pc=%06x fault=%s\n",entry,ref,c.pc,c.reason?c.reason:"none");return 0;}
 return 1;
}
int main(void){
 c.rom=rom_data;c.rom_size=sizeof rom_data;c.sr=0x2700;c.audio_mode=AUDIO_MUTE;
 c.io_tx[0]=c.io_tx[1]=0xff;c.io_tx[2]=0xfb;
 c.a[7]=read_mem(&c,0,4);c.ssp=c.a[7];c.pc=read_mem(&c,4,4)&0xffffff;
 while(!c.fault && c.vdp.frames<4944){
  unsigned f=c.vdp.frames;
  c.pad_buttons[0]=f>=3350?0:f>600 && f%180<8?PAD_START:f>900 && f%180>=90 && f%180<98?PAD_C:0;
  machine_step(&c);
 }
 if(c.fault)return 1;
 unsigned unit=0;
 for(unsigned u=0x1000;u<0x47cc;u+=140)
  if((read_mem(&c,0xff0000+u+4,2)&3)==3 && c.ram[u+2]<17){unit=0xff0000+u;break;}
 if(!unit)return 2;
 unsigned building=0xff4eb8;
 unsigned refs[]={0,0x4000|read_mem(&c,unit,2),0x8000|read_mem(&c,building,2),0xc082};
 baseline=c;
 for(unsigned k=0;k<4;k++){
  if(!call(0x2e35a,refs[k]))return 3;
  unsigned expected=k==1?unit:k==2?building:0;
  if((c.a[0]&0xffffff)!=expected){fprintf(stderr,"FAIL lookup ref=%04x got=%06x expected=%06x\n",refs[k],c.a[0]&0xffffff,expected);return 4;}
  if(!call(0x2e302,refs[k]) || ((c.d[0]&65535)!=0)!=(k!=0))return 5;
  if(!call(0x2e25e,refs[k]))return 6;
  if(k==0 && c.d[0]!=0)return 7;
  if(k==1 && c.d[0]!=read_mem(&baseline,unit+10,4))return 8;
  if(k==3 && c.d[0]!=0x00800100)return 9;
  printf("PASS native reference kind=%u ref=%04x\n",k,refs[k]);
 }
 return 0;
}
