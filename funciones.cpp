#include "biblioteca.h"
#include "masks.h"

unsigned long int contador;
unsigned long int contador_nivel[20];;
int blancas, negras;
int profundidad_objetivo;
int ab;

uint64_t peon_white_advance[64][64];
uint64_t peon_white_takes[64][64];
uint64_t peon_white_takes_en_passant[64][64];

uint64_t peon_black_advance[64][64];
uint64_t peon_black_takes[64][64];
uint64_t peon_black_takes_en_passant[64][64];

uint64_t torre_exclusive[64][64];
uint64_t torre_inclusive[64][64];

uint64_t alfil_exclusive[64][64];
uint64_t alfil_inclusive[64][64];

uint64_t dama_exclusive[64][64];
uint64_t dama_inclusive[64][64];

uint64_t rey_exclusive[64][64];
uint64_t rey_inclusive[64][64];

uint64_t piezas[64];
uint64_t piezas2d[8][8];
struct POS *movimientos[7][64][29];

clock_t temporizador_start[10];
clock_t temporizador_stop[10];
clock_t temporizador_value[10];
uint64_t crc[100000][4];
char piezas_id[16];
int piezas_valor[16];

CEstadoDelJuego tc[10];

double PCFreq = 0.0;
//__int64 CounterStart = 0;
__int64 _CounterStart;
__int64 _CounterStop;
__int64 _CounterValue;

short turno_global;

void inicializar_pieza(struct PIEZA *p, unsigned char y, unsigned char x, int VALOR, char ID, char PIEZA_ID, char VALIDA)
{
//config
//16 bits
//6: b0-b5 = posicion
//1: b6 = valida
//3: b7-b9 = pieza_id
//1: b10 = color

    short i=1;
    //printf("\ny: %d, x: %d, valor: %d, ID: %c, pieza_id: %d, valida: %d", y, x, VALOR, ID, PIEZA_ID, VALIDA);
    if (ID>='A' && ID<='Z')
        i=0;
        
    i=(i<<3);
    i|=PIEZA_ID;
    
    i=(i<<1);
    if(VALIDA)
        i|=1;
    
    i=(i<<6);
    i|=(x+y*8);
    
    p->config=i;
}

int CEstadoDelJuego::copiar(CEstadoDelJuego *n)
{
    n->sig=sig;
    n->alpaso=alpaso;
    n->tablero[0]=tablero[0];
    n->tablero[1]=tablero[1];
    n->rey[0]=rey[0];
    n->rey[1]=rey[1];
    n->umov[0]=umov[0];
    n->umov[1]=umov[1];
	n->value=value;
    n->misc=misc;
    n->coronacion=coronacion;
    //copiar aqui lista de piezas
    //for(int i=0;i<8;i++)
    //    n->l[i]=l[i];

    n->l[0]=l[0];
    n->l[1]=l[1];
    n->l[2]=l[2];
    n->l[3]=l[3];
    n->l[4]=l[4];
    n->l[5]=l[5];
    n->l[6]=l[6];
    n->l[7]=l[7];
    
    //1561//981
    return 0;
}

void CEstadoDelJuego::imprimir_piezas( void )
{

//config
//16 bits
//6: b0-b5 = posicion
//1: b6 = valida
//3: b7-b9 = pieza_id
//1: b10 = color
    int i;
    for(i=0;i<32;i++)
    {
        short c = lista[i].config;
        printf("\nPieza %d: 0x%.4x.... posicion: %d, valida: %d, pieza_id: %d, color: %d, coord: %c%d", i, c, c&0x003f, (c&0x0040)>>6, (c&0x0380)>>7, (c&0x0400)>>10, (c&0x003f)%8+'a', (c&0x003f)/8+1);
    }
    printf("\n");
}


char CEstadoDelJuego::obtener_cuadro(int x, int y)
{
    short int m;
    m = x+y*8;
    char ret='-';
    int i;
    for (i=0;i<32;i++)
    {
        if((lista[i].config&0x003f)==m && (lista[i].config&0x0040))
        {
            ret=config_id(lista[i].config);
            break;
        }
    }
    return ret;
}

char config_id(short c)
{
    char ret='-';
    if(((c&0x0380)>>7)==PEON_WHITE || ((c&0x0380)>>7)==PEON_BLACK)
        ret='P';
    else if(((c&0x0380)>>7)==TORRE)
        ret='R';
    else if(((c&0x0380)>>7)==CABALLO)
        ret='N';
    else if(((c&0x0380)>>7)==ALFIL)
        ret='B';
    else if(((c&0x0380)>>7)==DAMA)
        ret='Q';
    else if(((c&0x0380)>>7)==REY)
        ret='K';
    if((c&0x0400) && ret!='-')
        ret|=0x20;
    return ret;
}

int config_valor(short c)
{
    int ret=0;
    if(((c&0x0380)>>7)==PEON_WHITE || ((c&0x0380)>>7)==PEON_BLACK)
        ret=100;
    else if(((c&0x0380)>>7)==TORRE)
        ret=500;
    else if(((c&0x0380)>>7)==CABALLO)
        ret=300;
    else if(((c&0x0380)>>7)==ALFIL)
        ret=300;
    else if(((c&0x0380)>>7)==DAMA)
        ret=900;
    else if(((c&0x0380)>>7)==REY)
        ret=0;
    if(c&0x0400)
        ret*=-1;
    return ret;
}

int CEstadoDelJuego::cuadro_bajo_ataque(int x, int y, char atacante)
{
    
    int ret=0;
    int debug=0;
    //debug=100;
    if (debug==100)
        printf("\nAhora revisando coordenadas x: %d, y: %d, (%c%d), para ataques de \'%s\'", x, y, 'a'+x, y+1, (atacante=='n')?"negras":"blancas");
    short m, inicio, fin, i, j;
    m = x + y*8;
    if (atacante=='b')
    {
        inicio=16;
        fin=32;
    }
    else
    {
        inicio=0;
        fin=16;
    }
    
    if (debug==100)
        printf("\ninicio: %d, fin: %d", inicio, fin);
    //iterar las piezas atacantes
    
    for(i=inicio;i<fin && ret==0;i++)
    {
        //solo iterar piezas validas
        if (lista[i].config&0x0040)
        {
            PIEZA *p=&lista[i];
            if (debug==100)
                printf("\nPieza %d: 0x%.4x.... posicion: %d, valida: %d, pieza_id: %d, color: %d, coord: %c%d", i, p->config, p->config&0x003f, (p->config&0x0040)>>6, (p->config&0x0380)>>7, (p->config&0x0400)>>10, (p->config&0x003f)%8+'a', (p->config&0x003f)/8+1);
            //iterar movimientos de las piezas
            for(j=0;movimientos[(0x0380&(p->config))>>7][p->config&0x003f][j]!=NULL && ret==0;j++)
            {            
                //se revisa si el destino esta en la lista
                if(movimientos[(0x0380&(p->config))>>7][p->config&0x003f][j]->x==x && movimientos[(0x0380&(p->config))>>7][p->config&0x003f][j]->y==y)
                {
                    //si esta, se checa si es valido para este tablero
                    if (debug==100)
                        printf(" . ");
                    short m2 = m|((p->config&0x003f)<<8);
                    //validar que movimiento sea valido
                    if(es_ataque_valido(m2, config_id(p->config), 1, 0))
                    {
                        //el movimiento es valido, por lo tanto, el cuadro esta siendo atacado   
                        if (debug==100)
                            printf("\nMovimiento valido!");
                        ret = 1;
                        break;
                    }
                }
            }
        }
    }
    //printf("    Jaques: %c%c", (cuadro_bajo_ataque((lista[4].config&0x003f)%8, (lista[4].config&0x003f)/8, 'b'))==1?'K':'-', (cuadro_bajo_ataque((lista[28].config&0x003f)%8, (lista[28].config&0x003f)/8, 'w'))==1?'k':'-');
    if (debug==100)
        printf("\nMovimiento NO valido!");    
    return ret;
}

int aplicar_movimiento(CEstadoDelJuego &ts, struct POS mov1, struct POS mov2, short query)
{
    
    //convertir movimiento a movimiento de 16 bits
    int debug=0;
    //debug=100;
    int balpaso;
    
    
    char pza=ts.obtener_cuadro(mov1.x, mov1.y);
    if (debug==100)
        printf("\nLa pieza que se intenta mover es un \'%c\' (%d)", pza, pza);
    if (debug==100)
        printf("\nMovimiento a aplicar es mov1.x: %d, mov1.y: %d, mov2.x: %d, mov2.y: %d.", mov1.x, mov1.y, mov2.x, mov2.y);
    int m;
    m = (mov1.x+mov1.y*8)<<8;
    m |= (mov2.x+mov2.y*8);
    if (debug==100)
        printf("\nMovimiento convertido es: 0x%.4x.", m);
    if(ts.es_movimiento_valido(m, pza)==0)//si no es valido, 
    {   //regresar 1
        printf("\nEL MOVIMIENTO NO ES VALIDO!");
        return 1;
    }
    else //si es valido, 
    {
        //hacer tablero dummy
        CEstadoDelJuego dummy;
        ts.copiar(&dummy);
        //aplicar movimiento candidato
        
        //guardar temporalmente (local space) el estado de la bandera de peon al paso
        if(dummy.misc&0x10)
            balpaso=1;
        else
            balpaso=0;
        //deshabilitar bandera de peon al paso 
        dummy.misc&=0xef;
        //si la torre se esta moviendo, eliminar el enroque de ese lado
        if(pza=='R')
        {
            if(debug==100)
                printf("\nmisc es: 0x%x", dummy.misc);
            if(mov1.x==0 && mov1.y==0)//es enroque del lado de la reina
                dummy.misc&=0xfd;
            else if(mov1.x==7 && mov1.y==0)//es enroque del lado del rey
                dummy.misc&=0xfe;
            if(debug==100)
                printf("\n... y ahora misc es: 0x%x", dummy.misc);
        }
        else if(pza=='r')
        {
            if(debug==100)
                printf("\nmisc es: 0x%x", dummy.misc);
            if(mov1.x==0 && mov1.y==7)//es enroque del lado de la reina
                dummy.misc&=0xf7;
            else if(mov1.x==7 && mov1.y==7)//es enroque del lado del rey
                dummy.misc&=0xfb;
            if(debug==100)
                printf("\n... y ahora misc es: 0x%x", dummy.misc);
        }
        if (debug==100)
            printf("\nMovimiento valido!");
        int n, x1, y1;
        struct PIEZA *p1, *p2;
        p1=NULL;
        p2=NULL;
        //registrar nueva posicion en mapa de bits
        for(n=0;n<32;n++)
		{
            if((dummy.lista[n].config&0x003f)==(m>>8) && (dummy.lista[n].config&0x0040))
                p1=&(dummy.lista[n]);
            if((dummy.lista[n].config&0x003f)==(m&0x00ff) && (dummy.lista[n].config&0x0040))
                p2=&(dummy.lista[n]);
        }
        y1=((p1->config&0x003f)/8);
        x1=((p1->config&0x003f)%8);
        if(debug==100)
            printf("\nLa posicion inicio es x:%d, y:%d", x1, y1);
        if(dummy.sig=='w')
        {
            dummy.tablero[0]&=(~piezas[x1+y1*8]);
        }
        else 
        {
            dummy.tablero[1]&=(~piezas[x1+y1*8]);
        }
        
        if(debug==100)
            printf("\nPieza modificada de %c%d ", x1+'a', y1+1);
        p1->config&=0xffc0;
        p1->config|=(mov2.x+mov2.y*8);
        y1=((p1->config&0x003f)/8);
        x1=((p1->config&0x003f)%8);
        if(debug==100)
            printf("a %c%d ", x1+'a', y1+1);
            
        x1=mov2.x;
        y1=mov2.y;
        if(debug==100)
            printf("\nLa posicion aplicada es x:%d, y:%d", x1, y1);
        if(dummy.sig=='w')
        {
            dummy.tablero[0]|=(piezas[x1+y1*8]);
            dummy.tablero[1]&=(~piezas[x1+y1*8]);
        }
        else 
        {
            dummy.tablero[1]|=(piezas[x1+y1*8]);
            dummy.tablero[0]&=(~piezas[x1+y1*8]);
        }
		//actualizar bitmaps
        (dummy.rey[1])=(piezas[dummy.lista[28].config&0x003f]);
        (dummy.rey[0])=(piezas[dummy.lista[4].config&0x003f]);
		//actualizar lista de piezas, y tal vez se deba borrar alguna
		if (p2!=NULL && ((p2->config)&0x0040))
        {
            //marcar pieza comida como 'no valida'
            p2->config&=0xffbf;
            //restar el valor de la pieza al valor del tablero
            dummy.value-=(config_valor(p2->config));
            if(mov2.x==7 && mov2.y==0)
                dummy.misc&=0xfe;
            else if(mov2.x==0 && mov2.y==0)
                dummy.misc&=0xfd;
            else if(mov2.x==7 && mov2.y==7)
                dummy.misc&=0xfb;
            else if(mov2.x==0 && mov2.y==7)
                dummy.misc&=0xf7;
        }
        //regresar 0
        
        if (debug==100)
            printf("\nRevisando situaciones especiales...");
        //es un enroque?
        if(pza=='K')
        {
            //debug=100;
            int lado_rey;
            int lado_reina;
            if (debug==100)
                printf("\nconfig(7)=0x%.4x, config(0)=0x%.4x", dummy.lista[7].config, dummy.lista[0].config);
            if((dummy.lista[7].config&0x003f)==7 && (dummy.lista[7].config&0x0040))
            {
                lado_rey=7;
                lado_reina=0;
                if (debug==100)
                    printf("\n1");
            }
            else if((dummy.lista[0].config&0x003f)==7 && (dummy.lista[0].config&0x0040))
            {
                lado_rey=0;
                lado_reina=7;
                if (debug==100)
                    printf("\n2");
            }
            else if((dummy.lista[7].config&0x003f)==0 && (dummy.lista[7].config&0x0040))
            {
                lado_rey=0;
                lado_reina=7;
                if (debug==100)
                    printf("\n1");
            }
            else if((dummy.lista[0].config&0x003f)==0 && (dummy.lista[0].config&0x0040))
            {
                lado_rey=7;
                lado_reina=0;
                if (debug==100)
                    printf("\n2");
            }
            if (debug==100)
                printf("\nRevisando enroques (blancas)...!");
            //deshabilitar enroques
            dummy.misc&=0xfc;
            if(m==0x0406)//del lado del rey?
            {
                if (debug==100)
                    printf("\nlado del rey ...!");
                //borrar la torre del lado del rey del mapa de bits
                dummy.tablero[0]&=(~(0x0000000000000080));
                //agregar la torre del lado del rey  del mapa de bits
                dummy.tablero[0]|=(0x0000000000000020);
                //cambiar la ubicacion de la torre
                if (debug==100)
                    printf("\nposicion es %c%d", ((dummy.lista[lado_rey].config&0x003f)%8)+'a', ((dummy.lista[lado_rey].config&0x003f)/8)+1);
                dummy.lista[lado_rey].config&=0xffc0;
                dummy.lista[lado_rey].config|=0x0005;
                if (debug==100)
                    printf("\nahora posicion es %c%d", ((dummy.lista[lado_rey].config&0x003f)%8)+'a', ((dummy.lista[lado_rey].config&0x003f)/8)+1);
            }
            else if(m==0x0402)//del lado de la reina?
            {
                if (debug==100)
                    printf("\nlado de la reina ...!");
                //borrar la torre del lado de la reina del mapa de bits
                dummy.tablero[0]&=(~(0x0000000000000001));
                //agregar la torre del lado del rey  del mapa de bits
                dummy.tablero[0]|=(0x0000000000000008);
                //cambiar la ubicacion de la torre
                dummy.lista[lado_reina].config&=0xffc0;
                dummy.lista[lado_reina].config|=0x0003;
            }
        }
        else if(pza=='k')
        {
            //debug=100;
            if (debug==100)
                printf("\nconfig(7)=0x%.4x, config(0)=0x%.4x", dummy.lista[7].config, dummy.lista[0].config);
            if (debug==100)
                printf("\nRevisando enroques (negras)...!");
            int lado_rey;
            int lado_reina;
            if((dummy.lista[31].config&0x003f)==63 && (dummy.lista[31].config&0x0040))
            {
                lado_rey=31;
                lado_reina=24;
                if (debug==100)
                    printf("\n1");
            }
            else if((dummy.lista[24].config&0x003f)==63 && (dummy.lista[24].config&0x0040))
            {
                lado_rey=24;
                lado_reina=31;
                if (debug==100)
                    printf("\n2");
            }
            else if((dummy.lista[31].config&0x003f)==56 && (dummy.lista[31].config&0x0040))
            {
                lado_rey=24;
                lado_reina=31;
                if (debug==100)
                    printf("\n1");
            }
            else if((dummy.lista[24].config&0x003f)==56 && (dummy.lista[24].config&0x0040))
            {
                lado_rey=31;
                lado_reina=24;
                if (debug==100)
                    printf("\n2");
            }
            //deshabilitar enroques
            dummy.misc&=0xf3;
            if(m==0x3c3e)//del lado del rey?
            {
                //borrar la torre del lado del rey del mapa de bits
                dummy.tablero[1]&=(~(0x8000000000000000));
                //agregar la torre del lado del rey  del mapa de bits
                dummy.tablero[1]|=(0x2000000000000000);
                //cambiar la ubicacion de la torre
                dummy.lista[lado_rey].config&=0xffc0;
                dummy.lista[lado_rey].config|=61;
            }
            else if(m==0x3c3a)//del lado de la reina?
            {
                //borrar la torre del lado de la reina del mapa de bits
                dummy.tablero[1]&=(~(0x0100000000000000));
                //agregar la torre del lado del rey  del mapa de bits
                dummy.tablero[1]|=(0x0800000000000000);
                //cambiar la ubicacion de la torre
                dummy.lista[lado_reina].config&=0xffc0;
                dummy.lista[lado_reina].config|=59;
            }
        }
        else if (pza=='P')
        {
            if (debug==100)
                printf("\nRevisando comer al paso de blancas...! (balpaso:%d, alpaso: %c %d)", balpaso, dummy.alpaso.x+'a', dummy.alpaso.y+1);

            if(mov2.x==mov1.x && (mov2.y==(mov1.y+2)))//checar si el peon se mueve al paso
            {
                if (debug==100)
                    printf("\nEl peon se movio al paso!");
                //peon al paso
                //activar bandera
                dummy.misc|=0x10;
                //grabar posicion de al paso
                dummy.alpaso.x=mov2.x;
                dummy.alpaso.y=mov2.y-1;
            }
            else if(balpaso==1 && dummy.alpaso.x==mov2.x && dummy.alpaso.y==mov2.y)
            {
                if (debug==100)
                    printf("\nEl peon come al paso!");
                //esta comiendo al paso
                //eliminar el peon que se come
                for(n=16;n<24;n++)
                {
                    if((dummy.lista[n].config&0x003f)==((m&0x00ff)-8) && (dummy.lista[n].config&0x0040) && config_id(dummy.lista[n].config)=='P')
                    {
                        p1=&(dummy.lista[n]);
                        if (debug==100)
                            printf("\nse elimina peon con config=0x%x", p1->config);
                        p1->config&=0xffbf;
                        //borrar del tablero de bits
                        dummy.tablero[1]&=(~(piezas[p1->config&0x003f]));
                        break;
                    }
                }
                if (debug==100)
                    printf("\nNo se encontro peon para borrar... (error?)");
            }
            if(mov2.y==7)
            {
                while(query==1)
                {
                    //preguntar que pieza se quiere
                    printf("\nEscoger pieza:\n1. Torre\n2. Caballo\n3. Alfil\n4. Reina\n# de Pieza?: ");
                    char eleccion[81];
                    fgets(eleccion, sizeof(eleccion), stdin);
                    if(strcmp(eleccion, "1\n")==0)
                    {
                        query=2;
                    }
                    else if(strcmp(eleccion, "2\n")==0)
                    {
                        query=3;
                    }
                    else if(strcmp(eleccion, "3\n")==0)
                    {
                        query=4;
                    }
                    else if(strcmp(eleccion, "4\n")==0)
                    {
                        query=5;
                    }
                    else
                    {
                        printf("\nOpcion incorrecta. Intente de nuevo. ");
                    }
                }
                //Coronacion del peon
                //se hace reina por default
                //printf("\nAqui se deberia coronar un peon!!");
                
                p1->config&=0xfc7f;     //se borra id
                p1->config|=(query<<7);     //se corona como reina
                dummy.value+=(config_valor(p1->config)-100);
            }
        }
        else if (pza=='p')
        {
            if (debug==100)
                printf("\nRevisando comer al paso de negras...! (balpaso:%d, alpaso: %c %d)", balpaso, dummy.alpaso.x+'a', dummy.alpaso.y+1);
                
            if(mov2.x==mov1.x && (mov2.y==(mov1.y-2)))//checar si el peon se mueve al paso
            {
                if (debug==100)
                    printf("\nEl peon se movio al paso!");
                //peon al paso
                //activar bandera
                dummy.misc|=0x10;
                //grabar posicion de al paso
                dummy.alpaso.x=mov2.x;
                dummy.alpaso.y=mov2.y+1;
            }
            else if(balpaso==1 && dummy.alpaso.x==mov2.x && dummy.alpaso.y==mov2.y)
            {
                if (debug==100)
                    printf("\nEl peon come al paso!");
                //esta comiendo al paso
                //eliminar el peon que se come
                for(n=8;n<16;n++)
                {
                    //short c=lista[n].config;
                    //printf("\nPieza %d: 0x%.4x.... posicion: %d, valida: %d, pieza_id: %d, color: %d, coord: %c%d", n, c, c&0x003f, (c&0x0040)>>6, (c&0x0380)>>7, (c&0x0400)>>10, (c&0x3f)%8+'a', (c&0x3f)/8+1);
                    if((dummy.lista[n].config&0x003f)==((m&0x00ff)+8) && (dummy.lista[n].config&0x0040) && config_id(dummy.lista[n].config)=='p')
                    {
                        p1=&(dummy.lista[n]);
                        if (debug==100)
                            printf("\nse elimina peon con config=0x%x", p1->config);
                        p1->config&=0xffbf;
                        //borrar del tablero de bits
                        dummy.tablero[0]&=(~(piezas[p1->config&0x003f]));
                        break;
                    }
                }
                if (debug==100)
                    printf("\nNo se encontro peon para borrar... (error?)");
            }
            if(mov2.y==0)
            {
                while(query==1)
                {
                    //preguntar que pieza se quiere
                    printf("\nEscoger pieza:\n1. Torre\n2. Caballo\n3. Alfil\n4. Reina\n# de Pieza?: ");
                    char eleccion[81];
                    fgets(eleccion, sizeof(eleccion), stdin);
                    if(strcmp(eleccion, "1\n")==0)
                    {
                        query=2;
                    }
                    else if(strcmp(eleccion, "2\n")==0)
                    {
                        query=3;
                    }
                    else if(strcmp(eleccion, "3\n")==0)
                    {
                        query=4;
                    }
                    else if(strcmp(eleccion, "4\n")==0)
                    {
                        query=5;
                    }
                    else
                    {
                        printf("\nOpcion incorrecta. Intente de nuevo. ");
                    }
                }
                //Coronacion del peon
                //se hace reina por default
                //printf("\nAqui se deberia coronar un peon!!");
                
                p1->config&=0xfc7f;     //se borra id
                p1->config|=(query<<7);     //se corona como reina
                dummy.value+=(config_valor(p1->config)+100);
            }
        }
        
    //    int cuadro_bajo_ataque(int x, int y, char atacante);
    //else if (cuadro_bajo_ataque((lista[sig=='w'?4:28].config&0x003f)%8, (lista[sig=='w'?4:28].config&0x003f)/8, sig=='w'?'b':'w'))
        if (dummy.cuadro_bajo_ataque((dummy.lista[dummy.sig=='w'?4:28].config&0x003f)%8, (dummy.lista[dummy.sig=='w'?4:28].config&0x003f)/8, dummy.sig=='w'?'b':'w'))
        {   //regresar 1
            printf("\nEL MOVIMIENTO NO ES VALIDO! (ESTAS EN JAQUE O QUEDARIAS EN JAQUE)");
            return 1;
        }
        dummy.copiar(&ts);
        if (debug==100)
            printf("\nahora posicion en ts es %c%d", ((ts.lista[7].config&0x003f)%8)+'a', ((ts.lista[7].config&0x003f)/8)+1);
        return 0;
    }
}

int CEstadoDelJuego::aplicar_movimiento2(int npieza, int &m, struct POS &mov1, struct POS &mov2, int x, int query)
{   
    //convertir movimiento a movimiento de 16 bits
    int debug=0;
    //debug=100;
    int balpaso;
    
    //guardar temporalmente (local space) el estado de la bandera de peon al paso
    if(misc&0x10)
        balpaso=1;
    else
        balpaso=0;
    //deshabilitar bandera de peon al paso 
    misc&=0xef;
    
    if (debug==100)
        printf("\nMovimiento a aplicar es mov1.x: %d, mov1.y: %d, mov2.x: %d, mov2.y: %d.", mov1.x, mov1.y, mov2.x, mov2.y);
    int n, x1, y1;
    struct PIEZA *p1, *p2;
    //3431
    //registrar nueva posicion en mapa de bits
    short m2, inicio, fin;
    //si la torre se esta moviendo, eliminar el enroque de ese lado
    if(npieza==0 || npieza==7)
    {
        if(mov1.x==0 && mov1.y==0)//es enroque del lado de la reina
            misc&=0xfd;
        else if(mov1.x==7 && mov1.y==0)//es enroque del lado del rey
            misc&=0xfe;
    }
    else if(npieza==24 || npieza==31)
    {
        if(mov1.x==0 && mov1.y==7)//es enroque del lado de la reina
            misc&=0xf7;
        else if(mov1.x==7 && mov1.y==7)//es enroque del lado del rey
            misc&=0xfb;
    }
    inicio=16;
    fin=32;
    p1=&lista[npieza];
    m2=(m&0x00ff);
    if(p1->config&0x0400)//es negra
    {
        inicio=0;
        fin=16;
    }
    for(p2=NULL, n=inicio;n<fin;n++)
    {
        if((lista[n].config&0x003f)==m2 && (lista[n].config&0x0040))
        {
            p2=&lista[n];
            break;
        }
    }
    y1=((p1->config&0x003f)/8);
    x1=((p1->config&0x003f)%8);
    //(1<<(x1+y1*8))
    if(sig=='w')
    {
        tablero[0]&=(~piezas2d[y1][x1]);
    }
    else 
    {
        tablero[1]&=(~piezas2d[y1][x1]);
    }
    p1->config&=0xffc0;
    p1->config|=(mov2.x+mov2.y*8);
    x1=mov2.x;
    y1=mov2.y;
    if(sig=='w')
    {
        //(1<<(x1+y1*8))
        tablero[0]|=(piezas[x1+y1*8]);
        tablero[1]&=(~piezas[x1+y1*8]);
    }
    else 
    {
        tablero[1]|=(piezas[x1+y1*8]);
        tablero[0]&=(~piezas[x1+y1*8]);
    }
    //actualizar bitmaps
    //(piezas[lista[28].config&0x003f])
    (rey[1])=(piezas[lista[28].config&0x003f]);
    (rey[0])=(piezas[lista[4].config&0x003f]);
    //actualizar lista de piezas, y tal vez se deba borrar alguna
	if (p2!=NULL && ((p2->config)&0x0040))
	{
        //marcar pieza comida como 'no valida'
        p2->config&=0xffbf;
        //restar el valor de la pieza al valor del tablero
        value-=(config_valor(p2->config));
        if(mov2.x==7 && mov2.y==0)
            misc&=0xfe;
        else if(mov2.x==0 && mov2.y==0)
            misc&=0xfd;
        else if(mov2.x==7 && mov2.y==7)
            misc&=0xfb;
        else if(mov2.x==0 && mov2.y==7)
            misc&=0xf7;
	}
    //es un enroque?
    if(npieza==4)
    {
        //deshabilitar enroques
        misc&=0xfc;
        int lado_rey;
        int lado_reina;
        if((lista[7].config&0x003f)==7 && (lista[7].config&0x0040))
        {
            lado_rey=7;
            lado_reina=0;
            if (debug==100)
                printf("\n1");
        }
        else if((lista[0].config&0x003f)==7 && (lista[0].config&0x0040))
        {
            lado_rey=0;
            lado_reina=7;
            if (debug==100)
                printf("\n2");
        }
        else if((lista[7].config&0x003f)==0 && (lista[7].config&0x0040))
        {
            lado_rey=0;
            lado_reina=7;
            if (debug==100)
                printf("\n1");
        }
        else if((lista[0].config&0x003f)==0 && (lista[0].config&0x0040))
        {
            lado_rey=7;
            lado_reina=0;
            if (debug==100)
                printf("\n2");
        }
        if(m==0x0406)//del lado del rey?
        {
            //borrar la torre del lado del rey del mapa de bits
            tablero[0]&=(~(0x0000000000000080));
            //agregar la torre del lado del rey  del mapa de bits
            tablero[0]|=(0x0000000000000020);
            //cambiar la ubicacion de la torre
            lista[lado_rey].config&=0xffc0;
            lista[lado_rey].config|=0x0005;
        }
        else if(m==0x0402)//del lado de la reina?
        {
            //borrar la torre del lado de la reina del mapa de bits
            tablero[0]&=(~(0x0000000000000001));
            //agregar la torre del lado del rey  del mapa de bits
            tablero[0]|=(0x0000000000000008);
            //cambiar la ubicacion de la torre
            lista[lado_reina].config&=0xffc0;
            lista[lado_reina].config|=0x0003;
        }
    }
    else if(npieza==28)
    {
        //deshabilitar enroques
        misc&=0xf3;
        int lado_rey;
        int lado_reina;
        if((lista[31].config&0x003f)==63 && (lista[31].config&0x0040))
        {
            lado_rey=31;
            lado_reina=24;
            if (debug==100)
                printf("\n1");
        }
        else if((lista[24].config&0x003f)==63 && (lista[24].config&0x0040))
        {
            lado_rey=24;
            lado_reina=31;
            if (debug==100)
                printf("\n2");
        }
        else if((lista[31].config&0x003f)==56 && (lista[31].config&0x0040))
        {
            lado_rey=24;
            lado_reina=31;
            if (debug==100)
                printf("\n1");
        }
        else if((lista[24].config&0x003f)==56 && (lista[24].config&0x0040))
        {
            lado_rey=31;
            lado_reina=24;
            if (debug==100)
                printf("\n2");
        }
        if(m==0x3c3e)//del lado del rey?
        {
            //borrar la torre del lado del rey del mapa de bits
            tablero[1]&=(~(0x8000000000000000));
            //agregar la torre del lado del rey  del mapa de bits
            tablero[1]|=(0x2000000000000000);
            //cambiar la ubicacion de la torre
            lista[lado_rey].config&=0xffc0;
            lista[lado_rey].config|=61;
        }
        else if(m==0x3c3a)//del lado de la reina?
        {
            //borrar la torre del lado de la reina del mapa de bits
            tablero[1]&=(~(0x0100000000000000));
            //agregar la torre del lado del rey  del mapa de bits
            tablero[1]|=(0x0800000000000000);
            //cambiar la ubicacion de la torre
            lista[lado_reina].config&=0xffc0;
            lista[lado_reina].config|=59;
        }
    }
    else if (npieza>=8 && npieza<16)
    {
        if (debug==100)
            printf("\nRevisando comer al paso de blancas...!");
            
        if(mov2.x==mov1.x && (mov2.y==(mov1.y+2)))//checar si el peon se mueve al paso
        {//peon al paso
            //activar bandera
            misc|=0x10;
            //grabar posicion de al paso
            alpaso.x=mov2.x;
            alpaso.y=mov2.y-1;
        }
        else if(balpaso==1 && alpaso.x==mov2.x && alpaso.y==mov2.y)
        {
            //esta comiendo al paso
            //eliminar el peon que se comio            
            for(n=16;n<24;n++)
            {
                if((lista[n].config&0x003f)==((m&0x00ff)-8) && (lista[n].config&0x0040))
                {
                    p1=&lista[n];
                    p1->config&=0xffbf;
                    //borrar del tablero de bits
                    tablero[1]&=(~(piezas[p1->config&0x003f]));
                    break;
                }
            }
        }
        if(mov2.y==7 && query!=0)
        {
            //Coronacion del peon            
            p1->config&=0xfc7f;     //se borra id
            p1->config|=(query<<7);     //se corona como reina
            value+=(config_valor(p1->config)-100);
        }
    }
    else if (npieza>=16 && npieza<24)
    {
        if (debug==100)
            printf("\nRevisando comer al paso de negras...!");
            
        if(mov2.x==mov1.x && (mov2.y==(mov1.y-2)))//checar si el peon se mueve al paso
        {//peon al paso
            //activar bandera
            misc|=0x10;
            //grabar posicion de al paso
            alpaso.x=mov2.x;
            alpaso.y=mov2.y+1;
        }
        else if(balpaso==1 && alpaso.x==mov2.x && alpaso.y==mov2.y)
        {
            //esta comiendo al paso
            //eliminar el peon que se comio            
            for(n=8;n<16;n++)
            {
                if((lista[n].config&0x003f)==((m&0x00ff)+8) && (lista[n].config&0x0040))
                {
                    p1=&lista[n];
                    p1->config&=0xffbf;
                    //borrar del tablero de bits
                    tablero[0]&=(~(piezas[p1->config&0x003f]));
                    break;
                }
            }
        }
        if(mov2.y==0 && query!=0)
        {
            //Coronacion del peon            
            p1->config&=0xfc7f;     //se borra id
            p1->config|=(query<<7);     //se corona como reina
            value+=(config_valor(p1->config)+100);
        }
    }
    //regresar 0
    return 0;
}

int CEstadoDelJuego::examinar_entrada(char *str)
{
    int debug=0;
    //debug=100;
    if(debug==100)
        printf("\nExaminando entrada...(%s)", str);
    if(strcmp(str, "0")==0)
        return 0;
    if(strcmp(str, "1")==0)
        return 1;
    if(strcmp(str, "ab")==0)
    {
        if(ab==1)
            printf(": SI");
        else
            printf(": NO");
    }
    else if(strcmp(str, "ab off")==0)
        ab=0;
    else if(strcmp(str, "ab on")==0)
        ab=1;
    else if(strcmp(str, "nivel")==0)
    {
        printf("\nNuevo nivel: ");
        scanf("%d", &profundidad_objetivo);
		return 2;
    }
    else return 3;
    return 2;
}

void CEstadoDelJuego::procesar(unsigned long &game)
{
    int i, j;
    uint64_t nodos_totales[20];
    nodos_totales[0]=0;
    nodos_totales[1]=1;
    nodos_totales[2]=20;
    nodos_totales[3]=400;
    nodos_totales[4]=8902;
    nodos_totales[5]=197281;
    nodos_totales[6]=4865609;
    nodos_totales[7]=119060324;
    nodos_totales[8]=3195901860U;
    nodos_totales[9]=84998978956U;
    nodos_totales[10]=2439530234167U;
    nodos_totales[11]=69352859712417U;
    
    //hago una recursion con la lista de movimientos
    int profundidad=1;
    CEstadoDelJuego elmejor;
	contador=0;
    long  res;
    clock_t start;
    clock_t end;
    printf("\n");
    printf("\n N   Mov    Valor   Nodos       Mn      T (s)   Mn/s    F. de ramificacion");
    printf("\n --------------------------------------------------------------------------");
    printf("\n");
    char sign[81];
    unsigned long ctmp=1;
    for(i=1;i<=profundidad_objetivo;i++)
    //for(i=profundidad_objetivo;i<=profundidad_objetivo;i++)
    {
        contador_nivel[0]=1;
        for(j=1;j<20;j++)
        {
            contador_nivel[j]=0;
        }
        contador=0;
        CEstadoDelJuego ts_tmp;
        copiar(&ts_tmp);
        
        start = clock();
        ResetCounter();
        res=recursion(profundidad, i, this, &elmejor, game, -10000, 10000, 0);
        //printf("\ntiempo en recursion: %f", GetCounter());
        end = clock();
        float seconds = (float)((float)end - (float)start) / CLOCKS_PER_SEC;
        if (seconds==0)
            seconds=(float)0.01;
        //bf^depth=contador
        //depth*ln(bf)=ln(contador)
        //ln(bf)= ln(contador) / depth
        //bf = e^( ln(contador) / depth )
        //bf = exp(log(contador)/((float)i))
      //printf("\nNivel    Movimiento    Valor   Nodos explorados (Mnps)    Tiempo (s)     Rapidez   (Mnps)")
        printf("\n");
        printf(" %d%s", i, i<10?" ":"");
        printf("  %c%d-%c%d", elmejor.umov[0].x+'a', elmejor.umov[0].y+1, elmejor.umov[1].x+'a', elmejor.umov[1].y+1);
        printf("   %s%d%s", res<0?"":" ", res, tami(abs(res), 0));
        printf(" %lu%s", contador_nivel[i], tami(contador_nivel[i], 1));
        printf("  %.2f%s", ((float)contador_nivel[i]/1000000), tamf(((float)contador_nivel[i]/1000000), 0));
        printf("%.2f%s", seconds, tamf(seconds, 0));
        printf("%.2f%s", (((float)contador_nivel[i]/1000000)/seconds), tamf((((float)contador_nivel[i]/1000000)/seconds), 0));
        printf("%.1f", exp(log((float)(contador_nivel[i]))/((float)i)));
        ctmp=contador;
        //printf("    g:%d", game);
        if(game==0 || game==2)
        {
            //printf("jaque!");
            //printf("\nNivel: %d   Movimiento: %c%d-%c%d (%d)", i, elmejor.umov[0].x+'a', elmejor.umov[0].y+1, elmejor.umov[1].x+'a', elmejor.umov[1].y+1, res);
            //printf("\n %lu (%.2f Mn) nodos en %.2f s (%.2f Mnps)", contador, ((float)contador/1000000), seconds, (((float)contador/1000000)/seconds));
            if(i==profundidad_objetivo)
            {
                if(aplicar_movimiento(*this, elmejor.umov[0], elmejor.umov[1], elmejor.coronacion)==0)
                {
                    //si es valido, ejecutar el movimiento
                }
                else
                {
                    //si no es valido, imprimir mensaje de error y preguntar de nuevo
                    printf("\nError en la entrada, presione enter para continuar....");
                }
            }
            if(game==2)
            {
                printf("\nJaque Mate");
            }
        }
        else
        {
            printf("\nJaque mate!!");
            break;
        }
    }
    printf("\n");
    printf("\nMovimiento: %c%d-%c%d\n\n", elmejor.umov[0].x+'a', elmejor.umov[0].y+1, elmejor.umov[1].x+'a', elmejor.umov[1].y+1);
}
//d2-d4 e2-e4 c2-c4 f2-f4 d4-d5 b2-b3 a2-a3 g2-g4 h2-h3 g4-h5 h5-g6 g6-f7
long recursion(int profundidad, int profundidad_meta, CEstadoDelJuego *ts, CEstadoDelJuego *elmejor, unsigned long &game, long alpha, long beta, int db)
{
    char pad[81];
    contador_nivel[profundidad-1]++;
    contador++;
    int debug;
    //debug=0;
    //debug=99;
    if(debug=100)
    {
        if(profundidad==1)
            strcpy(pad, "|- ");
        else if (profundidad==2)
            strcpy(pad, "| |- ");
        else if (profundidad==3)
            strcpy(pad, "| | |- ");
        else if (profundidad==4)
            strcpy(pad, "| | | |- ");
        else if (profundidad==5)
            strcpy(pad, "| | | | |- ");
        else if (profundidad==6)
            strcpy(pad, "| | | | | |- ");
    }
    if(profundidad==(profundidad_meta+1))
    {
        //if(profundidad<5)
            return ts->value+ts->evaluar();
        return ts->value;
    }
    long valor_mejor, valor_hija;
    //if (debug==100)
    //    printf("\nrecursion: %d, M1", profundidad);
    valor_mejor=0;
    valor_hija=0;
    long alpha1, beta1;
    alpha1=alpha;
    beta1=beta;
    //si no, continuar
    //consulto la lista de movimientos correspondiente a mi color
    //hago una iteracion en esa lista, en cada iteracion, mando llamar esta misma funcion
    long inicio, fin, c, minmax, diff;
    //printf(" %c ", ts->sig);
	if(ts->sig=='b')
		minmax=MAXINF;
	else
		minmax=MININF;
    if(ts->sig=='w')
        inicio=0;
    else
        inicio=16;
    if(ts->sig=='w')
        fin=16;
    else
        fin=32;
    struct PIEZA *p;
    struct POS mov1, *mov2;
    char id;
    char pid;
    int i,j;
    CEstadoDelJuego *n=&tc[profundidad];
    int movc, valido;
    int separator=0;
    int salir=0;
    for(c=inicio;c<fin && salir==0;c++)
    {
        p=&(ts->lista[c]);
        if((p->config&0x0040)==0)
            continue;
        //busco mis piezas
        mov1.x=(p->config&0x003f)%8;
        mov1.y=(p->config&0x003f)/8;
        id = config_id(p->config);
        pid = (0x0380&(p->config))>>7;

        //if (debug==100)
        //    printf("\n    recursion: %d, M4. Pieza encontrada #%d: \'%c\', en %c%d, valort: %d", profundidad, c, id, mov1.x+'a', mov1.y+1, ts->value);
        for(i=0, mov2=movimientos[pid][mov1.x+mov1.y*8][0];p->config&0x0040 && mov2!=NULL && salir==0;i++, mov2=movimientos[pid][mov1.x+mov1.y*8][i])
        {
            movc = (((mov1.x+mov1.y*8)<<8)|(mov2->x+mov2->y*8));
            valido = ts->es_movimiento_valido2(movc, id, 0, 0);
            debug=0;
            if(valido)//si es valido, generar nuevo juego y aplicar movimiento
            {
                int corona=1;
                int pieza_nueva=0;
                int c1;
                //si se movio un peon a la fila de coronacion, probar las 4 combinaciones de piezas
            
                if(config_id(p->config)=='P' && mov2->y==7)//es peon blanco en fila de coronacion
                {
                    //printf("\npeon blanco coronandose...");
                    corona=4;
                    pieza_nueva=2;
                }
                else if(config_id(p->config)=='p' && mov2->y==0)//es peon negro en fila de coronacion
                {
                    //printf("\npeon negro coronandose...");
                    corona=4;
                    pieza_nueva=2;
                }
                
                for(c1=0;c1<corona;c1++, pieza_nueva++)
                {
                    
                    ts->copiar(n);//2955/12590
                    if(corona!=1)
                        n->coronacion=pieza_nueva;
                    //aplicar movimiento
                    n->aplicar_movimiento2(c, movc, mov1, *mov2, 0, pieza_nueva);
                    
                    if(n->cuadro_bajo_ataque((n->lista[n->sig=='w'?4:28].config&0x003f)%8, (n->lista[n->sig=='w'?4:28].config&0x003f)/8, n->sig=='w'?'b':'w'))
                    {
                        valido=0;
                        break;
                    }
                    separator++;
                    if(ts->sig=='b')
                        n->sig='w';
                    else
                        n->sig='b';
                        
                    if(debug==99)
                    {
                        printf("\n%s%c %d: %c%d-%c%d, v: %d, A=%d, B=%d, minmax: %d, vmejor=%d", pad, config_id(ts->lista[c].config), profundidad, mov1.x+'a', mov1.y+1, mov2->x+'a', mov2->y+1, n->value, alpha, beta, minmax, valor_mejor);
                    }
                    if(0 && profundidad==1)
                        db=1;
                    unsigned long tmp1 = contador_nivel[profundidad_objetivo];
                    valor_hija = recursion(profundidad+1, profundidad_meta, n, NULL, game, alpha1, beta1, db);
                    //if(1 && profundidad==(profundidad_objetivo-1))
                    if(0 && profundidad==1)
                    {
                        printf("\n%c%d-%c%d: %lu nodos", mov1.x+'a', mov1.y+1, mov2->x+'a', mov2->y+1, contador_nivel[profundidad_objetivo]-tmp1);
                        db=0;
                    }
                    if(db==1)
                    {
                        printf("\n\t%s%c %d: %c%d-%c%d, v: %d, A=%d, B=%d, minmax: %d, vmejor=%d", pad, config_id(ts->lista[c].config), profundidad, mov1.x+'a', mov1.y+1, mov2->x+'a', mov2->y+1, n->value, alpha, beta, minmax, valor_mejor);
                    }
                    //si son negras
                    if(ts->sig=='b')
                    {
                        if(valor_hija<beta1)
                            beta1=valor_hija;
                        if(valor_hija<minmax)
                        {
                            minmax = valor_hija;
                            valor_mejor=valor_hija;
                            if(profundidad==1)
                            {
                                n->umov[0]=mov1;
                                n->umov[1]=(*mov2);
                                n->copiar(elmejor);
                            }
                        }
                    }
                    else    //si son blancas
                    {
                        if(valor_hija>alpha1)
                            alpha1=valor_hija;
                        if(valor_hija>minmax)
                        {
                            minmax = valor_hija;
                            valor_mejor=valor_hija;
                            if(profundidad==1)
                            {
                                n->umov[0]=mov1;
                                n->umov[1]=(*mov2);
                                n->copiar(elmejor);
                            }
                        }
                    }
                    //si alpha>beta, hay que salirnos!
                    if(alpha>beta && ab==1)
                        salir=1;
                }
            }
        }
    }
    if(separator==0)
    {
        //checar si el rey esta en jaque
        //si no, regresar jaque/profundidad/2
        
        if(ts->cuadro_bajo_ataque((ts->lista[n->sig=='w'?4:28].config&0x003f)%8, (ts->lista[n->sig=='w'?4:28].config&0x003f)/8, ts->sig=='w'?'b':'w'))
        {
            //si esta en jaque, regresar jaque/profundidad
            if(profundidad==1)
                game=1;
            if(profundidad==2)
                game=2;
            if(ts->sig=='b')
            {
                //printf("\nmate of white: (%d) at %dth", short( ((float)10000) / ((float)profundidad) ), profundidad);
                return long(((float)100000)/(float)profundidad);
            }
            
            if(ts->sig=='w')
            {
                //printf("\nmate of black: (%d) at %dth", short( ((float)10000) / ((float)profundidad) ), profundidad);
                return long(((float)-100000)/(float)profundidad);
            }
        }
        else
        {
            //si esta en jaque, regresar jaque/profundidad
            if(profundidad==1)
                game=3;
            if(profundidad==2)
                game=4;
            if(ts->sig=='b')
            {
                //printf("\nmate of white: (%d) at %dth", short( ((float)10000) / ((float)profundidad) ), profundidad);
                return long(((float)100000)/(float)profundidad/2.0);
            }
            if(ts->sig=='w')
            {
                //printf("\nmate of black: (%d) at %dth", short( ((float)10000) / ((float)profundidad) ), profundidad);
                return long(((float)-100000)/(float)profundidad/2.0);
            }        
        }
    }
    return valor_mejor;
}

unsigned int convertir_movimiento(struct POS m1, struct POS m2)
{
    unsigned int ret;
    ret = m1.x+m1.y*8;
    ret = ret<<8;
    ret |= (m2.x+m2.y*8);
    return ret;
}

int CEstadoDelJuego::evaluar(void)
{
    int i, valor, debug;
    valor=0;
	debug=0;
	//debug=100;
    for(i=0;i<32;i++)
    {
        if(lista[i].config&0x0040)
        {
            if(config_id(lista[i].config)=='P')
            {
                //sumar valor a los peones adelantados               
                valor+=((lista[i].config&0x003f)/8)*8;
            }
            else if(config_id(lista[i].config)=='p')
            {
                //sumar valor a los peones adelantados               
                valor-=(7-(lista[i].config&0x003f)/8)*8;
            }
        }
    }
	if(debug==100)
		printf("\nfirst count: %d, tablero 0: 0x%.16x", valor, tablero[0]);
    //sumar valor a los cuadros del centro ocupados
    valor+=((((tablero[0]&0x0000000000080000)!=0)*50)+(((tablero[0]&0x000000000100000)!=0)*50)+(((tablero[0]&0x000000008000000)!=0)*100)+(((tablero[0]&0x000000010000000)!=0))*100);
    valor-=((((tablero[1]&0x0000000800000000)!=0)*100)+(((tablero[1]&0x000001000000000)!=0)*100)+(((tablero[1]&0x000080000000000)!=0)*50)+(((tablero[1]&0x000100000000000)!=0))*50);
	if(debug==100)
		printf("\nsecond count: %d", valor);
    //sumar valor a los cuadros del centro atacados
    if(cuadro_bajo_ataque(3, 3, 'w')==1)
        valor+=80;
    if(cuadro_bajo_ataque(4, 3, 'w')==1)
        valor+=80;
    if(cuadro_bajo_ataque(3, 4, 'w')==1)
        valor+=80;
    if(cuadro_bajo_ataque(4, 4, 'w')==1)
        valor+=80;
    if(cuadro_bajo_ataque(3, 3, 'b')==1)
        valor-=80;
    if(cuadro_bajo_ataque(4, 3, 'b')==1)
        valor-=80;
    if(cuadro_bajo_ataque(3, 4, 'b')==1)
        valor-=80;
    if(cuadro_bajo_ataque(4, 4, 'b')==1)
        valor-=80;
	if(debug==100)
		printf("\nthird count: %d", valor);
    return valor;
}

int inicializar_desde_fen(CEstadoDelJuego *t, char *fen)
{
    int debug=0;
    //debug=100;
    CEstadoDelJuego ts;
    t->copiar(&ts);
    ts.inicializar(0);
    //debug=100;
    CEstadoDelJuego tmp;
    ts.copiar(&tmp);
    tmp.misc=0;
    tmp.coronacion=0;
    int size = strlen(fen);
    char str[200];
    char fields[6][81];
    char rows[8][81];
    strcpy(str, fen);
    int c1, c2, c3, spaces_counter, slashes_counter;
    for(c1=0, spaces_counter=0, c2=0;c1<size;c1++)
    {
        if(str[c1]==' ')
        {
            fields[spaces_counter][c2]='\0';
            spaces_counter++;
            c2=0;
        }
        else if(spaces_counter<6)
        {
            fields[spaces_counter][c2]=str[c1];
            c2++;
        }
    }
    fields[spaces_counter][c2]='\0';
    
    if(debug==100)
    {
        if(spaces_counter!=5)
        {
            printf("\nError en la cadena FEN, no esta formada por 6 campos!");
            return -1;
        }
        else
        {
            printf("\nLos campos capturados son: ");
            for(c1=0;c1<6;c1++)
            {
                printf("\n\t%d:\t%s", c1, fields[c1]);
            }
            printf("\n");
        }
    }
    int num_piezas=0;
    size=strlen(fields[0]);
    for(c1=0, slashes_counter=0, c2=0;c1<size;c1++)
    {
        if((fields[0][c1]>='a' && fields[0][c1]<='z') || (fields[0][c1]>='A' && fields[0][c1]<='Z'))
        {
            //printf("\nn:%d, %c", num_piezas, fields[0][c1]);
            num_piezas++;
        }
        if(fields[0][c1]=='/')
        {
            rows[slashes_counter][c2]='\0';
            if(c2>8)
                return -2;
            slashes_counter++;
            c2=0;
        }
        else if(slashes_counter<8)
        {
            rows[slashes_counter][c2]=fields[0][c1];
            c2++;
        }
    }
    rows[slashes_counter][c2]='\0';
    
    if(slashes_counter!=7)
    {
        printf("\nError en el primer campo de la cadena FEN, no esta formada por 8 partes!");
        return -3;
    }
    if(num_piezas>32)
    {
        printf("\nEl numero de piezas es incorrecto (%d>32)", (strlen(fields[0])-7));
        return -4;
    }
    int kc, Kc;
    for(c1=0, Kc=0, kc=0;c1<strlen(fields[0]);c1++)
    {
        if(fields[0][c1]=='K')
            Kc++;
        if(fields[0][c1]=='k')
            kc++;
    }
    if(Kc!=1 || kc!=1)
    {
        printf("\nError, el numero de reyes de algun color es incorrecto!");
        return -5;
    }
    if(debug==100)
    {
        printf("\nLas filas capturadas son: ");
        for(c1=0;c1<8;c1++)
        {
            printf("\n\t%d:\t%s", c1, rows[c1]);
        }
        printf("\n");
    }
    if (strcmp("w", fields[1])==0)
        tmp.sig='w';
    else if (strcmp("b", fields[1])==0)
        tmp.sig='b';
    else
    {
        printf("\nError en el segundo campo de la cadena fen: \'%s\' (se espera \'w\' o \'b\'.", fields[1]);
        return -6;
    }
    
    if(strcmp("-", fields[2])!=0)
    {
        char enr=0;
        int largo_enroques = strlen(fields[2]);
        if(largo_enroques==0 || largo_enroques>4)
        {
            printf("\nError en el campo de enroques: %s, len:%d", fields[2], largo_enroques);
            return -7;
        }
        char k, q, K, Q;
        for(c1=0, k=0, q=0, K=0, Q=0;c1<largo_enroques;c1++)
        {
            if(fields[2][c1]=='k')
            {
                k++;
                enr|=0x04;
            }
            else if(fields[2][c1]=='q')
            {
                q++;
                enr|=0x08;
            }
            else if(fields[2][c1]=='K')
            {
                K++;
                enr|=0x01;
            }
            else if(fields[2][c1]=='Q')
            {
                Q++;
                enr|=0x02;
            }
            else
            {
                printf("\nError en el campo de enroques: %s", fields[2]);
                return -8;
            }
        }
        if(k>1 || q>1 || K>1 || Q>1)
        {
            printf("\nError en el campo de enroques: %s", fields[2]);
                return -9;
        }
        else
        {
            tmp.misc&=0xf0;
            tmp.misc|=enr;
        }
    }
    else
    {
        tmp.misc&=0xf0;
    }
    
    if(strcmp("-", fields[3])!=0)
    {
        if((fields[3][0]>='a' && fields[3][0]<='h') &&  (fields[3][1]=='2' || fields[3][1]=='8'))
        {
            tmp.alpaso.x=(fields[3][0]-'a');
            tmp.alpaso.y=(fields[3][1]-'1');
            tmp.misc|=0x10;
        }
        else
        {
            printf("\nError en el campo de \'en passant\'", fields[4]);
            return -10;
        }
    }
    
    if(strcmp("0", fields[4])!=0)
    {
        if(atoi(fields[4])==0)
        {
            printf("\nError en el campo de contador de 50 movimientos: %s", fields[4]);
            return -11;
        }
    }
    
    if(strcmp("0", fields[5])!=0 && strcmp("0\n", fields[5])!=0)
    {
        if(atoi(fields[5])==0)
        {
            printf("\nError en el campo de movimientos totales: \'%s\'", fields[5]);
            return -12;
        }
    }
    
    int x, y;
    if(debug==100)
    {
        printf("\nSe comienza parsing de tablero...");
    }  
    for(y=0;y<8;y++)
    {
        if (debug==100)
            printf("\ny: %d", y);
        if(strlen(rows[y])>8)
        {
            printf("\nError en tablero segun cadena FEN: fila %d incorrecta.", (7-y)+1);
            return -13;
        }
        for(c2=0, x=0;rows[y][c2]!='\0';c2++)
        {
            if (debug==100)
                printf("\nc2: %d, x: %d  (%c)", c2, x, rows[y][c2]);
            if(rows[y][c2]>='1' && rows[y][c2]<='8')
            {
                if(debug==100)
                    printf(" ... incrementando x (%d) ... por %d .. (rows[y]c2]=%c)", x, (rows[y][c2]-'0')), rows[y][c2];
                x+=(rows[y][c2]-'0');
                if(debug==100)
                    printf(" ... ahora x es (%d) ... ", x);
            }
            else
            {
                if(debug==100)
                    printf("\n Buscar %c en %d%c", rows[y][c2], 7-y+1, x+'a');
                int i;
                for(i=0; i<32; i++)
                {
                    if(debug==100)
                    {
                            printf("\n    Ahora analizando: %d%c (%c)..... %d%c. pza: (%c)", (7-y)+1, x+'a', rows[y][c2], ((tmp.lista[i].config&0x003f)/8)+1, ((tmp.lista[i].config&0x003f)%8)+'a', config_id(tmp.lista[i].config));
                    }
                    if(config_id(tmp.lista[i].config)==rows[y][c2] && (tmp.lista[i].config&0x0040)==0)
                    {
                        if(debug==100)
                        {
                                printf("\n    Encontrada, ahora sustituir: config es 0x%.2x ... y: %d, x: %d", tmp.lista[i].config, y, x);
                        }
                        tmp.lista[i].config|=0x0040;
                        tmp.lista[i].config&=0xffc0;
                        tmp.lista[i].config|=((7-y)*8+x);
                        if(debug==100)
                        {
                                printf("\n    Ahora config es 0x%.2x", tmp.lista[i].config);
                        }
                        break;
                    }
                }
                if(i==32)
                {
                    printf("\nError en fila %d, columna %d", y, x);
                    return -14;
                }
                //acomodar en piezas...
                x++;
            }
        }
    }
    int valor=0;
    
    
    long int i, j;
    uint64_t uno = 1;
    tmp.tablero[0]=0;
    tmp.tablero[1]=0;
    tmp.rey[0]=0;
    tmp.rey[1]=0;
    for(i=0;i<32;i++)
    {
        if (tmp.lista[i].config&0x0040)
        {
            if(config_id(tmp.lista[i].config)=='K')
            {
                tmp.rey[0]=(uno<<(tmp.lista[i].config&0x003f));
            }
            else if(config_id(tmp.lista[i].config)=='k')
            {
                tmp.rey[1]=(uno<<(tmp.lista[i].config&0x003f));
            }
            if(config_id(tmp.lista[i].config)>='A' && config_id(tmp.lista[i].config)<='Z')
                tmp.tablero[0]|=(uno<<(tmp.lista[i].config&0x003f));
            if(config_id(tmp.lista[i].config)>='a' && config_id(tmp.lista[i].config)<='z')
                tmp.tablero[1]|=(uno<<(tmp.lista[i].config&0x003f));
        }
    }
   
    for(i=0;i<32;i++)
    {
        if(tmp.lista[i].config&0x0040)
        {
            valor+=config_valor(tmp.lista[i].config);
        }
    }
    tmp.value=valor;
    
    tmp.copiar(t);
}
/*
Los campos capturados son:
        0:      r1bq1b1r/pppp2k1/2n4p/4P1NQ/3P4/4p2P/PPP3P1/RN3RK1
        1:      b
        2:      -
        3:      -
        4:      0

Las filas capturadas son:
        0:      r1bq1b1r
        1:      pppp2k1
        2:      2n4p
        3:      4P1NQ
        4:      3P4
        5:      4p2P
        6:      PPP3P1
        7:      RN3RK1

Se comienza parsing de tablero...
*/

void CEstadoDelJuego::inicializar(int enable)
{
    int enable2;
    if(enable==0)
        enable2=0;
    else
        enable2=1;
    //se inicializa la lista de piezas, con sus posiciones y valores
    //void inicializar_pieza(struct PIEZA *p, unsigned char y, unsigned char x, int VALOR, char ID, char PIEZA_ID, char VALIDA);
    inicializar_pieza(&lista[0], 0, 0, 500, 'R', 2, enable2 );
    inicializar_pieza(&lista[1], 0, 1, 300, 'N', 3, enable2 );
    inicializar_pieza(&lista[2], 0, 2, 300, 'B', 4, enable2 );
    inicializar_pieza(&lista[3], 0, 3, 900, 'Q', 5, enable2 );
    inicializar_pieza(&lista[4], 0, 4, 0, 'K', 6, enable2 );
    inicializar_pieza(&lista[5], 0, 5, 300, 'B', 4, enable2 );
    inicializar_pieza(&lista[6], 0, 6, 300, 'N', 3, enable2 );
    inicializar_pieza(&lista[7], 0, 7, 500, 'R', 2, enable2 );
    inicializar_pieza(&lista[8], 1, 0, 100, 'P', 0, enable2 );
    inicializar_pieza(&lista[9], 1, 1, 100, 'P', 0, enable2 );
    inicializar_pieza(&lista[10], 1, 2, 100, 'P', 0, enable2 );
    inicializar_pieza(&lista[11], 1, 3, 100, 'P', 0, enable2 );
    inicializar_pieza(&lista[12], 1, 4, 100, 'P', 0, enable2 );
    inicializar_pieza(&lista[13], 1, 5, 100, 'P', 0, enable2 );
    inicializar_pieza(&lista[14], 1, 6, 100, 'P', 0, enable2 );
    
    inicializar_pieza(&lista[15], 1, 7, 100, 'P', 0, enable2 );
    //inicializar_pieza(&lista[15], 4, 7, 100, 'P', 0, enable2 );
    //misc|=0x10;
    //alpaso.x=7;
    //alpaso.y=2;
    //sig='b';
    
    inicializar_pieza(&lista[16], 6, 0, -100, 'p', 1, enable2 );
    //inicializar_pieza(&lista[16], 5, 0, -100, 'p', 1, enable2 );
    
    inicializar_pieza(&lista[17], 6, 1, -100, 'p', 1, enable2 );
    inicializar_pieza(&lista[18], 6, 2, -100, 'p', 1, enable2 );
    inicializar_pieza(&lista[19], 6, 3, -100, 'p', 1, enable2 );
    inicializar_pieza(&lista[20], 6, 4, -100, 'p', 1, enable2 );
    inicializar_pieza(&lista[21], 6, 5, -100, 'p', 1, enable2 );
    inicializar_pieza(&lista[22], 6, 6, -100, 'p', 1, enable2 );
    inicializar_pieza(&lista[23], 6, 7, -100, 'p', 1, enable2 );
    inicializar_pieza(&lista[24], 7, 0, -500, 'r', 2, enable2 );
    inicializar_pieza(&lista[25], 7, 1, -300, 'n', 3, enable2 );
    inicializar_pieza(&lista[26], 7, 2, -300, 'b', 4, enable2 );
    inicializar_pieza(&lista[27], 7, 3, -900, 'q', 5, enable2 );
    inicializar_pieza(&lista[28], 7, 4, 0, 'k', 6, enable2 );
    inicializar_pieza(&lista[29], 7, 5, -300, 'b', 4, enable2 );
    inicializar_pieza(&lista[30], 7, 6, -300, 'n', 3, enable2 );
    inicializar_pieza(&lista[31], 7, 7, -300, 'r', 2, enable2 );
    
    long int i, j;
    uint64_t uno = 1;
    tablero[0]=0;
    tablero[1]=0;
    coronacion=0;
    for(i=0;i<32;i++)
    {
        if(config_id(lista[i].config)=='K')
        {
            rey[0]=(uno<<(lista[i].config&0x003f));
        }
        else if(config_id(lista[i].config)=='k')
        {
            rey[1]=(uno<<(lista[i].config&0x003f));
        }
        if(config_id(lista[i].config)>='A' && config_id(lista[i].config)<='Z')
            tablero[0]|=(uno<<(lista[i].config&0x003f));
        if(config_id(lista[i].config)>='a' && config_id(lista[i].config)<='z')
            tablero[1]|=(uno<<(lista[i].config&0x003f));
    }
    
    //rey[0]=0x10;
    //rey[1]=0x8000000000000;
    //tablero[0]=0x000000000000ffff;
    //tablero[1]=0xffff000000000000;
    
    misc=0x0f;
    
    for (i=0;i<100000;i++)
    {
        for(j=0;j<8;j++)
        {
            crc[i][j]=0;
        }
    }
    
}

long int existe_crc(struct PIEZA a[32])
{
    short i, j, ret;
    uint64_t lcrc[4];
    lcrc[0]=0;
    lcrc[1]=0;
    lcrc[2]=0;
    lcrc[3]=0;
    for(i=0;i<32;i++)
    {
        if(a[i].config & 0x0400)
           lcrc[i/8]|=40;
        lcrc[i/8]|=(a[i].config&0x007f);
        lcrc[i/8]=lcrc[i/8]<<8;
    }
    
    ret=0;
    for (i=0;i<100000;i++)
    {
        for(j=0;j<4;j++)
        {
            if(crc[i][0]==0 && crc[i][1]==0 && crc[i][2]==0 && crc[i][3]==0)
            {
                crc[i][0]=lcrc[0];
                crc[i][1]=lcrc[1];
                crc[i][2]=lcrc[2];
                crc[i][3]=lcrc[3];
                return 0;
            }
            else if(crc[i][0]==lcrc[0] && crc[i][1]==lcrc[1] && crc[i][2]==lcrc[2] && crc[i][3]==lcrc[3])
                return 1;
        }
    }
    return ret;
}


void CEstadoDelJuego::imprimir_tablero(void)
{
    char t[8][8];
    int x, y, i;
    unsigned int columnas, filas;
    printf("\n");
    for(x=0;x<8;x++)
    {
        for(y=0;y<8;y++)
        {
            t[y][x]='-';
        }
    }
    //printf("\nahora posicion en ts es %c%d (valido:%d)", ((lista[7].config&0x003f)%8)+'a', ((lista[7].config&0x003f)/8)+1, lista[7].config&0x0040);
    for(i=0;i<32;i++)
    {
        //printf("\np->config (%d) == 0x%.4x", i, lista[i].config);
        if(lista[i].config&0x0040)
        {
            //printf("%c", config_id(lista[i].config));
            t[(lista[i].config&0x003f)/8][(lista[i].config&0x003f)%8]=config_id(lista[i].config);
        }
    }
    
    printf("\n");
	printf("      \t  a b c d e f g h\n\n");
    for(filas=7;filas>=0 && filas<=7;filas--)
    {
		printf("      %d  ", filas+1);
        for(columnas=0;columnas<8;columnas++)
        {
                printf(" %c", t[filas][columnas]);
        }
        if(filas==7)
        {
            char str[3]="--";
            if(misc&0x10)
            {
                str[0]=alpaso.x+'a';
                str[1]=alpaso.y+'1';
            }
            printf("         Peon al paso: %s", str);
        }
        if(filas==6)
        {   //KQkq
            printf("         Enroques: %c%c%c%c", (misc&0x01)?'K':' ', (misc&0x02)?'Q':' ', (misc&0x04)?'k':' ', (misc&0x08)?'q':' ');
        }
        if(filas==5)
        {   //KQkq
            printf("         Turno: %d (%s)", turno_global, (sig=='b')?"Negras":"Blancas");
        }
        if(filas==4)
        {   //KQkq
            printf("         Valor del tablero: %d  (%d)", value, evaluar());
        }
        if(filas==3)
        {   //KQkq
            printf("         Jaques: %c%c", (cuadro_bajo_ataque((lista[4].config&0x003f)%8, (lista[4].config&0x003f)/8, 'b'))==1?'K':'-', (cuadro_bajo_ataque((lista[28].config&0x003f)%8, (lista[28].config&0x003f)/8, 'w'))==1?'k':'-');
        }
        if(filas==2)
        {   //KQkq
            printf("         Nivel actual: %d", profundidad_objetivo);
        }
        
        //int CEstadoDelJuego::cuadro_bajo_ataque(int x, int y, char atacante)
        printf("\n");
    }
}

inline unsigned int CEstadoDelJuego::es_movimiento_valido(unsigned int m, char p)
{
    int debug=0;
    int ret=0;
    //debug=100;
    //51127244
    if (debug==100)
    {   
        printf("\nComienza validacion...");
        printf("\nMovimiento es: 0x%.4x", m);
    }
    //Si el final es de las propias fichas, no sirve el movimiento
    if(piezas[m&0x00ff]&(tablero[sig=='b']))
    {
        cout << "\n" << "tablero: " << hex << tablero[ sig=='b' ] << endl;
        if (debug==100)
            printf(" M1: %c", p);
        ret=0;
    }
    //se checa si la pieza a mover corresponde a mi turno
    else if((piezas[(m)>>8]&(tablero[sig=='b']))==0)
    {
        if (debug==100)
            printf(" M2: %c, sig: %c", p, sig);
        ret=0;
    }
    //se checa si el destino es igual al origen, esto seria invalido
    else if (((m)>>8)==(m&0x00ff))
    {
        if (debug==100)
            printf(" M3: %c", p);
        ret=0;
    }
    //Si el final es cualquier rey, no sirve el movimiento
    else if(piezas[m&0x00ff]&(rey[0]|rey[1]))
    {
        if (debug==100)
            printf(" M4: %c", p);
        ret=0;
    }
    //se checa el tipo de pieza
    //revisamos si hay alguna pieza atravesandose
    else if(p=='P')
    {
        if (debug==100)
            printf(" M5: %c", p);
        
        //primero revisar si el movimiento está listado. De no estarlo, es invalido
        //POS *movimientos[7][64][29]
        int c, flag;
        for(c=0, flag=0;c<29 && movimientos[PEON_WHITE][m>>8][c]!=NULL;c++)
        {
            //si el destino no esta en la lista de movimientos, regresar 0
            if( (movimientos[PEON_WHITE][m>>8][c]->x == ((m&0x00ff)%8)) && (movimientos[PEON_WHITE][m>>8][c]->y == ((m&0x00ff)/8)) )
            {
                flag=1;
            }
        }
        if (debug==100)
        {
            printf(" M5.0 flag is: %d, ret is %d", flag, ret);
            printf("alpaso: %d, m: %d, (m&0x00ff)/8: %d, misc: 0x%x", (alpaso.x+alpaso.y*8), m&0x00ff, (m&0xff)/8, misc);
        }
        if(flag==0)
        {
            ret=0;
        }
        
        //si no hay pieza 2 cuadros enfrente y esta en la fila 2 o 7, puede hacerlo
        //si no hay pieza enfrente, puede moverse ahi
        else if(((peon_white_advance[(m)>>8][m&0x00ff])&(tablero[0]|tablero[1]))==0)
		{
            if (debug==100)
                printf(" M5.1 ");
            ret=1;
		}
        //si hay pieza en diagonal, puede moverse ahi
		else if(peon_white_takes[(m)>>8][m&0x00ff]&(tablero[1]))
		{
            if (debug==100)
                printf(" M5.2 ");
            ret=1;
		}
        //si el movimiento final es un cuadro en passant
        //entonces es una toma en passant valida
		else if(((alpaso.x+alpaso.y*8)==(m&0x00ff))&&(((m&0x00ff)/8)==5) && (misc&0x10))
		{
            if (debug==100)
                printf(" M5.3 ");
            ret=1;
		}
    }
    else if(p=='p')
    {
        if (debug==100)
            printf(" M5: %c", p);
        //primero revisar si el movimiento está listado. De no estarlo, es invalido
        //POS *movimientos[7][64][29]
        int c, flag;
        for(c=0, flag=0;c<29 && movimientos[PEON_BLACK][m>>8][c]!=NULL;c++)
        {
            //si el destino no esta en la lista de movimientos, regresar 0
            if( (movimientos[PEON_BLACK][m>>8][c]->x == ((m&0x00ff)%8)) && (movimientos[PEON_BLACK][m>>8][c]->y == ((m&0x00ff)/8)) )
                flag=1;
        }
        if(flag==0)
            ret=0;
            
        //si no hay pieza 2 cuadros enfrente y esta en la fila 2 o 7, puede hacerlo
        //si no hay pieza enfrente, puede moverse ahi
        else if(((peon_black_advance[(m)>>8][m&0x00ff])&(tablero[0]|tablero[1]))==0)
		{
            if (debug==100)
                printf(" M5.1 ");
            ret=1;
		}
        //si hay pieza en diagonal, puede moverse ahi
		else if(peon_black_takes[(m)>>8][m&0x00ff]&(tablero[0]))
		{
            if (debug==100)
                printf(" M5.2 ");
            ret=1;
		}
        //si el movimiento final es un cuadro en passant
        //entonces es una toma en passant valida
		else if(((alpaso.x+alpaso.y*8)==(m&0x00ff))&&(((m&0x00ff)/8)==2) && (misc&0x10))
		{
            if (debug==100)
                printf(" M5.3 ");
            ret=1;
		}
    }
    else if((p|0x20)=='r')
    {
        if (debug==100)
            printf(" M5: %c", p);
        //primero revisar si el movimiento está listado. De no estarlo, es invalido
        //POS *movimientos[7][64][29]
        int c, flag;
        for(c=0, flag=0;c<29 && movimientos[TORRE][m>>8][c]!=NULL;c++)
        {
            //si el destino no esta en la lista de movimientos, regresar 0
            if( (movimientos[TORRE][m>>8][c]->x == ((m&0x00ff)%8)) && (movimientos[TORRE][m>>8][c]->y == ((m&0x00ff)/8)) )
                flag=1;
        }
        if(flag==0)
            ret=0;
        else if (torre_exclusive[(m)>>8][m&0x00ff]&(tablero[0]|tablero[1]))
        {
            ret=0;
        }
        else
            ret=1;
    }
    else if((p|0x20)=='n')
    {
        if (debug==100)
            printf(" M5: %c", p);
        //primero revisar si el movimiento está listado. De no estarlo, es invalido
        //POS *movimientos[7][64][29]
        int c, flag;
        for(c=0, flag=0;c<29 && movimientos[CABALLO][m>>8][c]!=NULL;c++)
        {
            //si el destino no esta en la lista de movimientos, regresar 0
            if( (movimientos[CABALLO][m>>8][c]->x == ((m&0x00ff)%8)) && (movimientos[CABALLO][m>>8][c]->y == ((m&0x00ff)/8)) )
                flag=1;
        }
        if(flag==0)
            ret=0;
        else
            ret=1;
        //siempre es valido
    }
    else if((p|0x20)=='b')
    {
    //d2-d4, b7-b6, c2-c4, e7-e6, d4-d5, b6-b5, c4-c5, f8-c5 X
        if (debug==100)
            printf(" M5: %c, ret: %d", p, ret);
        //primero revisar si el movimiento está listado. De no estarlo, es invalido
        //POS *movimientos[7][64][29]
        int c, flag;
        for(c=0, flag=0;c<29 && movimientos[ALFIL][m>>8][c]!=NULL;c++)
        {
            //si el destino no esta en la lista de movimientos, regresar 0
            if( (movimientos[ALFIL][m>>8][c]->x == ((m&0x00ff)%8)) && (movimientos[ALFIL][m>>8][c]->y == ((m&0x00ff)/8)) )
                flag=1;
        }
        if(flag==0)
        {
            ret=0;
            if(debug==100)
                printf("\nEl movimiento no esta en la lista");
        }
        else if (alfil_exclusive[(m)>>8][m&0x00ff]&(tablero[0]|tablero[1]))
        {
            ret=0;
            if(debug==100)
                printf("\nEl movimiento esta obtaculizado");
        }
        else
        {
            ret=1;
            if(debug==100)
                printf("\nEl movimiento SI esta en la lista, y no esta obstaculizado, ret es: %d", ret);
        }
    }
    else if((p|0x20)=='q')
    {
        if (debug==100)
            printf(" M5: %c", p);
        //primero revisar si el movimiento está listado. De no estarlo, es invalido
        //POS *movimientos[7][64][29]
        if (debug==100)
            printf("\nAhora revisando movimiento de la reina: ");
        int c, flag;
        for(c=0, flag=0;c<29 && movimientos[DAMA][m>>8][c]!=NULL;c++)
        {
            if (debug==100)
                printf("\n%d: %c%d", c, movimientos[DAMA][m>>8][c]->x+'a', movimientos[DAMA][m>>8][c]->y+1);
            //si el destino no esta en la lista de movimientos, regresar 0
            if( (movimientos[DAMA][m>>8][c]->x == ((m&0x00ff)%8)) && (movimientos[DAMA][m>>8][c]->y == ((m&0x00ff)/8)) )
                flag=1;
        }
        if(flag==0)
        {
            if (debug==100)
                printf("\nError, este movimiento no esta en lista de movimientos para su posicion (se revisaron %d movs)", c);
            ret=0;
        }    
        else if (dama_exclusive[(m)>>8][m&0x00ff]&(tablero[0]|tablero[1]))
        {
            if (debug==100)
                printf("\nError, hay obstaculo en el movimiento");
            ret=0;
        }
        else
            ret=1;
    }
    else if(p=='K')
    {
        if (debug==100)
            printf(" M5: %c", p);
        //primero revisar si el movimiento está listado. De no estarlo, es invalido
        //POS *movimientos[7][64][29]
        int c, flag;
        for(c=0, flag=0;c<29 && movimientos[REY][m>>8][c]!=NULL;c++)
        {
            if (debug==100)
                printf("\n%d: %c%d", c, movimientos[REY][m>>8][c]->x+'a', movimientos[REY][m>>8][c]->y+1);
            //si el destino no esta en la lista de movimientos, regresar 0
            if( (movimientos[REY][m>>8][c]->x == ((m&0x00ff)%8)) && (movimientos[REY][m>>8][c]->y == ((m&0x00ff)/8)) )
                flag=1;
        }
        if(flag==0)
            ret=0;
        //se mueve 2 cuadros?
        else if(m==0x0406)//Enroque de blancas, lado del rey
        {
            if (debug==100)
                printf("\nEnroque de blancas, lado del rey...");
            //es enroque valido y los cuadros intermedios estan vacios
            if(misc&0x01 && ((tablero[0]|tablero[1])&0x0000000000000060)==0 )
            {
                //revisar si los cuadros interiores estan siendo atacados
                //    int cuadro_bajo_ataque(int x, int y, char atacante);
                if(cuadro_bajo_ataque(4, 0, 'b')==0 && cuadro_bajo_ataque(5, 0, 'b')==0 && cuadro_bajo_ataque(6, 0, 'b')==0)
                    //no, es valido
                    ret= 1;
                else
                    ret=0;
            }
            //no, es invalido
            else ret=0;
        }
        else if(m==0x0402)//Enroque de blancas, lado de la reina
        {
            if (debug==100)
                printf("\nEnroque de blancas, lado de la reina...");
            //es enroque valido y los cuadros intermedios estan vacios?
            if(misc&0x02 && ((tablero[0]|tablero[1])&0x000000000000000e)==0 )
            {
                //revisar si los cuadros interiores estan siendo atacados
                //    int cuadro_bajo_ataque(int x, int y, char atacante);
                if(cuadro_bajo_ataque(4, 0, 'b')==0 && cuadro_bajo_ataque(3, 0, 'b')==0 && cuadro_bajo_ataque(2, 0, 'b')==0)
                {
                    //no, es valido
                    ret=1;
                }
                else
                    ret=0;
            }
            //no, es invalido
            else
                ret=0;
        }
        else
        {
            if(debug==100)
                printf("\nchecando si es movimiento de rey normal...");
            int x1, y1, x2, y2, ladox, ladoy;
            x1=((m>>8)%8);
            y1=((m>>8)/8);
            x2=((m&0x00ff)%8);
            y2=((m&0x00ff)/8);
            ladox=(x2-x1)*(x2-x1);
            ladoy=(y2-y1)*(y2-y1);
            if(debug==100)
                printf("\nDatos de movimiento de rey, x1: %d, y1: %d, x2: %d, y2: %d, ladox: %d, ladoy: %d", x1, y1, x2, y2, ladox, ladoy);
            if(ladox<2 && ladoy<2)
                ret=1;
            else
                ret=0;
        }
    }
    else if(p=='k')
    {
        if (debug==100)
            printf(" M5: %c", p);
        //primero revisar si el movimiento está listado. De no estarlo, es invalido
        //POS *movimientos[7][64][29]
        int c, flag;
        for(c=0, flag=0;c<29 && movimientos[REY][m>>8][c]!=NULL;c++)
        {
            if (debug==100)
                printf("\n%d: %c%d", c, movimientos[REY][m>>8][c]->x+'a', movimientos[REY][m>>8][c]->y+1);
            //si el destino no esta en la lista de movimientos, regresar 0
            if( (movimientos[REY][m>>8][c]->x == ((m&0x00ff)%8)) && (movimientos[REY][m>>8][c]->y == ((m&0x00ff)/8)) )
                flag=1;
        }
        if(flag==0)
            ret=0;
        //se mueve 2 cuadros?
        else if(m==0x3c3e)//Enroque de negras, lado del rey
        {
            if (debug==100)
                printf("\nEnroque de negras, lado del rey... (1)");
            //si
            //es enroque valido y los cuadros intermedios estan vacios
            if(misc&0x04 && ((tablero[0]|tablero[1])&0x6000000000000000)==0 )
            {
                if (debug==100)
                    printf("\nEnroque de negras, lado del rey... (2)");
                //revisar si los cuadros interiores estan siendo atacados
                //    int cuadro_bajo_ataque(int x, int y, char atacante);
                if(cuadro_bajo_ataque(4, 7, 'w')==0 && cuadro_bajo_ataque(5, 7, 'w')==0 && cuadro_bajo_ataque(6, 7, 'w')==0)
                {
                    //no, es valido
                    ret=1;
                }
                else
                    ret=0;
            }
            else
                ret=0;
        }
        else if(m==0x3c3a)//Enroque de negras, lado de la reina
        {
            if (debug==100)
                printf("\nEnroque de negras, lado de la reina...(1)");
            //si
            //es enroque valido y los cuadros intermedios estan vacios
            if(misc&0x08 && ((tablero[0]|tablero[1])&0x0e00000000000000)==0 )
            {
                if (debug==100)
                    printf("\nEnroque de negras, lado de la reina...(2)");
                //revisar si los cuadros interiores estan siendo atacados
                //    int cuadro_bajo_ataque(int x, int y, char atacante);
                if(cuadro_bajo_ataque(4, 7, 'w')==0 && cuadro_bajo_ataque(3, 7, 'w')==0 && cuadro_bajo_ataque(2, 7, 'w')==0)
                {
                    //no, es valido
                    ret=1;
                }
                else
                    ret=0;
            }
            //no, es invalido
            else
                ret=0;
        }
        else
        {
            if(debug==100)
                printf("\nchecando si es movimiento de rey normal...");
            int x1, y1, x2, y2, ladox, ladoy;
            x1=((m>>8)%8);
            y1=((m>>8)/8);
            x2=((m&0x00ff)%8);
            y2=((m&0x00ff)/8);
            ladox=(x2-x1)*(x2-x1);
            ladoy=(y2-y1)*(y2-y1);
            if(debug==100)
                printf("\nDatos de movimiento de rey, x1: %d, y1: %d, x2: %d, y2: %d, ladox: %d, ladoy: %d", x1, y1, x2, y2, ladox, ladoy);
            if(ladox<2 && ladoy<2)
                ret=1;
            else
                ret=0;
        }
    }
    return ret;
}

inline unsigned int CEstadoDelJuego::es_movimiento_valido2(unsigned int m, char p, int king_capture_allowed, int query)
{
    char test = (p>='a' && p<='z')?'b':'w';
    int debug=0;
    int ret=0;
    uint64_t beam = 0;
    //debug=100;
    //51127244
    if (debug==100)
    {   
        printf("\nComienza validacion...(%c), color: %c", p, test);
        printf("\nMovimiento es: 0x%.4x", m);
    }
    //Si el final es de las propias fichas, no sirve el movimiento
    if(piezas[m&0x00ff]&(tablero[test=='b']))
    {
        if (debug==100)
            printf(" M1: %c", p);
        ret=0;
    }
    //se checa el tipo de pieza
    //revisamos si hay alguna pieza atravesandose
    else if(p=='P')
    {
        if (debug==100)
            printf(" M5: %c", p);
        //si no hay pieza 2 cuadros enfrente y esta en la fila 2 o 7, puede hacerlo
        //si no hay pieza enfrente, puede moverse ahi
        if(((peon_white_advance[(m)>>8][m&0x00ff])&(tablero[0]|tablero[1]))==0)
		{
            if (debug==100)
                printf(" M5.1 ");
            ret=1;
		}
        //si hay pieza en diagonal, puede moverse ahi
		else if(peon_white_takes[(m)>>8][m&0x00ff]&(tablero[1]))
		{
            if (debug==100)
                printf(" M5.2 ");
            ret=1;
		}
        //si el movimiento final es un cuadro en passant
        //entonces es una toma en passant valida
		else if(((alpaso.x+alpaso.y*8)==(m&0x00ff))&&(((m&0x00ff)/8)==5) && (misc&0x10))
		{
            if (debug==100)
                printf(" M5.3 ");
            ret=1;
		}
    }
    else if(p=='p')
    {
        if (debug==100)
            printf(" M5: %c", p);
        //si no hay pieza 2 cuadros enfrente y esta en la fila 2 o 7, puede hacerlo
        //si no hay pieza enfrente, puede moverse ahi
        if(((peon_black_advance[(m)>>8][m&0x00ff])&(tablero[0]|tablero[1]))==0)
		{
            if (debug==100)
                printf(" M5.1 ");
            ret=1;
		}
        //si hay pieza en diagonal, puede moverse ahi
		else if(peon_black_takes[(m)>>8][m&0x00ff]&(tablero[0]))
		{
            if (debug==100)
                printf(" M5.2 ");
            ret=1;
		}
        //si el movimiento final es un cuadro en passant
        //entonces es una toma en passant valida
		else if(((alpaso.x+alpaso.y*8)==(m&0x00ff))&&(((m&0x00ff)/8)==2) && (misc&0x10))
		{
            if (debug==100)
                printf(" M5.3 ");
            ret=1;
		}
    }
    else if((p|0x20)=='r')
    {
        if (torre_exclusive[(m)>>8][m&0x00ff]&(tablero[0]|tablero[1]))
        {
            ret=0;
        }
        else
            ret=1;
    }
    else if((p|0x20)=='n')
    {
            ret=1;
        //siempre es valido
    }
    else if((p|0x20)=='b')
    {
        if (alfil_exclusive[(m)>>8][m&0x00ff]&(tablero[0]|tablero[1]))
        {
            ret=0;
        }
        else 
            ret=1;
    }
    else if((p|0x20)=='q')
    {
        if (debug==100)
            printf(" M5: %c, revisando reina", p);
        if (dama_exclusive[(m)>>8][m&0x00ff]&(tablero[0]|tablero[1]))
        {
            if (debug==100)
                printf(" M5: %c, movimiento de reina no valido!", p);
            ret=0;
        }
        else
            ret=1;
    }
    else if(p=='K')
    {
        //se mueve 2 cuadros?
        if(m==0x0406)//Enroque de blancas, lado del rey
        {
            //es enroque valido y los cuadros intermedios estan vacios
            if(misc&0x01 && ((tablero[0]|tablero[1])&0x0000000000000060)==0 )
            {
                //revisar si los cuadros interiores estan siendo atacados
                //    int cuadro_bajo_ataque(int x, int y, char atacante);
                if(cuadro_bajo_ataque(4, 0, 'b')==0 && cuadro_bajo_ataque(5, 0, 'b')==0 && cuadro_bajo_ataque(6, 0, 'b')==0)
                {
                    //no, es valido
                    ret=1;
                }
                else
                    ret=0;
            }
            //no, es invalido
            else 
                ret=0;
        }
        else if(m==0x0402)//Enroque de blancas, lado de la reina
        {
            //es enroque valido y los cuadros intermedios estan vacios?
            if(misc&0x02 && ((tablero[0]|tablero[1])&0x000000000000000e)==0 )
            {
                //revisar si los cuadros interiores estan siendo atacados
                //    int cuadro_bajo_ataque(int x, int y, char atacante);
                if(cuadro_bajo_ataque(4, 0, 'b')==0 && cuadro_bajo_ataque(3, 0, 'b')==0 && cuadro_bajo_ataque(2, 0, 'b')==0)
                {
                    //no, es valido
                    ret=1;
                }
                else
                    ret=0;
            }
            //no, es invalido
            else
                ret=0;
        }
        else
        {
            if(debug==100)
                printf("\nchecando si es movimiento de rey normal...");
            int x1, y1, x2, y2, ladox, ladoy;
            x1=((m>>8)%8);
            y1=((m>>8)/8);
            x2=((m&0x00ff)%8);
            y2=((m&0x00ff)/8);
            ladox=(x2-x1)*(x2-x1);
            ladoy=(y2-y1)*(y2-y1);
            if(debug==100)
                printf("\nDatos de movimiento de rey, x1: %d, y1: %d, x2: %d, y2: %d, ladox: %d, ladoy: %d", x1, y1, x2, y2, ladox, ladoy);
            if(ladox<2 && ladoy<2)
                ret=1;
            else
                ret=0;
        }
    }
    else if(p=='k')
    {
        //se mueve 2 cuadros?
        if(m==0x3c3e)//Enroque de blancas, lado del rey
        {
            //si
            //es enroque valido y los cuadros intermedios estan vacios
            if(misc&0x04 && ((tablero[0]|tablero[1])&0x6000000000000000)==0 )
            {
                //revisar si los cuadros interiores estan siendo atacados
                //    int cuadro_bajo_ataque(int x, int y, char atacante);
                if(cuadro_bajo_ataque(4, 7, 'w')==0 && cuadro_bajo_ataque(5, 7, 'w')==0 && cuadro_bajo_ataque(6, 7, 'w')==0)
                {
                    //printf("\nchecando ataque hacia cuadros de enroque...");
                    //no, es valido
                    ret=1;
                }
                else
                    ret=0;
            }
            //no, es invalido
            else
                ret=0;
        }
        else if(m==0x3c3a)//Enroque de blancas, lado de la reina
        {
            //si
            //es enroque valido y los cuadros intermedios estan vacios
            if(misc&0x08 && ((tablero[0]|tablero[1])&0x0e00000000000000)==0 )
            {
                //revisar si los cuadros interiores estan siendo atacados
                //    int cuadro_bajo_ataque(int x, int y, char atacante);
                if(cuadro_bajo_ataque(4, 7, 'w')==0 && cuadro_bajo_ataque(3, 7, 'w')==0 && cuadro_bajo_ataque(2, 7, 'w')==0)
                {
                    //no, es valido
                    ret=1;
                }
                else
                    ret=0;
            }
            //no, es invalido
            else
                return 0;
        }
        else
        {
            if(debug==100)
                printf("\nchecando si es movimiento de rey normal...");
            int x1, y1, x2, y2, ladox, ladoy;
            x1=((m>>8)%8);
            y1=((m>>8)/8);
            x2=((m&0x00ff)%8);
            y2=((m&0x00ff)/8);
            ladox=(x2-x1)*(x2-x1);
            ladoy=(y2-y1)*(y2-y1);
            if(debug==100)
                printf("\nDatos de movimiento de rey, x1: %d, y1: %d, x2: %d, y2: %d, ladox: %d, ladoy: %d", x1, y1, x2, y2, ladox, ladoy);
            if(ladox<2 && ladoy<2)
                ret=1;
            else
                ret=0;
        }
    }
    //Si el final es cualquier rey, no sirve el movimiento (a menos que se permita mediante la bandera) Y ADEMAS ES UN JAQUE AL OPONENTE
    if((piezas[m&0x00ff]&(rey[0]|rey[1])) && king_capture_allowed==0)
    {
        if (debug==100)
            printf(" M2: %c", p);
        ret=0;
        //marcar esta posicion como un jaque
        //check_beam|=beam;
    }
    return ret;
}

inline unsigned int CEstadoDelJuego::es_ataque_valido(unsigned int m, char p, int king_capture_allowed, int query)
{
    char test = (p>='a' && p<='z')?'b':'w';
    int debug=0;
    int ret=0;
    uint64_t beam = 0;
    //debug=100;
    //51127244
    if (debug==100)
    {   
        printf("\nComienza validacion...(%c), color: %c", p, test);
        printf("\nMovimiento es: 0x%.4x", m);
    }
    //se checa el tipo de pieza
    //revisamos si hay alguna pieza atravesandose
    else if(p=='P')
    {
        if (debug==100)
            printf(" M5: %c", p);
        //si hay pieza en diagonal, puede moverse ahi
		else if(peon_white_takes[(m)>>8][m&0x00ff]&(piezas[m&0x00ff]))
		{
            if (debug==100)
                printf(" M5.2 ");
            ret=1;
		}
    }
    else if(p=='p')
    {
        if (debug==100)
            printf(" M5: %c", p);
        //si hay pieza en diagonal, puede moverse ahi
		else if(peon_black_takes[(m)>>8][m&0x00ff]&(piezas[m&0x00ff]))
		{
            if (debug==100)
                printf(" M5.2 ");
            ret=1;
		}
    }
    else if((p|0x20)=='r')
    {
        if (torre_exclusive[(m)>>8][m&0x00ff]&(tablero[0]|tablero[1]))
        {
            ret=0;
        }
        else
            ret=1;
    }
    else if((p|0x20)=='n')
    {
            ret=1;
        //siempre es valido
    }
    else if((p|0x20)=='b')
    {
        if (alfil_exclusive[(m)>>8][m&0x00ff]&(tablero[0]|tablero[1]))
        {
            ret=0;
        }
        else 
            ret=1;
    }
    else if((p|0x20)=='q')
    {
        if (debug==100)
            printf(" M5: %c, revisando reina", p);
        if (dama_exclusive[(m)>>8][m&0x00ff]&(tablero[0]|tablero[1]))
        {
            if (debug==100)
                printf(" M5: %c, movimiento de reina no valido!", p);
            ret=0;
        }
        else
            ret=1;
    }
    else if(p=='K')
    {
        //se mueve 2 cuadros?
        if(m==0x0406)//Enroque de blancas, lado del rey
        {
            //es enroque valido y los cuadros intermedios estan vacios
            if(misc&0x01 && ((tablero[0]|tablero[1])&0x0000000000000060)==0 )
            {
                //revisar si los cuadros interiores estan siendo atacados
                //    int cuadro_bajo_ataque(int x, int y, char atacante);
                if(cuadro_bajo_ataque(4, 0, 'b')==0 && cuadro_bajo_ataque(5, 0, 'b')==0 && cuadro_bajo_ataque(6, 0, 'b')==0)
                {
                    //no, es valido
                    ret=1;
                }
                else
                    ret=0;
            }
            //no, es invalido
            else 
                ret=0;
        }
        else if(m==0x0402)//Enroque de blancas, lado de la reina
        {
            //es enroque valido y los cuadros intermedios estan vacios?
            if(misc&0x02 && ((tablero[0]|tablero[1])&0x000000000000000e)==0 )
            {
                //revisar si los cuadros interiores estan siendo atacados
                //    int cuadro_bajo_ataque(int x, int y, char atacante);
                if(cuadro_bajo_ataque(4, 0, 'b')==0 && cuadro_bajo_ataque(3, 0, 'b')==0 && cuadro_bajo_ataque(2, 0, 'b')==0)
                {
                    //no, es valido
                    ret=1;
                }
                else
                    ret=0;
            }
            //no, es invalido
            else
                ret=0;
        }
        else
        {
            if(debug==100)
                printf("\nchecando si es movimiento de rey normal...");
            int x1, y1, x2, y2, ladox, ladoy;
            x1=((m>>8)%8);
            y1=((m>>8)/8);
            x2=((m&0x00ff)%8);
            y2=((m&0x00ff)/8);
            ladox=(x2-x1)*(x2-x1);
            ladoy=(y2-y1)*(y2-y1);
            if(debug==100)
                printf("\nDatos de movimiento de rey, x1: %d, y1: %d, x2: %d, y2: %d, ladox: %d, ladoy: %d", x1, y1, x2, y2, ladox, ladoy);
            if(ladox<2 && ladoy<2)
                ret=1;
            else
                ret=0;
        }
    }
    else if(p=='k')
    {
        //se mueve 2 cuadros?
        if(m==0x3c3e)//Enroque de blancas, lado del rey
        {
            //si
            //es enroque valido y los cuadros intermedios estan vacios
            if(misc&0x04 && ((tablero[0]|tablero[1])&0x6000000000000000)==0 )
            {
                //revisar si los cuadros interiores estan siendo atacados
                //    int cuadro_bajo_ataque(int x, int y, char atacante);
                if(cuadro_bajo_ataque(4, 7, 'w')==0 && cuadro_bajo_ataque(5, 7, 'w')==0 && cuadro_bajo_ataque(6, 7, 'w')==0)
                {
                    printf("\nchecando ataque hacia cuadros de enroque...");
                    //no, es valido
                    ret=1;
                }
                else
                    ret=0;
            }
            //no, es invalido
            else
                ret=0;
        }
        else if(m==0x3c3a)//Enroque de blancas, lado de la reina
        {
            //si
            //es enroque valido y los cuadros intermedios estan vacios
            if(misc&0x08 && ((tablero[0]|tablero[1])&0x0e00000000000000)==0 )
            {
                //revisar si los cuadros interiores estan siendo atacados
                //    int cuadro_bajo_ataque(int x, int y, char atacante);
                if(cuadro_bajo_ataque(4, 7, 'w')==0 && cuadro_bajo_ataque(3, 7, 'w')==0 && cuadro_bajo_ataque(2, 7, 'w')==0)
                {
                    //no, es valido
                    ret=1;
                }
                else
                    ret=0;
            }
            //no, es invalido
            else
                return 0;
        }
        else
        {
            if(debug==100)
                printf("\nchecando si es movimiento de rey normal...");
            int x1, y1, x2, y2, ladox, ladoy;
            x1=((m>>8)%8);
            y1=((m>>8)/8);
            x2=((m&0x00ff)%8);
            y2=((m&0x00ff)/8);
            ladox=(x2-x1)*(x2-x1);
            ladoy=(y2-y1)*(y2-y1);
            if(debug==100)
                printf("\nDatos de movimiento de rey, x1: %d, y1: %d, x2: %d, y2: %d, ladox: %d, ladoy: %d", x1, y1, x2, y2, ladox, ladoy);
            if(ladox<2 && ladoy<2)
                ret=1;
            else
                ret=0;
        }
    }
    //Si el final es cualquier rey, no sirve el movimiento (a menos que se permita mediante la bandera) Y ADEMAS ES UN JAQUE AL OPONENTE
    if((piezas[m&0x00ff]&(rey[0]|rey[1])) && king_capture_allowed==0)
    {
        if (debug==100)
            printf(" M2: %c", p);
        ret=0;
        //marcar esta posicion como un jaque
        //check_beam|=beam;
    }
    return ret;
}

unsigned int cadena_a_numero(char *cadena)
{
  unsigned int cntr2;
  unsigned int numero;
  for(cntr2=0;cadena[cntr2]!='\0';cntr2++)
  {
	if(cadena[cntr2]=='\n' && cntr2>=0)
	{
		cadena[cntr2]='\0';
		continue;
	}
    if(cadena[cntr2]<'0' || cadena[cntr2]>'9')
      return -1;
  }
  if (cntr2==0)
	return -2;
  numero = atoi((char *)cadena);
  if(numero>0 && numero <=300)
    return numero;
  else
    return -3;
}

unsigned int convertir_texto_de_entrada_a_movimiento(char *cadena, struct POS *movimiento1, struct POS *movimiento2)
{
	if(strlen(cadena)>6)
	{
		printf("\nTexto de movimiento demasiado largo");
		return 1;
	}	
	movimiento1->x=cadena[0]-'a';
	movimiento1->y=cadena[1]-'1';
	movimiento2->x=cadena[3]-'a';
	movimiento2->y=cadena[4]-'1';
	return 0;
}

void _imprimir_tablero(char t[8][8])
{
   unsigned int columnas, filas;
    printf("\n");
	printf("      \tabcdefgh\n");
    for(filas=7;filas>=0 && filas<=7;filas--)
    {
		printf("      %d ", filas+1);
        for(columnas=0;columnas<8;columnas++)
        {
                printf("%c", t[filas][columnas]);
        }
        printf("\n");
    }
	printf("\n");	
}
// 1. Kb8 Rd8+ 2. Ka7 Re8 3. Ka6 Ra8# [*]

/*
****************************************************************************************************
Mejoras:

	 1. (ok) Dar bono a PEON_WHITEes avanzados
	 2. (ok) Dar bono a PEON_WHITEes centrales b<x<g, 2<y<7, mejor posicion, mejor bono
	 3. (  ) Dar bono a PEON_WHITEes que cuidan al rey
	 4. (  ) Dar bono a enroque
	 5. (  ) Dar bono a mejoras rapidas (velocidad)
	 6. (  ) Dar bono a parejas de caballos y de alfiles
	 7. (  ) Dar bono a piezas atacadas y defendidas
	 8. (  ) Medir en milisegundos
	 9. (  ) Hacer intentos de resolucion reiterativos
	10. (  ) Ordenar las ramas
	11. (  ) Usar tablas de transposicion
	12. (  ) Agregar quiescence
	13. (  ) Convertir matriz a lista
	14. (  ) Agregar opcion de fichas negras
	15. (  ) Agregar opcion de nivel o de tiempo
	16. (  ) Agregar Entrada FEN
	17. (  ) Agregar Retroceder
	18. (  ) Agregar calculo de mejor linea
	19. (  ) Agregar historial
	20. (  ) Agregar mejor movimiento por nivel
***************************************************************************************************
*/

void inicializar_mascaras(void)
{
    int debug=0;
    //debug=100;
    if(debug==100)
        printf("\nInicializando mascaras...");
    int x, y, i, j, m, n;
    uint64_t one, counter_pieces, moves_counter1, moves_counter2;
    one = 1;

    piezas_id[0]='P';
    piezas_id[1]='R';
    piezas_id[2]='N';
    piezas_id[3]='B';
    piezas_id[4]='Q';
    piezas_id[5]='K';
    piezas_id[6]='-';
    piezas_id[7]='-';
    piezas_id[8]='p';
    piezas_id[9]='r';
    piezas_id[10]='n';
    piezas_id[11]='b';
    piezas_id[12]='q';
    piezas_id[13]='k';
    piezas_id[14]='-';
    piezas_id[15]='-';
    
    piezas_valor[0]=100;
    piezas_valor[1]=500;
    piezas_valor[2]=300;
    piezas_valor[3]=300;
    piezas_valor[4]=900;
    piezas_valor[5]=0;
    piezas_valor[6]=0;
    piezas_valor[7]=0;
    piezas_valor[8]=-100;
    piezas_valor[9]=-500;
    piezas_valor[10]=-300;
    piezas_valor[11]=-300;
    piezas_valor[12]=-900;
    piezas_valor[13]=0;
    piezas_valor[14]=0;
    piezas_valor[15]=0;
    //inicializar lista que contiene las posiciones a las que cada pieza puede ir
    for(x=0;x<7;x++)
    {
        for(y=0;y<64;y++)
        {
            for(i=0;i<28;i++)
            {
                movimientos[x][y][i]=NULL;
            }
        }
    }
    
    
    //Inicializar mascaras de peones
    for(counter_pieces=0, y=0;y<8;y++)
    {
        for(x=0;x<8;x++)
        {
            for(j=0, moves_counter1=0, moves_counter2=0;j<8;j++)
            {
                for(i=0;i<8;i++)
                {
                    if(x==i && y==j)
                        continue;
                    counter_pieces++;
                    peon_white_advance[x+y*8][i+j*8]=FILL;
                    peon_black_advance[x+y*8][i+j*8]=FILL;
					peon_white_takes[x+y*8][i+j*8]=0;
					peon_black_takes[x+y*8][i+j*8]=0;
                    if(x==i && j==y+1)
					{
                        if(movimientos[PEON_WHITE][x+y*8][moves_counter1]==NULL) movimientos[PEON_WHITE][x+y*8][moves_counter1] = new struct POS;
                        movimientos[PEON_WHITE][x+y*8][moves_counter1]->x=i;
                        movimientos[PEON_WHITE][x+y*8][moves_counter1]->y=j;
                        moves_counter1++;
                        
						peon_white_advance[x+y*8][i+j*8]=0;
						peon_white_advance[x+y*8][i+j*8]|=(one<<(i+j*8));
					}
                    else if(x==i && y==1 && j==3)
					{
                        if(movimientos[PEON_WHITE][x+y*8][moves_counter1]==NULL) movimientos[PEON_WHITE][x+y*8][moves_counter1] = new struct POS;
                        movimientos[PEON_WHITE][x+y*8][moves_counter1]->x=i;
                        movimientos[PEON_WHITE][x+y*8][moves_counter1]->y=j;
                        moves_counter1++;
                        
						peon_white_advance[x+y*8][i+j*8]=0;
						peon_white_advance[x+y*8][i+j*8]|=(one<<(i+j*8));
						peon_white_advance[x+y*8][i+j*8]|=(one<<(i+(j-1)*8));
					}
                    else if(x==i && j==y-1)
					{
                        if(movimientos[PEON_BLACK][x+y*8][moves_counter2]==NULL) movimientos[PEON_BLACK][x+y*8][moves_counter2] = new struct POS;
                        movimientos[PEON_BLACK][x+y*8][moves_counter2]->x=i;
                        movimientos[PEON_BLACK][x+y*8][moves_counter2]->y=j;
                        moves_counter2++;
                        
						peon_black_advance[x+y*8][i+j*8]=0;
						peon_black_advance[x+y*8][i+j*8]|=(one<<(i+j*8));
					}
                    else if(x==i && y==6 && j==4)
					{
                        if(movimientos[PEON_BLACK][x+y*8][moves_counter2]==NULL) movimientos[PEON_BLACK][x+y*8][moves_counter2] = new struct POS;
                        movimientos[PEON_BLACK][x+y*8][moves_counter2]->x=i;
                        movimientos[PEON_BLACK][x+y*8][moves_counter2]->y=j;
                        moves_counter2++;
                        
						peon_black_advance[x+y*8][i+j*8]=0;
						peon_black_advance[x+y*8][i+j*8]|=(one<<(i+j*8));
						peon_black_advance[x+y*8][i+j*8]|=(one<<(i+(j+1)*8));
					}
					else if(((i==x+1)||(i==x-1)) && (j==y+1))
					{
						//el peon BLANCO hace movimiento para comer
                        if(movimientos[PEON_WHITE][x+y*8][moves_counter1]==NULL) movimientos[PEON_WHITE][x+y*8][moves_counter1] = new struct POS;
                        movimientos[PEON_WHITE][x+y*8][moves_counter1]->x=i;
                        movimientos[PEON_WHITE][x+y*8][moves_counter1]->y=j;
                        moves_counter1++;
						
						peon_white_takes[x+y*8][i+j*8]=0;
						peon_white_takes[x+y*8][i+j*8]|=(one<<(i+j*8));
					}
					else if(((i==x+1)||(i==x-1)) && (j==y-1))
					{
						//el peon NEGRO hace movimiento para comer
                        if(movimientos[PEON_BLACK][x+y*8][moves_counter2]==NULL) movimientos[PEON_BLACK][x+y*8][moves_counter2] = new struct POS;
                        movimientos[PEON_BLACK][x+y*8][moves_counter2]->x=i;
                        movimientos[PEON_BLACK][x+y*8][moves_counter2]->y=j;
                        moves_counter2++;
						peon_black_takes[x+y*8][i+j*8]=0;
						peon_black_takes[x+y*8][i+j*8]|=(one<<(i+j*8));
					}
                }
            }
        }
    }
    if(debug==100)
        std::cout << "\nMascara de PEON ..." << counter_pieces;
    
    //Inicializar mascaras de torres
    for(counter_pieces=0, y=0;y<8;y++)
    {
        for(x=0;x<8;x++)
        {
            for(j=0, moves_counter1=0;j<8;j++)
            {
                for(i=0;i<8;i++)
                {
                    if(x==i && y==j)
                        continue;
                    counter_pieces++;
                    torre_inclusive[x+y*8][i+j*8]=FILL;
                    torre_exclusive[x+y*8][i+j*8]=FILL;
                    if((x==i && y!=j) || (x!=i && y==j))
                    {
                        if(movimientos[TORRE][x+y*8][moves_counter1]==NULL) movimientos[TORRE][x+y*8][moves_counter1] = new struct POS;
                        movimientos[TORRE][x+y*8][moves_counter1]->x=i;
                        movimientos[TORRE][x+y*8][moves_counter1]->y=j;
                        moves_counter1++;
                        
						torre_inclusive[x+y*8][i+j*8]=0;
						torre_exclusive[x+y*8][i+j*8]=0;
                        //x=0
                        //y=7
                        //i=0
                        //j=0
                        for(n=0;n<8;n++)
                        {
                            for(m=0;m<8;m++)
                            {
                                //m=0
                                //n=2
                                //ex: y
                                //in: y
                                if((x==i && x==m) && ((n<y && n>j)||(n<j && n>y)))
								{
                                    torre_inclusive[x+y*8][i+j*8]|=(one<<(m+n*8));
									if(n!=j)
										torre_exclusive[x+y*8][i+j*8]|=(one<<(m+n*8));
								}
                                else if((y==j && y==n) && ((m<x && m>i)||(m<i && m>x)))
								{
                                    torre_inclusive[x+y*8][i+j*8]|=(one<<(m+n*8));
									if(m!=i)
										torre_exclusive[x+y*8][i+j*8]|=(one<<(m+n*8));
								}
                            }
                        }
                    }
                }
            }
        }
    }
    if(debug==100)
        std::cout << "\nMascara de torre inicializada..." << counter_pieces;
    
    //Inicializar mascaras del caballo
	for(counter_pieces=0, y=0;y<8;y++)
	{
		for(x=0;x<8;x++)
		{
            for(j=0, moves_counter1=0;j<8;j++)
            {
                for(i=0;i<8;i++)
                {
                    if(x==i && y==j)
                        continue;
                    int ladox, ladoy;
                    ladox = (x-i)*(x-i);
                    ladoy = (y-j)*(y-j);
					if((ladox==4 && ladoy==1)||(ladox==1 && ladoy==4))
					{
						//printf("-");
						if(movimientos[CABALLO][x+y*8][moves_counter1]==NULL) movimientos[CABALLO][x+y*8][moves_counter1] = new struct POS;
                        movimientos[CABALLO][x+y*8][moves_counter1]->x=i;
                        movimientos[CABALLO][x+y*8][moves_counter1]->y=j;
                        moves_counter1++;
					}
				}
			}

		}
	}
    if(debug==100)
        std::cout << "\nMascara del caballo inicializada..." << counter_pieces;
    
    //Inicializar mascaras de alfiles
    for(counter_pieces=0, y=0;y<8;y++)
    {
        for(x=0;x<8;x++)
        {
            for(j=0, moves_counter1=0;j<8;j++)
            {
                for(i=0;i<8;i++)
                {
                    if(x==i && y==j)
                        continue;
                    counter_pieces++;
                    alfil_inclusive[x+y*8][i+j*8]=FILL;
                    alfil_exclusive[x+y*8][i+j*8]=FILL;
                    int ladox, ladoy;
                    ladox = abs(x-i);
                    ladoy = abs(y-j);
                    if(ladox==ladoy && ladox>0)
                    {
                        if(movimientos[ALFIL][x+y*8][moves_counter1]==NULL) movimientos[ALFIL][x+y*8][moves_counter1] = new struct POS;
                        movimientos[ALFIL][x+y*8][moves_counter1]->x=i;
                        movimientos[ALFIL][x+y*8][moves_counter1]->y=j;
                        moves_counter1++;
                        
						alfil_inclusive[x+y*8][i+j*8]=0;
						alfil_exclusive[x+y*8][i+j*8]=0;
                        int xa, ya, xa_base, ya_base;
                        xa_base = 1;
                        ya_base = 1;
                        
                        if (x>i)
                            xa_base=-1;
                        if (y>j)
                            ya_base=-1;
                        for(n=1;n<=ladox;n++)
                        {
                            xa=n*xa_base;
                            ya=n*ya_base;
                            alfil_inclusive[x+y*8][i+j*8]|=one<<((x+xa)+(y+ya)*8);
                            if((x+xa)!=i && (y+ya)!=j)
                                alfil_exclusive[x+y*8][i+j*8]|=one<<((x+xa)+(y+ya)*8);
                        }
                    }
                }
            }
        }
    }
    if(debug==100)
        std::cout << "\nMascara de alfil inicializada..." << counter_pieces;
    
    int db=0;
    //Inicializar mascaras de la DAMA
    for(counter_pieces=0, y=0;y<8;y++)
    {
        for(x=0;x<8;x++)
        {
            for(j=0, moves_counter1=0;j<8;j++)
            {
                for(i=0;i<8;i++)
                {       
                    if(x==i && y==j)
                        continue;
                    counter_pieces++;
                    dama_inclusive[x+y*8][i+j*8]=FILL;
                    dama_exclusive[x+y*8][i+j*8]=FILL;
                    int ladox, ladoy;
                    ladox = abs(x-i);
                    ladoy = abs(y-j);
                    if(db)
                    {
                        cout << "\n1 dama_exclusive[(m)>>8][m&0x00ff] : " << hex << dama_exclusive[x+y*8][i+j*8];
                        getc(stdin);
                    }
                    if((x==i && y!=j) || (x!=i && y==j))
                    {
                        if(db)
                        {
                            cout << "\n1.1 dama_exclusive[(m)>>8][m&0x00ff] : " << hex << dama_exclusive[x+y*8][i+j*8];
                            getc(stdin);
                        }
                        if(movimientos[DAMA][x+y*8][moves_counter1]==NULL) movimientos[DAMA][x+y*8][moves_counter1] = new struct POS;
                        movimientos[DAMA][x+y*8][moves_counter1]->x=i;
                        movimientos[DAMA][x+y*8][moves_counter1]->y=j;
                        moves_counter1++;
                        
						dama_inclusive[x+y*8][i+j*8]=0;
						dama_exclusive[x+y*8][i+j*8]=0;
                        for(n=0;n<8;n++)
                        {
                            for(m=0;m<8;m++)
                            {
                                if((x==i && x==m) && ((n<y && n>j)||(n<j && n>y)))
								{
                                    dama_inclusive[x+y*8][i+j*8]|=(one<<(m+n*8));
									if(n!=j)
										dama_exclusive[x+y*8][i+j*8]|=(one<<(m+n*8));
								}
                                else if((y==j && y==n) && ((m<x && m>i)||(m<i && m>x)))
								{
                                    dama_inclusive[x+y*8][i+j*8]|=(one<<(m+n*8));
									if(m!=i)
										dama_exclusive[x+y*8][i+j*8]|=(one<<(m+n*8));
								}
                            }
                        }
                    }
                    else if(ladox==ladoy && ladox>0)
                    {
                        if(db)
                        {
                            cout << "\n1.2 dama_exclusive[(m)>>8][m&0x00ff] : " << hex << dama_exclusive[x+y*8][i+j*8];
                            getc(stdin);
                        }
                        if(movimientos[DAMA][x+y*8][moves_counter1]==NULL) movimientos[DAMA][x+y*8][moves_counter1] = new struct POS;
                        movimientos[DAMA][x+y*8][moves_counter1]->x=i;
                        movimientos[DAMA][x+y*8][moves_counter1]->y=j;
                        moves_counter1++;
                        
						dama_inclusive[x+y*8][i+j*8]=0;
						dama_exclusive[x+y*8][i+j*8]=0;
                        int xa, ya, xa_base, ya_base;
                        xa_base = 1;
                        ya_base = 1;
                        
                        if (x>i)
                            xa_base=-1;
                        if (y>j)
                            ya_base=-1;
                        for(n=1;n<=ladox;n++)
                        {
                            xa=n*xa_base;
                            ya=n*ya_base;
                            dama_inclusive[x+y*8][i+j*8]|=(one<<((x+xa)+(y+ya)*8));
                            if((x+xa)!=i && (y+ya)!=j)
                                dama_exclusive[x+y*8][i+j*8]|=(one<<((x+xa)+(y+ya)*8));
                            if(db)
                            {
                                cout << "\n2 dama_exclusive[(m)>>8][m&0x00ff] : " << hex << dama_exclusive[x+y*8][i+j*8];
                                printf(" --- xa: %d, ya: %d", xa, ya);
                                getc(stdin);
                            }
                        }
                    }
                    if(db)
                    {
                        cout << "\n3 dama_exclusive[(m)>>8][m&0x00ff] : " << hex << dama_exclusive[x+y*8][i+j*8];
                        getc(stdin);
                    }
                }
            }
        }
    }   
    if(debug==100)
        std::cout << "\nMascara de la DAMA inicializada..." << counter_pieces;
    
    //Inicializar mascaras del rey
    for(counter_pieces=0, y=0;y<8;y++)
    {
        for(x=0;x<8;x++)
        {
            for(j=0, moves_counter1=0;j<8;j++)
            {
                for(i=0;i<8;i++)
                {
                    if(x==i && y==j)
                        continue;
                    int db=0;
                    
                    counter_pieces++;
                    rey_inclusive[x+y*8][i+j*8]=FILL;
                    int ladox, ladoy;
                    ladox = abs(x-i);
                    ladoy = abs(y-j);
                    if((ladox<2 && ladoy<2 ) || (y==0 && j==0 && ((x==4 && i==2)||(x==4 && i==6)) ) || (y==7 && j==7 && ((x==4 && i==2)||(x==4 && i==6)) ))
                    {
                        if(x==i && y==j)
                            continue;
                        if(movimientos[REY][x+y*8][moves_counter1]==NULL) movimientos[REY][x+y*8][moves_counter1] = new struct POS;
                        movimientos[REY][x+y*8][moves_counter1]->x=i;
                        movimientos[REY][x+y*8][moves_counter1]->y=j;
                        moves_counter1++;
                        
                        if (db)
                            printf("\n%d: %c%d", moves_counter1, movimientos[REY][m>>8][moves_counter1]->x+'a', movimientos[REY][m>>8][moves_counter1]->y+1);
                        
						rey_inclusive[x+y*8][i+j*8]=0;
                        rey_inclusive[x+y*8][i+j*8]|=one<<(x+y*8);
                    }
                }
            }
        }
    }
    if(debug==100)
        std::cout << "\nMascara del rey inicializada..." << counter_pieces;
    
    
    
    
    //inicializar mascara que contiene la posicion de las piezas
    for(i=0;i<64;i++)
    {
        piezas[i]=0;
        piezas[i]|=one<<i;
    }
    for(y=0;y<8;y++)
    {
        for(x=0;x<8;x++)
        {
            piezas2d[y][x]=(one<<(x+y*8));
        }
    }
    
}


void copiar_matriz(char m1[8][8], char m2[8][8])
{
    int i, j;
    for(j=0;j<8;j++)
    {
        for(i=0;i<8;i++)
        {
            m2[j][i]=m1[j][i];
        }
    }
}

void ayuda(CEstadoDelJuego *ts)
{
    ;
/*	
	:::::::::::::::::::::::::::::::::::::::
	Durante juego:
	
		atras
		adelante
		sugerir
		extra
		debug
		alpha
		salir
		nuevo
		imprimir 	fen
					juego
		cambiar		color
					nivel
					tiempo
					
*/
    int entrada=1000;
    char str[81], trash[81];
    while(entrada!=0)
    {
        printf("\n\nAyuda:");
        printf("\n\nOpciones:\n");
        printf("\n0. Salir de la Ayuda");
        printf("\n1. Introducir cadena FEN");
        printf("\n2. Humano (blancas) vs maquina (negras)");
        printf("\n3. Maquina (blancas) vs humano (negras)");
        printf("\n4. Maquina vs Maquina");
        printf("\n5. Humano vs Humano\n");
        printf("\n\n Por favor seleccione una opcion: ");
        fgets(str, sizeof(str), stdin);
        str[strlen(str)-1]='\0';
        if(strcmp("0", str)==0)
        {
            return;
        }
        else if(strcmp("1", str)==0)
        {
            printf("\n cadena FEN: ");
            char fen[81];
            fgets(fen, sizeof(fen), stdin);
            //int inicializar_desde_fen(CEstadoDelJuego *ts, char *fen);
            inicializar_desde_fen(ts, fen);
        }
        else if(strcmp("2", str)==0)
        {
            blancas=HUMANO;
            negras=MAQUINA;
            printf( "\nAhora Humano: blancas, Shallow: negras" );
            printf( "\nPresione Enter para continuar:");
            fgets(trash, sizeof(trash), stdin);
        }
        else if(strcmp("3", str)==0)
        {
            blancas=MAQUINA;
            negras=HUMANO;
            printf( "\nAhora Humano: negras, Shallow: blancas" );
            printf( "\nPresione Enter para continuar:");
            fgets(trash, sizeof(trash), stdin);
        }
        else if(strcmp("4", str)==0)
        {
            blancas=MAQUINA;
            negras=MAQUINA;
            printf( "\nAhora Shallow1: blancas, Shallow2: negras" );
            printf( "\nPresione Enter para continuar:");
            fgets(trash, sizeof(trash), stdin);
        }
        else if(strcmp("5", str)==0)
        {
            blancas=HUMANO;
            negras=HUMANO;
            printf( "\nAhora Humano1: blancas, Humano2: negras" );
            printf( "\nPresione Enter para continuar:");
            fgets(trash, sizeof(trash), stdin);
        }
        else
        {
            printf ("\n ERROR: La entrada no corresponde a una opcion valida.\n");
        }
    }
    
    
    /*
    while entrada!='0':
        
        print "\nEsta es la ayuda del programa!"
        print("\nOpciones:\n")
        print("0. Salir de la Ayuda")
        print("1. Introducir cadena FEN")
        print("2. Humano (blancas) vs maquina (negras)")
        print("3. Maquina (blancas) vs humanoo (negras)")
        print("4. Maquina vs Maquina\n")
        
        entrada_del_usuario = raw_input("Por favor seleccione una opcion: ")
        if entrada_del_usuario=='0':
            break;
        ###### opcion 1 ######
        if entrada_del_usuario=='1':
            while True:
                fenstr = raw_input("Cadena FEN (o '0' para cancelar): ")
                if(fenstr=='0'):
                    print "Construccion de la cadena FEN cancelada!"
                    break
                else:
                    if(generar_desde_fen(tablero, fenstr)==0):
                        print "Cadena FEN aceptada. Construccion de la configuracion de tablero terminada."
                        break
                    else:
                        print "La cadena FEN es invalida!"
            raw_input("Presione Enter para continuar: ")
        #========= end =============
        
    */
}
/*

clock_t temporizador_start[10];
clock_t temporizador_stop[10];
clock_t temporizador_value[10];

*/

void resettimer(int timer_ID)
{
    temporizador_value[timer_ID]=0;
}

void starttimer(int timer_ID)
{
    temporizador_start[timer_ID]=clock();
}

void stoptimer(int timer_ID)
{
    temporizador_stop[timer_ID]=clock();
    temporizador_value[timer_ID]+=(temporizador_stop[timer_ID]-temporizador_start[timer_ID]);
    printf("\nstart: %d, stop: %d, value: %d", temporizador_start[timer_ID], temporizador_stop[timer_ID], temporizador_value[timer_ID]);
}

float gettimer(int timer_ID)
{
    cout << "\ntemporizador_value: " << temporizador_value[timer_ID] << " CLOCKS_PER_SEC: " << CLOCKS_PER_SEC;
	float seconds = (float)((float)temporizador_value[timer_ID]) / CLOCKS_PER_SEC;
	return seconds;
}
/*
void StartCounter()
{
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
	cout << "QueryPerformanceFrequency failed!\n";

    PCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}
double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}
*/
void ResetCounter()
{
    _CounterValue=0;
}
void StartCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    _CounterStart = li.QuadPart;
}
void StopCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    _CounterValue+=(li.QuadPart-_CounterStart);
}
double GetCounter()
{
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
	cout << "QueryPerformanceFrequency failed!\n";

    PCFreq = double(li.QuadPart)/1000.0;
    return double(_CounterValue)/PCFreq;    
}


const char *tami(unsigned long int n, int size)
{
    if(size==0)
    {
        if(n<10)
            return "    ";
        else if(n<100)
            return "   ";
        else if(n<1000)
            return "  ";
        else if(n<10000)
            return " ";
        else if(n<100000)
            return "";
        else
            return "";
    }
    else
    {
        if(n<10)
            return "         ";
        else if(n<100)
            return "        ";
        else if(n<1000)
            return "       ";
        else if(n<10000)
            return "      ";
        else if(n<100000)
            return "     ";
        else if(n<1000000)
            return "    ";
        else if(n<10000000)
            return "   ";
        else if(n<100000000)
            return "  ";
        else if(n<1000000000)
            return " ";
        else
            return "";
    }
}
const char *tamf(float n, int size)
{
    if(size==0)
    {
        if(n<10)
            return "    ";
        else if(n<100)
            return "   ";
        else if(n<1000)
            return "  ";
        else if(n<10000)
            return " ";
        else if(n<100000)
            return "";
        else
            return "";
    }
    else
    {
        if(n<10)
            return "         ";
        else if(n<100)
            return "        ";
        else if(n<1000)
            return "       ";
        else if(n<10000)
            return "      ";
        else if(n<100000)
            return "     ";
        else if(n<1000000)
            return "    ";
        else if(n<10000000)
            return "   ";
        else if(n<100000000)
            return "  ";
        else if(n<1000000000)
            return " ";
        else
            return "";
    }
}