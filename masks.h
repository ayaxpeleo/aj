#ifndef _MASKS_H_
#define _MASKS_H_


/*
En esta mascara:
El primer numero (6) es el numero de tipo de piezas:
    1. PEON_WHITE       4. alfil
    2. torre      5. DAMA
    3. caballo    6. rey
El segundo (64), es la posicion origen de la pieza 1
El tercero (64), es la posicion final de la pieza 2
El valor guardado es los bits 
*/
extern uint64_t peon_white_advance[64][64];
extern uint64_t peon_white_takes[64][64];
extern uint64_t peon_white_takes_en_passant[64][64];

extern uint64_t peon_black_advance[64][64];
extern uint64_t peon_black_takes[64][64];
extern uint64_t peon_black_takes_en_passant[64][64];

extern uint64_t torre_exclusive[64][64];
extern uint64_t torre_inclusive[64][64];

extern uint64_t alfil_exclusive[64][64];
extern uint64_t alfil_inclusive[64][64];

extern uint64_t dama_exclusive[64][64];
extern uint64_t dama_inclusive[64][64];

extern uint64_t rey_exclusive[64][64];
extern uint64_t rey_inclusive[64][64];

extern uint64_t piezas[64];
extern uint64_t piezas2d[8][8];
extern struct POS *movimientos[7][64][29];
extern char piezas_id[16];
extern int piezas_valor[16];

extern uint64_t crc[100000][4];
extern short turno_global;


#define PEON_WHITE 0
#define PEON_BLACK 1
#define TORRE 2
#define CABALLO 3
#define ALFIL 4
#define DAMA 5
#define REY 6

#define FILL 0xffffffffffffffff

#endif //_MASKS_H_