#ifndef _BIBLIOTECA_H_
#define _BIBLIOTECA_H_

//9.7.123.139

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <iostream>  // I/O 

#include <bitset>

#include <windows.h>

using namespace std;


#define LADO_DEL_REY 0
#define LADO_DE_LA_dama 1


#define JAQUEB  10000000
#define JAQUEN -10000000
#define MININF -10000000
#define MAXINF  10000000
#define MILLON  1000000

#define AB 1

#define HUMANO 0
#define MAQUINA 1

extern unsigned long int contador;
extern unsigned long int contador_nivel[20];
extern int blancas, negras;
extern int profundidad_objetivo;
extern int ab;
extern clock_t temporizador_start[10];
extern clock_t temporizador_stop[10];
extern clock_t temporizador_value[10];
extern double PCFreq;
extern __int64 _CounterStart;
extern __int64 _CounterStop;
extern __int64 _CounterValue;

extern uint64_t piezas2d[8][8];
extern uint64_t piezas[64];


extern char piezas_id[16];
extern int piezas_valor[16];

//extern uint64_t crc[5][1000000][

struct POS
{
    public:
    unsigned char x;//1
    unsigned char y;//1
};
//config
//16 bits
//6: b0-b5 = posicion
//1: b6 = valida
//3: b7-b9 = pieza_id
//1: b10 = color

struct PIEZA//24B//12//8
{
    public:
    short int config;//2
};


class CEstadoDelJuego//920B//856//848//832//824//816//432//304//
{
    public:
    uint64_t tablero[2];//16
    uint64_t rey[2];//16
	struct POS alpaso;//2
    struct POS umov[2];//4
    short value;//2
    char sig;//'w'=blanco, 'b'=negro//1
    char misc;//
    char coronacion;
    /*------B4-----B3-----B2-----B1--------B0--------*/
    /*    alpaso   q      k      Q         K         */
    
    //lista de piezas
    //
    union
    {
        struct PIEZA lista[32];
        uint64_t l[8];
    };
    
    void imprimir_tablero(void) ;
    unsigned int es_movimiento_valido(unsigned int m, char p);
    unsigned int es_movimiento_valido2(unsigned int m, char p, int king_capture_allowed, int query);
    unsigned int es_ataque_valido(unsigned int m, char p, int king_capture_allowed, int query);
    char obtener_cuadro(int x, int y);
    int aplicar_movimiento2(int npieza, int &m, struct POS &mov1, struct POS &mov2, int x, int query);
    void procesar(unsigned long &game);
    int evaluar();
    void inicializar(int enable);
    int copiar(CEstadoDelJuego *n);
    void imprimir_piezas( void );
    int cuadro_bajo_ataque(int x, int y, char atacante);
    int examinar_entrada(char *str);
};

extern CEstadoDelJuego tc[10];

int comparar(CEstadoDelJuego &j1, CEstadoDelJuego &j2);

int inicializar_desde_fen(CEstadoDelJuego *t, char *fen);
    int aplicar_movimiento(CEstadoDelJuego &ts, struct POS mov1, struct POS mov2, short query);
void print64(uint64_t n);
#define FLUSH {int c=0;while((c=getchar())!='\n' && c != EOF );}
void ayuda(CEstadoDelJuego *ts);
unsigned int cadena_a_numero(char *cadena);
unsigned int convertir_texto_de_entrada_a_movimiento(char *cadena, struct POS *movimiento1, struct POS *movimiento2);
void _imprimir_tablero(char t[8][8]);
void copiar_matriz(char m1[8][8], char m2[8][8]);
unsigned int esta_en_jaque(char t2[8][8], struct POS mov1, struct POS mov2, char color);
void inicializar_mascaras(void);
long recursion(int profundidad, int profundidad_meta, CEstadoDelJuego *ts, CEstadoDelJuego *elmejor, unsigned long &game, long alpha, long beta, int db);
unsigned int convertir_movimiento(struct POS m1, struct POS m2);
void inicializar_pieza(struct PIEZA *p, unsigned char y, unsigned char x, int VALOR, char ID, char PIEZA_ID, char VALIDA);

const char *tami(unsigned long int n, int size);
const char *tamf(float n, int size);

long int existe_crc(struct PIEZA a[32]);

int config_valor(short c);
char config_id(short c);

inline float gettimer(int timer_ID);
inline void stoptimer(int timer_ID);
inline void starttimer(int timer_ID);
inline void resettimer(int timer_ID);

inline double GetCounter( void );
inline void StartCounter( void );
inline void StopCounter( void );
inline void ResetCounter( void );
#endif //_BIBLIOTECA_H_