/*
 * =============================================================================
 * Archivo      : consola.h
 * Proyecto     : Gestor de Automatas Finitos Deterministas
 * Autor        : Victor Garza Maldonado
 * Fecha        : Julio 2026
 * =============================================================================
 * Descripcion:
 *   Interfaz de E/S de consola sin dependencia de <stdio.h>.
 *   En Windows utiliza WriteConsoleA / ReadConsoleA / GetStdHandle (Win32).
 *   En sistemas POSIX usa write(2) / read(2) de <unistd.h>.
 *
 *   Funciones expuestas:
 *     conEscribir(s)           -- escribe la cadena s en stdout
 *     conEscribirChar(c)       -- escribe un caracter en stdout
 *     conEscribirInt(n)        -- convierte n a decimal y escribe en stdout
 *     conSnprintf(b,n,fmt,...) -- formatea en buffer (sin stdio)
 *     conLeerLinea(b,tam)      -- lee una linea de stdin de forma segura
 *     conFlush()               -- vacia el buffer de salida (no-op en Win32)
 *     conEscribirError(s)      -- escribe s en stderr
 * =============================================================================
 */

#ifndef CONSOLA_H
#define CONSOLA_H

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdarg.h>
#include <string.h>

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS                                                                  */
/* --------------------------------------------------------------------------- */

/* Escribe la cadena s en stdout */
void conEscribir(const char *s);

/* Escribe un caracter c en stdout */
void conEscribirChar(char c);

/* Convierte n a cadena decimal y la escribe en stdout */
void conEscribirInt(int n);

/*
 * conSnprintf -- equivalente reducido de snprintf sin <stdio.h>.
 * Formatos soportados: %s %c %d %% (suficiente para este proyecto).
 * Retorna el numero de chars escritos en buf (sin '\0'), o -1 si tamMax <= 0.
 */
int conSnprintf(char *buf, int tamMax, const char *fmt, ...);

/*
 * conLeerLinea -- leer una linea completa de stdin de forma segura.
 * Elimina '\n' y '\r' finales. Descarta caracteres sobrantes del buffer.
 */
void conLeerLinea(char *buf, int tamMax);

/* Vacia el buffer de salida pendiente hacia la consola (no-op en Win32) */
void conFlush(void);

/* Escribe la cadena s en stderr */
void conEscribirError(const char *s);

#endif /* CONSOLA_H */
