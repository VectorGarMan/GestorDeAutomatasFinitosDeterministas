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
    conEscribir("\n  -- Estad\xed" "st\xed" "cas del ciclo del AFD --\n\n");
    conEscribir("    Total de palabras procesadas  : ");
    conEscribirInt(e->totalProcesadas);
    conEscribir("\n");

    conEscribir("    Palabras V\xc1LIDAS              : ");
    conEscribirInt(e->totalValidas);
    conEscribir("\n");

    conEscribir("    Palabras INV\xc1LIDAS            : ");
    conEscribirInt(e->totalInvalidas);
    conEscribir("\n");

    if (e->totalTraducidas > 0) {
        conEscribir("    Palabras TRADUCIDAS (AFDT)    : ");
        conEscribirInt(e->totalTraducidas);
        conEscribir("\n");
    }

    /* Verificacion de invariante RF-13: Total_proc = V + U */
    conEscribir("\n    Verificaci\xf3n: Total (");
    conEscribirInt(e->totalProcesadas);
    conEscribir(") = V\xe1lidas (");
    conEscribirInt(e->totalValidas);
    conEscribir(") + Inv\xe1lidas (");
    conEscribirInt(e->totalInvalidas);
    conEscribir(") -> ");
    conEscribir((e->totalProcesadas == e->totalValidas + e->totalInvalidas)
                ? "OK" : "INCONSISTENTE");
    conEscribir("\n\n");
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
/*  Parametros: g -- puntero a los contadores globales                        */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void mostrarEstadisticasGlobales(const EstadisticasGlobales *g) {
    int tGlob = g->totalValidadas + g->totalInvalidas;

    limpiarPantalla();
    mostrarEncabezado();
    conEscribir("\n  -- Reporte Estad\xed" "st\xed" "co Global de la Sesi\xf3n (RF-14) --\n\n");

    conEscribir("    Total de AFD registrados       (Z) : ");
    conEscribirInt(g->totalAFD);
    conEscribir("\n");

    conEscribir("    Total de palabras procesadas   (T) : ");
    conEscribirInt(tGlob);
    conEscribir("\n");

    conEscribir("    Total de palabras V\xc1LIDAS      (V) : ");
    conEscribirInt(g->totalValidadas);
    conEscribir("\n");

    conEscribir("    Total de palabras INV\xc1LIDAS    (U) : ");
    conEscribirInt(g->totalInvalidas);
    conEscribir("\n");

    conEscribir("    Total de palabras TRADUCIDAS   (W) : ");
    conEscribirInt(g->totalTraducidas);
    conEscribir("\n\n");
}
