/*
 * =============================================================================
 * Archivo      : controles.h
 * Proyecto     : Gestor de Automatas Finitos Deterministas
 * Autor        : Victor Garza Maldonado
 * Fecha        : Julio 2026
 * =============================================================================
 * Descripcion:
 *   Control de interrupciones y cierres externos sin uso de librerias externas.
 *
 *   Requerimientos cubiertos:
 *     RF-16 -- Control cierre externo
 *              Omega_ext = {Ctrl+C, Alt+F4, Ctrl+Z, Ctrl+D, SIGINT, SIGTERM, EOF}
 *              e en Omega_ext => flujoActual := flujoActual  (el flujo continua)
 *
 *   Nota de implementacion (ambiguedad ERS):
 *     El ERS indica que cuando ocurre un evento externo el sistema debe
 *     mantener flujoActual := flujoActual, es decir, ignorar el cierre y
 *     continuar. Para Ctrl+C y SIGINT/SIGTERM se registran manejadores
 *     mediante signal() de <signal.h> (parte de la biblioteca estandar C,
 *     NO una libreria externa). Para Alt+F4 en consola Windows, el sistema
 *     operativo envia CTRL_CLOSE_EVENT, interceptado con SetConsoleCtrlHandler
 *     de la API Win32 (windows.h, cabecera del SO, no libreria externa).
 * =============================================================================
 */

#ifndef CONTROLES_H
#define CONTROLES_H

#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS                                                                  */
/* --------------------------------------------------------------------------- */

/*
 * configurarManejoSenales -- RF-16
 * Registra los manejadores para SIGINT, SIGTERM (y en Windows, el handler
 * de SetConsoleCtrlHandler para Alt+F4 / Ctrl+Close).
 * Debe llamarse al inicio de main() antes de cualquier otra operacion.
 */
void configurarManejoSenales(void);

/*
 * manejadorSenalCtrlC -- RF-16 (uso interno, expuesto para pruebas)
 * Manejador de SIGINT (Ctrl+C): absorbe la senal y reanuda el flujo.
 */
void manejadorSenalCtrlC(int senial);

#ifdef _WIN32
/*
 * manejadorConsolaWindows -- RF-16 (uso interno)
 * Manejador Win32 para CTRL_C_EVENT, CTRL_CLOSE_EVENT (Alt+F4),
 * CTRL_BREAK_EVENT, CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT.
 * Retorna TRUE para indicar al SO que el evento fue gestionado.
 */
BOOL WINAPI manejadorConsolaWindows(DWORD tipoEvento);
#endif

#endif /* CONTROLES_H */
