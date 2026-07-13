/*
 * =============================================================================
 * Archivo      : afd.h
 * Proyecto     : Gestor de Automatas Finitos Deterministas
 * Autor        : Victor Garza Maldonado
 * Fecha        : Julio 2026
 * Revision     : 00.01
 * =============================================================================
 * Descripcion:
 *   Declaraciones de las estructuras de datos y prototipos de funciones para
 *   el registro y procesamiento de Automatas Finitos Deterministas (AFDV y AFDT).
 *
 *   Las estructuras siguen el diseno especificado en el ERS:
 *
 *   struct Nodo  { char[] Nodo; int Aceptacion; Nodo* Apuntador; Arista* Flecha; }
 *   struct Arista{ char ALFE; char ALPS; Arista* Apunt; Nodo* Flecha; }
 *
 *   Requerimientos cubiertos:
 *     RNF-01 -- Memoria dinamica con apuntadores
 *     RNF-02 -- Liberacion de memoria dinamica
 *     RF-01 a RF-16
 * =============================================================================
 */

#ifndef AFD_H
#define AFD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* --------------------------------------------------------------------------- */
/*  CONSTANTES GLOBALES                                                         */
/* --------------------------------------------------------------------------- */

/* Longitud maxima para el nombre de un estado (RF-04: 1 <= |q| <= 15) */
#define MAX_ESTADO    16   /* 15 chars + '\0' */

/* Tamano del buffer de lectura general de consola */
#define MAX_BUF       256

/* Tipo de AFD: validacion o traduccion */
#define TIPO_AFDV     'V'
#define TIPO_AFDT     'T'

/* --------------------------------------------------------------------------- */
/*  ESTRUCTURA: Arista                                                          */
/*                                                                              */
/*  Representa una transicion f(q, s) = q' y, para AFDT, la salida g(q,s)=y   */
/*                                                                              */
/*  Campos:                                                                     */
/*    ALFE  -- simbolo del alfabeto de ENTRADA que dispara la transicion (Sig_E)*/
/*    ALPS  -- simbolo del alfabeto de SALIDA producido por g (solo AFDT)       */
/*    Apunt -- siguiente Arista del mismo Nodo (lista enlazada de transiciones) */
/*    Flecha-- Nodo destino de la transicion (q')                               */
/* --------------------------------------------------------------------------- */
typedef struct Arista {
    char           ALFE;    /* RF-07: simbolo de entrada s en Sigma_E           */
    char           ALPS;    /* RF-08: simbolo de salida  y en Sigma_S (solo AFDT)*/
    struct Arista *Apunt;   /* RNF-01: siguiente arista (apuntador dinamico)    */
    struct Nodo   *Flecha;  /* RF-07: nodo destino q'                           */
} Arista;

/* --------------------------------------------------------------------------- */
/*  ESTRUCTURA: Nodo                                                            */
/*                                                                              */
/*  Representa un estado q en Q del AFD.                                        */
/*                                                                              */
/*  Campos:                                                                     */
/*    nombre     -- identificador del estado (max 15 chars, RF-04)              */
/*    aceptacion -- 1 si q en A, 0 si no (RF-06)                               */
/*    Apuntador  -- siguiente Nodo en la lista de estados Q                     */
/*    Flecha     -- cabeza de la lista de Aristas del nodo                      */
/* --------------------------------------------------------------------------- */
typedef struct Nodo {
    char          nombre[MAX_ESTADO]; /* RF-04: nombre del estado              */
    int           aceptacion;         /* RF-06: bandera de estado de acept.    */
    struct Nodo  *Apuntador;          /* RNF-01: siguiente Nodo                */
    Arista       *Flecha;             /* RF-07/RF-08: lista de transiciones     */
} Nodo;

/* --------------------------------------------------------------------------- */
/*  ESTRUCTURA: Simbolo                                                         */
/*                                                                              */
/*  Nodo de lista enlazada para almacenar un simbolo de Sigma_E o Sigma_S.     */
/* --------------------------------------------------------------------------- */
typedef struct Simbolo {
    char           valor;      /* el caracter del simbolo                      */
    struct Simbolo *siguiente; /* siguiente simbolo en el alfabeto             */
} Simbolo;

/* --------------------------------------------------------------------------- */
/*  ESTRUCTURA: AFD                                                             */
/*                                                                              */
/*  Tupla completa del automata finito determinista.                            */
/*  AFDV = (Q, Sigma_E, f, q0, A)                                              */
/*  AFDT = (Q, Sigma_E, Sigma_S, f, g, q0, A)                                  */
/* --------------------------------------------------------------------------- */
typedef struct {
    char    tipo;          /* TIPO_AFDV ('V') o TIPO_AFDT ('T')               */
    Nodo   *Q;             /* Lista enlazada de estados (RF-04)               */
    Simbolo *alfaEntrada;  /* Lista enlazada Sigma_E (RF-02)                  */
    Simbolo *alfaSalida;   /* Lista enlazada Sigma_S (RF-03, solo AFDT)       */
    Nodo   *q0;            /* Estado inicial (RF-05)                          */
    /* A (estados de aceptacion) se marca con Nodo.aceptacion = 1 (RF-06)    */
    /* f y g se almacenan en las Aristas de cada Nodo    (RF-07, RF-08)      */
} AFD;

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS -- Validacion de caracteres                                      */
/* --------------------------------------------------------------------------- */

/*
 * esPermitido -- RF-02, RF-03, RF-04
 * Verifica que el caracter c pertenezca a Sigma_perm =
 *   {a..z, A..Z, 0..9, n, N, vocales con tilde (Latin-1)}
 * Retorna: 1 si pertenece, 0 si no.
 */
int esPermitido(unsigned char c);

/*
 * cadenaPermitida -- RF-04
 * Verifica que TODOS los caracteres de la cadena s pertenezcan a Sigma_perm.
 * Retorna: 1 si todos son validos, 0 si alguno no lo es.
 */
int cadenaPermitida(const char *s);

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS -- Gestion de Simbolos (Sigma_E y Sigma_S)                      */
/* --------------------------------------------------------------------------- */

/* simboloExiste: retorna 1 si c ya esta en la lista, 0 si no. */
int      simboloExiste(const Simbolo *cabeza, char c);

/* agregarSimbolo: crea y enlaza un nuevo Simbolo al final de la lista. */
void     agregarSimbolo(Simbolo **cabeza, char c);

/* contarSimbolos: retorna la cantidad de simbolos en la lista. */
int      contarSimbolos(const Simbolo *cabeza);

/* liberarSimbolos: libera toda la lista de Simbolos (RNF-02). */
void     liberarSimbolos(Simbolo **cabeza);

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS -- Gestion de Nodos (Q)                                         */
/* --------------------------------------------------------------------------- */

/* nodoExiste: retorna el puntero al Nodo con nombre dado, o NULL. */
Nodo    *nodoExiste(const Nodo *cabeza, const char *nombre);

/* agregarNodo: crea y enlaza un nuevo Nodo al final de Q. */
void     agregarNodo(Nodo **cabeza, const char *nombre);

/* contarNodos: retorna la cantidad de nodos en Q. */
int      contarNodos(const Nodo *cabeza);

/* liberarNodos: libera toda la lista de Nodos y sus Aristas (RNF-02). */
void     liberarNodos(Nodo **cabeza);

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS -- Gestion de Aristas (f y g)                                   */
/* --------------------------------------------------------------------------- */

/* buscarArista: retorna la Arista de 'nodo' disparada por 'simbolo'. */
Arista  *buscarArista(const Nodo *nodo, char simbolo);

/* agregarArista: crea y enlaza una Arista al nodo origen. */
void     agregarArista(Nodo *nodoOrigen, char ALFE, char ALPS, Nodo *destino);

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS -- Registro del AFD (RF-02 a RF-08)                             */
/* --------------------------------------------------------------------------- */

/* RF-02: registrar Sigma_E simbolo por simbolo. */
void registrarAlfaEntrada(AFD *afd);

/* RF-03: registrar Sigma_S simbolo por simbolo (solo AFDT). */
void registrarAlfaSalida(AFD *afd);

/* RF-04: registrar Q estado por estado. */
void registrarEstados(AFD *afd);

/* RF-05: registrar q0. */
void registrarEstadoInicial(AFD *afd);

/* RF-06: registrar A (estados de aceptacion). */
void registrarEstadosAceptacion(AFD *afd);

/* RF-07: registrar f para todas las combinaciones (q, s). */
void registrarFuncionTransicion(AFD *afd);

/* RF-08: registrar g para todas las combinaciones (q, s). Solo AFDT. */
void registrarFuncionSalida(AFD *afd);

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS -- Procesamiento de la palabra (RF-09 a RF-11)                  */
/* --------------------------------------------------------------------------- */

/*
 * RF-09: capturar y validar la palabra en Sigma_E+.
 * Retorna cadena dinamica con la palabra (el llamador debe liberar).
 * Retorna NULL si hay fallo de malloc.
 */
char *capturarPalabra(const AFD *afd);

/*
 * RF-10 + RF-11: procesar la palabra con f (y g para AFDT).
 * Parametros de salida:
 *   esValida -- 1 si qn en A al finalizar, 0 si no
 * Retorna cadena de traduccion (solo AFDT) o NULL para AFDV (el llamador libera).
 */
char *procesarPalabra(const AFD *afd, const char *alpha, int *esValida);

/* --------------------------------------------------------------------------- */
/*  PROTOTIPOS -- Ciclo completo del AFD                                        */
/* --------------------------------------------------------------------------- */

/* liberarAFD: libera toda la memoria dinamica del AFD (RNF-02). */
void liberarAFD(AFD *afd);

#endif /* AFD_H */
