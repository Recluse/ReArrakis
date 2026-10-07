/* Execute the original signed projectile dispatch, including its bounds. */
#define GENESIS_NO_MAIN
#include "../build/dune.c"
static CPU c,baseline;
int main(void){
 const struct {uint16_t type;uint32_t target;} cases[]={
  {17,0x48534},{18,0x483c6},{19,0x483c6},{20,0x483c6},
  {21,0x483c6},{22,0x483c6},{23,0x48498},{24,0x48498},
  {25,0x48534},{0xffff,0x48534}
 };
 for(unsigned n=0;n<sizeof cases/sizeof cases[0];n++){
  memset(&c,0,sizeof c);c.rom=rom_data;c.rom_size=sizeof rom_data;
  c.sr=0x2700;c.pc=0x48396;c.d[4]=cases[n].type;c.a[7]=0xfff000;
  for(unsigned step=0;step<16 && !c.fault && c.pc!=cases[n].target;step++)translated_step(&c);
  if(c.fault || c.pc!=cases[n].target){
   fprintf(stderr,"FAIL type=%04x pc=%06x fault=%s\n",cases[n].type,c.pc,c.reason?c.reason:"none");return 1;
  }
  /* The destination itself must be translated, not just the jump. */
  translated_step(&c);
  if(c.fault){fprintf(stderr,"FAIL missing target=%06x\n",cases[n].target);return 1;}
 }
 puts("PASS native projectile dispatch: 7 choices and signed bounds");
 /* Initialize all game tables by running the original boot and first mission.
    Use a separate snapshot for each native function test. */
 memset(&c,0,sizeof c);c.rom=rom_data;c.rom_size=sizeof rom_data;c.sr=0x2700;
 c.audio_mode=AUDIO_MUTE;c.io_tx[0]=c.io_tx[1]=0xff;c.io_tx[2]=0xfb;
 c.a[7]=read_mem(&c,0,4);c.ssp=c.a[7];c.pc=read_mem(&c,4,4)&0xffffff;
 while(!c.fault && c.vdp.frames<4944){
  unsigned f=c.vdp.frames;
  c.pad_buttons[0]=f>=3350?0:f>600 && f%180<8?PAD_START:f>900 && f%180>=90 && f%180<98?PAD_C:0;
  machine_step(&c);
 }
 if(c.fault)return 2;
 unsigned source=0,target=0,owner=read_mem(&c,0xffc274,2);
 for(unsigned u=0x1000;u<0x47cc;u+=140){
  if((read_mem(&c,0xff0000+u+4,2)&3)!=3 || c.ram[u+2]>=17)continue;
  if(c.ram[u+8]==owner)source=u;else target=u;
 }
 if(!source || !target)return 3;
 baseline=c;
 for(unsigned type=18;type<=24;type++){
  c=baseline;c.pc=0x48352;c.sr=0x2700;c.a[7]=0xfff000;
  /* Original ABI: return PC, source position, projectile type, owner,
     damage, and an encoded reference to a live target unit. */
  write_mem(&c,c.a[7],4,0xdead00);
  write_mem(&c,c.a[7]+4,4,read_mem(&c,0xff0000+source+10,4));
  write_mem(&c,c.a[7]+8,2,type);write_mem(&c,c.a[7]+10,2,owner);
  write_mem(&c,c.a[7]+12,2,10);
  write_mem(&c,c.a[7]+14,2,0x4000|read_mem(&c,0xff0000+target,2));
  unsigned steps=0;
  while(!c.fault && c.pc!=0xdead00 && steps++<100000)translated_step(&c);
  unsigned unit=c.a[0]&0xffffff;
  if(c.fault || c.pc!=0xdead00 || unit<0xff1000 || unit>=0xff47cc || read_mem(&c,unit+2,1)!=type){
   fprintf(stderr,"FAIL projectile type=%u pc=%06x unit=%06x fault=%s\n",type,c.pc,unit,c.reason?c.reason:"none");return 4;
  }
  printf("PASS native projectile creation type=%u unit=%06x steps=%u\n",type,unit,steps);
 }
 return 0;
}
