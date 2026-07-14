/*
 * =============================================================================
 * Archivo      : afd.c
 * Proyecto     : Gestor de Automatas Finitos Deterministas
 * Autor        : Victor Garza Maldonado
 * Fecha        : Julio 2026
 * Revision     : 00.01
 * =============================================================================
 * Descripcion:
 *   Implementacion de todas las funciones de registro y procesamiento de AFD.
 *
 *   Requerimientos cubiertos:
 *     RF-02  -- Registrar Sigma_E
 *     RF-03  -- Registrar Sigma_S (solo AFDT)
 *     RF-04  -- Registrar Q
 *     RF-05  -- Registrar q0
 *     RF-06  -- Registrar A
 *     RF-07  -- Registrar f
 *     RF-08  -- Registrar g (solo AFDT)
 *     RF-09  -- Registrar la palabra de entrada
 *     RF-10  -- Procesar la palabra con f
 *     RF-11  -- Procesar la palabra con g (solo AFDT)
 *     RNF-01 -- Memoria dinamica con apuntadores
 *     RNF-02 -- Liberacion de memoria dinamica
 *     RNF-05 -- Manejo de errores sin exponer codigo fuente
 *     RNF-12 -- Idioma espanol
 * =============================================================================
 */

#include "../include/afd.h"
#include "../include/menu.h"

/* -------------------------------------- */
/*  SECCION 1: Validacion de caracteres   */
/* -------------------------------------- */

/*
 * esPermitido -- RF-02, RF-03, RF-04
 * Proposito   : Verificar si c pertenece a Sigma_perm.
 *               Sigma_perm = {a..z, A..Z, 0..9, vocales con tilde}
 * Parametros  : c -- caracter a validar (unsigned char para evitar UB)
 * Retorna     : 1 si c pertenece a Sigma_perm, 0 si no.
 */
int esPermitido(unsigned char c) {
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9')) {
        return 1;
    }
    /*
     * Caracteres especiales del espanol (codificacion Latin-1 / CP1252).
     */
    switch (c) {
        case 0xE1: case 0xE9: case 0xED: case 0xF3: case 0xFA:
        case 0xC1: case 0xC9: case 0xCD: case 0xD3: case 0xDA:
        case 0xF1: case 0xD1:
            return 1;
        default:
            return 0;
    }
}

/*
 * cadenaPermitida -- RF-04
 * Proposito   : Verificar que todos los chars de s pertenezcan a Sigma_perm.
 * Parametros  : s -- cadena a verificar
 * Retorna     : 1 si todos pertenecen, 0 si alguno no.
 */
int cadenaPermitida(const char *s) {
    if (s == NULL || s[0] == '\0') return 0;
    for (int i = 0; s[i] != '\0'; i++) {
        if (!esPermitido((unsigned char)s[i])) return 0;
    }
    return 1;
}

/* ------------------------------------------------------ */
/*  SECCION 2: Gestion de Simbolos (Sigma_E y Sigma_S)   */
/* ------------------------------------------------------ */

/*
 * simboloExiste -- RF-02, RF-03
 * Proposito   : Detectar simbolo duplicado en un alfabeto.
 * Parametros  : cabeza -- cabeza de la lista; c -- caracter buscado
 * Retorna     : 1 si c ya esta registrado, 0 si no.
 */
int simboloExiste(const Simbolo *cabeza, char c) {
    const Simbolo *actual = cabeza;
    while (actual != NULL) {
        if (actual->valor == c) return 1;
        actual = actual->siguiente;
    }
    return 0;
}

/*
 * agregarSimbolo -- RF-02, RF-03
 * Proposito   : Crear y enlazar un nuevo Simbolo al final de la lista.
 *               RNF-01: asignacion dinamica con malloc.
 * Parametros  : cabeza -- puntero a la cabeza de la lista (se modifica)
 *               c      -- caracter del simbolo
 * Retorna     : (void)
 */
void agregarSimbolo(Simbolo **cabeza, char c) {
    Simbolo *nuevo = (Simbolo *)malloc(sizeof(Simbolo));
    if (nuevo == NULL) {
        conEscribirError("Error: No se pudo asignar memoria.\n");
        return;
    }
    nuevo->valor     = c;
    nuevo->siguiente = NULL;

    if (*cabeza == NULL) {
        *cabeza = nuevo;
        return;
    }
    Simbolo *actual = *cabeza;
    while (actual->siguiente != NULL) {
        actual = actual->siguiente;
    }
    actual->siguiente = nuevo;
}

/*
 * contarSimbolos -- RF-02, RF-03
 * Proposito   : Contar los simbolos en una lista de Simbolos.
 * Parametros  : cabeza -- cabeza de la lista
 * Retorna     : numero de simbolos (int)
 */
int contarSimbolos(const Simbolo *cabeza) {
    int count = 0;
    const Simbolo *actual = cabeza;
    while (actual != NULL) {
        count++;
        actual = actual->siguiente;
    }
    return count;
}

/*
 * liberarSimbolos -- RNF-02
 * Proposito   : Liberar toda la memoria de una lista de Simbolos.
 * Parametros  : cabeza -- puntero a la cabeza (queda en NULL)
 * Retorna     : (void)
 */
void liberarSimbolos(Simbolo **cabeza) {
    Simbolo *actual = *cabeza;
    while (actual != NULL) {
        Simbolo *siguiente = actual->siguiente;
        free(actual);
        actual = siguiente;
    }
    *cabeza = NULL;
}

/* ------------------------------- */
/*  SECCION 3: Gestion de Nodos (Q)*/
/* ------------------------------- */

/*
 * nodoExiste -- RF-04, RF-05, RF-06, RF-07
 * Proposito   : Buscar un nodo por nombre en la lista Q.
 * Parametros  : cabeza -- cabeza de la lista Q; nombre -- nombre del estado
 * Retorna     : puntero al Nodo si existe, NULL si no.
 */
Nodo *nodoExiste(const Nodo *cabeza, const char *nombre) {
    const Nodo *actual = cabeza;
    while (actual != NULL) {
        if (strcmp(actual->nombre, nombre) == 0) return (Nodo *)actual;
        actual = actual->Apuntador;
    }
    return NULL;
}

/*
 * agregarNodo -- RF-04
 * Proposito   : Crear y enlazar un nuevo Nodo al final de Q.
 *               RNF-01: asignacion dinamica con malloc.
 * Parametros  : cabeza -- puntero a la cabeza de Q (se modifica)
 *               nombre -- nombre del estado
 * Retorna     : (void)
 */
void agregarNodo(Nodo **cabeza, const char *nombre) {
    Nodo *nuevo = (Nodo *)malloc(sizeof(Nodo));
    if (nuevo == NULL) {
        conEscribirError("Error: No se pudo asignar memoria.\n");
        return;
    }
    strncpy(nuevo->nombre, nombre, MAX_ESTADO - 1);
    nuevo->nombre[MAX_ESTADO - 1] = '\0';
    nuevo->aceptacion = 0;
    nuevo->Apuntador  = NULL;
    nuevo->Flecha     = NULL;

    if (*cabeza == NULL) {
        *cabeza = nuevo;
        return;
    }
    Nodo *actual = *cabeza;
    while (actual->Apuntador != NULL) {
        actual = actual->Apuntador;
    }
    actual->Apuntador = nuevo;
}

/*
 * contarNodos -- RF-04
 * Proposito   : Contar nodos en la lista Q.
 * Parametros  : cabeza -- cabeza de Q
 * Retorna     : numero de nodos (int)
 */
int contarNodos(const Nodo *cabeza) {
    int count = 0;
    const Nodo *actual = cabeza;
    while (actual != NULL) {
        count++;
        actual = actual->Apuntador;
    }
    return count;
}

/*
 * liberarAristas -- RNF-02 (auxiliar interno)
 * Proposito   : Liberar la lista de Aristas de un Nodo.
 */
static void liberarAristas(Arista **cabeza) {
    Arista *actual = *cabeza;
    while (actual != NULL) {
        Arista *siguiente = actual->Apunt;
        free(actual);
        actual = siguiente;
    }
    *cabeza = NULL;
}

/*
 * liberarNodos -- RNF-02
 * Proposito   : Liberar todos los Nodos de Q y sus Aristas asociadas.
 * Parametros  : cabeza -- puntero a la cabeza de Q (queda en NULL)
 * Retorna     : (void)
 */
void liberarNodos(Nodo **cabeza) {
    Nodo *actual = *cabeza;
    while (actual != NULL) {
        Nodo *siguiente = actual->Apuntador;
        liberarAristas(&actual->Flecha);
        free(actual);
        actual = siguiente;
    }
    *cabeza = NULL;
}

/* ------------------------------------- */
/*  SECCION 4: Gestion de Aristas (f y g)*/
/* ------------------------------------- */

/*
 * buscarArista -- RF-10, RF-11
 * Proposito   : Localizar la Arista de 'nodo' disparada por 'simbolo'.
 *               Implementa la consulta f(q, s).
 * Parametros  : nodo    -- estado origen; simbolo -- simbolo leido
 * Retorna     : puntero a la Arista, o NULL si no esta definida.
 */
Arista *buscarArista(const Nodo *nodo, char simbolo) {
    Arista *actual = nodo->Flecha;
    while (actual != NULL) {
        if (actual->ALFE == simbolo) return actual;
        actual = actual->Apunt;
    }
    return NULL;
}

/*
 * agregarArista -- RF-07, RF-08
 * Proposito   : Crear y enlazar una Arista al nodo origen.
 *               RNF-01: asignacion dinamica con malloc.
 * Parametros  : nodoOrigen -- nodo fuente de la transicion
 *               ALFE       -- simbolo de entrada (Sigma_E)
 *               ALPS       -- simbolo de salida (Sigma_S); '\0' para AFDV
 *               destino    -- nodo destino q'
 * Retorna     : (void)
 */
void agregarArista(Nodo *nodoOrigen, char ALFE, char ALPS, Nodo *destino) {
    Arista *nueva = (Arista *)malloc(sizeof(Arista));
    if (nueva == NULL) {
        conEscribirError("Error: No se pudo asignar memoria.\n");
        return;
    }
    nueva->ALFE   = ALFE;
    nueva->ALPS   = ALPS;
    nueva->Apunt  = NULL;
    nueva->Flecha = destino;

    if (nodoOrigen->Flecha == NULL) {
        nodoOrigen->Flecha = nueva;
        return;
    }
    Arista *actual = nodoOrigen->Flecha;
    while (actual->Apunt != NULL) {
        actual = actual->Apunt;
    }
    actual->Apunt = nueva;
}

/* --------------------------- */
/*  SECCION 5: Registro del AFD*/
/* --------------------------- */

/*
 * registrarAlfaEntrada -- RF-02
 * Proposito   : Permitir al usuario registrar Sigma_E simbolo por simbolo.
 *               Valida: |s|=1, s en Sigma_perm, s no vacio, s no en Sigma_E.
 *               Repite hasta que el usuario decida terminar (resp=N)
 *               y Sigma_E contenga al menos un simbolo (CP-09).
 * Parametros  : afd -- puntero al AFD en construccion
 * Retorna     : (void)
 */
void registrarAlfaEntrada(AFD *afd) {
    char buf[MAX_BUF];
    char resp;

    resp = 'S';
    do {
        limpiarPantalla();
        mostrarEncabezado();
        conEscribir("\n  -- Registro del Alfabeto de s\xedmbolos de entrada (Sigma_E) --\n\n");

        /* Mostrar Sigma_E actual */
        conEscribir("  S\xedmbolos registrados en el Alfabeto de s\xedmbolos de entrada (Sigma_E): { ");
        const Simbolo *actual = afd->alfaEntrada;
        if (actual == NULL) {
            conEscribir("(vacio)");
        } else {
            while (actual != NULL) {
                conEscribirChar(actual->valor);
                if (actual->siguiente != NULL) conEscribir(", ");
                actual = actual->siguiente;
            }
        }
        conEscribir(" }\n\n");

        /* Capturar simbolo */
        conEscribir("  Ingrese un s\xedmbolo para el Alfabeto de s\xedmbolos de entrada (Sigma_E): ");
        leerLinea(buf, MAX_BUF);

        /* Validar: exactamente 1 caracter, no vacio */
        if (buf[0] == '\0' || buf[1] != '\0') {
            conEscribir("  Error: Entrada inv\xe1lida.\n\n");
            resp = pedirConfirmacion("  \xbf" "Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarSimbolos(afd->alfaEntrada) == 0) {
                conEscribir("  Error: El Alfabeto de s\xedmbolos de entrada (Sigma_E) debe contener al menos un s\xedmbolo.\n\n");
                resp = 'S';
            }
            continue;
        }

        char c = buf[0];

        /* Validar: c en Sigma_perm */
        if (!esPermitido((unsigned char)c)) {
            conEscribir("  Error: El s\xedmbolo '");
            conEscribirChar(c);
            conEscribir("' no est\xe1 permitido.\n\n");
            resp = pedirConfirmacion("  \xbf" "Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarSimbolos(afd->alfaEntrada) == 0) {
                conEscribir("  Error: El Alfabeto de s\xedmbolos de entrada (Sigma_E) debe contener al menos un s\xedmbolo.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: no duplicado */
        if (simboloExiste(afd->alfaEntrada, c)) {
            conEscribir("  Aviso: El s\xedmbolo '");
            conEscribirChar(c);
            conEscribir("' ya est\xe1 registrado en el Alfabeto de s\xedmbolos de entrada (Sigma_E).\n\n");
        } else {
            agregarSimbolo(&afd->alfaEntrada, c);
            conEscribir("  S\xedmbolo '");
            conEscribirChar(c);
            conEscribir("' registrado exitosamente.\n\n");
        }

        /* Preguntar si continuar */
        char promptSigmaE[128];
        conSnprintf(promptSigmaE, sizeof(promptSigmaE),
                 "  \xbf" "Desea agregar otro s\xedmbolo al Alfabeto de s\xedmbolos de entrada (Sigma_E)? (S/N): ");
        resp = pedirConfirmacion(promptSigmaE);

        if (resp == 'N' && contarSimbolos(afd->alfaEntrada) == 0) {
            conEscribir("  Error: El Alfabeto de s\xedmbolos de entrada (Sigma_E) debe contener al menos un s\xedmbolo.\n\n");
            resp = 'S';
        }

    } while (resp == 'S');
}

/*
 * registrarAlfaSalida -- RF-03
 * Proposito   : Registrar Sigma_S simbolo por simbolo (exclusivo AFDT).
 *               Mismas validaciones que RF-02 pero sobre Sigma_S.
 * Parametros  : afd -- puntero al AFD (debe ser TIPO_AFDT)
 * Retorna     : (void)
 */
void registrarAlfaSalida(AFD *afd) {
    char buf[MAX_BUF];
    char resp;

    resp = 'S';
    do {
        limpiarPantalla();
        mostrarEncabezado();
        conEscribir("\n  -- Registro del Alfabeto de s\xedmbolos de salida (Sigma_S) --\n\n");

        /* Mostrar Sigma_S actual */
        conEscribir("  S\xedmbolos registrados en el Alfabeto de s\xedmbolos de salida (Sigma_S): { ");
        const Simbolo *actual = afd->alfaSalida;
        if (actual == NULL) {
            conEscribir("(vacio)");
        } else {
            while (actual != NULL) {
                conEscribirChar(actual->valor);
                if (actual->siguiente != NULL) conEscribir(", ");
                actual = actual->siguiente;
            }
        }
        conEscribir(" }\n\n");

        conEscribir("  Ingrese un s\xedmbolo para el Alfabeto de s\xedmbolos de salida (Sigma_S): ");
        leerLinea(buf, MAX_BUF);

        /* Validar: exactamente 1 caracter */
        if (buf[0] == '\0' || buf[1] != '\0') {
            conEscribir("  Error: Entrada inv\xe1lida.\n\n");
            resp = pedirConfirmacion("  \xbf" "Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarSimbolos(afd->alfaSalida) == 0) {
                conEscribir("  Error: El Alfabeto de s\xedmbolos de salida (Sigma_S) debe contener al menos un s\xedmbolo.\n\n");
                resp = 'S';
            }
            continue;
        }

        char c = buf[0];

        /* Validar: c en Sigma_perm */
        if (!esPermitido((unsigned char)c)) {
            conEscribir("  Error: El s\xedmbolo '");
            conEscribirChar(c);
            conEscribir("' no est\xe1 permitido.\n\n");
            resp = pedirConfirmacion("  \xbf" "Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarSimbolos(afd->alfaSalida) == 0) {
                conEscribir("  Error: El Alfabeto de s\xedmbolos de salida (Sigma_S) debe contener al menos un s\xedmbolo.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: no duplicado */
        if (simboloExiste(afd->alfaSalida, c)) {
            conEscribir("  Aviso: El s\xedmbolo '");
            conEscribirChar(c);
            conEscribir("' ya est\xe1 registrado en el Alfabeto de s\xedmbolos de salida (Sigma_S).\n\n");
        } else {
            agregarSimbolo(&afd->alfaSalida, c);
            conEscribir("  S\xedmbolo '");
            conEscribirChar(c);
            conEscribir("' registrado exitosamente.\n\n");
        }

        char promptSigmaS[128];
        conSnprintf(promptSigmaS, sizeof(promptSigmaS),
                 "  \xbf" "Desea agregar otro s\xedmbolo al Alfabeto de s\xedmbolos de salida (Sigma_S)? (S/N): ");
        resp = pedirConfirmacion(promptSigmaS);

        if (resp == 'N' && contarSimbolos(afd->alfaSalida) == 0) {
            conEscribir("  Error: El Alfabeto de s\xedmbolos de salida (Sigma_S) debe contener al menos un s\xedmbolo.\n\n");
            resp = 'S';
        }

    } while (resp == 'S');
}

/*
 * registrarEstados -- RF-04
 * Proposito   : Registrar Q estado por estado.
 *               Valida: q en Sigma_perm+, 1<=|q|<=15, q no en Q.
 *               Purga duplicados automaticamente (RF-04).
 * Parametros  : afd -- puntero al AFD
 * Retorna     : (void)
 */
void registrarEstados(AFD *afd) {
    char buf[MAX_BUF];
    char resp;

    resp = 'S';
    do {
        limpiarPantalla();
        mostrarEncabezado();
        conEscribir("\n  -- Registro del Conjunto de estados posibles (Q) --\n\n");

        /* Mostrar Q actual */
        conEscribir("  Estados registrados en el Conjunto de estados posibles (Q): { ");
        const Nodo *actual = afd->Q;
        if (actual == NULL) {
            conEscribir("(vacio)");
        } else {
            while (actual != NULL) {
                conEscribir(actual->nombre);
                if (actual->Apuntador != NULL) conEscribir(", ");
                actual = actual->Apuntador;
            }
        }
        conEscribir(" }\n\n");

        conEscribir("  Ingrese el nombre del estado (1-15 caracteres): ");
        leerLinea(buf, MAX_BUF);

        /* Validar: no vacio */
        if (buf[0] == '\0') {
            conEscribir("  Error: Entrada inv\xe1lida.\n\n");
            resp = pedirConfirmacion("  \xbf" "Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarNodos(afd->Q) == 0) {
                conEscribir("  Error: El Conjunto de estados posibles (Q) debe contener al menos un estado.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: longitud <= 15 */
        if ((int)strlen(buf) > 15) {
            conEscribir("  Error: El nombre del estado excede 15 caracteres.\n\n");
            resp = pedirConfirmacion("  \xbf" "Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarNodos(afd->Q) == 0) {
                conEscribir("  Error: El Conjunto de estados posibles (Q) debe contener al menos un estado.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: todos los chars en Sigma_perm */
        if (!cadenaPermitida(buf)) {
            conEscribir("  Error: El nombre '");
            conEscribir(buf);
            conEscribir("' contiene caracteres no permitidos.\n\n");
            resp = pedirConfirmacion("  \xbf" "Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarNodos(afd->Q) == 0) {
                conEscribir("  Error: El Conjunto de estados posibles (Q) debe contener al menos un estado.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: no duplicado */
        if (nodoExiste(afd->Q, buf) != NULL) {
            conEscribir("  Aviso: El estado '");
            conEscribir(buf);
            conEscribir("' ya est\xe1 registrado en el Conjunto de estados posibles (Q) (duplicado purgado).\n\n");
        } else {
            agregarNodo(&afd->Q, buf);
            conEscribir("  Estado '");
            conEscribir(buf);
            conEscribir("' registrado exitosamente.\n\n");
        }

        resp = pedirConfirmacion("  \xbf" "Desea agregar otro estado al Conjunto de estados posibles (Q)? (S/N): ");

        if (resp == 'N' && contarNodos(afd->Q) == 0) {
            conEscribir("  Error: El Conjunto de estados posibles (Q) debe contener al menos un estado.\n\n");
            resp = 'S';
        }

    } while (resp == 'S');
}

/*
 * registrarEstadoInicial -- RF-05
 * Proposito   : Solicitar q0 que debe pertenecer a Q.
 *               Si el valor no existe en Q, rechaza y repite.
 * Parametros  : afd -- puntero al AFD
 * Retorna     : (void)
 */
void registrarEstadoInicial(AFD *afd) {
    char buf[MAX_BUF];
    Nodo *nodo = NULL;
    char mensajeError[MAX_BUF + 64];
    mensajeError[0] = '\0';

    do {
        limpiarPantalla();
        mostrarEncabezado();
        conEscribir("\n  -- Registro del Estado inicial (q0) --\n\n");

        /* Mostrar Q como referencia */
        conEscribir("  Estados disponibles en el Conjunto de estados posibles (Q): { ");
        const Nodo *actual = afd->Q;
        while (actual != NULL) {
            conEscribir(actual->nombre);
            if (actual->Apuntador != NULL) conEscribir(", ");
            actual = actual->Apuntador;
        }
        conEscribir(" }\n\n");

        /* Mostrar error del intento anterior si lo hay */
        if (mensajeError[0] != '\0') {
            conEscribir("  ");
            conEscribir(mensajeError);
            conEscribir("\n\n");
        }

        conEscribir("  Ingrese el Estado inicial (q0): ");
        leerLinea(buf, MAX_BUF);

        /* Validar: no vacio */
        if (buf[0] == '\0') {
            conSnprintf(mensajeError, sizeof(mensajeError),
                     "Error: Entrada inv\xe1lida.");
            continue;
        }

        /* Validar: q0 en Q */
        nodo = nodoExiste(afd->Q, buf);
        if (nodo == NULL) {
            conSnprintf(mensajeError, sizeof(mensajeError),
                     "Error: El estado '%s' no existe en el Conjunto de estados posibles (Q).", buf);
        }

    } while (nodo == NULL);

    afd->q0 = nodo;
    conEscribir("  Estado inicial (q0) = '");
    conEscribir(afd->q0->nombre);
    conEscribir("' registrado exitosamente.\n\n");
}

/*
 * registrarEstadosAceptacion -- RF-06
 * Proposito   : Conformar A iterando la captura individual.
 *               Valida: a en Q, a no en A, a no vacio.
 *               No permite terminar con A vacio (CP-26).
 * Parametros  : afd -- puntero al AFD
 * Retorna     : (void)
 */
void registrarEstadosAceptacion(AFD *afd) {
    char buf[MAX_BUF];
    char resp;
    int  cantidadAceptacion = 0;

    resp = 'S';
    do {
        limpiarPantalla();
        mostrarEncabezado();
        conEscribir("\n  -- Registro de los Estados de aceptaci\xf3n (A) --\n\n");

        /* Mostrar A actual */
        conEscribir("  Estados de aceptaci\xf3n en A: { ");
        const Nodo *actual = afd->Q;
        int primero = 1;
        while (actual != NULL) {
            if (actual->aceptacion) {
                if (!primero) conEscribir(", ");
                conEscribir(actual->nombre);
                primero = 0;
            }
            actual = actual->Apuntador;
        }
        if (primero) conEscribir("(vacio)");
        conEscribir(" }\n\n");

        /* Mostrar Q como referencia */
        conEscribir("  Estados disponibles en el Conjunto de estados posibles (Q): { ");
        actual = afd->Q;
        while (actual != NULL) {
            conEscribir(actual->nombre);
            if (actual->Apuntador != NULL) conEscribir(", ");
            actual = actual->Apuntador;
        }
        conEscribir(" }\n\n");

        conEscribir("  Ingrese un Estado de aceptaci\xf3n (A): ");
        leerLinea(buf, MAX_BUF);

        /* Validar: no vacio */
        if (buf[0] == '\0') {
            conEscribir("  Error: Entrada inv\xe1lida.\n\n");
            resp = pedirConfirmacion("  \xbf" "Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && cantidadAceptacion == 0) {
                conEscribir("  Error: Los Estados de aceptaci\xf3n (A) deben contener al menos un estado.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: a en Q */
        Nodo *nodo = nodoExiste(afd->Q, buf);
        if (nodo == NULL) {
            conEscribir("  Error: El estado '");
            conEscribir(buf);
            conEscribir("' no existe en el Conjunto de estados posibles (Q).\n\n");
            resp = pedirConfirmacion("  \xbf" "Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && cantidadAceptacion == 0) {
                conEscribir("  Error: Los Estados de aceptaci\xf3n (A) deben contener al menos un estado.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: no duplicado en A */
        if (nodo->aceptacion) {
            conEscribir("  Aviso: El estado '");
            conEscribir(buf);
            conEscribir("' ya es Estado de aceptaci\xf3n (A).\n\n");
            char promptAdup[64];
            conSnprintf(promptAdup, sizeof(promptAdup),
                     "  \xbf" "Desea agregar otro Estado de aceptaci\xf3n (A)? (S/N): ");
            resp = pedirConfirmacion(promptAdup);
            if (resp == 'N' && cantidadAceptacion == 0) {
                conEscribir("  Error: Los Estados de aceptaci\xf3n (A) deben contener al menos un estado.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Registrar como estado de aceptacion */
        nodo->aceptacion = 1;
        cantidadAceptacion++;
        conEscribir("  Estado '");
        conEscribir(buf);
        conEscribir("' registrado como Estado de aceptaci\xf3n (A).\n\n");

        char promptA[64];
        conSnprintf(promptA, sizeof(promptA),
                 "  \xbf" "Desea agregar otro Estado de aceptaci\xf3n (A)? (S/N): ");
        resp = pedirConfirmacion(promptA);

        if (resp == 'N' && cantidadAceptacion == 0) {
            conEscribir("  Error: Los Estados de aceptaci\xf3n (A) deben contener al menos un estado.\n\n");
            resp = 'S';
        }

    } while (resp == 'S');
}

/*
 * registrarFuncionTransicion -- RF-07
 * Proposito   : Solicitar f(q, s) = q' para cada par (q, s) en Q x Sigma_E.
 *               Valida: q' en Q, q' no vacio.
 *               Itera automaticamente todos los pares hasta completar f.
 * Parametros  : afd -- puntero al AFD
 * Retorna     : (void)
 */
void registrarFuncionTransicion(AFD *afd) {
    char buf[MAX_BUF];

    limpiarPantalla();
    mostrarEncabezado();
    conEscribir("\n  -- Registro de la Funcion de transicion (f) --\n");
    conEscribir("  Se solicitara f(q, s) = q' para cada par (q, s) en el Conjunto de estados posibles (Q) x Alfabeto de simbolos de entrada (Sigma_E).\n\n");

    /* Iterar sobre Q x Sigma_E */
    Nodo *nodoActual = afd->Q;
    while (nodoActual != NULL) {
        Simbolo *simboloActual = afd->alfaEntrada;
        while (simboloActual != NULL) {
            Nodo *destino = NULL;

            do {
                conEscribir("  f(");
                conEscribir(nodoActual->nombre);
                conEscribir(", ");
                conEscribirChar(simboloActual->valor);
                conEscribir(") = ");
                leerLinea(buf, MAX_BUF);

                /* Validar: no vacio */
                if (buf[0] == '\0') {
                    conEscribir("  Error: Entrada inv\xe1lida. Intente de nuevo.\n\n");
                    continue;
                }

                /* Validar: q' en Q */
                destino = nodoExiste(afd->Q, buf);
                if (destino == NULL) {
                    conEscribir("  Error: El estado '");
                    conEscribir(buf);
                    conEscribir("' no existe en el Conjunto de estados posibles (Q). Intente de nuevo.\n\n");
                }

            } while (destino == NULL);

            agregarArista(nodoActual, simboloActual->valor, '\0', destino);
            conEscribir("  Transicion f(");
            conEscribir(nodoActual->nombre);
            conEscribir(", ");
            conEscribirChar(simboloActual->valor);
            conEscribir(") = ");
            conEscribir(destino->nombre);
            conEscribir(" registrada.\n\n");

            simboloActual = simboloActual->siguiente;
        }
        nodoActual = nodoActual->Apuntador;
    }

    conEscribir("  Funcion de transicion (f) completamente definida.\n\n");
}

/*
 * registrarFuncionSalida -- RF-08
 * Proposito   : Solicitar g(q, s) = y para cada par (q, s) en Q x Sigma_E.
 *               Valida: y en Sigma_S, |y|=1, y no vacio.
 *               Solo para AFDT. Actualiza el campo ALPS de cada Arista existente.
 * Parametros  : afd -- puntero al AFD (debe ser TIPO_AFDT)
 * Retorna     : (void)
 */
void registrarFuncionSalida(AFD *afd) {
    char buf[MAX_BUF];

    limpiarPantalla();
    mostrarEncabezado();
    conEscribir("\n  -- Registro de la Funcion de salida (g) --\n");
    conEscribir("  Se solicitara g(q, s) = y para cada par (q, s) en el Conjunto de estados posibles (Q) x Alfabeto de simbolos de entrada (Sigma_E).\n\n");

    Nodo *nodoActual = afd->Q;
    while (nodoActual != NULL) {
        Simbolo *simboloActual = afd->alfaEntrada;
        while (simboloActual != NULL) {
            char y = '\0';

            do {
                conEscribir("  g(");
                conEscribir(nodoActual->nombre);
                conEscribir(", ");
                conEscribirChar(simboloActual->valor);
                conEscribir(") = ");
                leerLinea(buf, MAX_BUF);

                /* Validar: exactamente 1 caracter */
                if (buf[0] == '\0' || buf[1] != '\0') {
                    conEscribir("  Error: Entrada inv\xe1lida. Intente de nuevo.\n\n");
                    continue;
                }

                /* Validar: y en Sigma_S */
                if (!simboloExiste(afd->alfaSalida, buf[0])) {
                    conEscribir("  Error: El simbolo '");
                    conEscribirChar(buf[0]);
                    conEscribir("' no pertenece al Alfabeto de simbolos de salida (Sigma_S). Intente de nuevo.\n\n");
                    continue;
                }

                y = buf[0];

            } while (y == '\0');

            /* Actualizar ALPS en la Arista correspondiente (creada en RF-07) */
            Arista *arista = buscarArista(nodoActual, simboloActual->valor);
            if (arista != NULL) {
                arista->ALPS = y;
            }

            conEscribir("  Salida g(");
            conEscribir(nodoActual->nombre);
            conEscribir(", ");
            conEscribirChar(simboloActual->valor);
            conEscribir(") = ");
            conEscribirChar(y);
            conEscribir(" registrada.\n\n");

            simboloActual = simboloActual->siguiente;
        }
        nodoActual = nodoActual->Apuntador;
    }

    conEscribir("  Funcion de salida (g) completamente definida.\n\n");
}

/* -------------------------------------- */
/*  SECCION 6: Procesamiento de la palabra*/
/* -------------------------------------- */

/*
 * capturarPalabra -- RF-09
 * Proposito   : Solicitar y validar la palabra de entrada en Sigma_E+.
 *               Valida: palabra no vacia, todos los simbolos en Sigma_E.
 * Parametros  : afd -- puntero al AFD (para consultar Sigma_E)
 * Retorna     : cadena dinamica con la palabra (el llamador debe liberar). RNF-01.
 */
char *capturarPalabra(const AFD *afd) {
    char buf[MAX_BUF];
    const char *tipoPalabra = (afd->tipo == TIPO_AFDT)
                              ? "a traducir" : "a validar";

    conEscribir("\n  -- Captura de la Palabra (a) --\n\n");

    /* Mostrar Sigma_E como referencia */
    conEscribir("  Alfabeto de simbolos de entrada (Sigma_E) = { ");
    const Simbolo *s = afd->alfaEntrada;
    while (s != NULL) {
        conEscribirChar(s->valor);
        if (s->siguiente != NULL) conEscribir(", ");
        s = s->siguiente;
    }
    conEscribir(" }\n\n");

    while (1) {
        conEscribir("  Ingrese la palabra ");
        conEscribir(tipoPalabra);
        conEscribir(": ");
        leerLinea(buf, MAX_BUF);

        /* Validar: no vacia */
        if (buf[0] == '\0') {
            conEscribir("  Error: Entrada inv\xe1lida. Ingrese al menos un s\xedmbolo.\n\n");
            continue;
        }

        /* Validar: todos los chars en Sigma_E */
        int valido = 1;
        for (int i = 0; buf[i] != '\0'; i++) {
            if (!simboloExiste(afd->alfaEntrada, buf[i])) {
                valido = 0;
                break;
            }
        }

        if (!valido) {
            conEscribir("  Error: La Palabra (a) contiene s\xedmbolos fuera del Alfabeto de s\xedmbolos de entrada (Sigma_E). Intente de nuevo.\n\n");
            continue;
        }

        /* RNF-01: copiar en memoria dinamica */
        size_t len = strlen(buf) + 1;
        char *alpha = (char *)malloc(len);
        if (alpha == NULL) {
            conEscribirError("Error: No se pudo asignar memoria.\n");
            return NULL;
        }
        strncpy(alpha, buf, len);
        return alpha;
    }
}

/*
 * procesarPalabra -- RF-10 (AFDV y AFDT) + RF-11 (solo AFDT)
 * Proposito   : Procesar la palabra simbolo por simbolo de izquierda a derecha,
 *               iniciando desde q0.
 *               - Muestra la traza: (qn, si) -> q' (RF-10).
 *               - Para AFDT tambien muestra g(qn,si)=y y construye la traduccion (RF-11).
 *               - Al terminar, determina si qn esta en A.
 *               - Si la transicion es indefinida (NULL), detiene y reporta error.
 * Parametros  : afd      -- puntero al AFD
 *               alpha    -- palabra de entrada
 *               esValida -- (salida) 1 si qn en A, 0 si no
 * Retorna     : cadena de traduccion dinamica para AFDT (llamador libera), NULL para AFDV.
 */
char *procesarPalabra(const AFD *afd, const char *alpha, int *esValida) {
    int    n       = (int)strlen(alpha);
    Nodo  *qActual = afd->q0;
    char  *beta    = NULL;
    int    longitudBeta = 0;

    *esValida = 0;

    /* RNF-01: reservar beta dinamicamente para AFDT */
    if (afd->tipo == TIPO_AFDT) {
        beta = (char *)malloc((size_t)(n + 1));
        if (beta == NULL) {
            conEscribirError("Error: No se pudo asignar memoria.\n");
            return NULL;
        }
        beta[0] = '\0';
    }

    if (afd->tipo == TIPO_AFDV) {
        conEscribir("\n  -- Procesamiento de la palabra \"");
        conEscribir(alpha);
        conEscribir("\" --\n\n");
        conEscribir("  Estado actual (qn)     Simbolo    Estado destino (q')\n");
        conEscribir("  ------------------     -------    -------------------\n");
    } else {
        conEscribir("\n  -- Traduccion de la palabra \"");
        conEscribir(alpha);
        conEscribir("\" --\n\n");
        conEscribir("  Estado actual (qn)     Simbolo    Salida (y)    Trad. parcial    Estado destino (q')\n");
        conEscribir("  ------------------     -------    ----------    -------------    -------------------\n");
    }

    /* Recorrer la palabra simbolo por simbolo */
    for (int i = 0; i < n; i++) {
        char si = alpha[i];

        /* RF-10: buscar transicion f(qActual, si) */
        Arista *arista = buscarArista(qActual, si);

        if (arista == NULL || arista->Flecha == NULL) {
            conEscribir("\n  Error: Transicion indefinida para f(");
            conEscribir(qActual->nombre);
            conEscribir(", ");
            conEscribirChar(si);
            conEscribir("). Procesamiento detenido.\n\n");
            *esValida = 0;
            return beta;
        }

        if (afd->tipo == TIPO_AFDV) {
            /* RF-10: imprimir traza con alineacion fija */
            char fila[128];
            /* columna 1: estado actual (18 chars) */
            int pos = 0;
            const char *nom = qActual->nombre;
            for (int k = 0; nom[k] != '\0' && pos < 18; k++) fila[pos++] = nom[k];
            while (pos < 20) fila[pos++] = ' ';
            /* columna 2: simbolo (10 chars) */
            fila[pos++] = si;
            while (pos < 31) fila[pos++] = ' ';
            /* columna 3: destino */
            const char *dest = arista->Flecha->nombre;
            for (int k = 0; dest[k] != '\0'; k++) fila[pos++] = dest[k];
            fila[pos++] = '\n';
            fila[pos]   = '\0';
            conEscribir("  ");
            conEscribir(fila);
        } else {
            /* RF-11: g(qn, si) = y; beta := beta + y */
            char y = arista->ALPS;
            beta[longitudBeta++] = y;
            beta[longitudBeta]   = '\0';

            char fila[160];
            int pos = 0;
            /* col 1: estado actual */
            const char *nom = qActual->nombre;
            for (int k = 0; nom[k] != '\0' && pos < 18; k++) fila[pos++] = nom[k];
            while (pos < 23) fila[pos++] = ' ';
            /* col 2: simbolo */
            fila[pos++] = si;
            while (pos < 34) fila[pos++] = ' ';
            /* col 3: salida */
            fila[pos++] = y;
            while (pos < 47) fila[pos++] = ' ';
            /* col 4: traduccion parcial */
            for (int k = 0; beta[k] != '\0'; k++) fila[pos++] = beta[k];
            while (pos < 64) fila[pos++] = ' ';
            /* col 5: destino */
            const char *dest = arista->Flecha->nombre;
            for (int k = 0; dest[k] != '\0'; k++) fila[pos++] = dest[k];
            fila[pos++] = '\n';
            fila[pos]   = '\0';
            conEscribir("  ");
            conEscribir(fila);
        }

        /* qn := q' */
        qActual = arista->Flecha;
    }

    /* Determinar si la palabra es valida: qn en A */
    *esValida = qActual->aceptacion;

    conEscribir("\n  Estado final alcanzado: ");
    conEscribir(qActual->nombre);
    conEscribir("\n");

    if (*esValida) {
        conEscribir("  Resultado: Palabra V\xc1LIDA (qn en A).\n");
    } else {
        conEscribir("  Resultado: Palabra INV\xc1LIDA (qn no pertenece a A).\n");
    }

    if (afd->tipo == TIPO_AFDT) {
        conEscribir("  Traduccion obtenida = \"");
        conEscribir(beta);
        conEscribir("\"\n");
    }

    conEscribir("\n");

    return beta;
}

/* ----------------------------- */
/*  SECCION 7: Liberacion del AFD*/
/* ----------------------------- */

/*
 * liberarAFD -- RNF-02
 * Proposito   : Liberar toda la memoria dinamica del AFD al finalizar su ciclo.
 *               Libera: Q (con sus Aristas), Sigma_E y Sigma_S.
 * Parametros  : afd -- puntero al AFD (sus listas quedan en NULL)
 * Retorna     : (void)
 */
void liberarAFD(AFD *afd) {
    liberarNodos(&afd->Q);
    liberarSimbolos(&afd->alfaEntrada);
    liberarSimbolos(&afd->alfaSalida);
    afd->q0 = NULL;
}
