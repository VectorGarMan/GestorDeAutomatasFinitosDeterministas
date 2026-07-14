/*
 * =============================================================================
 * Archivo      : menu.c
 * Proyecto     : Gestor de Automatas Finitos Deterministas
 * Autor        : Victor Garza Maldonado
 * Fecha        : Julio 2026
 * Revision     : 00.01
 * =============================================================================
 * Descripcion:
 *   Implementacion de las funciones de interfaz de consola.
 *
 *   Requerimientos cubiertos:
 *     RF-01  -- Menu de seleccion de tipo de AFD (T/V/S)
 *     RF-15  -- Finalizar sesion (mensaje "Gracias, adios!")
 *     RNF-06 -- Mantener encabezado visible durante toda la ejecucion
 *     RNF-12 -- Idioma espanol
 * =============================================================================
 */

#include "../include/menu.h"

/* ========================================================================== */
/*  Constantes de presentacion                                                */
/* ========================================================================== */
#define LINEA_SEP "  ================================================================\n"

/* ========================================================================== */
/*  mostrarEncabezado -- RNF-06                                               */
/*                                                                            */
/*  Proposito : Imprimir el encabezado del software con nombre, autor y       */
/*              fecha. Se llama al inicio de cada pantalla para garantizar    */
/*              que siempre sea visible (RNF-06).                             */
/*  Parametros: (ninguno)                                                     */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void mostrarEncabezado(void) {
    conEscribir(LINEA_SEP);
    conEscribir("  GESTOR DE AUTOMATAS FINITOS DETERMINISTAS\n");
    conEscribir("  Autor : Victor Garza Maldonado\n");
    conEscribir("  Fecha : Julio 2026  |  Rev. 00.01\n");
    conEscribir(LINEA_SEP);
}

/* ========================================================================== */
/*  limpiarPantalla                                                           */
/*                                                                            */
/*  Proposito : Limpiar la consola (Windows: system("cls")).                  */
/*              Nota: system() se usa UNICAMENTE para limpiar la pantalla;    */
/*              no se ejecutan comandos peligrosos (RNF-03).                  */
/*  Parametros: (ninguno)                                                     */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void limpiarPantalla(void) {
#ifdef _WIN32
    system("cls");
#else
    /* Secuencia ANSI de limpieza de pantalla como alternativa */
    conEscribir("\033[2J\033[H");
#endif
}

/* ========================================================================== */
/*  leerLinea -- RNF-05                                                       */
/*                                                                            */
/*  Proposito : Leer una linea completa de stdin de forma segura, sin         */
/*              desbordamiento de buffer y sin dejar caracteres residuales    */
/*              en el buffer de entrada.                                      */
/*  Parametros: buf    -- buffer destino                                      */
/*              tamMax -- tamano del buffer (incluye '\0')                    */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void leerLinea(char *buf, int tamMax) {
    conLeerLinea(buf, tamMax);
}

/* ========================================================================== */
/*  mostrarMenuPrincipal -- RF-01                                             */
/*                                                                            */
/*  Proposito : Imprimir las opciones del menu principal.                     */
/*              Omega_menu = {T, V, S}                                        */
/*  Parametros: (ninguno)                                                     */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void mostrarMenuPrincipal(void) {
    limpiarPantalla();
    mostrarEncabezado();
    conEscribir("\n  -- Menu Principal --\n\n");
    conEscribir("    [T]  Registrar y procesar un AFDT (Aut\xf3mata de Traducci\xf3n)\n");
    conEscribir("    [V]  Registrar y procesar un AFDV (Aut\xf3mata de Validaci\xf3n)\n");
    conEscribir("    [S]  Salir del software\n\n");
}

/* ========================================================================== */
/*  pedirOpcionMenu -- RF-01                                                  */
/*                                                                            */
/*  Proposito : Solicitar y validar la opcion del menu principal.             */
/*              op en Omega_menu = {T, V, S}.                                 */
/*              Ante entrada invalida: muestra error y repite (RF-01).        */
/*  Parametros: (ninguno)                                                     */
/*  Retorna   : char -- 'T', 'V' o 'S' (en mayuscula)                         */
/* ========================================================================== */
char pedirOpcionMenu(void) {
    char buf[MAX_BUF];
    char op;

    while (1) {
        mostrarMenuPrincipal();
        conEscribir("  Seleccione una opci\xf3n (T/V/S): ");
        leerLinea(buf, MAX_BUF);

        /* Validar: exactamente 1 caracter y pertenece a Omega_menu */
        if (buf[0] != '\0' && buf[1] == '\0') {
            op = (char)(buf[0] >= 'a' && buf[0] <= 'z'
                        ? buf[0] - ('a' - 'A')
                        : buf[0]);

            if (op == 'T' || op == 'V' || op == 'S') {
                return op;
            }
        }

        /* op no pertenece a Omega_menu o es vacio -> error (RF-01) */
        limpiarPantalla();
        mostrarEncabezado();
        conEscribir("\n  Error: Opci\xf3n inv\xe1lida. Intente de nuevo.\n\n");
    }
}

/* ========================================================================== */
/*  pedirConfirmacion -- RF-02, RF-03, RF-04, RF-06, RF-12                    */
/*                                                                            */
/*  Proposito : Solicitar resp en Omega_conf = {S, N}.                        */
/*              Ante entrada invalida: muestra error y repite.                */
/*  Parametros: prompt -- texto de la pregunta                                */
/*  Retorna   : char -- 'S' o 'N' (en mayuscula)                              */
/* ========================================================================== */
char pedirConfirmacion(const char *prompt) {
    char buf[MAX_BUF];
    char resp;

    while (1) {
        conEscribir(prompt);
        leerLinea(buf, MAX_BUF);

        if (buf[0] != '\0' && buf[1] == '\0') {
            resp = (char)(buf[0] >= 'a' && buf[0] <= 'z'
                          ? buf[0] - ('a' - 'A')
                          : buf[0]);

            if (resp == 'S' || resp == 'N') {
                return resp;
            }
        }

        /* Respuesta invalida */
        conEscribir("  Error: Respuesta inv\xe1lida. Ingrese S (s\xed) o N (no).\n\n");
    }
}

/* ========================================================================== */
/*  pausaTecnica -- RF-14                                                     */
/*                                                                            */
/*  Proposito : Congelar la pantalla hasta que el usuario presione ENTER,     */
/*              garantizando la visibilidad del reporte estadistico global    */
/*              antes de finalizar la sesion (RF-14).                         */
/*  Parametros: (ninguno)                                                     */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void pausaTecnica(void) {
    conEscribir("\n  Presione ENTER para continuar...\n");
    conFlush();

    /* Leer y descartar hasta ENTER */
    char buf[8];
    conLeerLinea(buf, sizeof(buf));
}
