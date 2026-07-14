/*
 * =============================================================================
 * Archivo      : menu.h
 * Proyecto     : Gestor de Automatas Finitos Deterministas
 * Autor        : Victor Garza Maldonado
 * Fecha        : Julio 2026
 * =============================================================================
 * Descripcion:
 *   Prototipos de las funciones de interfaz de usuario: encabezado, menu
 *   principal, confirmaciones, mensajes y lectura segura de entradas.
 *
 *   Requerimientos cubiertos:
 *     RF-01  -- Menu de seleccion de tipo de AFD
 *     RF-15  -- Finalizar sesion
 *     RNF-06 -- Mantener encabezado visible
 *     RNF-12 -- Idioma espanol
 * =============================================================================
 */

#ifndef MENU_H
#define MENU_H

#include <string.h>
#include "afd.h"
#include "consola.h"

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS -- Presentacion visual                                           */
/* --------------------------------------------------------------------------- */

/*
 * mostrarEncabezado -- RNF-06
 * Imprime el encabezado del software con nombre, autor y fecha.
 * Debe llamarse antes de cada pantalla principal.
 */
void mostrarEncabezado(void);

/*
 * limpiarPantalla -- RNF-06
 * Limpia la consola (Windows: system("cls")).
 * Nota: se usa system("cls") unicamente para control de pantalla,
 *       lo cual esta dentro del alcance del sistema operativo objetivo (Windows).
 */
void limpiarPantalla(void);

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS -- Lectura segura                                                */
/* --------------------------------------------------------------------------- */

/*
 * leerLinea -- RNF-05
 * Lee una linea completa de stdin en buf[0..tamMax-1], descarta el resto.
 * Elimina el '\n' final. Nunca desborda el buffer.
 */
void leerLinea(char *buf, int tamMax);

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS -- Menu y confirmaciones                                         */
/* --------------------------------------------------------------------------- */

/*
 * mostrarMenuPrincipal -- RF-01
 * Imprime las opciones del menu principal: T, V, S.
 */
void mostrarMenuPrincipal(void);

/*
 * pedirOpcionMenu -- RF-01
 * Solicita y valida op en {T, V, S}.
 * Retorna el caracter valido (en mayuscula).
 */
char pedirOpcionMenu(void);

/*
 * pedirConfirmacion -- RF-02, RF-03, RF-04, RF-06, RF-12
 * Solicita una respuesta de confirmacion resp en {S, N}.
 * prompt: texto de la pregunta.
 * Retorna 'S' o 'N' (en mayuscula).
 */
char pedirConfirmacion(const char *prompt);

/*
 * pausaTecnica -- RF-14
 * Congela la pantalla hasta que el usuario presione ENTER,
 * para garantizar la visibilidad del reporte estadistico global.
 */
void pausaTecnica(void);

#endif /* MENU_H */
