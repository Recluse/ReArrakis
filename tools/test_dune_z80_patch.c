/* Run the real translated driver writer, then the previously missing NOP. */
#define GENESIS_NO_MAIN
#include "../build/dune.c"
#include <assert.h>
static CPU c;
int main(void){
 c.rom=rom_data;c.rom_size=sizeof rom_data;c.audio_mode=AUDIO_MUTE;
 memcpy(c.z80_bus.ram,rom_data+0x2952,0x41dc-0x2952);
 for(unsigned command=9;command<=10;++command){
  c.z80_bus.ram[0x1423]=(uint8_t)command;c.z80_cpu.pc=0x13fe;
  unsigned steps=0;
  while(!c.fault && c.z80_cpu.pc!=0x140e && ++steps<16)translated_z80_step(&c);
  assert(!c.fault && c.z80_cpu.pc==0x140e);
  assert(c.z80_bus.ram[0x2cf]==(command==10?0:0xc9));
  c.z80_cpu.pc=0x2cf;c.z80_cpu.sp=0x1ffc;
  c.z80_bus.ram[0x1ffc]=0xbd;c.z80_bus.ram[0x1ffd]=2;
  translated_z80_step(&c);
  assert(!c.fault && c.z80_cpu.pc==(command==10?0x2d0:0x2bd));
 }
 puts("Z80 live RET/NOP patch and both guarded variants passed");return 0;
}
