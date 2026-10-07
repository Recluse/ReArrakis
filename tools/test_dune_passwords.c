/* Real password keyboard and native handlers on an unmodified first mission.
   The fixture supplies controller input, never password buffers or cheat flags. */
#define GENESIS_NO_MAIN
#include "../build/dune.c"
static CPU c,baseline,original;
static unsigned version_dialog;
static int fail_case(const char *code,const char *why){
 fprintf(stderr,"FAIL %s: %s pc=%06x frame=%llu fault=%s n=%08x xy=%u,%u animation=%u,%u key=%02x\n",code,why,c.pc,(unsigned long long)c.vdp.frames,c.reason?c.reason:"none",c.d[6],dune_word(&c,0xdbf0),dune_word(&c,0xdbf2),dune_word(&c,0xdbf8),dune_word(&c,0xdbfa),c.ram[0xbf2b]);return 0;
}
static int enter(const char *code){
 c=baseline;c.pc=0x215c0;c.sr=0x2000;c.a[7]=c.ssp=0xfff000;
 write_mem(&c,c.a[7],4,0xdead00);
 unsigned length=strlen(code),release=0,confirm=0,handled=0;version_dialog=0;
 uint64_t end=c.vdp.frames+1800;
 while(!c.fault && c.pc!=0xdead00 && c.vdp.frames<end){
  if(c.pc==0x216cc)handled=1;
  if(c.pc==0x21a16)++version_dialog;
  if(c.pc==0x2165a){
   c.pad_buttons[0]=0;
   if(release){release=0;}
   else if(handled){c.pad_buttons[0]=PAD_START;release=1;}
   else if(!dune_word(&c,0xdbf8) && !dune_word(&c,0xdbfa)){
    unsigned n=c.d[6]&65535;char ch=n<length?code[n]:'!';int cell=-1;
    for(unsigned i=0;i<30;++i)if(c.rom[0x21fca+i]==(unsigned char)ch){cell=i;break;}
    if(cell<0)return fail_case(code,"keyboard character not found");
    int x=84+16*(cell%10),y=52+16*(cell/10);
    int cx=dune_word(&c,0xdbf0),cy=dune_word(&c,0xdbf2);
    if(cx<x)c.pad_buttons[0]=PAD_RIGHT;else if(cx>x)c.pad_buttons[0]=PAD_LEFT;
    else if(cy<y)c.pad_buttons[0]=PAD_DOWN;else if(cy>y)c.pad_buttons[0]=PAD_UP;
    else {c.pad_buttons[0]=PAD_A;if(n>=length)confirm=1;}
    if(c.pad_buttons[0])release=1;
   }
  }
  /* Version dialog is a native YES/NO widget; accept YES using A. */
  if(c.pc==0x21ae0){c.pad_buttons[0]=c.vdp.frames%16<8?PAD_A:0;}
  machine_step(&c);
 }
 c.pad_buttons[0]=0;
 if(c.fault || c.pc!=0xdead00 || !confirm || !handled)return fail_case(code,"password screen did not return");
 return 1;
}
int main(void){
 c.rom=rom_data;c.rom_size=sizeof rom_data;c.sr=0x2700;c.audio_mode=AUDIO_MUTE;
 c.io_tx[0]=c.io_tx[1]=255;c.io_tx[2]=251;c.a[7]=read_mem(&c,0,4);c.ssp=c.a[7];c.pc=read_mem(&c,4,4)&0xffffff;
 while(!c.fault && c.vdp.frames<4944){unsigned f=c.vdp.frames;c.pad_buttons[0]=f>=3350?0:f>600 && f%180<8?PAD_START:f>900 && f%180>=90 && f%180<98?PAD_C:0;machine_step(&c);}
 if(c.fault || dune_mouse_context(&c)!=1)return !fail_case("boot","first mission unavailable");
 c.pad_buttons[0]=0;baseline=c;
 const char *codes[]={"DEMOLITION","DIPLOMATIC","DOMINATION","SPICESATYR","SPICEDANCE","SPICESABRE","BURNINGSUN","ETERNALSUN","ARRAKISSUN","DARKHUNTER","DEFTHUNTER","COLDHUNTER","EVILMENTAT","FAIRMENTAT","WILYMENTAT","ITSJOEBWAN","ASHLIKENNY","SLYMELANIE","DEVASTATOR","SONICBLAST","STEALTHWAR","DEATHRULER","DUNERUNNER","POWERCRUSH","DUNEFINALE","LOOKAROUND","SPLURGEOLA","PLAYTESTER","VERSIONNUM"};
 for(unsigned i=0;i<29;++i){
  if(!enter(codes[i]))return 1;
  int ok=1;
  if(i<24)ok=(c.d[0]&65535)==1 && dune_word(&c,0xc274)==i%3 && dune_word(&c,0xc04c)==2+i/3 && dune_word(&c,0xc050)==1+i/3;
  else if(i==24)ok=(c.d[0]&65535)==1 && dune_word(&c,0xc04c)==10 && dune_word(&c,0xc050)==9;
  else if(i==25)ok=c.ram[0xc198]==(baseline.ram[0xc198]^1);
  else if(i==26){unsigned player=dune_long(&c,0xc278)&65535;ok=dune_long(&c,0xc054)==25000 && dune_long(&c,player+18)==25000;}
  else if(i==27)ok=c.ram[0xc019]==(baseline.ram[0xc019]^1);
  else if(i==28)ok=version_dialog>0;
  if(!ok)return !fail_case(codes[i],"native effect mismatch");
  printf("PASS keyboard %s result=%u frames=%llu\n",codes[i],c.d[0]&65535,(unsigned long long)(c.vdp.frames-baseline.vdp.frames));fflush(stdout);
  if(i==25 || i==27){
   unsigned flag=i==25?0xc198:0xc019;original=baseline;baseline=c;
   if(!enter(codes[i]) || c.ram[flag]!=original.ram[flag])return !fail_case(codes[i],"repeat must toggle back");
   baseline=original;printf("PASS repeat toggles %s back off\n",codes[i]);
  }
 }
 const char *invalid[]={"","INVALID","PLAYTESTE"};
 for(unsigned i=0;i<3;++i){
  if(!enter(invalid[i]) || (c.d[0]&65535)!=0 ||
     dune_word(&c,0xc04c)!=dune_word(&baseline,0xc04c) ||
     dune_word(&c,0xc274)!=dune_word(&baseline,0xc274) ||
     c.ram[0xc019]!=baseline.ram[0xc019] || c.ram[0xc198]!=baseline.ram[0xc198] ||
     dune_long(&c,0xc054)!=dune_long(&baseline,0xc054))return !fail_case(invalid[i],"invalid code must not apply an effect");
  printf("PASS rejects '%s'\n",invalid[i]);
 }
 if(c.dune_wait_cycles<=baseline.dune_wait_cycles || c.dune_wait_cycles>c.cycles)return !fail_case("telemetry","native wait cycles not accounted");
 puts("PASS all 29 Dune USA passwords, toggle-back and invalid inputs through the native keyboard");return 0;
}
