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

/* ========================================================================== */
/*  manejadorSenalCtrlC -- RF-16                                              */
/*                                                                            */
/*  Proposito : Interceptar SIGINT (Ctrl+C) y evitar el cierre del proceso.   */
/*              flujoActual := flujoActual  (la senal es absorbida).          */
/*  Parametros: senial -- numero de la senal recibida (SIGINT = 2)            */
/*  Retorna   : (void)                                                        */
/* ========================================================================== */
void manejadorSenalCtrlC(int senial) {
    (void)senial;
    signal(SIGINT, manejadorSenalCtrlC);
}

/* ========================================================================== */
/*  manejadorSenalSIGTERM -- RF-16 (interno)                                  */
/*                                                                            */
/*  Proposito : Interceptar SIGTERM y evitar el cierre del proceso.           */
/* ========================================================================== */
static void manejadorSenalSIGTERM(int senial) {
    (void)senial;
    signal(SIGTERM, manejadorSenalSIGTERM);
}

#ifdef _WIN32
/* ========================================================================== */
/*  manejadorConsolaWindows -- RF-16                                          */
/*                                                                            */
/*  Proposito : Interceptar eventos de consola de Windows.                    */
/*              CTRL_C_EVENT      -> Ctrl+C                                   */
/*              CTRL_CLOSE_EVENT  -> Alt+F4 / clic en X de la consola         */
/*              CTRL_BREAK_EVENT  -> Ctrl+Break                               */
/*              CTRL_LOGOFF_EVENT -> Cierre de sesion                         */
/*              CTRL_SHUTDOWN_EVENT -> Apagado del sistema                    */
/*                                                                            */
/*  Para CTRL_CLOSE_EVENT (Alt+F4): Windows ejecuta este manejador en un      */
/*  hilo dedicado. Si el manejador retorna TRUE, Windows inicia un temporizador*/
/*  de ~5 segundos tras el cual termina el proceso de todas formas. Para      */
/*  bloquear el cierre indefinidamente bloqueamos ese hilo con Sleep(INFINITE) */
/*  mientras el hilo principal sigue ejecutandose normalmente.                */
/*                                                                            */
/*  Parametros: tipoEvento -- codigo del evento Win32                         */
/*  Retorna   : BOOL TRUE si el evento fue manejado                           */
/* ========================================================================== */
BOOL WINAPI manejadorConsolaWindows(DWORD tipoEvento) {
    switch (tipoEvento) {
        case CTRL_C_EVENT:
            return TRUE;

        case CTRL_CLOSE_EVENT:
        case CTRL_BREAK_EVENT:
            /*
             * Bloquear el hilo del manejador indefinidamente.
             * El proceso principal continua; Windows no puede terminar el
             * proceso mientras este hilo este dormido y el principal siga vivo.
             */
            Sleep(INFINITE);
            return TRUE;

        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
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
    signal(SIGINT, manejadorSenalCtrlC);
    signal(SIGTERM, manejadorSenalSIGTERM);

#ifdef _WIN32
    SetConsoleCtrlHandler(manejadorConsolaWindows, TRUE);
    SetConsoleOutputCP(1252);
#endif
}
