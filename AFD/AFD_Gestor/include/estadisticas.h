/*
 * =============================================================================
 * Archivo      : estadisticas.h
 * Proyecto     : Gestor de Automatas Finitos Deterministas
 * Autor        : Victor Garza Maldonado
 * Fecha        : Julio 2026
 * =============================================================================
 * Descripcion:
 *   Estructuras y prototipos para el manejo de contadores estadisticos.
 *
 *   Requerimientos cubiertos:
 *     RF-13 -- Estadisticas del ciclo del AFD (contadores locales)
 *     RF-14 -- Estadisticas del ciclo completo (contadores globales de sesion)
 * =============================================================================
 */

#ifndef ESTADISTICAS_H
#define ESTADISTICAS_H

/* --------------------------------------------------------------------------- */
/*  ESTRUCTURA: EstadisticasAFD                                                 */
/*                                                                              */
/*  Contadores locales para un unico AFD durante su ciclo de evaluacion.       */
/*  RF-13: Total_proc, W (traducidas), V (validas), U (invalidas)              */
/* --------------------------------------------------------------------------- */
typedef struct {
    int totalProcesadas; /* Total de cadenas procesadas con este AFD           */
    int totalValidas;    /* Palabras cuyo qn en A al terminar                  */
    int totalInvalidas;  /* Palabras cuyo qn no en A al terminar               */
    int totalTraducidas; /* Palabras traducidas (solo AFDT; = totalProcesadas) */
} EstadisticasAFD;

/* --------------------------------------------------------------------------- */
/*  ESTRUCTURA: EstadisticasGlobales                                            */
/*                                                                              */
/*  Contadores acumulados a lo largo de toda la sesion de uso.                 */
/*  RF-14: Z (AFD registrados), V_glob, U_glob, W_glob                         */
/* --------------------------------------------------------------------------- */
typedef struct {
    int totalAFD;        /* Z: total de AFD registrados en la sesion           */
    int totalValidadas;  /* V_glob: total historico de palabras validas         */
    int totalInvalidas;  /* U_glob: total historico de palabras invalidas       */
    int totalTraducidas; /* W_glob: total historico de palabras traducidas      */
} EstadisticasGlobales;

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS                                                                  */
/* --------------------------------------------------------------------------- */

/*
 * inicializarEstadisticasAFD -- RF-13
 * Pone a cero todos los contadores locales de un AFD.
 */
void inicializarEstadisticasAFD(EstadisticasAFD *e);

/*
 * actualizarEstadisticasAFD -- RF-13
 * Incrementa los contadores segun el resultado del procesamiento de una palabra.
 * Parametros:
 *   e         -- puntero al contador local del AFD
 *   esValida  -- 1 si la palabra fue aceptada, 0 si fue rechazada
 *   esAFDT    -- 1 si el AFD es de traduccion (suma tambien totalTraducidas)
 */
void actualizarEstadisticasAFD(EstadisticasAFD *e, int esValida, int esAFDT);

/*
 * mostrarEstadisticasAFD -- RF-13
 * Imprime los contadores locales al finalizar el ciclo de un AFD.
 */
void mostrarEstadisticasAFD(const EstadisticasAFD *e);

/*
 * inicializarEstadisticasGlobales -- RF-14
 * Pone a cero todos los contadores globales de la sesion.
 */
void inicializarEstadisticasGlobales(EstadisticasGlobales *g);

/*
 * acumularEstadisticasGlobales -- RF-14
 * Suma los contadores de un AFD terminado a los globales de sesion.
 */
void acumularEstadisticasGlobales(EstadisticasGlobales *g,
                                   const EstadisticasAFD *e,
                                   char tipoAFD);

/*
 * mostrarEstadisticasGlobales -- RF-14
 * Imprime el reporte estadistico global antes de finalizar la sesion.
 */
void mostrarEstadisticasGlobales(const EstadisticasGlobales *g);

#endif /* ESTADISTICAS_H */
