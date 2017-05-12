extern void initAndResetChip();
extern void resetChip();
extern void step();
extern void chipStatus();
extern unsigned short readPC();
extern unsigned char readA();
extern unsigned char readX();
extern unsigned char readY();
extern unsigned char readSP();
extern unsigned char readP();
extern unsigned int readRW();
extern unsigned short readAddressBus();
extern void writeDataBus(unsigned char);
extern unsigned char readDataBus();
extern unsigned char readIR();
extern void setIRQ(int level);
extern void setNMI(int level);

extern unsigned char memory[65536];
//extern unsigned int cycle;
extern unsigned int transistors;

#ifdef BROKEN_TRANSISTORS
extern unsigned int broken_transistor;
#endif
