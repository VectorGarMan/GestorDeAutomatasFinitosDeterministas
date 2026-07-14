/*
 * =============================================================================
 * Archivo      : main.c
 * Proyecto     : Gestor de Automatas Finitos Deterministas
 * Autor        : Victor Garza Maldonado
 * Fecha        : Julio 2026
 * Revision     : 00.01
 * =============================================================================
 * Descripcion:
 *   Punto de entrada del programa. Integra y coordina todos los modulos:
 *   menu, registro de AFD, procesamiento de palabras, estadisticas y control
 *   de senales.
 *
 *   Flujo principal (derivado de la notacion formal del ERS):
 *
 *     configurarManejoSenales()           [RF-16]
 *     REPETIR:
 *       op = pedirOpcionMenu()            [RF-01]
 *       SI op = S -> RF-14 + RF-15 -> FIN
 *       SI op = T -> Registrar AFDT        [RF-02..RF-08]
 *       SI op = V -> Registrar AFDV        [RF-02, RF-04..RF-07]
 *       REPETIR:
 *         alfa = capturarPalabra()         [RF-09]
 *         procesarPalabra()               [RF-10, RF-11]
 *         actualizarEstadisticasAFD()     [RF-13]
 *         resp_pal = pedirConfirmacion()  [RF-12]
 *       HASTA resp_pal = N
 *       mostrarEstadisticasAFD()          [RF-13]
 *       acumularEstadisticasGlobales()    [RF-14]
 *       liberarAFD()                      [RNF-02]
 *       resp_afd = pedirConfirmacion()    [RF-12]
 *     HASTA resp_afd = N
 *     mostrarEstadisticasGlobales()       [RF-14]
 *     pausaTecnica()                      [RF-14]
 *     "Gracias, adios!"                   [RF-15]
 *
 *   Requerimientos cubiertos: RF-01 a RF-16, RNF-01 a RNF-14
 * =============================================================================
 */

#include <stdlib.h>

#include "../include/afd.h"
#include "../include/menu.h"
#include "../include/estadisticas.h"
#include "../include/controles.h"

/* -------------------- */
/*  Prototipo interno   */
/* -------------------- */

/*
 * cicloAFD -- RF-02 a RF-13
 * Proposito : Gestionar el ciclo completo de un AFD: registro, captura de
 *             palabras y estadisticas locales.
 * Parametros: tipoAFD -- 'T' para AFDT, 'V' para AFDV
 *             global  -- puntero a los contadores globales de sesion
 * Retorna   : (void)
 */
static void cicloAFD(char tipoAFD, EstadisticasGlobales *global);

/* ========================================================================== */
/*  main                                                                      */
/*                                                                            */
/*  Proposito : Funcion principal. Coordina el flujo completo del programa    */
/*              desde el menu hasta el cierre de sesion.                      */
/*  Retorna   : 0 al terminar correctamente.                                  */
/* ========================================================================== */
int main(void) {
    /* RF-16: Configurar manejo de senales externas */
    configurarManejoSenales();

    /* RF-14: Inicializar contadores globales de sesion */
    EstadisticasGlobales global;
    inicializarEstadisticasGlobales(&global);

    char op;
    char resp_afd = 'S';

    /* Bucle principal de sesion */
    do {
        /* RF-01: Mostrar menu y obtener opcion valida */
        op = pedirOpcionMenu();

        if (op == 'S') {
            /* RF-15: Salida desde el menu principal */
            break;
        }

        /* op = 'T' (AFDT) o op = 'V' (AFDV) */
        cicloAFD(op, &global);

        /* RF-12: Preguntar si se desea registrar otro AFD */
        limpiarPantalla();
        mostrarEncabezado();
        conEscribir("\n");
        resp_afd = pedirConfirmacion("  \xbf" "Desea registrar otro AFD? (S/N): ");

    } while (resp_afd == 'S');

    /* RF-14: Reporte estadistico global */
    mostrarEstadisticasGlobales(&global);

    /* RF-15: Mensaje de despedida */
    conEscribir("\n\xa1" "Gracias, adi\xf3s!\n\n");

    /* RF-14: Pausa tecnica (congelar pantalla) */
    pausaTecnica();

    return 0;
}

/* ========================================================================== */
/*  cicloAFD                                                                  */
/*                                                                            */
/*  Proposito : Gestionar el ciclo completo de un unico AFD:                  */
/*              - Registro de todos los componentes (RF-02..RF-08)            */
/*              - Captura y procesamiento de palabras (RF-09..RF-11)          */
/*              - Estadisticas locales (RF-13)                                */
/*              - Acumulacion a globales (RF-14)                              */
/*              - Liberacion de memoria (RNF-02)                              */
/*  Parametros: tipoAFD -- TIPO_AFDT ('T') o TIPO_AFDV ('V')                  */
/*              global  -- puntero a contadores globales de la sesion         */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
static void cicloAFD(char tipoAFD, EstadisticasGlobales *global) {
    /* RNF-01: Inicializar la estructura AFD */
    AFD afd;
    afd.tipo        = tipoAFD;
    afd.Q           = NULL;
    afd.alfaEntrada = NULL;
    afd.alfaSalida  = NULL;
    afd.q0          = NULL;

    /* RF-14: Estadisticas locales para este AFD */
    EstadisticasAFD local;
    inicializarEstadisticasAFD(&local);

    /* REGISTRO DEL AFD */

    /* RF-02: Registrar Sigma_E (comun a AFDV y AFDT) */
    registrarAlfaEntrada(&afd);

    /* RF-03: Registrar Sigma_S (solo AFDT) */
    if (tipoAFD == TIPO_AFDT) {
        registrarAlfaSalida(&afd);
    }

    /* RF-04: Registrar Q */
    registrarEstados(&afd);

    /* RF-05: Registrar q0 */
    registrarEstadoInicial(&afd);

    /* RF-06: Registrar A */
    registrarEstadosAceptacion(&afd);

    /* RF-07: Registrar f */
    registrarFuncionTransicion(&afd);

    /* RF-08: Registrar g (solo AFDT) */
    if (tipoAFD == TIPO_AFDT) {
        registrarFuncionSalida(&afd);
    }

    /* CICLO DE PROCESAMIENTO DE PALABRAS */
    char resp_pal = 'S';

    do {
        limpiarPantalla();
        mostrarEncabezado();

        /* RF-09: Capturar y validar la palabra */
        char *alpha = capturarPalabra(&afd);
        if (alpha == NULL) break;

        /* RF-10 + RF-11: Procesar la palabra */
        int esValida = 0;
        char *beta = procesarPalabra(&afd, alpha, &esValida);

        /* RF-13: Actualizar contadores locales */
        actualizarEstadisticasAFD(&local, esValida, (tipoAFD == TIPO_AFDT));

        /* RNF-02: Liberar la palabra de entrada y la traduccion */
        free(alpha);
        alpha = NULL;
        if (beta != NULL) {
            free(beta);
            beta = NULL;
        }

        /* RF-12: Procesar otra palabra con el mismo AFD? */
        resp_pal = pedirConfirmacion("  \xbf" "Desea procesar otra palabra con este AFD? (S/N): ");

    } while (resp_pal == 'S');

    /* RF-13: Mostrar estadisticas del ciclo del AFD */
    limpiarPantalla();
    mostrarEncabezado();
    mostrarEstadisticasAFD(&local);

    pausaTecnica();

    /* RF-14: Acumular en globales */
    acumularEstadisticasGlobales(global, &local, tipoAFD);

    /* RNF-02: Liberar toda la memoria dinamica del AFD */
    liberarAFD(&afd);
}
