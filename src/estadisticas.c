/*
 * =============================================================================
 * Archivo      : estadisticas.c
 * Proyecto     : Gestor de Automatas Finitos Deterministas
 * Autor        : Victor Garza Maldonado
 * Fecha        : Julio 2026
 * Revision     : 00.01
 * =============================================================================
 * Descripcion:
 *   Implementacion del manejo de contadores estadisticos locales y globales.
 *
 *   Requerimientos cubiertos:
 *     RF-13 -- Estadisticas del ciclo del AFD
 *              Total_proc, W (traducidas), V (validas), U (invalidas)
 *     RF-14 -- Estadisticas del ciclo completo de la sesion
 *              Z (AFD registrados), V_glob, U_glob, W_glob
 *              Se cumple Total_proc = V + U
 * =============================================================================
 */

#include <stdio.h>
#include "../include/estadisticas.h"
#include "../include/menu.h"
#include "../include/afd.h"

/* ========================================================================== */
/*  inicializarEstadisticasAFD -- RF-13                                       */
/*                                                                            */
/*  Proposito : Poner a cero los contadores locales del AFD actual.           */
/*  Parametros: e -- puntero a la estructura de estadisticas locales          */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void inicializarEstadisticasAFD(EstadisticasAFD *e) {
    e->totalProcesadas = 0;
    e->totalValidas    = 0;
    e->totalInvalidas  = 0;
    e->totalTraducidas = 0;
}

/* ========================================================================== */
/*  actualizarEstadisticasAFD -- RF-13                                        */
/*                                                                            */
/*  Proposito : Incrementar los contadores segun el resultado de la palabra.  */
/*              i := i + 1 (incremento del total)                             */
/*              V += (qn en A ? 1 : 0)                                        */
/*              U += (qn no en A ? 1 : 0)                                     */
/*              W += 1 si es AFDT (toda traduccion se cuenta)                 */
/*  Parametros: e        -- contadores del AFD actual                         */
/*              esValida -- 1 si la palabra fue aceptada                      */
/*              esAFDT   -- 1 si el tipo es TIPO_AFDT                         */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void actualizarEstadisticasAFD(EstadisticasAFD *e, int esValida, int esAFDT) {
    e->totalProcesadas++;
    if (esValida) {
        e->totalValidas++;
    } else {
        e->totalInvalidas++;
    }
    if (esAFDT) {
        e->totalTraducidas++;
    }
}

/* ========================================================================== */
/*  mostrarEstadisticasAFD -- RF-13                                           */
/*                                                                            */
/*  Proposito : Imprimir el reporte local al finalizar el ciclo del AFD.      */
/*              Verifica la invariante: Total_proc = V + U.                   */
/*  Parametros: e -- puntero a los contadores del AFD                         */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void mostrarEstadisticasAFD(const EstadisticasAFD *e) {
    printf("\n  -- Estad%csticas del ciclo del AFD --\n\n", 237);
    printf("    Total de palabras procesadas  : %d\n", e->totalProcesadas);
    printf("    Palabras V%cLIDAS              : %d\n", 193, e->totalValidas);
    printf("    Palabras INV%cLIDAS            : %d\n", 193, e->totalInvalidas);
    if (e->totalTraducidas > 0) {
        printf("    Palabras TRADUCIDAS (AFDT)    : %d\n", e->totalTraducidas);
    }
    /* Verificacion de invariante RF-13: Total_proc = V + U */
    printf("\n    Verificaci%cn: Total (%d) = V%clidas (%d) + Inv%clidas (%d) -> %s\n", 243, 225, 160,
           e->totalProcesadas,
           e->totalValidas,
           e->totalInvalidas,
           (e->totalProcesadas == e->totalValidas + e->totalInvalidas) ? "OK" : "INCONSISTENTE");
    printf("\n");
}

/* ========================================================================== */
/*  inicializarEstadisticasGlobales -- RF-14                                  */
/*                                                                            */
/*  Proposito : Poner a cero los contadores globales de la sesion.            */
/*  Parametros: g -- puntero a la estructura de estadisticas globales         */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void inicializarEstadisticasGlobales(EstadisticasGlobales *g) {
    g->totalAFD       = 0;
    g->totalValidadas = 0;
    g->totalInvalidas = 0;
    g->totalTraducidas= 0;
}

/* ========================================================================== */
/*  acumularEstadisticasGlobales -- RF-14                                     */
/*                                                                            */
/*  Proposito : Sumar contadores del AFD terminado a los globales.            */
/*              Z += 1; V_glob += V; U_glob += U; W_glob += W (si AFDT)       */
/*  Parametros: g       -- contadores globales de sesion                      */
/*              e       -- contadores locales del AFD recien terminado        */
/*              tipoAFD -- TIPO_AFDT o TIPO_AFDV                              */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void acumularEstadisticasGlobales(EstadisticasGlobales *g,
                                   const EstadisticasAFD *e,
                                   char tipoAFD) {
    g->totalAFD++;
    g->totalValidadas += e->totalValidas;
    g->totalInvalidas += e->totalInvalidas;
    if (tipoAFD == TIPO_AFDT) {
        g->totalTraducidas += e->totalTraducidas;
    }
}

/* ========================================================================== */
/*  mostrarEstadisticasGlobales -- RF-14                                      */
/*                                                                            */
/*  Proposito : Imprimir el reporte estadistico global antes del cierre.      */
/*              Incluye Z, T_glob = V_glob + U_glob, V_glob, U_glob, W_glob.  */
/*              Verifica: T_glob = V_glob + U_glob.                           */
/*  Parametros: g -- puntero a los contadores globales                        */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void mostrarEstadisticasGlobales(const EstadisticasGlobales *g) {
    int tGlob = g->totalValidadas + g->totalInvalidas;

    limpiarPantalla();
    mostrarEncabezado();
    printf("\n  -- Reporte Estad%cstico Global de la Sesi%cn (RF-14) --\n\n", 237, 243);
    printf("    Total de AFD registrados       (Z) : %d\n", g->totalAFD);
    printf("    Total de palabras procesadas   (T) : %d\n", tGlob);
    printf("    Total de palabras V%cLIDAS      (V) : %d\n", 193, g->totalValidadas);
    printf("    Total de palabras INV%cLIDAS    (U) : %d\n", 193, g->totalInvalidas);
    printf("    Total de palabras TRADUCIDAS   (W) : %d\n", g->totalTraducidas);
    printf("\n");
}
