#ifndef __ks0107b_h
#define __ks0107b_h

#define EN 			0	//PC0
#define RS 			1	//PC1, D_I?
#define R_W 		2	//PC2
#define RESET 		3	//PC3
#define CS1 		4	//PC4
#define CS2 		5	//PC5

void 		ks0107bWriteData(uint8_t data);
uint8_t 	ks0107bReadData(void);
void 		ks0107bWriteCmd(uint8_t command);
uint8_t 	ks0107bReadStatus(void);

#endif
