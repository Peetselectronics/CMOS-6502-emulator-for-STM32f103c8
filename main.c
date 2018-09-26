/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_flash.h"
#include "misc.h"
#include "bios.h"
#include "main.h"

//usart
#define NUM 10
int i,j;
uint8_t a;
char name[NUM+1] = {'\0'};
ErrorStatus HSEStartUpStatus;
//char memtest[7] =   {'1', '6', '3', '8', '4', '\r', '\n'};
uint8_t once_ram = 0;

uint8_t z = 0;
uint8_t line_flag = 0;

//6502
#define RAM_SIZE 16384
uint8_t RAM[RAM_SIZE];

//6502 CPU registers
uint8_t sp, a, x, y, cpustatus;
uint16_t pc;

//helper variables
uint32_t instructions = 0; //keep track of total instructions executed
int32_t clockticks6502 = 0, clockgoal6502 = 0;
uint16_t oldpc, ea, reladdr, value, result;
uint8_t opcode, oldcpustatus, useaccum;

#define FLAG_CARRY     0x01
#define FLAG_ZERO      0x02
#define FLAG_INTERRUPT 0x04
#define FLAG_DECIMAL   0x08
#define FLAG_BREAK     0x10
#define FLAG_CONSTANT  0x20
#define FLAG_OVERFLOW  0x40
#define FLAG_SIGN      0x80

//flag calculation macros
#define zerocalc(n) { if ((n) & 0x00FF) clearzero(); else setzero(); }
#define signcalc(n) { if ((n) & 0x0080) setsign(); else clearsign(); }
#define carrycalc(n) { if ((n) & 0xFF00) setcarry(); else clearcarry(); }
#define overflowcalc(n, m, o) { if (((n) ^ (uint16_t)(m)) & ((n) ^ (o)) & 0x0080) setoverflow(); else clearoverflow(); }

//flag modifier macros
#define setcarry() cpustatus |= FLAG_CARRY
#define clearcarry() cpustatus &= (~FLAG_CARRY)
#define setzero() cpustatus |= FLAG_ZERO
#define clearzero() cpustatus &= (~FLAG_ZERO)
#define setinterrupt() cpustatus |= FLAG_INTERRUPT
#define clearinterrupt() cpustatus &= (~FLAG_INTERRUPT)
#define setdecimal() cpustatus |= FLAG_DECIMAL
#define cleardecimal() cpustatus &= (~FLAG_DECIMAL)
#define setoverflow() cpustatus |= FLAG_OVERFLOW
#define clearoverflow() cpustatus &= (~FLAG_OVERFLOW)
#define setsign() cpustatus |= FLAG_SIGN
#define clearsign() cpustatus &= (~FLAG_SIGN)

#define BASE_STACK     0x100

#define saveaccum(n) a = (uint8_t)((n) & 0x00FF)

#define setinterrupt() cpustatus |= FLAG_INTERRUPT

//serial var
uint8_t curkey = 'C';


char memtest[7] =   "16384\r\n";
char memtest2[20] = "10 FOR N = 1 TO 10\r\n";
char memtest3[19] = "20 FOR J = 1 TO N\r\n";
char memtest4[13] = "30 PRINT J;\r\n";
char memtest5[11] = "40 NEXT J\r\n";
char memtest6[10] = "50 PRINT\r\n";
char memtest7[11] = "60 NEXT N\r\n";
char memtest8[5] =  "RUN\r\n";


char benchline1[22] = "10 FOR N = 1 TO 10000\r\n";
char benchline2[11] = "20 NEXT N\r\n";
char benchline3[5] =  "RUN\r\n";

char benchV2line1[25] = "10 FOR I=0 TO 10000:NEXT\r\n";
char benchV2line2[5] =  "RUN\r\n";

int main(void)
{
	const unsigned char menu[] = " 6502 CPU emulator V0.1!\r\n";

	/* Enable USART1 and GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	SetSysClockTo72();
	NVIC_Configuration();/* NVIC Configuration usart */
	GPIO_Configuration();/* Configure the GPIOs usart */
	USART_Configuration();/* Configure the USART1 */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);/* Enable the USART1 Receive interrupt */

	/* print welcome information */
	UARTSend(menu, sizeof(menu));

	reset6502();

    while(1)
    {
    	exec6502(800);
    	if(line_flag == 1){ //ram
    		curkey = memtest[z];
    		z++;
    		//uint8_t strlengte = strlen(memtest) - 48;
    		if(z > 6){
				line_flag = 2;
				z = 0;
    		}
    	}
    	exec6502(800);
    	if(line_flag == 2){ //for
    		curkey = benchV2line1[z];
    		z++;
    		if(z > 24){
				line_flag = 3;
				z = 0;
    		}
    	}
    	exec6502(800);
    	if(line_flag == 3){ //run
    	    curkey = benchV2line2[z];
    	    z++;
			 if(z > 4){
				line_flag = 0;
				z = 0;
			 }
    	 }

/*
    	exec6502(300);
    	if(line_flag == 2){ //for
    		curkey = benchline1[z];
    		z++;
    		if(z > 21){
				line_flag = 3;
				z = 0;
    		}
    	}
    	exec6502(300);
    	if(line_flag == 3){ //print
    	    curkey = benchline2[z];
    	    z++;
			 if(z > 10){
				line_flag = 4;
				z = 0;
			 }
    	 }
    	exec6502(300);
    	if(line_flag == 4){ //run
    	    curkey = benchline3[z];
    	    z++;
			 if(z > 4){
				line_flag = 0;
				z = 0;
			 }
    	 }
*/
//////////////////

/*
    	///start program 10
    	exec6502(400);

    	if(line_flag == 2){
    		curkey = memtest2[z];
    		z++;
    		if(z > 19){
				line_flag = 3;
				z = 0;
    		}
    	}

    	exec6502(400);

    	if(line_flag == 3){
    		curkey = memtest3[z];
    		z++;
    		if(z > 18){
				line_flag = 4;
				z = 0;
    		}
    	}

    	exec6502(400);

    	if(line_flag == 4){
    	    curkey = memtest4[z];
    	    z++;
			 if(z > 12){
				line_flag = 5;
				z = 0;
			 }
    	 }

    	exec6502(400);

    	if(line_flag == 5){
    	    curkey = memtest5[z];
    	    z++;
			 if(z > 10){
				line_flag = 6;
				z = 0;
			 }
    	 }
    	exec6502(400);

    	if(line_flag == 6){
    	    curkey = memtest6[z];
    	    z++;
			 if(z > 9){
				line_flag = 7;
				z = 0;
			 }
    	 }
    	exec6502(400);

    	if(line_flag == 7){
    	    curkey = memtest7[z];
    	    z++;
			 if(z > 10){
				line_flag = 8;
				z = 0;
			 }
    	 }
    	exec6502(400);

    	if(line_flag == 8){ //run
    	    curkey = memtest8[z];
    	    z++;
			 if(z > 4){
				line_flag = 0;
				z = 0;
			 }
    	 }
*/
    	//print_RXbuf();

    }
}

void print_RXbuf(){

	if(strlen(name) > 1){
		if(strlen(name) > 2){

			curkey = name[a]  & 0x7F;
			//USART_SendData(USART1, curkey);// Last Version USART_SendData(USART1,(uint16_t) *pucBuffer++);
			//while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)

			if(a >= strlen(name)){
				a = 0;
			}
			a++;
		}
		else{
			curkey = name[0]  & 0x7F;
			//USART_SendData(USART1, curkey);// Last Version USART_SendData(USART1,(uint16_t) *pucBuffer++);
			//while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)

			name[0] = '\0';
			j = 0;
		}
	}
}

void USART1_IRQHandler(void)
{
    if ((USART1->SR & USART_FLAG_RXNE) != (u16)RESET)
	{
			i = USART_ReceiveData(USART1);
			if(j == NUM)
			{
                name[j] = i;
			    j = 0;
			}
			else
			{
                name[j++] = i;
			}
			name[j] = '\0';
			if(once_ram == 0){
			line_flag = 1;
			once_ram = 1;
			}
			else{
				line_flag = 2;
			}
	}
}

void reset6502() {
    pc = (uint16_t)read6502(0xFFFC) | ((uint16_t)read6502(0xFFFD) << 8);
    a = 0;
    x = 0;
    y = 0;
    sp = 0xFD;
    cpustatus |= FLAG_CONSTANT;
}


uint8_t read6502(uint16_t address) {
  uint16_t BIOSaddr;
  uint8_t tempval = 0;

  if (address == 0xF004) { //EhBASIC simulated ASIC input
    tempval = getkey();
    clearkey();
    return(tempval);
  }

  if (address >= 0xC000) {
    BIOSaddr = address - 0xC000;
    if (BIOSaddr < 0x2900) return(BIOS[BIOSaddr]);
    //if (BIOSaddr < 0x2900) return(BIOS + BIOSaddr);
    if (BIOSaddr >= 0x3F00) return(BIOStop[BIOSaddr - 0x3F00]);
    //if (BIOSaddr >= 0x3F00) return(BIOStop + BIOSaddr - 0x3F00);
  }

  if (address < RAM_SIZE) return(RAM[address]);
  return(0);
}

void write6502(uint16_t address, uint8_t value) {
  if (address < RAM_SIZE) RAM[address] = value;
  if (address == 0xF001) { //EhBASIC simulated ASIC output
    //serout(value); //print serial
	  //UARTSend(value, sizeof(value));
	  USART_SendData(USART1, value);
	  while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	          {
	          }

  }
}

void exec6502(int32_t tickcount) {
#ifdef USE_TIMING
  clockgoal6502 += tickcount;

  while (clockgoal6502 > 0) {
#else
  while (tickcount--) {
#endif
    opcode = read6502(pc++);
    cpustatus |= FLAG_CONSTANT;

    useaccum = 0;

	switch (opcode) {
		case 0x0:
			imp();
			brk();
			break;
		case 0x1:
			indx();
			ora();
			break;
		case 0x5:
			zp();
			ora();
			break;
		case 0x6:
			zp();
			asl();
			break;
		case 0x8:
			imp();
			php();
			break;
		case 0x9:
			imm();
			ora();
			break;
		case 0xA:
			acc();
			asl();
			break;
		case 0xD:
			abso();
			ora();
			break;
		case 0xE:
			abso();
			asl();
			break;
		case 0x10:
			rel();
			bpl();
			break;
		case 0x11:
			indy();
			ora();
			break;
		case 0x15:
			zpx();
			ora();
			break;
		case 0x16:
			zpx();
			asl();
			break;
		case 0x18:
			imp();
			clc();
			break;
		case 0x19:
			absy();
			ora();
			break;
		case 0x1D:
			absx();
			ora();
			break;
		case 0x1E:
			absx();
			asl();
			break;
		case 0x20:
			abso();
			jsr();
			break;
		case 0x21:
			indx();
			op_and();
			break;
		case 0x24:
			zp();
			op_bit();
			break;
		case 0x25:
			zp();
			op_and();
			break;
		case 0x26:
			zp();
			rol();
			break;
		case 0x28:
			imp();
			plp();
			break;
		case 0x29:
			imm();
			op_and();
			break;
		case 0x2A:
			acc();
			rol();
			break;
		case 0x2C:
			abso();
			op_bit();
			break;
		case 0x2D:
			abso();
			op_and();
			break;
		case 0x2E:
			abso();
			rol();
			break;
		case 0x30:
			rel();
			bmi();
			break;
		case 0x31:
			indy();
			op_and();
			break;
		case 0x35:
			zpx();
			op_and();
			break;
		case 0x36:
			zpx();
			rol();
			break;
		case 0x38:
			imp();
			sec();
			break;
		case 0x39:
			absy();
			op_and();
			break;
		case 0x3D:
			absx();
			op_and();
			break;
		case 0x3E:
			absx();
			rol();
			break;
		case 0x40:
			imp();
			rti();
			break;
		case 0x41:
			indx();
			eor();
			break;
		case 0x45:
			zp();
			eor();
			break;
		case 0x46:
			zp();
			lsr();
			break;
		case 0x48:
			imp();
			pha();
			break;
		case 0x49:
			imm();
			eor();
			break;
		case 0x4A:
			acc();
			lsr();
			break;
		case 0x4C:
			abso();
			jmp();
			break;
		case 0x4D:
			abso();
			eor();
			break;
		case 0x4E:
			abso();
			lsr();
			break;
		case 0x50:
			rel();
			bvc();
			break;
		case 0x51:
			indy();
			eor();
			break;
		case 0x55:
			zpx();
			eor();
			break;
		case 0x56:
			zpx();
			lsr();
			break;
		case 0x58:
			imp();
			cli();
			break;
		case 0x59:
			absy();
			eor();
			break;
		case 0x5D:
			absx();
			eor();
			break;
		case 0x5E:
			absx();
			lsr();
			break;
		case 0x60:
			imp();
			rts();
			break;
		case 0x61:
			indx();
			adc();
			break;
		case 0x65:
			zp();
			adc();
			break;
		case 0x66:
			zp();
			ror();
			break;
		case 0x68:
			imp();
			pla();
			break;
		case 0x69:
			imm();
			adc();
			break;
		case 0x6A:
			acc();
			ror();
			break;
		case 0x6C:
			ind();
			jmp();
			break;
		case 0x6D:
			abso();
			adc();
			break;
		case 0x6E:
			abso();
			ror();
			break;
		case 0x70:
			rel();
			bvs();
			break;
		case 0x71:
			indy();
			adc();
			break;
		case 0x75:
			zpx();
			adc();
			break;
		case 0x76:
			zpx();
			ror();
			break;
		case 0x78:
			imp();
			sei();
			break;
		case 0x79:
			absy();
			adc();
			break;
		case 0x7D:
			absx();
			adc();
			break;
		case 0x7E:
			absx();
			ror();
			break;
		case 0x81:
			indx();
			sta();
			break;
		case 0x84:
			zp();
			sty();
			break;
		case 0x85:
			zp();
			sta();
			break;
		case 0x86:
			zp();
			stx();
			break;
		case 0x88:
			imp();
			dey();
			break;
		case 0x8A:
			imp();
			txa();
			break;
		case 0x8C:
			abso();
			sty();
			break;
		case 0x8D:
			abso();
			sta();
			break;
		case 0x8E:
			abso();
			stx();
			break;
		case 0x90:
			rel();
			bcc();
			break;
		case 0x91:
			indy();
			sta();
			break;
		case 0x94:
			zpx();
			sty();
			break;
		case 0x95:
			zpx();
			sta();
			break;
		case 0x96:
			zpy();
			stx();
			break;
		case 0x98:
			imp();
			tya();
			break;
		case 0x99:
			absy();
			sta();
			break;
		case 0x9A:
			imp();
			txs();
			break;
		case 0x9D:
			absx();
			sta();
			break;
		case 0xA0:
			imm();
			ldy();
			break;
		case 0xA1:
			indx();
			lda();
			break;
		case 0xA2:
			imm();
			ldx();
			break;
		case 0xA4:
			zp();
			ldy();
			break;
		case 0xA5:
			zp();
			lda();
			break;
		case 0xA6:
			zp();
			ldx();
			break;
		case 0xA8:
			imp();
			tay();
			break;
		case 0xA9:
			imm();
			lda();
			break;
		case 0xAA:
			imp();
			tax();
			break;
		case 0xAC:
			abso();
			ldy();
			break;
		case 0xAD:
			abso();
			lda();
			break;
		case 0xAE:
			abso();
			ldx();
			break;
		case 0xB0:
			rel();
			bcs();
			break;
		case 0xB1:
			indy();
			lda();
			break;
		case 0xB4:
			zpx();
			ldy();
			break;
		case 0xB5:
			zpx();
			lda();
			break;
		case 0xB6:
			zpy();
			ldx();
			break;
		case 0xB8:
			imp();
			clv();
			break;
		case 0xB9:
			absy();
			lda();
			break;
		case 0xBA:
			imp();
			tsx();
			break;
		case 0xBC:
			absx();
			ldy();
			break;
		case 0xBD:
			absx();
			lda();
			break;
		case 0xBE:
			absy();
			ldx();
			break;
		case 0xC0:
			imm();
			cpy();
			break;
		case 0xC1:
			indx();
			cmp();
			break;
		case 0xC4:
			zp();
			cpy();
			break;
		case 0xC5:
			zp();
			cmp();
			break;
		case 0xC6:
			zp();
			dec();
			break;
		case 0xC8:
			imp();
			iny();
			break;
		case 0xC9:
			imm();
			cmp();
			break;
		case 0xCA:
			imp();
			dex();
			break;
		case 0xCC:
			abso();
			cpy();
			break;
		case 0xCD:
			abso();
			cmp();
			break;
		case 0xCE:
			abso();
			dec();
			break;
		case 0xD0:
			rel();
			bne();
			break;
		case 0xD1:
			indy();
			cmp();
			break;
		case 0xD5:
			zpx();
			cmp();
			break;
		case 0xD6:
			zpx();
			dec();
			break;
		case 0xD8:
			imp();
			cld();
			break;
		case 0xD9:
			absy();
			cmp();
			break;
		case 0xDD:
			absx();
			cmp();
			break;
		case 0xDE:
			absx();
			dec();
			break;
		case 0xE0:
			imm();
			cpx();
			break;
		case 0xE1:
			indx();
			sbc();
			break;
		case 0xE4:
			zp();
			cpx();
			break;
		case 0xE5:
			zp();
			sbc();
			break;
		case 0xE6:
			zp();
			inc();
			break;
		case 0xE8:
			imp();
			inx();
			break;
		case 0xE9:
			imm();
			sbc();
			break;
		case 0xEB:
			imm();
			sbc();
			break;
		case 0xEC:
			abso();
			cpx();
			break;
		case 0xED:
			abso();
			sbc();
			break;
		case 0xEE:
			abso();
			inc();
			break;
		case 0xF0:
			rel();
			beq();
			break;
		case 0xF1:
			indy();
			sbc();
			break;
		case 0xF5:
			zpx();
			sbc();
			break;
		case 0xF6:
			zpx();
			inc();
			break;
		case 0xF8:
			imp();
			sed();
			break;
		case 0xF9:
			absy();
			sbc();
			break;
		case 0xFD:
			absx();
			sbc();
			break;
		case 0xFE:
			absx();
			inc();
			break;
		}
#ifdef USE_TIMING
      clockgoal6502 -= (int32_t)pgm_read_byte_near(ticktable + opcode);
#endif
      instructions++;
  }
}

uint8_t getkey() {
    return(curkey);
}

void clearkey() {
  curkey = 0;
}

//instructies
//addressing mode functions, calculates effective addresses
void imp() { //implied
}

void brk() {
    pc++;
    push16(pc); //push next instruction address onto stack
    push8(cpustatus | FLAG_BREAK); //push CPU cpustatus to stack
    setinterrupt(); //set interrupt flag
    pc = (uint16_t)read6502(0xFFFE) | ((uint16_t)read6502(0xFFFF) << 8);
}

void indx() { // (indirect,X)
    uint16_t eahelp;
    eahelp = (uint16_t)(((uint16_t)read6502(pc++) + (uint16_t)x) & 0xFF); //zero-page wraparound for table pointer
    ea = (uint16_t)read6502(eahelp & 0x00FF) | ((uint16_t)read6502((eahelp+1) & 0x00FF) << 8);
}

void ora() {
    value = getvalue();
    result = (uint16_t)a | value;

    zerocalc(result);
    signcalc(result);

    saveaccum(result);
}

static uint16_t getvalue() {
    if (useaccum) return((uint16_t)a);
        else return((uint16_t)read6502(ea));
}

void putvalue(uint16_t saveval) {
    if (useaccum) a = (uint8_t)(saveval & 0x00FF);
        else write6502(ea, (saveval & 0x00FF));
}

void zp() { //zero-page
    ea = (uint16_t)read6502((uint16_t)pc++);
}

void asl() {
    value = getvalue();
    result = value << 1;

    carrycalc(result);
    zerocalc(result);
    signcalc(result);

    putvalue(result);
}

void php() {
    push8(cpustatus | FLAG_BREAK);
}

void imm() { //immediate
    ea = pc++;
}

void acc() { //accumulator
  useaccum = 1;
}

void abso() { //absolute
    ea = (uint16_t)read6502(pc) | ((uint16_t)read6502(pc+1) << 8);
    pc += 2;
}

void rel() { //relative for branch ops (8-bit immediate value, sign-extended)
    reladdr = (uint16_t)read6502(pc++);
    if (reladdr & 0x80) reladdr |= 0xFF00;
}

void bpl() {
    if ((cpustatus & FLAG_SIGN) == 0) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void indy() { // (indirect),Y
    uint16_t eahelp, eahelp2, startpage;
    eahelp = (uint16_t)read6502(pc++);
    eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //zero-page wraparound
    ea = (uint16_t)read6502(eahelp) | ((uint16_t)read6502(eahelp2) << 8);
    startpage = ea & 0xFF00;
    ea += (uint16_t)y;

}

void zpx() { //zero-page,X
    ea = ((uint16_t)read6502((uint16_t)pc++) + (uint16_t)x) & 0xFF; //zero-page wraparound
}

void clc() {
    clearcarry();
}

void absx() { //absolute,X
    uint16_t startpage;
    ea = ((uint16_t)read6502(pc) | ((uint16_t)read6502(pc+1) << 8));
    startpage = ea & 0xFF00;
    ea += (uint16_t)x;

    pc += 2;
}

void absy() { //absolute,Y
    uint16_t startpage;
    ea = ((uint16_t)read6502(pc) | ((uint16_t)read6502(pc+1) << 8));
    startpage = ea & 0xFF00;
    ea += (uint16_t)y;

    pc += 2;
}

void jmp() {
    pc = ea;
}

void jsr() {
    push16(pc - 1);
    pc = ea;
}

void op_and() {
    value = getvalue();
    result = (uint16_t)a & value;

    zerocalc(result);
    signcalc(result);

    saveaccum(result);
}

void op_bit() {
    value = getvalue();
    result = (uint16_t)a & value;

    zerocalc(result);
    cpustatus = (cpustatus & 0x3F) | (uint8_t)(value & 0xC0);
}

void rol() {
    value = getvalue();
    result = (value << 1) | (cpustatus & FLAG_CARRY);

    carrycalc(result);
    zerocalc(result);
    signcalc(result);

    putvalue(result);
}

void ror() {
    value = getvalue();
    result = (value >> 1) | ((cpustatus & FLAG_CARRY) << 7);

    if (value & 1) setcarry();
        else clearcarry();
    zerocalc(result);
    signcalc(result);

    putvalue(result);
}

void plp() {
    cpustatus = pull8() | FLAG_CONSTANT;
}

void eor() {
    value = getvalue();
    result = (uint16_t)a ^ value;

    zerocalc(result);
    signcalc(result);

    saveaccum(result);
}

void bmi() {
    if ((cpustatus & FLAG_SIGN) == FLAG_SIGN) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void rti() {
    cpustatus = pull8();
    value = pull16();
    pc = value;
}

void rts() {
    value = pull16();
    pc = value + 1;
}

void sec() {
    setcarry();
}

void sed() {
    setdecimal();
}

void sei() {
    setinterrupt();
}

void sta() {
    putvalue(a);
}

void lsr() {
    value = getvalue();
    result = value >> 1;

    if (value & 1) setcarry();
        else clearcarry();
    zerocalc(result);
    signcalc(result);

    putvalue(result);
}

void pha() {
    push8(a);
}

void bvc() {
    if ((cpustatus & FLAG_OVERFLOW) == 0) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void cli() {
    clearinterrupt();
}

void adc() {
    value = getvalue();
    result = (uint16_t)a + value + (uint16_t)(cpustatus & FLAG_CARRY);

    carrycalc(result);
    zerocalc(result);
    overflowcalc(result, a, value);
    signcalc(result);

    #ifndef NES_CPU
    if (cpustatus & FLAG_DECIMAL) {
        clearcarry();

        if ((a & 0x0F) > 0x09) {
            a += 0x06;
        }
        if ((a & 0xF0) > 0x90) {
            a += 0x60;
            setcarry();
        }

        clockticks6502++;
    }
    #endif

    saveaccum(result);
}

void pla() {
    a = pull8();

    zerocalc(a);
    signcalc(a);
}

void ind() { //indirect
    uint16_t eahelp, eahelp2;
    eahelp = (uint16_t)read6502(pc) | (uint16_t)((uint16_t)read6502(pc+1) << 8);
    eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //replicate 6502 page-boundary wraparound bug
    ea = (uint16_t)read6502(eahelp) | ((uint16_t)read6502(eahelp2) << 8);
    pc += 2;
}

void bvs() {
    if ((cpustatus & FLAG_OVERFLOW) == FLAG_OVERFLOW) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void stx() {
    putvalue(x);
}

void sty() {
    putvalue(y);
}

void dey() {
    y--;

    zerocalc(y);
    signcalc(y);
}

void txa() {
    a = x;

    zerocalc(a);
    signcalc(a);
}

void bcc() {
    if ((cpustatus & FLAG_CARRY) == 0) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void zpy() { //zero-page,Y
    ea = ((uint16_t)read6502((uint16_t)pc++) + (uint16_t)y) & 0xFF; //zero-page wraparound
}

void tya() {
    a = y;

    zerocalc(a);
    signcalc(a);
}

void txs() {
    sp = x;
}

void ldy() {
    value = getvalue();
    y = (uint8_t)(value & 0x00FF);

    zerocalc(y);
    signcalc(y);
}

void lda() {
    value = getvalue();
    a = (uint8_t)(value & 0x00FF);

    zerocalc(a);
    signcalc(a);
}

void ldx() {
    value = getvalue();
    x = (uint8_t)(value & 0x00FF);

    zerocalc(x);
    signcalc(x);
}

void tay() {
    y = a;

    zerocalc(y);
    signcalc(y);
}

void tax() {
    x = a;

    zerocalc(x);
    signcalc(x);
}

void bcs() {
    if ((cpustatus & FLAG_CARRY) == FLAG_CARRY) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void clv() {
    clearoverflow();
}

void tsx() {
    x = sp;

    zerocalc(x);
    signcalc(x);
}

void cpy() {
    value = getvalue();
    result = (uint16_t)y - value;

    if (y >= (uint8_t)(value & 0x00FF)) setcarry();
        else clearcarry();
    if (y == (uint8_t)(value & 0x00FF)) setzero();
        else clearzero();
    signcalc(result);
}

void cmp() {
    value = getvalue();
    result = (uint16_t)a - value;

    if (a >= (uint8_t)(value & 0x00FF)) setcarry();
        else clearcarry();
    if (a == (uint8_t)(value & 0x00FF)) setzero();
        else clearzero();
    signcalc(result);
}

void dec() {
    value = getvalue();
    result = value - 1;

    zerocalc(result);
    signcalc(result);

    putvalue(result);
}
void iny() {
    y++;

    zerocalc(y);
    signcalc(y);
}

void dex() {
    x--;

    zerocalc(x);
    signcalc(x);
}
void bne() {
    if ((cpustatus & FLAG_ZERO) == 0) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}
void cld() {
    cleardecimal();
}
void cpx() {
    value = getvalue();
    result = (uint16_t)x - value;

    if (x >= (uint8_t)(value & 0x00FF)) setcarry();
        else clearcarry();
    if (x == (uint8_t)(value & 0x00FF)) setzero();
        else clearzero();
    signcalc(result);
}

void sbc() {
    value = getvalue() ^ 0x00FF;
    result = (uint16_t)a + value + (uint16_t)(cpustatus & FLAG_CARRY);

    carrycalc(result);
    zerocalc(result);
    overflowcalc(result, a, value);
    signcalc(result);

    #ifndef NES_CPU
    if (cpustatus & FLAG_DECIMAL) {
        clearcarry();

        a -= 0x66;
        if ((a & 0x0F) > 0x09) {
            a += 0x06;
        }
        if ((a & 0xF0) > 0x90) {
            a += 0x60;
            setcarry();
        }

        clockticks6502++;
    }
    #endif

    saveaccum(result);
}

void inc() {
    value = getvalue();
    result = value + 1;

    zerocalc(result);
    signcalc(result);

    putvalue(result);
}
void inx() {
    x++;

    zerocalc(x);
    signcalc(x);
}
void beq() {
    if ((cpustatus & FLAG_ZERO) == FLAG_ZERO) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

//undocumented instructions
#ifdef UNDOCUMENTED
    void lax() {
        lda();
        ldx();
    }

    void sax() {
        sta();
        stx();
        putvalue(a & x);
    }

    void dcp() {
        dec();
        cmp();
    }

    void isb() {
        inc();
        sbc();
    }

    void slo() {
        asl();
        ora();
    }

    void rla() {
        rol();
        op_and();
    }

    void sre() {
        lsr();
        eor();
    }

    void rra() {
        ror();
        adc();
    }
#else
    #define lax nop
    #define sax nop
    #define dcp nop
    #define isb nop
    #define slo nop
    #define rla nop
    #define sre nop
    #define rra nop
#endif

//a few general functions used by various other functions
void push16(uint16_t pushval) {
    write6502(BASE_STACK + sp, (pushval >> 8) & 0xFF);
    write6502(BASE_STACK + ((sp - 1) & 0xFF), pushval & 0xFF);
    sp -= 2;
}

void push8(uint8_t pushval) {
    write6502(BASE_STACK + sp--, pushval);
}

uint16_t pull16() {
    uint16_t temp16;
    temp16 = read6502(BASE_STACK + ((sp + 1) & 0xFF)) | ((uint16_t)read6502(BASE_STACK + ((sp + 2) & 0xFF)) << 8);
    sp += 2;
    return(temp16);
}

uint8_t pull8() {
    return (read6502(BASE_STACK + ++sp));
}

/////////////usart


/*
void USART1_IRQHandler(void)
{
    if ((USART1->SR & USART_FLAG_RXNE) != (u16)RESET)
	{
    		curkey = USART_ReceiveData(USART1) & 0x7F;
	}
}
*/
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure USART1 Tx (PA.09) as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure USART1 Rx (PA.10) as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name  : USART_Configuration
* Description    : Configures the USART1.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART_Configuration(void)
{
  USART_InitTypeDef USART_InitStructure;

/* USART1 configuration ------------------------------------------------------*/
  /* USART1 configured as follow:
        - BaudRate = 115200 baud
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
        - USART Clock disabled
        - USART CPOL: Clock is active low
        - USART CPHA: Data is captured on the middle
        - USART LastBit: The clock pulse of the last data bit is not output to
                         the SCLK pin
  */
  USART_InitStructure.USART_BaudRate = 250000;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART1, &USART_InitStructure);

  /* Enable USART1 */
  USART_Cmd(USART1, ENABLE);
}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : UARTSend
* Description    : Send a string to the UART.
* Input          : - pucBuffer: buffers to be printed.
*                : - ulCount  : buffer's length
* Output         : None
* Return         : None
*******************************************************************************/
void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount)
{
    //
    // Loop while there are more characters to send.
    //
    while(ulCount--)
    {
        USART_SendData(USART1, *pucBuffer++);// Last Version USART_SendData(USART1,(uint16_t) *pucBuffer++);
        /* Loop until the end of transmission */
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        {
        }
    }
}

void SetSysClockTo72(void)
{
    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig( RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if (HSEStartUpStatus == SUCCESS)
    {
        /* Enable Prefetch Buffer */
        FLASH_PrefetchBufferCmd( FLASH_PrefetchBuffer_Enable);

        /* Flash 2 wait state */
        FLASH_SetLatency( FLASH_Latency_2);

        /* HCLK = SYSCLK */
        RCC_HCLKConfig( RCC_SYSCLK_Div1);

        /* PCLK2 = HCLK */
        RCC_PCLK2Config( RCC_HCLK_Div1);

        /* PCLK1 = HCLK/2 */
        RCC_PCLK1Config( RCC_HCLK_Div2);

        /* PLLCLK = 8MHz * 9 = 72 MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //Most STM32F103 are stable @ 8MHz x 12 = 96Mhz

        /* Enable PLL */
        RCC_PLLCmd( ENABLE);

        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
    else
    { /* If HSE fails to start-up, the application will have wrong clock configuration.
     User can add here some code to deal with this error */

        /* Go to infinite loop */
        while (1)
        {
        }
    }
}

void Delay(__IO uint32_t nCount)
{
  /* Decrement nCount value */
  while (nCount != 0)
  {
    nCount--;
  }
}