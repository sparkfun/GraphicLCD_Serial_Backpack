#ifndef __t6963_h
#define __t6963_h

//160x128 definitions
#define WR 		0	//PC0
#define RD 		1	//PC1
#define CE 		2	//PC2
#define CD 		3	//PC3
#define HALT 	4	//PC4
#define RST 	5	//PC5

void t6963WriteData(uint8_t data);
uint8_t t6963ReadData(void);
void t6963WriteCmd(uint8_t command);
uint8_t t6963ReadStatus(void);

#endif
