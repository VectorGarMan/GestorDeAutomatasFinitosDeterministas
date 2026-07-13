/*
 * =============================================================================
 * Archivo      : controles.c
 * Proyecto     : Gestor de Automatas Finitos Deterministas
 * Autor        : Victor Garza Maldonado
 * Fecha        : Julio 2026
 * Revision     : 00.01
 * =============================================================================
 * Descripcion:
 *   Control de interrupciones y cierres externos (RF-16) sin uso de librerias
 *   externas. Solo se usan:
 *     - <signal.h>  : parte de la biblioteca estandar C (ISO C99/C11)
 *     - <windows.h> : cabecera del sistema operativo Windows (no es libreria
 *                     externa; es la API del SO objetivo especificado en RNF-10)
 *
 *   Requerimientos cubiertos:
 *     RF-16 -- Control cierre externo
 *              Omega_ext = {Ctrl+C, Alt+F4, Ctrl+Z, Ctrl+D, SIGINT, SIGTERM, EOF}
 *              e en Omega_ext => flujoActual := flujoActual  (el flujo NO cambia)
 * =============================================================================
 */

#include "../include/controles.h"
#include <stdio.h>

/* ========================================================================== */
/*  manejadorSenalCtrlC -- RF-16                                              */
/*                                                                            */
/*  Proposito : Interceptar SIGINT (Ctrl+C) y evitar el cierre del proceso.   */
/*              flujoActual := flujoActual  (la senal es absorbida).          */
/*  Parametros: senial -- numero de la senal recibida (SIGINT = 2)            */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void manejadorSenalCtrlC(int senial) {
    (void)senial; /* Suprimir advertencia de parametro no usado */
    /* RF-16: absorber la senal y continuar el flujo sin mostrar ningun mensaje */
    signal(SIGINT, manejadorSenalCtrlC);
}

/* ========================================================================== */
/*  manejadorSenalSIGTERM -- RF-16 (interno)                                  */
/*                                                                            */
/*  Proposito : Interceptar SIGTERM y evitar el cierre del proceso.           */
/* ========================================================================== */
static void manejadorSenalSIGTERM(int senial) {
    (void)senial;
    /* RF-16: absorber la senal y continuar el flujo sin mostrar ningun mensaje */
    signal(SIGTERM, manejadorSenalSIGTERM);
}

#ifdef _WIN32
/* ========================================================================== */
/*  manejadorConsolaWindows -- RF-16                                          */
/*                                                                            */
/*  Proposito : Interceptar eventos de consola de Windows:                    */
/*              CTRL_C_EVENT      -> Ctrl+C                                   */
/*              CTRL_CLOSE_EVENT  -> Alt+F4 / clic en X de la consola         */
/*              CTRL_BREAK_EVENT  -> Ctrl+Break                               */
/*              CTRL_LOGOFF_EVENT -> Cierre de sesion                         */
/*              CTRL_SHUTDOWN_EVENT -> Apagado del sistema                    */
/*              Retorna TRUE para indicar que el evento fue absorbido y       */
/*              el proceso NO debe terminar (flujoActual := flujoActual).     */
/*  Parametros: tipoEvento -- codigo del evento Win32                         */
/*  Retorna   : BOOL TRUE si el evento fue manejado, FALSE en caso contrario  */
/* ========================================================================== */
BOOL WINAPI manejadorConsolaWindows(DWORD tipoEvento) {
    switch (tipoEvento) {
        case CTRL_C_EVENT:
            /* RF-16: absorber el evento y continuar el flujo sin mostrar mensaje */
            return TRUE;

        case CTRL_CLOSE_EVENT:
            /* RF-16: absorber el evento y continuar el flujo sin mostrar mensaje */
            return TRUE;

        case CTRL_BREAK_EVENT:
            /* RF-16: absorber el evento y continuar el flujo sin mostrar mensaje */
            return TRUE;

        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            /* Para eventos de apagado del SO no bloqueamos el cierre */
            return FALSE;

        default:
            return FALSE;
    }
}
#endif /* _WIN32 */

/* ========================================================================== */
/*  configurarManejoSenales -- RF-16                                          */
/*                                                                            */
/*  Proposito : Registrar todos los manejadores de senales y eventos de       */
/*              consola. Debe llamarse al inicio de main().                   */
/*  Parametros: (ninguno)                                                     */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void configurarManejoSenales(void) {
    /* SIGINT  -- Ctrl+C en cualquier sistema POSIX/Windows */
    signal(SIGINT, manejadorSenalCtrlC);

    /* SIGTERM -- kill/terminacion en sistemas POSIX */
    signal(SIGTERM, manejadorSenalSIGTERM);

#ifdef _WIN32
    /*
     * SetConsoleCtrlHandler -- API Win32 estandar.
     * Intercepta: Ctrl+C, Ctrl+Break, Alt+F4, cierre de sesion y apagado.
     * No es una libreria externa; es la interfaz oficial del SO Windows
     * especificado en RNF-10.
     */
    SetConsoleCtrlHandler(manejadorConsolaWindows, TRUE);

    /*
     * SetConsoleOutputCP(1252) -- Forzar Windows-1252 como pagina de codigo
     * de salida para que los caracteres especiales del espanol (acentos,
     * signos de interrogacion/exclamacion) se muestren correctamente en
     * cualquier consola CMD independientemente de su codepage por defecto.
     */
    SetConsoleOutputCP(1252);
#endif
}
