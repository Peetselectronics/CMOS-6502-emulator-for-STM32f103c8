//functions
void push16(uint16_t pushval);
void push8(uint8_t pushval);
uint16_t pull16();
uint8_t pull8();

//cpu
uint8_t read6502(uint16_t address);

//serial
uint8_t getkey();
void clearkey();

//instructies
void imp();
void brk();
void indx();
void ora();
static uint16_t getvalue();
void zp();
void asl();
void php();
void imm();
void acc();
void abso();
void rel();
void bpl();
void indy();
void zpx();
void clc();
void absx();
void absy();
void jmp();
void jsr();
void op_and();
void op_bit();
void rol();
void ror();
void plp();
void eor();
void bmi();
void rti();
void rts();
void sec();
void sed();
void sei();
void sta();
void lsr();
void pha();
void bvc();
void cli();
void adc();
void pla();
void ind();
void bvs();
void stx();
void sty();
void dey();
void txa();
void bcc();
void zpy();
void tya();
void txs();
void ldy();
void lda();
void ldx();
void tay();
void tax();
void bcs();
void clv();
void tsx();
void cpy();
void cmp();
void dec();
void iny();
void dex();
void bne();
void cld();
void cpx();
void sbc();
void inc();
void inx();
void beq();

/* Private function prototypes -----------------------------------------------*/
void NVIC_Configuration(void);
void GPIO_Configuration(void);
void USART_Configuration(void);
void USART1_IRQHandler(void);
void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount);
void SetSysClockTo72(void);
