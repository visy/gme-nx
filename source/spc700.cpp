#include "spc700.hpp"

namespace Processor {

#include "algorithms.cpp"
#include "instructions.cpp"
#include "disassembler.cpp"

#define call (this->*op)

template<uint8_t (SPC700::*op)(uint8_t)>
void SPC700::op_adjust(uint8_t& r) {
  op_io();
  r = call(r);
}

template<uint8_t (SPC700::*op)(uint8_t)>
void SPC700::op_adjust_addr() {
  dp.l = op_readpc();
  dp.h = op_readpc();
  rd = op_read(dp);
  rd = call(rd);
  op_write(dp, rd);
}

template<uint8_t (SPC700::*op)(uint8_t)>
void SPC700::op_adjust_dp() {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd = call(rd);
  op_writedp(dp, rd);
}

void SPC700::op_adjust_dpw(signed n) {
  dp = op_readpc();
  rd.w = op_readdp(dp) + n;
  op_writedp(dp++, rd.l);
  rd.h += op_readdp(dp);
  op_writedp(dp++, rd.h);
  regs.p.n = rd & 0x8000;
  regs.p.z = rd == 0;
}

template<uint8_t (SPC700::*op)(uint8_t)>
void SPC700::op_adjust_dpx() {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  rd = call(rd);
  op_writedp(dp + regs.x, rd);
}

void SPC700::op_branch(bool condition) {
  rd = op_readpc();
  if(condition == false) return;
  op_io();
  op_io();
  regs.pc += (int8_t)rd;
}

void SPC700::op_branch_bit() {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((bool)(sp & (1 << (opcode >> 5))) == (bool)(opcode & 0x10)) return;
  op_io();
  op_io();
  regs.pc += (int8_t)rd;
}

void SPC700::op_pull(uint8_t& r) {
  op_io();
  op_io();
  r = op_readsp();
}

void SPC700::op_push(uint8_t r) {
  op_io();
  op_io();
  op_writesp(r);
}

template<uint8_t (SPC700::*op)(uint8_t, uint8_t)>
void SPC700::op_read_addr(uint8_t& r) {
  dp.l = op_readpc();
  dp.h = op_readpc();
  rd = op_read(dp);
  r = call(r, rd);
}

template<uint8_t (SPC700::*op)(uint8_t, uint8_t)>
void SPC700::op_read_addri(uint8_t& r) {
  dp.l = op_readpc();
  dp.h = op_readpc();
  op_io();
  rd = op_read(dp + r);
  regs.a = call(regs.a, rd);
}

template<uint8_t (SPC700::*op)(uint8_t, uint8_t)>
void SPC700::op_read_const(uint8_t& r) {
  rd = op_readpc();
  r = call(r, rd);
}

template<uint8_t (SPC700::*op)(uint8_t, uint8_t)>
void SPC700::op_read_dp(uint8_t& r) {
  dp = op_readpc();
  rd = op_readdp(dp);
  r = call(r, rd);
}

template<uint8_t (SPC700::*op)(uint8_t, uint8_t)>
void SPC700::op_read_dpi(uint8_t& r, uint8_t& i) {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + i);
  r = call(r, rd);
}

template<uint16_t (SPC700::*op)(uint16_t, uint16_t)>
void SPC700::op_read_dpw() {
  dp = op_readpc();
  rd.l = op_readdp(dp++);
  if(op != &SPC700::op_cpw) op_io();
  rd.h = op_readdp(dp++);
  regs.ya = call(regs.ya, rd);
}

template<uint8_t (SPC700::*op)(uint8_t, uint8_t)>
void SPC700::op_read_idpx() {
  dp = op_readpc() + regs.x;
  op_io();
  sp.l = op_readdp(dp++);
  sp.h = op_readdp(dp++);
  rd = op_read(sp);
  regs.a = call(regs.a, rd);
}

template<uint8_t (SPC700::*op)(uint8_t, uint8_t)>
void SPC700::op_read_idpy() {
  dp = op_readpc();
  op_io();
  sp.l = op_readdp(dp++);
  sp.h = op_readdp(dp++);
  rd = op_read(sp + regs.y);
  regs.a = call(regs.a, rd);
}

template<uint8_t (SPC700::*op)(uint8_t, uint8_t)>
void SPC700::op_read_ix() {
  op_io();
  rd = op_readdp(regs.x);
  regs.a = call(regs.a, rd);
}

void SPC700::op_set_addr_bit() {
  dp.l = op_readpc();
  dp.h = op_readpc();
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_read(dp);
  switch(opcode >> 5) {
  case 0:  //orc  addr:bit
  case 1:  //orc !addr:bit
    op_io();
    regs.p.c |= (rd & (1 << bit)) ^ (bool)(opcode & 0x20);
    break;
  case 2:  //and  addr:bit
  case 3:  //and !addr:bit
    regs.p.c &= (rd & (1 << bit)) ^ (bool)(opcode & 0x20);
    break;
  case 4:  //eor  addr:bit
    op_io();
    regs.p.c ^= (bool)(rd & (1 << bit));
    break;
  case 5:  //ldc  addr:bit
    regs.p.c  = (rd & (1 << bit));
    break;
  case 6:  //stc  addr:bit
    op_io();
    rd = (rd & ~(1 << bit)) | (regs.p.c << bit);
    op_write(dp, rd);
    break;
  case 7:  //not  addr:bit
    rd ^= 1 << bit;
    op_write(dp, rd);
    break;
  }
}

void SPC700::op_set_bit() {
  dp = op_readpc();
  rd = op_readdp(dp) & ~(1 << (opcode >> 5));
  op_writedp(dp, rd | (!(opcode & 0x10) << (opcode >> 5)));
}

void SPC700::op_set_flag(bool& flag, bool data) {
  op_io();
  if(&flag == &regs.p.i) op_io();
  flag = data;
}

void SPC700::op_test_addr(bool set) {
  dp.l = op_readpc();
  dp.h = op_readpc();
  rd = op_read(dp);
  regs.p.n = (regs.a - rd) & 0x80;
  regs.p.z = (regs.a - rd) == 0;
  op_read(dp);
  op_write(dp, set ? rd | regs.a : rd & ~regs.a);
}

void SPC700::op_transfer(uint8_t& from, uint8_t& to) {
  op_io();
  to = from;
  if(&to == &regs.s) return;
  regs.p.n = (to & 0x80);
  regs.p.z = (to == 0);
}

void SPC700::op_write_addr(uint8_t& r) {
  dp.l = op_readpc();
  dp.h = op_readpc();
  op_read(dp);
  op_write(dp, r);
}

void SPC700::op_write_addri(uint8_t& i) {
  dp.l = op_readpc();
  dp.h = op_readpc();
  op_io();
  dp += i;
  op_read(dp);
  op_write(dp, regs.a);
}

void SPC700::op_write_dp(uint8_t& r) {
  dp = op_readpc();
  op_readdp(dp);
  op_writedp(dp, r);
}

void SPC700::op_write_dpi(uint8_t& r, uint8_t& i) {
  dp = op_readpc() + i;
  op_io();
  op_readdp(dp);
  op_writedp(dp, r);
}

template<uint8_t (SPC700::*op)(uint8_t, uint8_t)>
void SPC700::op_write_dp_const() {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = call(wr, rd);
  op != &SPC700::op_cmp ? op_writedp(dp, wr) : op_io();
}

template<uint8_t (SPC700::*op)(uint8_t, uint8_t)>
void SPC700::op_write_dp_dp() {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  if(op != &SPC700::op_st) wr = op_readdp(dp);
  wr = call(wr, rd);
  op != &SPC700::op_cmp ? op_writedp(dp, wr) : op_io();
}

template<uint8_t (SPC700::*op)(uint8_t, uint8_t)>
void SPC700::op_write_ix_iy() {
  op_io();
  rd = op_readdp(regs.y);
  wr = op_readdp(regs.x);
  wr = call(wr, rd);
  op != &SPC700::op_cmp ? op_writedp(regs.x, wr) : op_io();
}

//

void SPC700::op_bne_dp() {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if(regs.a == sp) return;
  op_io();
  op_io();
  regs.pc += (int8_t)rd;
}

void SPC700::op_bne_dpdec() {
  dp = op_readpc();
  wr = op_readdp(dp);
  op_writedp(dp, --wr);
  rd = op_readpc();
  if(wr == 0) return;
  op_io();
  op_io();
  regs.pc += (int8_t)rd;
}

void SPC700::op_bne_dpx() {
  dp = op_readpc();
  op_io();
  sp = op_readdp(dp + regs.x);
  rd = op_readpc();
  op_io();
  if(regs.a == sp) return;
  op_io();
  op_io();
  regs.pc += (int8_t)rd;
}

void SPC700::op_bne_ydec() {
  rd = op_readpc();
  op_io();
  op_io();
  if(--regs.y == 0) return;
  op_io();
  op_io();
  regs.pc += (int8_t)rd;
}

void SPC700::op_brk() {
  rd.l = op_read(0xffde);
  rd.h = op_read(0xffdf);
  op_io();
  op_io();
  op_writesp(regs.pc.h);
  op_writesp(regs.pc.l);
  op_writesp(regs.p);
  regs.pc = rd;
  regs.p.b = 1;
  regs.p.i = 0;
}

void SPC700::op_clv() {
  op_io();
  regs.p.v = 0;
  regs.p.h = 0;
}

void SPC700::op_cmc() {
  op_io();
  op_io();
  regs.p.c = !regs.p.c;
}

void SPC700::op_daa() {
  op_io();
  op_io();
  if(regs.p.c || (regs.a) > 0x99) {
    regs.a += 0x60;
    regs.p.c = 1;
  }
  if(regs.p.h || (regs.a & 15) > 0x09) {
    regs.a += 0x06;
  }
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void SPC700::op_das() {
  op_io();
  op_io();
  if(!regs.p.c || (regs.a) > 0x99) {
    regs.a -= 0x60;
    regs.p.c = 0;
  }
  if(!regs.p.h || (regs.a & 15) > 0x09) {
    regs.a -= 0x06;
  }
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void SPC700::op_div_ya_x() {
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  ya = regs.ya;
  //overflow set if quotient >= 256
  regs.p.v = (regs.y >= regs.x);
  regs.p.h = ((regs.y & 15) >= (regs.x & 15));
  if(regs.y < (regs.x << 1)) {
    //if quotient is <= 511 (will fit into 9-bit result)
    regs.a = ya / regs.x;
    regs.y = ya % regs.x;
  } else {
    //otherwise, the quotient won't fit into regs.p.v + regs.a
    //this emulates the odd behavior of the S-SMP in this case
    regs.a = 255    - (ya - (regs.x << 9)) / (256 - regs.x);
    regs.y = regs.x + (ya - (regs.x << 9)) % (256 - regs.x);
  }
  //result is set based on a (quotient) only
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void SPC700::op_jmp_addr() {
  rd.l = op_readpc();
  rd.h = op_readpc();
  regs.pc = rd;
}

void SPC700::op_jmp_iaddrx() {
  dp.l = op_readpc();
  dp.h = op_readpc();
  op_io();
  dp += regs.x;
  rd.l = op_read(dp++);
  rd.h = op_read(dp++);
  regs.pc = rd;
}

void SPC700::op_jsp_dp() {
  rd = op_readpc();
  op_io();
  op_io();
  op_writesp(regs.pc.h);
  op_writesp(regs.pc.l);
  regs.pc = 0xff00 | rd;
}

void SPC700::op_jsr_addr() {
  rd.l = op_readpc();
  rd.h = op_readpc();
  op_io();
  op_io();
  op_io();
  op_writesp(regs.pc.h);
  op_writesp(regs.pc.l);
  regs.pc = rd;
}

void SPC700::op_jst() {
  dp = 0xffde - ((opcode >> 4) << 1);
  rd.l = op_read(dp++);
  rd.h = op_read(dp++);
  op_io();
  op_io();
  op_io();
  op_writesp(regs.pc.h);
  op_writesp(regs.pc.l);
  regs.pc = rd;
}

void SPC700::op_lda_ixinc() {
  op_io();
  regs.a = op_readdp(regs.x++);
  op_io();
  regs.p.n = regs.a & 0x80;
  regs.p.z = regs.a == 0;
}

void SPC700::op_mul_ya() {
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  ya = regs.y * regs.a;
  regs.a = ya;
  regs.y = ya >> 8;
  //result is set based on y (high-byte) only
  regs.p.n = (regs.y & 0x80);
  regs.p.z = (regs.y == 0);
}

void SPC700::op_nop() {
  op_io();
}

void SPC700::op_plp() {
  op_io();
  op_io();
  regs.p = op_readsp();
}

void SPC700::op_rti() {
  regs.p = op_readsp();
  rd.l = op_readsp();
  rd.h = op_readsp();
  op_io();
  op_io();
  regs.pc = rd;
}

void SPC700::op_rts() {
  rd.l = op_readsp();
  rd.h = op_readsp();
  op_io();
  op_io();
  regs.pc = rd;
}

void SPC700::op_sta_idpx() {
  sp = op_readpc() + regs.x;
  op_io();
  dp.l = op_readdp(sp++);
  dp.h = op_readdp(sp++);
  op_read(dp);
  op_write(dp, regs.a);
}

void SPC700::op_sta_idpy() {
  sp = op_readpc();
  dp.l = op_readdp(sp++);
  dp.h = op_readdp(sp++);
  op_io();
  dp += regs.y;
  op_read(dp);
  op_write(dp, regs.a);
}

void SPC700::op_sta_ix() {
  op_io();
  op_readdp(regs.x);
  op_writedp(regs.x, regs.a);
}

void SPC700::op_sta_ixinc() {
  op_io();
  op_io();
  op_writedp(regs.x++, regs.a);
}

void SPC700::op_stw_dp() {
  dp = op_readpc();
  op_readdp(dp);
  op_writedp(dp++, regs.a);
  op_writedp(dp++, regs.y);
}

void SPC700::op_wait() {
  op_io();
  op_io();
  regs.pc--;
}

void SPC700::op_xcn() {
  op_io();
  op_io();
  op_io();
  op_io();
  regs.a = (regs.a >> 4) | (regs.a << 4);
  regs.p.n = regs.a & 0x80;
  regs.p.z = regs.a == 0;
}

#undef call


uint8_t SPC700::op_adc(uint8_t x, uint8_t y) {
  int r = x + y + regs.p.c;
  regs.p.n = r & 0x80;
  regs.p.v = ~(x ^ y) & (x ^ r) & 0x80;
  regs.p.h = (x ^ y ^ r) & 0x10;
  regs.p.z = (uint8_t)r == 0;
  regs.p.c = r > 0xff;
  return r;
}

uint8_t SPC700::op_and(uint8_t x, uint8_t y) {
  x &= y;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8_t SPC700::op_asl(uint8_t x) {
  regs.p.c = x & 0x80;
  x <<= 1;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8_t SPC700::op_cmp(uint8_t x, uint8_t y) {
  int r = x - y;
  regs.p.n = r & 0x80;
  regs.p.z = (uint8_t)r == 0;
  regs.p.c = r >= 0;
  return x;
}

uint8_t SPC700::op_dec(uint8_t x) {
  x--;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8_t SPC700::op_eor(uint8_t x, uint8_t y) {
  x ^= y;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8_t SPC700::op_inc(uint8_t x) {
  x++;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8_t SPC700::op_ld(uint8_t x, uint8_t y) {
  regs.p.n = y & 0x80;
  regs.p.z = y == 0;
  return y;
}

uint8_t SPC700::op_lsr(uint8_t x) {
  regs.p.c = x & 0x01;
  x >>= 1;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8_t SPC700::op_or(uint8_t x, uint8_t y) {
  x |= y;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8_t SPC700::op_rol(uint8_t x) {
  unsigned carry = regs.p.c << 0;
  regs.p.c = x & 0x80;
  x = (x << 1) | carry;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8_t SPC700::op_ror(uint8_t x) {
  unsigned carry = regs.p.c << 7;
  regs.p.c = x & 0x01;
  x = carry | (x >> 1);
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8_t SPC700::op_sbc(uint8_t x, uint8_t y) {
  return op_adc(x, ~y);
}

uint8_t SPC700::op_st(uint8_t x, uint8_t y) {
  return y;
}

//

uint16_t SPC700::op_adw(uint16_t x, uint16_t y) {
  uint16_t r;
  regs.p.c = 0;
  r  = op_adc(x, y);
  r |= op_adc(x >> 8, y >> 8) << 8;
  regs.p.z = r == 0;
  return r;
}

uint16_t SPC700::op_cpw(uint16_t x, uint16_t y) {
  int r = x - y;
  regs.p.n = r & 0x8000;
  regs.p.z = (uint16_t)r == 0;
  regs.p.c = r >= 0;
  return x;
}

uint16_t SPC700::op_ldw(uint16_t x, uint16_t y) {
  regs.p.n = y & 0x8000;
  regs.p.z = y == 0;
  return y;
}

uint16_t SPC700::op_sbw(uint16_t x, uint16_t y) {
  uint16_t r;
  regs.p.c = 1;
  r  = op_sbc(x, y);
  r |= op_sbc(x >> 8, y >> 8) << 8;
  regs.p.z = r == 0;
  return r;
}


void SPC700::op_step() {
#if 0
  std::string disasm = disassemble_opcode(regs.pc) + "\n";
  fputs(disasm.c_str(), f);
#endif
  switch(opcode = op_readpc()) {
  case 0x00: return op_nop();
  case 0x01: return op_jst();
  case 0x02: return op_set_bit();
  case 0x03: return op_branch_bit();
  case 0x04: return op_read_dp<&SPC700::op_or>(regs.a);
  case 0x05: return op_read_addr<&SPC700::op_or>(regs.a);
  case 0x06: return op_read_ix<&SPC700::op_or>();
  case 0x07: return op_read_idpx<&SPC700::op_or>();
  case 0x08: return op_read_const<&SPC700::op_or>(regs.a);
  case 0x09: return op_write_dp_dp<&SPC700::op_or>();
  case 0x0a: return op_set_addr_bit();
  case 0x0b: return op_adjust_dp<&SPC700::op_asl>();
  case 0x0c: return op_adjust_addr<&SPC700::op_asl>();
  case 0x0d: return op_push(regs.p);
  case 0x0e: return op_test_addr(1);
  case 0x0f: return op_brk();
  case 0x10: return op_branch(regs.p.n == 0);
  case 0x11: return op_jst();
  case 0x12: return op_set_bit();
  case 0x13: return op_branch_bit();
  case 0x14: return op_read_dpi<&SPC700::op_or>(regs.a, regs.x);
  case 0x15: return op_read_addri<&SPC700::op_or>(regs.x);
  case 0x16: return op_read_addri<&SPC700::op_or>(regs.y);
  case 0x17: return op_read_idpy<&SPC700::op_or>();
  case 0x18: return op_write_dp_const<&SPC700::op_or>();
  case 0x19: return op_write_ix_iy<&SPC700::op_or>();
  case 0x1a: return op_adjust_dpw(-1);
  case 0x1b: return op_adjust_dpx<&SPC700::op_asl>();
  case 0x1c: return op_adjust<&SPC700::op_asl>(regs.a);
  case 0x1d: return op_adjust<&SPC700::op_dec>(regs.x);
  case 0x1e: return op_read_addr<&SPC700::op_cmp>(regs.x);
  case 0x1f: return op_jmp_iaddrx();
  case 0x20: return op_set_flag(regs.p.p, 0);
  case 0x21: return op_jst();
  case 0x22: return op_set_bit();
  case 0x23: return op_branch_bit();
  case 0x24: return op_read_dp<&SPC700::op_and>(regs.a);
  case 0x25: return op_read_addr<&SPC700::op_and>(regs.a);
  case 0x26: return op_read_ix<&SPC700::op_and>();
  case 0x27: return op_read_idpx<&SPC700::op_and>();
  case 0x28: return op_read_const<&SPC700::op_and>(regs.a);
  case 0x29: return op_write_dp_dp<&SPC700::op_and>();
  case 0x2a: return op_set_addr_bit();
  case 0x2b: return op_adjust_dp<&SPC700::op_rol>();
  case 0x2c: return op_adjust_addr<&SPC700::op_rol>();
  case 0x2d: return op_push(regs.a);
  case 0x2e: return op_bne_dp();
  case 0x2f: return op_branch(true);
  case 0x30: return op_branch(regs.p.n == 1);
  case 0x31: return op_jst();
  case 0x32: return op_set_bit();
  case 0x33: return op_branch_bit();
  case 0x34: return op_read_dpi<&SPC700::op_and>(regs.a, regs.x);
  case 0x35: return op_read_addri<&SPC700::op_and>(regs.x);
  case 0x36: return op_read_addri<&SPC700::op_and>(regs.y);
  case 0x37: return op_read_idpy<&SPC700::op_and>();
  case 0x38: return op_write_dp_const<&SPC700::op_and>();
  case 0x39: return op_write_ix_iy<&SPC700::op_and>();
  case 0x3a: return op_adjust_dpw(+1);
  case 0x3b: return op_adjust_dpx<&SPC700::op_rol>();
  case 0x3c: return op_adjust<&SPC700::op_rol>(regs.a);
  case 0x3d: return op_adjust<&SPC700::op_inc>(regs.x);
  case 0x3e: return op_read_dp<&SPC700::op_cmp>(regs.x);
  case 0x3f: return op_jsr_addr();
  case 0x40: return op_set_flag(regs.p.p, 1);
  case 0x41: return op_jst();
  case 0x42: return op_set_bit();
  case 0x43: return op_branch_bit();
  case 0x44: return op_read_dp<&SPC700::op_eor>(regs.a);
  case 0x45: return op_read_addr<&SPC700::op_eor>(regs.a);
  case 0x46: return op_read_ix<&SPC700::op_eor>();
  case 0x47: return op_read_idpx<&SPC700::op_eor>();
  case 0x48: return op_read_const<&SPC700::op_eor>(regs.a);
  case 0x49: return op_write_dp_dp<&SPC700::op_eor>();
  case 0x4a: return op_set_addr_bit();
  case 0x4b: return op_adjust_dp<&SPC700::op_lsr>();
  case 0x4c: return op_adjust_addr<&SPC700::op_lsr>();
  case 0x4d: return op_push(regs.x);
  case 0x4e: return op_test_addr(0);
  case 0x4f: return op_jsp_dp();
  case 0x50: return op_branch(regs.p.v == 0);
  case 0x51: return op_jst();
  case 0x52: return op_set_bit();
  case 0x53: return op_branch_bit();
  case 0x54: return op_read_dpi<&SPC700::op_eor>(regs.a, regs.x);
  case 0x55: return op_read_addri<&SPC700::op_eor>(regs.x);
  case 0x56: return op_read_addri<&SPC700::op_eor>(regs.y);
  case 0x57: return op_read_idpy<&SPC700::op_eor>();
  case 0x58: return op_write_dp_const<&SPC700::op_eor>();
  case 0x59: return op_write_ix_iy<&SPC700::op_eor>();
  case 0x5a: return op_read_dpw<&SPC700::op_cpw>();
  case 0x5b: return op_adjust_dpx<&SPC700::op_lsr>();
  case 0x5c: return op_adjust<&SPC700::op_lsr>(regs.a);
  case 0x5d: return op_transfer(regs.a, regs.x);
  case 0x5e: return op_read_addr<&SPC700::op_cmp>(regs.y);
  case 0x5f: return op_jmp_addr();
  case 0x60: return op_set_flag(regs.p.c, 0);
  case 0x61: return op_jst();
  case 0x62: return op_set_bit();
  case 0x63: return op_branch_bit();
  case 0x64: return op_read_dp<&SPC700::op_cmp>(regs.a);
  case 0x65: return op_read_addr<&SPC700::op_cmp>(regs.a);
  case 0x66: return op_read_ix<&SPC700::op_cmp>();
  case 0x67: return op_read_idpx<&SPC700::op_cmp>();
  case 0x68: return op_read_const<&SPC700::op_cmp>(regs.a);
  case 0x69: return op_write_dp_dp<&SPC700::op_cmp>();
  case 0x6a: return op_set_addr_bit();
  case 0x6b: return op_adjust_dp<&SPC700::op_ror>();
  case 0x6c: return op_adjust_addr<&SPC700::op_ror>();
  case 0x6d: return op_push(regs.y);
  case 0x6e: return op_bne_dpdec();
  case 0x6f: return op_rts();
  case 0x70: return op_branch(regs.p.v == 1);
  case 0x71: return op_jst();
  case 0x72: return op_set_bit();
  case 0x73: return op_branch_bit();
  case 0x74: return op_read_dpi<&SPC700::op_cmp>(regs.a, regs.x);
  case 0x75: return op_read_addri<&SPC700::op_cmp>(regs.x);
  case 0x76: return op_read_addri<&SPC700::op_cmp>(regs.y);
  case 0x77: return op_read_idpy<&SPC700::op_cmp>();
  case 0x78: return op_write_dp_const<&SPC700::op_cmp>();
  case 0x79: return op_write_ix_iy<&SPC700::op_cmp>();
  case 0x7a: return op_read_dpw<&SPC700::op_adw>();
  case 0x7b: return op_adjust_dpx<&SPC700::op_ror>();
  case 0x7c: return op_adjust<&SPC700::op_ror>(regs.a);
  case 0x7d: return op_transfer(regs.x, regs.a);
  case 0x7e: return op_read_dp<&SPC700::op_cmp>(regs.y);
  case 0x7f: return op_rti();
  case 0x80: return op_set_flag(regs.p.c, 1);
  case 0x81: return op_jst();
  case 0x82: return op_set_bit();
  case 0x83: return op_branch_bit();
  case 0x84: return op_read_dp<&SPC700::op_adc>(regs.a);
  case 0x85: return op_read_addr<&SPC700::op_adc>(regs.a);
  case 0x86: return op_read_ix<&SPC700::op_adc>();
  case 0x87: return op_read_idpx<&SPC700::op_adc>();
  case 0x88: return op_read_const<&SPC700::op_adc>(regs.a);
  case 0x89: return op_write_dp_dp<&SPC700::op_adc>();
  case 0x8a: return op_set_addr_bit();
  case 0x8b: return op_adjust_dp<&SPC700::op_dec>();
  case 0x8c: return op_adjust_addr<&SPC700::op_dec>();
  case 0x8d: return op_read_const<&SPC700::op_ld>(regs.y);
  case 0x8e: return op_plp();
  case 0x8f: return op_write_dp_const<&SPC700::op_st>();
  case 0x90: return op_branch(regs.p.c == 0);
  case 0x91: return op_jst();
  case 0x92: return op_set_bit();
  case 0x93: return op_branch_bit();
  case 0x94: return op_read_dpi<&SPC700::op_adc>(regs.a, regs.x);
  case 0x95: return op_read_addri<&SPC700::op_adc>(regs.x);
  case 0x96: return op_read_addri<&SPC700::op_adc>(regs.y);
  case 0x97: return op_read_idpy<&SPC700::op_adc>();
  case 0x98: return op_write_dp_const<&SPC700::op_adc>();
  case 0x99: return op_write_ix_iy<&SPC700::op_adc>();
  case 0x9a: return op_read_dpw<&SPC700::op_sbw>();
  case 0x9b: return op_adjust_dpx<&SPC700::op_dec>();
  case 0x9c: return op_adjust<&SPC700::op_dec>(regs.a);
  case 0x9d: return op_transfer(regs.s, regs.x);
  case 0x9e: return op_div_ya_x();
  case 0x9f: return op_xcn();
  case 0xa0: return op_set_flag(regs.p.i, 1);
  case 0xa1: return op_jst();
  case 0xa2: return op_set_bit();
  case 0xa3: return op_branch_bit();
  case 0xa4: return op_read_dp<&SPC700::op_sbc>(regs.a);
  case 0xa5: return op_read_addr<&SPC700::op_sbc>(regs.a);
  case 0xa6: return op_read_ix<&SPC700::op_sbc>();
  case 0xa7: return op_read_idpx<&SPC700::op_sbc>();
  case 0xa8: return op_read_const<&SPC700::op_sbc>(regs.a);
  case 0xa9: return op_write_dp_dp<&SPC700::op_sbc>();
  case 0xaa: return op_set_addr_bit();
  case 0xab: return op_adjust_dp<&SPC700::op_inc>();
  case 0xac: return op_adjust_addr<&SPC700::op_inc>();
  case 0xad: return op_read_const<&SPC700::op_cmp>(regs.y);
  case 0xae: return op_pull(regs.a);
  case 0xaf: return op_sta_ixinc();
  case 0xb0: return op_branch(regs.p.c == 1);
  case 0xb1: return op_jst();
  case 0xb2: return op_set_bit();
  case 0xb3: return op_branch_bit();
  case 0xb4: return op_read_dpi<&SPC700::op_sbc>(regs.a, regs.x);
  case 0xb5: return op_read_addri<&SPC700::op_sbc>(regs.x);
  case 0xb6: return op_read_addri<&SPC700::op_sbc>(regs.y);
  case 0xb7: return op_read_idpy<&SPC700::op_sbc>();
  case 0xb8: return op_write_dp_const<&SPC700::op_sbc>();
  case 0xb9: return op_write_ix_iy<&SPC700::op_sbc>();
  case 0xba: return op_read_dpw<&SPC700::op_ldw>();
  case 0xbb: return op_adjust_dpx<&SPC700::op_inc>();
  case 0xbc: return op_adjust<&SPC700::op_inc>(regs.a);
  case 0xbd: return op_transfer(regs.x, regs.s);
  case 0xbe: return op_das();
  case 0xbf: return op_lda_ixinc();
  case 0xc0: return op_set_flag(regs.p.i, 0);
  case 0xc1: return op_jst();
  case 0xc2: return op_set_bit();
  case 0xc3: return op_branch_bit();
  case 0xc4: return op_write_dp(regs.a);
  case 0xc5: return op_write_addr(regs.a);
  case 0xc6: return op_sta_ix();
  case 0xc7: return op_sta_idpx();
  case 0xc8: return op_read_const<&SPC700::op_cmp>(regs.x);
  case 0xc9: return op_write_addr(regs.x);
  case 0xca: return op_set_addr_bit();
  case 0xcb: return op_write_dp(regs.y);
  case 0xcc: return op_write_addr(regs.y);
  case 0xcd: return op_read_const<&SPC700::op_ld>(regs.x);
  case 0xce: return op_pull(regs.x);
  case 0xcf: return op_mul_ya();
  case 0xd0: return op_branch(regs.p.z == 0);
  case 0xd1: return op_jst();
  case 0xd2: return op_set_bit();
  case 0xd3: return op_branch_bit();
  case 0xd4: return op_write_dpi(regs.a, regs.x);
  case 0xd5: return op_write_addri(regs.x);
  case 0xd6: return op_write_addri(regs.y);
  case 0xd7: return op_sta_idpy();
  case 0xd8: return op_write_dp(regs.x);
  case 0xd9: return op_write_dpi(regs.x, regs.y);
  case 0xda: return op_stw_dp();
  case 0xdb: return op_write_dpi(regs.y, regs.x);
  case 0xdc: return op_adjust<&SPC700::op_dec>(regs.y);
  case 0xdd: return op_transfer(regs.y, regs.a);
  case 0xde: return op_bne_dpx();
  case 0xdf: return op_daa();
  case 0xe0: return op_clv();
  case 0xe1: return op_jst();
  case 0xe2: return op_set_bit();
  case 0xe3: return op_branch_bit();
  case 0xe4: return op_read_dp<&SPC700::op_ld>(regs.a);
  case 0xe5: return op_read_addr<&SPC700::op_ld>(regs.a);
  case 0xe6: return op_read_ix<&SPC700::op_ld>();
  case 0xe7: return op_read_idpx<&SPC700::op_ld>();
  case 0xe8: return op_read_const<&SPC700::op_ld>(regs.a);
  case 0xe9: return op_read_addr<&SPC700::op_ld>(regs.x);
  case 0xea: return op_set_addr_bit();
  case 0xeb: return op_read_dp<&SPC700::op_ld>(regs.y);
  case 0xec: return op_read_addr<&SPC700::op_ld>(regs.y);
  case 0xed: return op_cmc();
  case 0xee: return op_pull(regs.y);
  case 0xef: return op_wait();
  case 0xf0: return op_branch(regs.p.z == 1);
  case 0xf1: return op_jst();
  case 0xf2: return op_set_bit();
  case 0xf3: return op_branch_bit();
  case 0xf4: return op_read_dpi<&SPC700::op_ld>(regs.a, regs.x);
  case 0xf5: return op_read_addri<&SPC700::op_ld>(regs.x);
  case 0xf6: return op_read_addri<&SPC700::op_ld>(regs.y);
  case 0xf7: return op_read_idpy<&SPC700::op_ld>();
  case 0xf8: return op_read_dp<&SPC700::op_ld>(regs.x);
  case 0xf9: return op_read_dpi<&SPC700::op_ld>(regs.x, regs.y);
  case 0xfa: return op_write_dp_dp<&SPC700::op_st>();
  case 0xfb: return op_read_dpi<&SPC700::op_ld>(regs.y, regs.x);
  case 0xfc: return op_adjust<&SPC700::op_inc>(regs.y);
  case 0xfd: return op_transfer(regs.a, regs.y);
  case 0xfe: return op_bne_ydec();
  case 0xff: return op_wait();
  }
}

}
