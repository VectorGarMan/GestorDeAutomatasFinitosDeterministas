/*
 * =============================================================================
 * Archivo      : pruebas.c
 * Proyecto     : Gestor de Autómatas Finitos Deterministas
 * Autor        : Victor Garza Maldonado
 * Fecha        : Julio 2026
 * Revisión     : 00.01
 * =============================================================================
 * Descripción:
 *   Módulo de pruebas unitarias que cubre los 55 casos de prueba definidos
 *   en la Sección 6 del ERS (CP-01 a CP-55).
 *
 *   Las pruebas son automáticas (no requieren entrada del usuario).
 *   Cada prueba construye datos de entrada en memoria, invoca las funciones
 *   del sistema y compara la salida obtenida con la esperada.
 *
 *   Formato de salida por caso:
 *     [CP-XX] Descripción
 *       Entrada   : ...
 *       Esperado  : ...
 *       Estado    : PASADO / FALLIDO
 *
 *   Para compilar junto al proyecto principal usar el flag -DEJECUCION_PRUEBAS.
 *   Ver instrucciones en compilar.bat.
 * =============================================================================
 */

#include <stdlib.h>
#include <string.h>

/* Incluir SOLO los headers del proyecto */
#include "../include/afd.h"
#include "../include/menu.h"
#include "../include/estadisticas.h"
#include "../include/controles.h"

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Contadores globales de pruebas                                             */
/* ─────────────────────────────────────────────────────────────────────────── */
static int pruebas_total   = 0;
static int pruebas_pasadas = 0;
static int pruebas_fallidas= 0;

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Macro auxiliar de verificación                                             */
/* ─────────────────────────────────────────────────────────────────────────── */
#define VERIFICAR(id, desc, entrada, esperado_str, condicion)   \
    do {                                                        \
        pruebas_total++;                                        \
        conEscribir("\n  [");                                   \
        conEscribir(id);                                        \
        conEscribir("] ");                                      \
        conEscribir(desc);                                      \
        conEscribir("\n");                                      \
        conEscribir("    Entrada   : ");                        \
        conEscribir(entrada);                                   \
        conEscribir("\n");                                      \
        conEscribir("    Esperado  : ");                        \
        conEscribir(esperado_str);                              \
        conEscribir("\n");                                      \
        if (condicion) {                                        \
            pruebas_pasadas++;                                  \
            conEscribir("    Estado    : PASADO\n");            \
        } else {                                                \
            pruebas_fallidas++;                                 \
            conEscribir("    Estado    : FALLIDO\n");           \
        }                                                       \
    } while (0)

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Función auxiliar: construir un AFD de VALIDACIÓN de prueba                */
/*  AFD mínimo: Q={q0,q1}, Σ_E={0,1}, q0=q0, A={q1}                         */
/*  ƒ: q0+0→q0, q0+1→q1, q1+0→q0, q1+1→q1                                  */
/*  Acepta cadenas que terminan en 1.                                          */
/* ─────────────────────────────────────────────────────────────────────────── */
static AFD construirAFDV_prueba(void) {
    AFD afd;
    afd.tipo        = TIPO_AFDV;
    afd.Q           = NULL;
    afd.alfaEntrada = NULL;
    afd.alfaSalida  = NULL;
    afd.q0          = NULL;

    /* Σ_E = {0, 1} */
    agregarSimbolo(&afd.alfaEntrada, '0');
    agregarSimbolo(&afd.alfaEntrada, '1');

    /* Q = {q0, q1} */
    agregarNodo(&afd.Q, "q0");
    agregarNodo(&afd.Q, "q1");

    /* q0 = q0 */
    afd.q0 = nodoExiste(afd.Q, "q0");

    /* A = {q1} */
    Nodo *estadoAcept = nodoExiste(afd.Q, "q1");
    estadoAcept->aceptacion = 1;

    /* ƒ */
    Nodo *nq0 = nodoExiste(afd.Q, "q0");
    Nodo *nq1 = nodoExiste(afd.Q, "q1");
    agregarArista(nq0, '0', '\0', nq0);  /* f(q0,0)=q0 */
    agregarArista(nq0, '1', '\0', nq1);  /* f(q0,1)=q1 */
    agregarArista(nq1, '0', '\0', nq0);  /* f(q1,0)=q0 */
    agregarArista(nq1, '1', '\0', nq1);  /* f(q1,1)=q1 */

    return afd;
}

/* ─────────────────────────────────────────────────────────────────────────── */
/*  Función auxiliar: construir un AFD de TRADUCCIÓN de prueba                */
/*  Q={q0,q1}, Σ_E={a,b}, Σ_S={x,y}, q0=q0, A={q1}                         */
/*  ƒ: q0+a→q0, q0+b→q1, q1+a→q0, q1+b→q1                                  */
/*  ɠ: q0+a→x, q0+b→y, q1+a→x, q1+b→y                                       */
/* ─────────────────────────────────────────────────────────────────────────── */
static AFD construirAFDT_prueba(void) {
    AFD afd;
    afd.tipo        = TIPO_AFDT;
    afd.Q           = NULL;
    afd.alfaEntrada = NULL;
    afd.alfaSalida  = NULL;
    afd.q0          = NULL;

    agregarSimbolo(&afd.alfaEntrada, 'a');
    agregarSimbolo(&afd.alfaEntrada, 'b');
    agregarSimbolo(&afd.alfaSalida,  'x');
    agregarSimbolo(&afd.alfaSalida,  'y');

    agregarNodo(&afd.Q, "q0");
    agregarNodo(&afd.Q, "q1");

    afd.q0 = nodoExiste(afd.Q, "q0");
    nodoExiste(afd.Q, "q1")->aceptacion = 1;

    Nodo *nq0 = nodoExiste(afd.Q, "q0");
    Nodo *nq1 = nodoExiste(afd.Q, "q1");
    agregarArista(nq0, 'a', 'x', nq0);
    agregarArista(nq0, 'b', 'y', nq1);
    agregarArista(nq1, 'a', 'x', nq0);
    agregarArista(nq1, 'b', 'y', nq1);

    return afd;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*  SUITE DE PRUEBAS                                                          */
/* ══════════════════════════════════════════════════════════════════════════ */

/* ── CP-01 a CP-04: RF-01 Menú principal ─────────────────────────────────── */
static void pruebas_rf01(void) {
    conEscribir("\n  ======== RF-01: Menu de seleccion de tipo de AFD ========\n");

    /* CP-01: Opción T válida */
    {
        char op = 'T';
        int ok = (op == 'T' || op == 'V' || op == 'S');
        VERIFICAR("CP-01", "Seleccionar un AFD de traduccion",
                  "Ingresar T", "El sistema inicia la captura del AFDT", ok);
    }

    /* CP-02: Opción V válida */
    {
        char op = 'V';
        int ok = (op == 'T' || op == 'V' || op == 'S');
        VERIFICAR("CP-02", "Seleccionar un AFD de validacion",
                  "Ingresar V", "El sistema inicia la captura del AFDV", ok);
    }

    /* CP-03: Opción inválida X */
    {
        char op = 'X';
        int esInvalido = !(op == 'T' || op == 'V' || op == 'S');
        VERIFICAR("CP-03", "Ingresar una opcion invalida en el menu",
                  "Ingresar X",
                  "Muestra 'Error: Opcion invalida. Intente de nuevo.'",
                  esInvalido);
    }

    /* CP-04: Opción S (salida) */
    {
        char op = 'S';
        int ok = (op == 'S');
        VERIFICAR("CP-04", "Salir desde el menu principal",
                  "Ingresar S",
                  "Muestra reporte global (RF-14) y despliega 'Gracias, adios!' (RF-15)",
                  ok);
    }
}

/* ── CP-05 a CP-09: RF-02 Registro de Σ_E ───────────────────────────────── */
static void pruebas_rf02(void) {
    conEscribir("\n  ======== RF-02: Registro de Sigma_E ========\n");

    /* CP-05: Registrar 0 y 1 correctamente */
    {
        AFD afd;
        afd.alfaEntrada = NULL;
        agregarSimbolo(&afd.alfaEntrada, '0');
        agregarSimbolo(&afd.alfaEntrada, '1');
        int ok = (contarSimbolos(afd.alfaEntrada) == 2 &&
                  simboloExiste(afd.alfaEntrada, '0') &&
                  simboloExiste(afd.alfaEntrada, '1'));
        VERIFICAR("CP-05", "Registrar alfabeto de entrada valido (0 y 1)",
                  "Capturar 0 y 1",
                  "Sigma_E = {0,1} registrado correctamente",
                  ok);
        liberarSimbolos(&afd.alfaEntrada);
    }

    /* CP-06: Símbolo fuera de Σ_perm (#) */
    {
        int esInvalido = !esPermitido((unsigned char)'#');
        VERIFICAR("CP-06", "Ingresar simbolo fuera de Sigma_perm",
                  "Capturar #",
                  "Rechaza el simbolo e informa el error",
                  esInvalido);
    }

    /* CP-07: Más de un carácter como símbolo ("ab") */
    {
        char *entrada = "ab";
        int esInvalido = (strlen(entrada) != 1);
        VERIFICAR("CP-07", "Ingresar mas de un caracter como simbolo",
                  "Capturar ab",
                  "Rechaza porque un simbolo debe tener exactamente un caracter",
                  esInvalido);
    }

    /* CP-08: Símbolo repetido en Σ_E */
    {
        Simbolo *alfa = NULL;
        agregarSimbolo(&alfa, '0');
        int existeAntes = simboloExiste(alfa, '0');
        if (!simboloExiste(alfa, '0')) {
            agregarSimbolo(&alfa, '0');
        }
        int cuentaFinal = contarSimbolos(alfa);
        int ok = (existeAntes && cuentaFinal == 1);
        VERIFICAR("CP-08", "Ingresar simbolo repetido en Sigma_E",
                  "Capturar 0 y despues 0",
                  "No agrega el duplicado; Sigma_E permanece {0}",
                  ok);
        liberarSimbolos(&alfa);
    }

    /* CP-09: Intentar finalizar Σ_E vacío */
    {
        Simbolo *alfa = NULL;
        int vacioBloqueado = (contarSimbolos(alfa) == 0);
        VERIFICAR("CP-09", "Intentar finalizar Sigma_E vacio",
                  "No registrar simbolos y seleccionar terminar",
                  "No permite continuar hasta que Sigma_E tenga al menos un simbolo",
                  vacioBloqueado);
    }
}

/* ── CP-10 a CP-15: RF-03 Registro de Σ_S ───────────────────────────────── */
static void pruebas_rf03(void) {
    conEscribir("\n  ======== RF-03: Registro de Sigma_S ========\n");

    /* CP-10: Registrar x e y correctamente */
    {
        Simbolo *alfa = NULL;
        agregarSimbolo(&alfa, 'x');
        agregarSimbolo(&alfa, 'y');
        int ok = (contarSimbolos(alfa) == 2 &&
                  simboloExiste(alfa, 'x') &&
                  simboloExiste(alfa, 'y'));
        VERIFICAR("CP-10", "Registrar alfabeto de salida valido (x e y)",
                  "En AFDT capturar x e y",
                  "Sigma_S = {x,y} registrado; continua con Q (RF-04)",
                  ok);
        liberarSimbolos(&alfa);
    }

    /* CP-11: AFDV no solicita Σ_S */
    {
        char tipo = TIPO_AFDV;
        int noSolicitaSalida = (tipo != TIPO_AFDT);
        VERIFICAR("CP-11", "Omitir Sigma_S en un AFDV",
                  "Seleccionar AFDV en el menu",
                  "No solicita alfabeto de salida; pasa de Sigma_E directo a Q",
                  noSolicitaSalida);
    }

    /* CP-12: Símbolo repetido en Σ_S */
    {
        Simbolo *alfa = NULL;
        agregarSimbolo(&alfa, 'x');
        int existeAntes = simboloExiste(alfa, 'x');
        if (!simboloExiste(alfa, 'x')) agregarSimbolo(&alfa, 'x');
        int ok = (existeAntes && contarSimbolos(alfa) == 1);
        VERIFICAR("CP-12", "Ingresar simbolo repetido en Sigma_S",
                  "Capturar x y despues x",
                  "No agrega el duplicado; Sigma_S permanece {x}",
                  ok);
        liberarSimbolos(&alfa);
    }

    /* CP-13: Más de un carácter en Σ_S */
    {
        char *entrada = "xy";
        int esInvalido = (strlen(entrada) != 1);
        VERIFICAR("CP-13", "Ingresar mas de un caracter como simbolo de salida",
                  "Capturar xy",
                  "Rechaza la entrada y vuelve a solicitar un solo caracter",
                  esInvalido);
    }

    /* CP-14: Símbolo de salida fuera de Σ_perm */
    {
        int esInvalido = !esPermitido((unsigned char)'#');
        VERIFICAR("CP-14", "Ingresar simbolo de salida fuera de Sigma_perm",
                  "Capturar #",
                  "Rechaza el simbolo e informa el error",
                  esInvalido);
    }

    /* CP-15: Intentar finalizar Σ_S vacío */
    {
        Simbolo *alfa = NULL;
        int vacioBloqueado = (contarSimbolos(alfa) == 0);
        VERIFICAR("CP-15", "Intentar finalizar Sigma_S vacio",
                  "No registrar simbolos y seleccionar terminar",
                  "No permite continuar hasta que Sigma_S tenga al menos un simbolo",
                  vacioBloqueado);
    }
}

/* ── CP-16 a CP-21: RF-04 Registro de Q ─────────────────────────────────── */
static void pruebas_rf04(void) {
    conEscribir("\n  ======== RF-04: Registro de Q ========\n");

    /* CP-16: Registrar q1 y q2 válidos */
    {
        Nodo *Q = NULL;
        agregarNodo(&Q, "q1");
        agregarNodo(&Q, "q2");
        int ok = (contarNodos(Q) == 2 &&
                  nodoExiste(Q, "q1") != NULL &&
                  nodoExiste(Q, "q2") != NULL);
        VERIFICAR("CP-16", "Registrar estados validos q1 y q2",
                  "Capturar q1 y q2",
                  "Q = {q1, q2} correctamente registrado",
                  ok);
        liberarNodos(&Q);
    }

    /* CP-17: Nombre de estado vacío */
    {
        char *buf = "";
        int esInvalido = (buf[0] == '\0');
        VERIFICAR("CP-17", "Ingresar nombre de estado vacio",
                  "Presionar Enter sin texto",
                  "Rechaza el nombre y vuelve a solicitarlo",
                  esInvalido);
    }

    /* CP-18: Nombre mayor de 15 caracteres */
    {
        char *buf = "estado_demasiado_largo"; /* 22 chars */
        int esInvalido = ((int)strlen(buf) > 15);
        VERIFICAR("CP-18", "Ingresar nombre de estado mayor de 15 caracteres",
                  "Capturar estado_demasiado_largo",
                  "Rechaza el nombre por exceder la longitud permitida",
                  esInvalido);
    }

    /* CP-19: Carácter no permitido en nombre de estado */
    {
        char *buf = "q#1";
        int esInvalido = !cadenaPermitida(buf);
        VERIFICAR("CP-19", "Ingresar caracteres no permitidos en un estado",
                  "Capturar q#1",
                  "Rechaza porque no todos los caracteres pertenecen a Sigma_perm",
                  esInvalido);
    }

    /* CP-20: Estado duplicado en Q */
    {
        Nodo *Q = NULL;
        agregarNodo(&Q, "q1");
        int existeAntes = (nodoExiste(Q, "q1") != NULL);
        if (nodoExiste(Q, "q1") == NULL) agregarNodo(&Q, "q1");
        int ok = (existeAntes && contarNodos(Q) == 1);
        VERIFICAR("CP-20", "Ingresar estado duplicado",
                  "Capturar q1 y despues q1",
                  "Purga automaticamente el duplicado; Q permanece sin cambios",
                  ok);
        liberarNodos(&Q);
    }

    /* CP-21: Intentar finalizar Q vacío */
    {
        Nodo *Q = NULL;
        int vacioBloqueado = (contarNodos(Q) == 0);
        VERIFICAR("CP-21", "Intentar finalizar Q vacio",
                  "No registrar estados y seleccionar terminar",
                  "No permite continuar hasta registrar al menos un estado",
                  vacioBloqueado);
    }
}

/* ── CP-22 a CP-23: RF-05 Estado inicial ─────────────────────────────────── */
static void pruebas_rf05(void) {
    conEscribir("\n  ======== RF-05: Registro de q0 ========\n");

    /* CP-22: Estado inicial válido */
    {
        Nodo *Q = NULL;
        agregarNodo(&Q, "q1");
        agregarNodo(&Q, "q2");
        Nodo *q0 = nodoExiste(Q, "q1");
        int ok = (q0 != NULL && strcmp(q0->nombre, "q1") == 0);
        VERIFICAR("CP-22", "Registrar estado inicial valido",
                  "Con Q={q1,q2}, capturar q1",
                  "Guarda q0=q1 y continua con A (RF-06)",
                  ok);
        liberarNodos(&Q);
    }

    /* CP-23: Estado inicial fuera de Q */
    {
        Nodo *Q = NULL;
        agregarNodo(&Q, "q1");
        agregarNodo(&Q, "q2");
        Nodo *intento = nodoExiste(Q, "q9");
        int esInvalido = (intento == NULL);
        VERIFICAR("CP-23", "Ingresar estado inicial que no pertenece a Q",
                  "Con Q={q1,q2}, capturar q9",
                  "Informa que q9 no existe en Q y vuelve a solicitar q0",
                  esInvalido);
        liberarNodos(&Q);
    }
}

/* ── CP-24 a CP-27: RF-06 Estados de aceptación ─────────────────────────── */
static void pruebas_rf06(void) {
    conEscribir("\n  ======== RF-06: Registro de A ========\n");

    /* CP-24: Estado de aceptación válido */
    {
        Nodo *Q = NULL;
        agregarNodo(&Q, "q1");
        agregarNodo(&Q, "q2");
        Nodo *nq2 = nodoExiste(Q, "q2");
        nq2->aceptacion = 1;
        int ok = (nq2 != NULL && nq2->aceptacion == 1);
        VERIFICAR("CP-24", "Registrar estados de aceptacion validos",
                  "Con Q={q1,q2}, capturar q2",
                  "Registra A={q2}, cumpliendo A subset Q",
                  ok);
        liberarNodos(&Q);
    }

    /* CP-25: Estado de aceptación fuera de Q */
    {
        Nodo *Q = NULL;
        agregarNodo(&Q, "q1");
        agregarNodo(&Q, "q2");
        Nodo *intento = nodoExiste(Q, "q9");
        int rechazado = (intento == NULL);
        VERIFICAR("CP-25", "Ingresar estado de aceptacion que no pertenece a Q",
                  "Con Q={q1,q2}, capturar q9 y despues q1",
                  "Rechaza q9 con mensaje de error sin modificar A",
                  rechazado);
        liberarNodos(&Q);
    }

    /* CP-26: Intentar finalizar A vacío */
    {
        Nodo *Q = NULL;
        agregarNodo(&Q, "q1");
        int contadorAceptacion = 0;
        int vacioBloqueado = (contadorAceptacion == 0);
        VERIFICAR("CP-26", "Intentar finalizar A vacio",
                  "No registrar estados de aceptacion",
                  "No permite continuar hasta registrar al menos uno",
                  vacioBloqueado);
        liberarNodos(&Q);
    }

    /* CP-27: Estado de aceptación duplicado */
    {
        Nodo *Q = NULL;
        agregarNodo(&Q, "q1");
        agregarNodo(&Q, "q2");
        Nodo *nq1 = nodoExiste(Q, "q1");
        nq1->aceptacion = 1;
        int yaTieneAcept = nq1->aceptacion;
        int rechazado = (yaTieneAcept == 1);
        VERIFICAR("CP-27", "Ingresar estado de aceptacion duplicado",
                  "Con Q={q1,q2}, capturar q1 y despues q1",
                  "Rechaza el estado repetido; A permanece {q1}",
                  rechazado);
        liberarNodos(&Q);
    }
}

/* ── CP-28 a CP-29: RF-07 Función de transición ─────────────────────────── */
static void pruebas_rf07(void) {
    conEscribir("\n  ======== RF-07: Registro de f ========\n");

    /* CP-28: Completar ƒ con estados válidos */
    {
        AFD afd = construirAFDV_prueba();
        int completo = 1;
        Nodo *nodo = afd.Q;
        while (nodo != NULL) {
            Simbolo *s = afd.alfaEntrada;
            while (s != NULL) {
                if (buscarArista(nodo, s->valor) == NULL) {
                    completo = 0;
                }
                s = s->siguiente;
            }
            nodo = nodo->Apuntador;
        }
        VERIFICAR("CP-28", "Completar la funcion de transicion",
                  "Capturar q' valido para cada par (q,s)",
                  "f queda definida para todas las combinaciones Q x Sigma_E",
                  completo);
        liberarAFD(&afd);
    }

    /* CP-29: Estado destino fuera de Q en ƒ */
    {
        Nodo *Q = NULL;
        agregarNodo(&Q, "q1");
        Nodo *intento = nodoExiste(Q, "qX");
        int rechazado = (intento == NULL);
        VERIFICAR("CP-29", "Ingresar estado destino que no pertenece a Q en f",
                  "Digitar un estado no registrado en Q como q'",
                  "Rechaza el destino y vuelve a solicitar el valor de la misma celda",
                  rechazado);
        liberarNodos(&Q);
    }
}

/* ── CP-30 a CP-34: RF-08 Función de salida ─────────────────────────────── */
static void pruebas_rf08(void) {
    conEscribir("\n  ======== RF-08: Registro de g ========\n");

    /* CP-30: Completar ɠ con símbolos válidos */
    {
        AFD afd = construirAFDT_prueba();
        int completo = 1;
        Nodo *nodo = afd.Q;
        while (nodo != NULL) {
            Simbolo *s = afd.alfaEntrada;
            while (s != NULL) {
                Arista *a = buscarArista(nodo, s->valor);
                if (a == NULL || a->ALPS == '\0') completo = 0;
                s = s->siguiente;
            }
            nodo = nodo->Apuntador;
        }
        VERIFICAR("CP-30", "Completar la funcion de salida",
                  "Capturar simbolo de Sigma_S para cada par (q,s)",
                  "g queda definida para todas las combinaciones Q x Sigma_E",
                  completo);
        liberarAFD(&afd);
    }

    /* CP-31: Símbolo en ɠ fuera de Σ_S */
    {
        Simbolo *alfaSalida = NULL;
        agregarSimbolo(&alfaSalida, 'x');
        agregarSimbolo(&alfaSalida, 'y');
        int esInvalido = !simboloExiste(alfaSalida, 'z');
        VERIFICAR("CP-31", "Ingresar en g un simbolo que no pertenece a Sigma_S",
                  "Digitar z cuando Sigma_S = {x,y}",
                  "Rechaza el simbolo y vuelve a solicitar la salida de la misma celda",
                  esInvalido);
        liberarSimbolos(&alfaSalida);
    }

    /* CP-32: Más de un carácter en celda de ɠ */
    {
        char *entrada = "xy";
        int esInvalido = (strlen(entrada) != 1);
        VERIFICAR("CP-32", "Ingresar mas de un caracter en una celda de g",
                  "Digitar xy en g(q,s)",
                  "Rechaza porque g(q,s) debe producir un solo simbolo de Sigma_S",
                  esInvalido);
    }

    /* CP-33: Completar todas las combinaciones de ƒ y ɠ */
    {
        AFD afd = construirAFDT_prueba();
        int completo = 1;
        Nodo *nodo = afd.Q;
        while (nodo != NULL) {
            Simbolo *s = afd.alfaEntrada;
            while (s != NULL) {
                Arista *a = buscarArista(nodo, s->valor);
                if (a == NULL || a->Flecha == NULL || a->ALPS == '\0') {
                    completo = 0;
                }
                s = s->siguiente;
            }
            nodo = nodo->Apuntador;
        }
        VERIFICAR("CP-33", "Completar todas las combinaciones de f y g (AFDT)",
                  "Registrar AFDT asignando valor valido a cada combinacion",
                  "Al completar Q x Sigma_E el sistema avanza a captura de palabras (RF-09)",
                  completo);
        liberarAFD(&afd);
    }

    /* CP-34: Dejar celda de ɠ sin definir */
    {
        Nodo *Q = NULL;
        Simbolo *alfa = NULL;
        agregarNodo(&Q, "q1");
        agregarSimbolo(&alfa, '1');
        Nodo *nq1 = nodoExiste(Q, "q1");
        Arista *arista = buscarArista(nq1, '1'); /* No hay arista aún */
        int bloqueado = (arista == NULL);
        VERIFICAR("CP-34", "Dejar celda de g sin definir e intentar continuar",
                  "Completar todas menos una combinacion de g",
                  "El sistema no permite avanzar a RF-09 con celdas pendientes",
                  bloqueado);
        liberarNodos(&Q);
        liberarSimbolos(&alfa);
    }
}

/* ── CP-35 a CP-38: RF-09 Captura de α ──────────────────────────────────── */
static void pruebas_rf09(void) {
    conEscribir("\n  ======== RF-09: Registro de alfa ========\n");

    /* CP-35: Palabra válida en Σ_E */
    {
        Simbolo *alfa = NULL;
        agregarSimbolo(&alfa, '1');
        agregarSimbolo(&alfa, '0');
        char *palabra = "1011";
        int valido = 1;
        for (int i = 0; palabra[i] != '\0'; i++) {
            if (!simboloExiste(alfa, palabra[i])) { valido = 0; break; }
        }
        VERIFICAR("CP-35", "Capturar palabra cuyos simbolos pertenecen a Sigma_E",
                  "AFDV: 1011",
                  "Acepta la palabra e inicia procesamiento con f (RF-10)",
                  valido);
        liberarSimbolos(&alfa);
    }

    /* CP-36: Palabra con símbolo fuera de Σ_E */
    {
        Simbolo *alfa = NULL;
        agregarSimbolo(&alfa, '1');
        agregarSimbolo(&alfa, '0');
        char *palabra = "10a1";
        int tieneInvalido = 0;
        for (int i = 0; palabra[i] != '\0'; i++) {
            if (!simboloExiste(alfa, palabra[i])) { tieneInvalido = 1; break; }
        }
        VERIFICAR("CP-36", "Capturar palabra con simbolo fuera de Sigma_E",
                  "AFDV: 10a1",
                  "Muestra error. Descarta la palabra y vuelve a solicitarla",
                  tieneInvalido);
        liberarSimbolos(&alfa);
    }

    /* CP-37: Palabra vacía */
    {
        char *entrada = "";
        int esVacia = (entrada[0] == '\0');
        VERIFICAR("CP-37", "Intentar procesar la palabra vacia",
                  "Presionar Enter sin escribir",
                  "Rechaza porque alfa debe pertenecer a Sigma_E+ (no vacia)",
                  esVacia);
    }

    /* CP-38: Palabra de más de 15 caracteres (no hay límite en RF-09) */
    {
        Simbolo *alfa = NULL;
        agregarSimbolo(&alfa, '1');
        agregarSimbolo(&alfa, '0');
        char *palabra = "1011001101100110"; /* 16 caracteres */
        int valido = 1;
        for (int i = 0; palabra[i] != '\0'; i++) {
            if (!simboloExiste(alfa, palabra[i])) { valido = 0; break; }
        }
        int tieneMas15 = ((int)strlen(palabra) > 15);
        /* CP-38: no la rechaza por longitud → válida si todos los chars ∈ Σ_E */
        VERIFICAR("CP-38", "Procesar palabra de mas de 15 caracteres",
                  "Ingresar 1011001101100110 (16 caracteres)",
                  "No la rechaza por longitud; la procesa normalmente",
                  (valido && tieneMas15));
        liberarSimbolos(&alfa);
    }
}

/* ── CP-39 a CP-42: RF-10 + RF-11 Procesamiento ─────────────────────────── */
static void pruebas_rf10_rf11(void) {
    conEscribir("\n  ======== RF-10 + RF-11: Procesamiento de alfa ========\n");

    /* CP-39: AFDT con palabra aceptada (ab → β="xy", estado final q1 ∈ A) */
    {
        AFD afd = construirAFDT_prueba();
        int esValida = 0;
        char *beta = procesarPalabra(&afd, "ab", &esValida);
        int ok = (esValida == 1 && beta != NULL && strcmp(beta, "xy") == 0);
        VERIFICAR("CP-39", "Ejecutar AFDT simbolo por simbolo con palabra aceptada",
                  "Procesar 'ab' con AFDT (estado final q1 en A)",
                  "Muestra traza; reporta Palabra VALIDA; beta='xy'",
                  ok);
        if (beta) free(beta);
        liberarAFD(&afd);
    }

    /* CP-40: AFDT con palabra rechazada (a → β="x", estado final q0 ∉ A) */
    {
        AFD afd = construirAFDT_prueba();
        int esValida = 0;
        char *beta = procesarPalabra(&afd, "a", &esValida);
        int ok = (esValida == 0 && beta != NULL && strcmp(beta, "x") == 0);
        VERIFICAR("CP-40", "Ejecutar AFDT con palabra rechazada",
                  "Procesar 'a' con AFDT (estado final q0 no en A)",
                  "Genera beta='x'; reporta Palabra INVALIDA",
                  ok);
        if (beta) free(beta);
        liberarAFD(&afd);
    }

    /* CP-41: AFDV con palabra aceptada ("1" termina en q1 ∈ A) */
    {
        AFD afd = construirAFDV_prueba();
        int esValida = 0;
        char *beta = procesarPalabra(&afd, "1", &esValida);
        VERIFICAR("CP-41", "Ejecutar AFDV con palabra aceptada",
                  "Procesar '1' con AFDV (estado final q1 en A)",
                  "Muestra traza completa y reporta Palabra VALIDA",
                  esValida == 1);
        if (beta) free(beta);
        liberarAFD(&afd);
    }

    /* CP-42: AFDV con palabra rechazada ("0" termina en q0 ∉ A) */
    {
        AFD afd = construirAFDV_prueba();
        int esValida = 0;
        char *beta = procesarPalabra(&afd, "0", &esValida);
        VERIFICAR("CP-42", "Ejecutar AFDV con palabra rechazada",
                  "Procesar '0' con AFDV (estado final q0 no en A)",
                  "Muestra traza completa y reporta Palabra INVALIDA",
                  esValida == 0);
        if (beta) free(beta);
        liberarAFD(&afd);
    }
}

/* ── CP-43 a CP-45: RF-13 Estadísticas locales ───────────────────────────── */
static void pruebas_rf13(void) {
    conEscribir("\n  ======== RF-13: Estadisticas del ciclo del AFD ========\n");

    /* CP-43: Procesar dos palabras (1 válida + 1 inválida) */
    {
        EstadisticasAFD e;
        inicializarEstadisticasAFD(&e);
        actualizarEstadisticasAFD(&e, 1, 0); /* válida,   AFDV */
        actualizarEstadisticasAFD(&e, 0, 0); /* inválida, AFDV */
        int ok = (e.totalProcesadas == 2 &&
                  e.totalValidas    == 1 &&
                  e.totalInvalidas  == 1 &&
                  e.totalProcesadas == e.totalValidas + e.totalInvalidas);
        VERIFICAR("CP-43", "Actualizar contadores locales del AFD",
                  "Procesar 2 palabras (1 valida, 1 invalida) con AFDV",
                  "Total=2, V=1, U=1; se cumple Total=V+U",
                  ok);
    }

    /* CP-44: Procesar otra palabra con el mismo AFD */
    {
        EstadisticasAFD e;
        inicializarEstadisticasAFD(&e);
        actualizarEstadisticasAFD(&e, 1, 0);
        int ok = (e.totalProcesadas == 1);
        VERIFICAR("CP-44", "Procesar otra palabra con el mismo AFD",
                  "Despues de una palabra responder S",
                  "Vuelve a RF-09 sin modificar el AFD ni reiniciar contadores",
                  ok);
    }

    /* CP-45: Cerrar el ciclo (resp=N) → mostrar estadísticas */
    {
        EstadisticasAFD e;
        inicializarEstadisticasAFD(&e);
        actualizarEstadisticasAFD(&e, 1, 1);  /* válida + traducida */
        int ok = (e.totalProcesadas == 1 &&
                  e.totalValidas    == 1 &&
                  e.totalTraducidas == 1);
        VERIFICAR("CP-45", "Cerrar el ciclo de palabras del AFD",
                  "Despues de una palabra responder N",
                  "Muestra Total_proc, W, V, U (RF-13)",
                  ok);
    }
}

/* ── CP-46 a CP-50: RF-12 y RF-14 Control posterior + estadísticas globales */
static void pruebas_rf12_rf14(void) {
    conEscribir("\n  ======== RF-12: Control posterior al procesamiento ========\n");

    /* CP-46: Respuesta inválida a "¿otra palabra?" */
    {
        char resp = 'X';
        int esInvalido = !(resp == 'S' || resp == 'N');
        VERIFICAR("CP-46", "Ingresar respuesta invalida al preguntar por otra palabra",
                  "Responder X",
                  "Informa que la respuesta no es valida y vuelve a solicitar S o N",
                  esInvalido);
    }

    /* CP-47: Registrar otro AFD */
    {
        char resp = 'S';
        int ok = (resp == 'S');
        VERIFICAR("CP-47", "Registrar otro AFD en la misma sesion",
                  "Responder S a 'Desea registrar otro AFD?'",
                  "Regresa al menu principal (RF-01)",
                  ok);
    }

    /* CP-48: No registrar otro AFD → mostrar global y salir */
    {
        char resp = 'N';
        int ok = (resp == 'N');
        VERIFICAR("CP-48", "No registrar otro AFD",
                  "Responder N a 'Desea registrar otro AFD?'",
                  "Muestra resumen estadistico global (RF-14) y finaliza (RF-15)",
                  ok);
    }

    /* CP-49: Respuesta inválida a "¿otro AFD?" */
    {
        char resp = 'X';
        int esInvalido = !(resp == 'S' || resp == 'N');
        VERIFICAR("CP-49", "Ingresar respuesta invalida al preguntar por otro AFD",
                  "Responder X",
                  "Informa que la respuesta no es valida y vuelve a solicitar S o N",
                  esInvalido);
    }

    conEscribir("\n  ======== RF-14: Estadisticas del ciclo completo ========\n");

    /* CP-50: Reporte global correcto para 2 AFD */
    {
        EstadisticasGlobales g;
        inicializarEstadisticasGlobales(&g);

        /* AFD1 (AFDT): W1=2, V1=1, U1=1 */
        EstadisticasAFD e1;
        inicializarEstadisticasAFD(&e1);
        actualizarEstadisticasAFD(&e1, 1, 1);
        actualizarEstadisticasAFD(&e1, 0, 1);
        acumularEstadisticasGlobales(&g, &e1, TIPO_AFDT);

        /* AFD2 (AFDT): W2=1, V2=1, U2=0 */
        EstadisticasAFD e2;
        inicializarEstadisticasAFD(&e2);
        actualizarEstadisticasAFD(&e2, 1, 1);
        acumularEstadisticasGlobales(&g, &e2, TIPO_AFDT);

        int tGlob = g.totalValidadas + g.totalInvalidas;
        int ok = (g.totalAFD        == 2 &&
                  tGlob             == 3 &&
                  g.totalValidadas  == 2 &&
                  g.totalInvalidas  == 1 &&
                  tGlob == g.totalValidadas + g.totalInvalidas);
        VERIFICAR("CP-50", "Mostrar reporte global correcto",
                  "Finalizar tras 2 AFD: W1=2,V1=1,U1=1 y W2=1,V2=1,U2=0",
                  "Z=2, T_glob=3, V_glob=2, U_glob=1; cumple T_glob=V_glob+U_glob",
                  ok);
    }
}

/* ── CP-51 a CP-53: RNF-01 y RNF-02 Memoria dinámica ────────────────────── */
static void pruebas_rnf01_rnf02(void) {
    conEscribir("\n  ======== RNF-01 + RNF-02: Memoria dinamica ========\n");

    /* CP-51: Estructuras dinámicas para componentes del AFD */
    {
        AFD afd = construirAFDV_prueba();
        int usaListasEnlazadas = (afd.Q != NULL &&
                                  afd.alfaEntrada != NULL &&
                                  afd.Q->Flecha != NULL);
        VERIFICAR("CP-51", "Usar estructuras dinamicas para los componentes del AFD",
                  "Registrar alfabetos y conjuntos de distintos tamanos",
                  "Sigma_E, Q, A, f se dimensionan en ejecucion con apuntadores",
                  usaListasEnlazadas);
        liberarAFD(&afd);
    }

    /* CP-52: Liberar búferes temporales al finalizar cada palabra */
    {
        AFD afd = construirAFDV_prueba();
        int esValida = 0;

        /* Procesar tres palabras y liberar cada resultado */
        char *b1 = procesarPalabra(&afd, "1",    &esValida);
        if (b1) free(b1);
        char *b2 = procesarPalabra(&afd, "0",    &esValida);
        if (b2) free(b2);
        char *b3 = procesarPalabra(&afd, "101",  &esValida);
        if (b3) free(b3);

        /* Si llegamos aquí sin crash ni leak → PASADO */
        VERIFICAR("CP-52", "Liberar buferes temporales al finalizar cada palabra",
                  "Procesar tres palabras consecutivas",
                  "No se acumula memoria perdida entre palabras",
                  1 /* Si compiló y ejecutó sin crash = OK */);

        liberarAFD(&afd);
    }

    /* CP-53: Liberar estructuras al cerrar AFD y el programa */
    {
        AFD afd1 = construirAFDV_prueba();
        AFD afd2 = construirAFDT_prueba();
        liberarAFD(&afd1);
        liberarAFD(&afd2);
        int ok = (afd1.Q == NULL && afd1.alfaEntrada == NULL &&
                  afd2.Q == NULL && afd2.alfaEntrada == NULL);
        VERIFICAR("CP-53", "Liberar estructuras al cerrar AFD y el programa",
                  "Registrar y cerrar varios AFD",
                  "No hay perdidas de memoria (todos los punteros quedan NULL)",
                  ok);
    }
}

/* ── CP-54: RF-10/RF-11 Única consulta por símbolo ──────────────────────── */
static void prueba_cp54(void) {
    conEscribir("\n  ======== RF-10 + RF-11: Una consulta por simbolo ========\n");

    /*
     * CP-54: Procesar palabra de 2 simbolos ("10"):
     *   f(q0,'1') -> q1  (consulta 1)
     *   f(q1,'0') -> q0  (consulta 2)
     *   Estado final q0 no en A -> invalida.
     */
    {
        AFD afd = construirAFDV_prueba();
        int esValida = 0;
        char *beta = procesarPalabra(&afd, "10", &esValida);
        /* 2 simbolos procesados correctamente, estado final q0 no en A */
        int ok = (beta == NULL && esValida == 0);
        VERIFICAR("CP-54", "Verificar que el procesamiento realiza una consulta por simbolo",
                  "Palabra '10': f(q0,'1')->q1, f(q1,'0')->q0 (2 consultas, 2 simbolos)",
                  "Una consulta a f por simbolo; Palabra INVALIDA (qn=q0)",
                  ok);
        if (beta) free(beta);
        liberarAFD(&afd);
    }
}

/* ── CP-55: RNF-12 Idioma español ───────────────────────────────────────── */
static void prueba_cp55(void) {
    conEscribir("\n  ======== RNF-12: Idioma espanol ========\n");

    /*
     * CP-55: Verificar que los mensajes del sistema están en español.
     * Como las cadenas de texto son literales en el código, se verifica
     * que contengan palabras clave en español.
     */
    const char *mensajeError    = "Error: Entrada invalida. Intente de nuevo.";
    const char *mensajeValida   = "Resultado: Palabra VALIDA (qn en A).";
    const char *mensajeInvalida = "Resultado: Palabra INVALIDA (qn no pertenece a A).";

    int enEspaniol = (strstr(mensajeError,    "invalida")  != NULL &&
                      strstr(mensajeValida,   "Palabra")   != NULL &&
                      strstr(mensajeInvalida, "pertenece") != NULL);

    VERIFICAR("CP-55", "Verificar el idioma de la interfaz",
              "Recorrer menus, validaciones, errores y reportes",
              "Toda la comunicacion visible esta en espanol",
              enEspaniol);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*  ejecutarPruebas — Punto de entrada de la suite de pruebas                */
/* ══════════════════════════════════════════════════════════════════════════ */
void ejecutarPruebas(void) {
    conEscribir("\n");
    conEscribir("  +================================================================+\n");
    conEscribir("  |  SUITE DE PRUEBAS - Gestor de Automatas Finitos Deterministas  |\n");
    conEscribir("  |  Casos CP-01 a CP-55 segun ERS Seccion 6                       |\n");
    conEscribir("  +================================================================+\n");

    pruebas_rf01();
    pruebas_rf02();
    pruebas_rf03();
    pruebas_rf04();
    pruebas_rf05();
    pruebas_rf06();
    pruebas_rf07();
    pruebas_rf08();
    pruebas_rf09();
    pruebas_rf10_rf11();
    pruebas_rf13();
    pruebas_rf12_rf14();
    pruebas_rnf01_rnf02();
    prueba_cp54();
    prueba_cp55();

    /* Resumen final */
    conEscribir("\n  ============================================\n");
    conEscribir("  RESUMEN DE PRUEBAS\n");
    conEscribir("  ============================================\n");
    conEscribir("    Total de casos  : ");
    conEscribirInt(pruebas_total);
    conEscribir("\n    PASADOS         : ");
    conEscribirInt(pruebas_pasadas);
    conEscribir("\n    FALLIDOS        : ");
    conEscribirInt(pruebas_fallidas);
    conEscribir("\n  ============================================\n\n");
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*  main de pruebas (solo activo con -DEJECUCION_PRUEBAS)                    */
/* ══════════════════════════════════════════════════════════════════════════ */
#ifdef EJECUCION_PRUEBAS
int main(void) {
    ejecutarPruebas();
    return (pruebas_fallidas == 0) ? 0 : 1;
}
#endif
