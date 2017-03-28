#include "biblioteca.h"
#include "masks.h"
//d5-e6, e7-c5, e6-e7, h8-f8 (menos nodos)
//d5-e6, e7-c5, e6-e7, 
int main()
{
    printf("\n                                                                 ");
    printf("\n        _____ _           _ _                 ____  _            ");
    printf("\n       / ____| |         | | |               |  _ \\| |           ");
    printf("\n      | (___ | |__   __ _| | | _____      __ | |_) | |_   _  ___ ");
    printf("\n       \\___ \\| '_ \\ / _` | | |/ _ \\ \\ /\\ / / |  _ <| | | | |/ _ \\");
    printf("\n       ____) | | | | (_| | | | (_) \\ V  V /  | |_) | | |_| |  __/");
    printf("\n      |_____/|_| |_|\\__,_|_|_|\\___/ \\_/\\_/   |____/|_|\\__,_|\\___| ");
    printf("\n                         ___          __                         ");
    printf("\n                        |__  \\       / _ \\                         ");
    printf("\n                          __) |     | | | |                      ");
    printf("\n                         |__ <      | | | |                      ");
    printf("\n                         ___) |  _  | |_| |                       ");
    printf("\n                        |____/  (_)  \\___/               (2013)");
    printf("\n\n\n");
                    
                    
                    
    int i;
    //blancas=MAQUINA;
    //negras=HUMANO;
    blancas=HUMANO;
    negras=MAQUINA;
    contador=0;
    profundidad_objetivo=5;//impares:buenos
    ab=1;
    int debug=0;
    //debug=100;
    //d2-d4, b7-b6, c2-c4, e7-e6, d4-d5, b6-b5, c4-c5, f8-c5 X
    //r1bq1b1r/pppp2k1/2k4p/4P1KQ/3P4/4p2P/PPP3P1/RK3RK1 b - 1 0
    
    inicializar_mascaras();
    struct PIEZA a;
    class CEstadoDelJuego b;
    POS c;
    if(debug==100)
        printf("\nCPieza: %d B, CEstadoDelJuego: %d B, POS: %d", sizeof(a), sizeof(b), sizeof(c));
	class CEstadoDelJuego ts;
	//printf("\nstruct CEstadoDelJuego ts ===>  %d", sizeof(class CEstadoDelJuego));
	//return 0;
	ts.alpaso.x=0;
	ts.alpaso.y=0;
    ts.sig='w';
    ts.inicializar(1);
    ts.evaluar();
    
    /*
    2bqkbn1/2pppp2/np2N3/r3P1p1/p2N2B1/5Q2/PPPPKPP1/RNB2r2 w KQkq - 0 1
8/6K1/1p1B1RB1/8/2Q5/2n1kP1N/3b4/4n3 w - - 0 1
B7/K1B1p1Q1/5r2/7p/1P1kp1bR/3P3R/1P1NP3/2n5 w - - 0 1
8/5p2/5Q2/8/8/3K4/8/1kN5 w - - 0 1
n6k/7b/Q4Kp1/8/8/8/8/8 w - - 0 1
8/8/8/3K4/8/5nP1/5Q1b/5N1k w - - 0 1
2nB4/r3q3/N3pN2/4kp2/3R1R2/7B/4b2K/6Q1 w - - 0 1
7Q/6p1/6kN/4P3/5KP1/8/8/8 w - - 0 1
1K3B2/4n3/4P3/1Pk1N2R/P1B2nR1/3P4/1P3P1Q/2N5 w - - 0 1
    */
    
    
    //if (inicializar_desde_fen(&ts, "r1bq1b1r/pppp2k1/2n4p/4P1NQ/3P4/4p2P/PPP3P1/RN3RK1 b - - 0 0")<0)
    //if(inicializar_desde_fen(&ts, "K7/8/k7/8/2r/8/8/8 b - - 0 0")<0)
    //if(inicializar_desde_fen(&ts, "5Q2/5B1k/6r1/6p1/6N1/6K1/8/8 w - - 0 0")<0)
    //if(inicializar_desde_fen(&ts, "1K3B2/4n3/4P3/1Pk1N2R/P1B2nR1/3P4/1P3P1Q/2N5 w - - 0 0")<0)
    //if(inicializar_desde_fen(&ts, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")<0)
    //if(inicializar_desde_fen(&ts, "1k6/8/2K1R3/8/8/8/8/8 b - - 0 1")<0)
    //if(inicializar_desde_fen(&ts, "1kP5/2P5/KP6/PP6/8/8/8/8 b - - 0 1")<0)
    //if(inicializar_desde_fen(&ts, "1k6/8/K7/8/8/8/8/8 b - - 0 1")<0)
    char inicio[81];
    //8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 1 0
    strcpy(inicio, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    //strcpy(inicio, "k7/8/8/8/7P/8/p7/7K w - - 1 0");
    //strcpy(inicio, "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1");
    if(inicializar_desde_fen(&ts, inicio)<0)
    {
        printf("\nError en la cadena FEN ... ");
        return 0;
    }
    //
    turno_global=1;
	char movimiento[81];
    struct POS mov1, mov2;
    unsigned long game=0;
    printf("\n blancas: %s", (blancas==HUMANO?"HUMANO":"MAQUINA"));
    printf("\n negras: %s", (negras==HUMANO?"HUMANO":"MAQUINA"));
    do
    {
        printf("\n");
        if(blancas==HUMANO && negras==MAQUINA)
        {    
            if(ts.sig=='w')
            {
                printf("\n\t=========   Piezas blancas   ===========\n");
                printf("\n\tPresiona 1 para ayuda, 0 para salir.\n");
                
                ts.imprimir_tablero();
                //preguntar movimiento
                printf("\n  Movimiento (p.e. a2-a3): ");
                fgets(movimiento, sizeof(movimiento), stdin);
				if(strcmp("\n", movimiento)==0)
					fgets(movimiento, sizeof(movimiento), stdin);
				//printf("\nMovimiento es \'%s\'", movimiento);
                movimiento[strlen(movimiento)-1]='\0';
                int res= ts.examinar_entrada(movimiento);
                if(res==0)
				{
                    return 0; //si la opcion elegida es 0, salir
				}
                else if(res==1)            //si la opcion elegida es 1, abrir el menu de ayuda
                {
                    ayuda(&ts);
                    continue;
                }
                else if(res==2)
				{
                    continue;
				}
                if(convertir_texto_de_entrada_a_movimiento(movimiento, &mov1, &mov2)==0) //cualquier otra entrada, checar si es movimiento valido
                {
                    //validar el movimiento
                    if(aplicar_movimiento(ts, mov1, mov2, 1)==0)
                    {
                        //si es valido, ejecutar el movimiento
                        printf("\nEjecutando movimiento....");
                    }
                    else
                    {
                        //si no es valido, imprimir mensaje de error y preguntar de nuevo
                        printf("\nError en la entrada, presione enter para continuar....");
                        fgets(movimiento, sizeof(movimiento), stdin);
                        continue;
                    }
                }
                else //si no, presentar mensaje de error y continuar pidiendo mensaje
                    printf("\nError en el movimiento o comando, por favor intente de nuevo.");
                ts.sig='b';
                turno_global++;
            }
            if(ts.sig=='b')
            {
                printf("\n\t=========   Piezas negras   ===========\n");
                ts.imprimir_tablero();
                ts.procesar(game);
                if(game!=0)
                    break;
                ts.sig='w';
                turno_global++;
            }
        }
        else if(blancas==MAQUINA && negras==HUMANO)
        {    
            if(ts.sig=='b')
            {
                printf("\n\t=========   Piezas negras   ===========\n");
                printf("\n\tPresiona 1 para ayuda, 0 para salir.\n");
                ts.imprimir_tablero();
                //preguntar movimiento
                printf("\n  Movimiento (p.e. a2-a3): ");
                fgets(movimiento, sizeof(movimiento), stdin);
				if(strcmp("\n", movimiento)==0)
					fgets(movimiento, sizeof(movimiento), stdin);
                movimiento[strlen(movimiento)-1]='\0';
                int res= ts.examinar_entrada(movimiento);
                if(res==0)
                    return 0; //si la opcion elegida es 0, salir
                else if(res==1)            //si la opcion elegida es 1, abrir el menu de ayuda
                {
                    ayuda(&ts);
                    continue;
                }
                else if(res==2)
                    continue;
                else if(convertir_texto_de_entrada_a_movimiento(movimiento, &mov1, &mov2)==0) //cualquier otra entrada, checar si es movimiento valido
                {
                    //validar el movimiento
                    if(aplicar_movimiento(ts, mov1, mov2, 1)==0)
                    {
                        //si es valido, ejecutar el movimiento
                        printf("\nEjecutando movimiento....");
                    }
                    else
                    {
                        //si no es valido, imprimir mensaje de error y preguntar de nuevo
                        printf("\nError en la entrada, presione enter para continuar....");
                        fgets(movimiento, sizeof(movimiento), stdin);
                        continue;
                    }
                }
                else //si no, presentar mensaje de error y continuar pidiendo mensaje
                    printf("\nError en el movimiento o comando, por favor intente de nuevo.");
                ts.sig='w';
                turno_global++;
            }
            if(ts.sig=='w')
            {
                printf("\n\t=========   Piezas blancas   ===========\n");
                ts.imprimir_tablero();
                ts.procesar(game);
                if(game!=0)
                    break;
                ts.sig='b';
                turno_global++;
            }
        }
        else if(blancas==MAQUINA && negras==MAQUINA)
        {
            if(ts.sig=='w')
            {
                printf("\n\t=========   Piezas blancas   ===========\n");
                ts.imprimir_tablero();
                //Ejecutar movimiento de las piezas blancas
                ts.procesar(game);
                if(game!=0)
                    break;
                ts.sig='b';
                turno_global++;
            }
            if(ts.sig=='b')
            {
                printf("\n\t=========   Piezas negras   ===========\n");
                ts.imprimir_tablero();
                //Ejecutar movimiento de las piezas negras
                ts.procesar(game);
                if(game!=0)
                    break;
                ts.sig='w';
                turno_global++;
            }
        }
        else if(blancas==HUMANO && negras==HUMANO)
        {
            if(ts.sig=='w')
            {
                printf("\n\t=========   Piezas blancas   ===========\n");
                printf("\n\tPresiona 1 para ayuda, 0 para salir.\n");
                ts.imprimir_tablero();
                //preguntar movimiento
                printf("\n  Movimiento (p.e. a2-a3): ");
				if(strcmp("\n", movimiento)==0)
					fgets(movimiento, sizeof(movimiento), stdin);
                fgets(movimiento, sizeof(movimiento), stdin);
                movimiento[strlen(movimiento)-1]='\0';
                int res= ts.examinar_entrada(movimiento);
                if(res==0)
                    return 0; //si la opcion elegida es 0, salir
                else if(res==1)            //si la opcion elegida es 1, abrir el menu de ayuda
                {
                    ayuda(&ts);
                    continue;
                }
                else if(res==2)
                    continue;
                else if(convertir_texto_de_entrada_a_movimiento(movimiento, &mov1, &mov2)==0) //cualquier otra entrada, checar si es movimiento valido
                {
                    //validar el movimiento
                    if(aplicar_movimiento(ts, mov1, mov2, 1)==0)
                    {
                        //si es valido, ejecutar el movimiento
                        printf("\nEjecutando movimiento....");
                    }
                    else
                    {
                        //si no es valido, imprimir mensaje de error y preguntar de nuevo
                        printf("\nError en la entrada, presione enter para continuar....");
                        fgets(movimiento, sizeof(movimiento), stdin);
                        continue;
                    }
                }
                else //si no, presentar mensaje de error y continuar pidiendo mensaje
                    printf("\nError en el movimiento o comando, por favor intente de nuevo.");
                ts.sig='b';
                turno_global++;
            }
            if(ts.sig=='b')
            {
                printf("\n\t=========   Piezas negras   ===========\n");
                printf("\n\tPresiona 1 para ayuda, 0 para salir.\n");
                ts.imprimir_tablero();
                //preguntar movimiento
                printf("\n  Movimiento (p.e. a2-a3): ");
                fgets(movimiento, sizeof(movimiento), stdin);
                movimiento[strlen(movimiento)-1]='\0';
                int res= ts.examinar_entrada(movimiento);
                if(res==0)
                    return 0; //si la opcion elegida es 0, salir
                else if(res==1)            //si la opcion elegida es 1, abrir el menu de ayuda
                {
                    ayuda(&ts);
                    continue;
                }
                else if(res==2)
                    continue;
                else if(convertir_texto_de_entrada_a_movimiento(movimiento, &mov1, &mov2)==0) //cualquier otra entrada, checar si es movimiento valido
                {
                    //validar el movimiento
                    if(aplicar_movimiento(ts, mov1, mov2, 1)==0)
                    {
                        //si es valido, ejecutar el movimiento
                        printf("\nEjecutando movimiento....");
                    }
                    else
                    {
                        //si no es valido, imprimir mensaje de error y preguntar de nuevo
                        printf("\nError en la entrada, presione enter para continuar....");
                        fgets(movimiento, sizeof(movimiento), stdin);
                        continue;
                    }
                }
                else //si no, presentar mensaje de error y continuar pidiendo mensaje
                    printf("\nError en el movimiento o comando, por favor intente de nuevo.");
                ts.sig='w';
                turno_global++;
            }
        }
        else
        {
            printf("\nOpcion no valida. Error.");
            return 0;
        }
    }while(1);
    if(game==1)
    {
        printf("\nJAQUE MATE!!");
    }
    printf("\n\nGracias por jugar con Shallow Blue!\n\n");
    return 0;
}

/*
SHALLOW BLUE IMPROVEMENTS (current 3.0)


	3.1		Se comienza a usar calculo dinámico de jaques

	3.2		Se comienza a usar ordenamiento de nodos

	3.3		Se usan tablas de transposicion
	
	3.4		Se quita la recursion

*/