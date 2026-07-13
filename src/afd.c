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
 *     RF-02  -- Registrar Σ_E
 *     RF-03  -- Registrar Σ_S (solo AFDT)
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
/*  SECCION 1: Validacion de caracteres                                       */
/* -------------------------------------- */

/*
 * esPermitido -- RF-02, RF-03, RF-04
 * Proposito   : Verificar si c pertenece a Sigma_perm.
 *               Sigma_perm = {a..z, A..Z, 0..9, n, N, vocales con tilde}
 * Parametros  : c -- caracter a validar (unsigned char para evitar UB con isalpha)
 * Retorna     : 1 si c pertenece a Sigma_perm, 0 si no.
 */
int esPermitido(unsigned char c) {
    /* Caracteres ASCII basicos permitidos */
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9')) {
        return 1;
    }
    /*
     * Caracteres especiales del espanol (codificacion Latin-1 / CP1252,
     * que es la pagina de codigo predeterminada en consolas Windows).
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
 * Retorna     : 1 si todos los chars pertenecen a Sigma_perm, 0 si alguno no.
 */
int cadenaPermitida(const char *s) {
    if (s == NULL || s[0] == '\0') return 0;
    for (int i = 0; s[i] != '\0'; i++) {
        if (!esPermitido((unsigned char)s[i])) return 0;
    }
    return 1;
}

/* ------------------------------------------------------ */
/*  SECCION 2: Gestion de Simbolos (Sigma_E y Sigma_S)*/
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
    /* RNF-01: memoria dinámica */
    Simbolo *nuevo = (Simbolo *)malloc(sizeof(Simbolo));
    if (nuevo == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria.\n");
        return;
    }
    nuevo->valor     = c;
    nuevo->siguiente = NULL;

    if (*cabeza == NULL) {
        *cabeza = nuevo;
        return;
    }
    /* Recorrer hasta el final y enlazar */
    Simbolo *actual = *cabeza;
    while (actual->siguiente != NULL) {
        actual = actual->siguiente;
    }
    actual->siguiente = nuevo;
}

/*
 * contarSimbolos -- RF-02, RF-03 (validacion de alfabeto no vacio)
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
 * Parametros  : cabeza -- cabeza de la lista Q; nombre — nombre del estado
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
    /* RNF-01: memoria dinámica */
    Nodo *nuevo = (Nodo *)malloc(sizeof(Nodo));
    if (nuevo == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria.\n");
        return;
    }
    strncpy(nuevo->nombre, nombre, MAX_ESTADO - 1);
    nuevo->nombre[MAX_ESTADO - 1] = '\0';
    nuevo->aceptacion = 0;   /* Por defecto no es estado de aceptacion */
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
 * contarNodos -- RF-04 (validacion Q no vacio)
 * Proposito   : Contar nodos en la lista Q.
 * Parámetros  : cabeza — cabeza de Q
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
 * Parámetros  : cabeza — puntero a la cabeza de Q (queda en NULL)
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
 * Parametros  : nodo    -- estado origen; simbolo -- simbolo leido de la palabra
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
    /* RNF-01: memoria dinámica */
    Arista *nueva = (Arista *)malloc(sizeof(Arista));
    if (nueva == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria.\n");
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
 * RF-02
 */
void registrarAlfaEntrada(AFD *afd) {
    char buf[MAX_BUF];
    char resp;

    resp = 'S';
    do {
        limpiarPantalla();
        mostrarEncabezado();
        printf("\n  -- Registro del Alfabeto de simbolos de entrada (Sigma_E) --\n\n");

        /* Mostrar Sigma_E actual */
        printf("  Simbolos registrados en el Alfabeto de simbolos de entrada (Sigma_E): { ");
        const Simbolo *actual = afd->alfaEntrada;
        if (actual == NULL) {
            printf("(vacio)");
        } else {
            while (actual != NULL) {
                printf("%c", actual->valor);
                if (actual->siguiente != NULL) printf(", ");
                actual = actual->siguiente;
            }
        }
        printf(" }\n\n");

        /* Capturar simbolo */
        printf("  Ingrese un simbolo para el Alfabeto de simbolos de entrada (Sigma_E): ");
        leerLinea(buf, MAX_BUF);

        /* Validar: exactamente 1 caracter, no vacio */
        if (buf[0] == '\0' || buf[1] != '\0') {
            printf("  Error: Entrada invalida.\n\n");
            resp = pedirConfirmacion("  Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarSimbolos(afd->alfaEntrada) == 0) {
                printf("  Error: El Alfabeto de simbolos de entrada (Sigma_E) debe contener al menos un simbolo.\n\n");
                resp = 'S';
            }
            continue;
        }

        char c = buf[0];

        /* Validar: c en Sigma_perm */
        if (!esPermitido((unsigned char)c)) {
            printf("  Error: El simbolo '%c' no esta permitido.\n\n", c);
            resp = pedirConfirmacion("  Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarSimbolos(afd->alfaEntrada) == 0) {
                printf("  Error: El Alfabeto de simbolos de entrada (Sigma_E) debe contener al menos un simbolo.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: no duplicado (RF-02: purga automaticamente) */
        if (simboloExiste(afd->alfaEntrada, c)) {
            printf("  Aviso: El simbolo '%c' ya esta registrado en el Alfabeto de simbolos de entrada (Sigma_E).\n\n", c);
            /* CP-08: no agrega el duplicado, Sigma_E permanece sin cambios */
        } else {
            /* RNF-01: agrega con memoria dinamica */
            agregarSimbolo(&afd->alfaEntrada, c);
            printf("  Simbolo '%c' registrado exitosamente.\n\n", c);
        }

        /* Preguntar si continuar */
        resp = pedirConfirmacion("  Desea agregar otro simbolo al Alfabeto de simbolos de entrada (Sigma_E)? (S/N): ");

        if (resp == 'N' && contarSimbolos(afd->alfaEntrada) == 0) {
            /* CP-09: no permite terminar con Sigma_E vacio */
            printf("  Error: El Alfabeto de simbolos de entrada (Sigma_E) debe contener al menos un simbolo.\n\n");
            resp = 'S'; /* Forzar continuar */
        }

    } while (resp == 'S');
}

/*
 * registrarAlfaSalida -- RF-03
 * Proposito   : Registrar Sigma_S simbolo por simbolo (exclusivo AFDT).
 *               Mismas validaciones que RF-02 pero sobre Sigma_S.
 * Parametros  : afd -- puntero al AFD (debe ser TIPO_AFDT)
 * Retorna     : (void)
 * RF-03
 */
void registrarAlfaSalida(AFD *afd) {
    char buf[MAX_BUF];
    char resp;

    resp = 'S';
    do {
        limpiarPantalla();
        mostrarEncabezado();
        printf("\n  -- Registro del Alfabeto de simbolos de salida (Sigma_S) --\n\n");

        /* Mostrar Sigma_S actual */
        printf("  Simbolos registrados en el Alfabeto de simbolos de salida (Sigma_S): { ");
        const Simbolo *actual = afd->alfaSalida;
        if (actual == NULL) {
            printf("(vacio)");
        } else {
            while (actual != NULL) {
                printf("%c", actual->valor);
                if (actual->siguiente != NULL) printf(", ");
                actual = actual->siguiente;
            }
        }
        printf(" }\n\n");

        printf("  Ingrese un simbolo para el Alfabeto de simbolos de salida (Sigma_S): ");
        leerLinea(buf, MAX_BUF);

        /* Validar: exactamente 1 caracter */
        if (buf[0] == '\0' || buf[1] != '\0') {
            printf("  Error: Entrada invalida.\n\n");
            resp = pedirConfirmacion("  Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarSimbolos(afd->alfaSalida) == 0) {
                printf("  Error: El Alfabeto de simbolos de salida (Sigma_S) debe contener al menos un simbolo.\n\n");
                resp = 'S';
            }
            continue;
        }

        char c = buf[0];

        /* Validar: c en Sigma_perm */
        if (!esPermitido((unsigned char)c)) {
            printf("  Error: El simbolo '%c' no esta permitido.\n\n", c);
            resp = pedirConfirmacion("  Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarSimbolos(afd->alfaSalida) == 0) {
                printf("  Error: El Alfabeto de simbolos de salida (Sigma_S) debe contener al menos un simbolo.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: no duplicado */
        if (simboloExiste(afd->alfaSalida, c)) {
            printf("  Aviso: El simbolo '%c' ya esta registrado en el Alfabeto de simbolos de salida (Sigma_S).\n\n", c);
        } else {
            agregarSimbolo(&afd->alfaSalida, c);
            printf("  Simbolo '%c' registrado exitosamente.\n\n", c);
        }

        resp = pedirConfirmacion("  Desea agregar otro simbolo al Alfabeto de simbolos de salida (Sigma_S)? (S/N): ");

        if (resp == 'N' && contarSimbolos(afd->alfaSalida) == 0) {
            /* CP-15: no permite terminar con Sigma_S vacio */
            printf("  Error: El Alfabeto de simbolos de salida (Sigma_S) debe contener al menos un simbolo.\n\n");
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
 * RF-04
 */
void registrarEstados(AFD *afd) {
    char buf[MAX_BUF];
    char resp;

    resp = 'S';
    do {
        limpiarPantalla();
        mostrarEncabezado();
        printf("\n  -- Registro del Conjunto de estados posibles (Q) --\n\n");

        /* Mostrar Q actual */
        printf("  Estados registrados en el Conjunto de estados posibles (Q): { ");
        const Nodo *actual = afd->Q;
        if (actual == NULL) {
            printf("(vacio)");
        } else {
            while (actual != NULL) {
                printf("%s", actual->nombre);
                if (actual->Apuntador != NULL) printf(", ");
                actual = actual->Apuntador;
            }
        }
        printf(" }\n\n");

        printf("  Ingrese el nombre del estado (1-15 caracteres): ");
        leerLinea(buf, MAX_BUF);

        /* Validar: no vacio */
        if (buf[0] == '\0') {
            printf("  Error: Entrada invalida.\n\n");
            resp = pedirConfirmacion("  Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarNodos(afd->Q) == 0) {
                printf("  Error: El Conjunto de estados posibles (Q) debe contener al menos un estado.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: longitud <= 15 (RF-04: |q| <= 15) */
        if ((int)strlen(buf) > 15) {
            printf("  Error: El nombre del estado excede 15 caracteres.\n\n");
            resp = pedirConfirmacion("  Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarNodos(afd->Q) == 0) {
                printf("  Error: El Conjunto de estados posibles (Q) debe contener al menos un estado.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: todos los chars en Sigma_perm */
        if (!cadenaPermitida(buf)) {
            printf("  Error: El nombre '%s' contiene caracteres no permitidos.\n\n", buf);
            resp = pedirConfirmacion("  Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && contarNodos(afd->Q) == 0) {
                printf("  Error: El Conjunto de estados posibles (Q) debe contener al menos un estado.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: no duplicado -- purga automaticamente (RF-04) */
        if (nodoExiste(afd->Q, buf) != NULL) {
            printf("  Aviso: El estado '%s' ya esta registrado en el Conjunto de estados posibles (Q) (duplicado purgado).\n\n", buf);
        } else {
            /* RNF-01: agrega con memoria dinamica */
            agregarNodo(&afd->Q, buf);
            printf("  Estado '%s' registrado exitosamente.\n\n", buf);
        }

        resp = pedirConfirmacion("  Desea agregar otro estado al Conjunto de estados posibles (Q)? (S/N): ");

        if (resp == 'N' && contarNodos(afd->Q) == 0) {
            /* CP-21: no permite terminar con Q vacio */
            printf("  Error: El Conjunto de estados posibles (Q) debe contener al menos un estado.\n\n");
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
 * RF-05
 */
void registrarEstadoInicial(AFD *afd) {
    char buf[MAX_BUF];
    Nodo *nodo = NULL;
    char mensajeError[MAX_BUF + 64];
    mensajeError[0] = '\0';

    do {
        limpiarPantalla();
        mostrarEncabezado();
        printf("\n  -- Registro del Estado inicial (q0) --\n\n");

        /* Mostrar Q como referencia */
        printf("  Estados disponibles en el Conjunto de estados posibles (Q): { ");
        const Nodo *actual = afd->Q;
        while (actual != NULL) {
            printf("%s", actual->nombre);
            if (actual->Apuntador != NULL) printf(", ");
            actual = actual->Apuntador;
        }
        printf(" }\n\n");

        /* Mostrar error del intento anterior si lo hay */
        if (mensajeError[0] != '\0') {
            printf("  %s\n\n", mensajeError);
        }

        printf("  Ingrese el Estado inicial (q0): ");
        leerLinea(buf, MAX_BUF);

        /* Validar: no vacio */
        if (buf[0] == '\0') {
            snprintf(mensajeError, sizeof(mensajeError),
                     "Error: Entrada invalida.");
            continue;
        }

        /* Validar: q0 en Q */
        nodo = nodoExiste(afd->Q, buf);
        if (nodo == NULL) {
            snprintf(mensajeError, sizeof(mensajeError),
                     "Error: El estado '%s' no existe en el Conjunto de estados posibles (Q).", buf);
        }

    } while (nodo == NULL);

    afd->q0 = nodo;
    printf("  Estado inicial (q0) = '%s' registrado exitosamente.\n\n", afd->q0->nombre);
}

/*
 * registrarEstadosAceptacion -- RF-06
 * Proposito   : Conformar A iterando la captura individual.
 *               Valida: a en Q, a no en A, a no vacio.
 *               No permite terminar con A vacio (CP-26).
 * Parametros  : afd -- puntero al AFD
 * Retorna     : (void)
 * RF-06
 */
void registrarEstadosAceptacion(AFD *afd) {
    char buf[MAX_BUF];
    char resp;
    int  cantidadAceptacion = 0;

    resp = 'S';
    do {
        limpiarPantalla();
        mostrarEncabezado();
        printf("\n  -- Registro de los Estados de aceptacion (A) --\n\n");

        /* Mostrar A actual */
        printf("  Estados de aceptacion en A (Estados de aceptacion): { ");
        const Nodo *actual = afd->Q;
        int primero = 1;
        while (actual != NULL) {
            if (actual->aceptacion) {
                if (!primero) printf(", ");
                printf("%s", actual->nombre);
                primero = 0;
            }
            actual = actual->Apuntador;
        }
        if (primero) printf("(vacio)");
        printf(" }\n\n");

        /* Mostrar Q como referencia */
        printf("  Estados disponibles en el Conjunto de estados posibles (Q): { ");
        actual = afd->Q;
        while (actual != NULL) {
            printf("%s", actual->nombre);
            if (actual->Apuntador != NULL) printf(", ");
            actual = actual->Apuntador;
        }
        printf(" }\n\n");

        printf("  Ingrese un Estado de aceptacion (A): ");
        leerLinea(buf, MAX_BUF);

        /* Validar: no vacio */
        if (buf[0] == '\0') {
            printf("  Error: Entrada invalida.\n\n");
            resp = pedirConfirmacion("  Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && cantidadAceptacion == 0) {
                printf("  Error: Los Estados de aceptacion (A) deben contener al menos un estado.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: a en Q */
        Nodo *nodo = nodoExiste(afd->Q, buf);
        if (nodo == NULL) {
            printf("  Error: El estado '%s' no existe en el Conjunto de estados posibles (Q).\n\n", buf);
            resp = pedirConfirmacion("  Desea intentar de nuevo? (S/N): ");
            if (resp == 'N' && cantidadAceptacion == 0) {
                printf("  Error: Los Estados de aceptacion (A) deben contener al menos un estado.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Validar: no duplicado en A -- avisa pero permite salir */
        if (nodo->aceptacion) {
            printf("  Aviso: El estado '%s' ya es Estado de aceptacion (A).\n\n", buf);
            resp = pedirConfirmacion("  Desea agregar otro Estado de aceptacion (A)? (S/N): ");
            if (resp == 'N' && cantidadAceptacion == 0) {
                printf("  Error: Los Estados de aceptacion (A) deben contener al menos un estado.\n\n");
                resp = 'S';
            }
            continue;
        }

        /* Registrar como estado de aceptacion */
        nodo->aceptacion = 1;
        cantidadAceptacion++;
        printf("  Estado '%s' registrado como Estado de aceptacion (A).\n\n", buf);

        resp = pedirConfirmacion("  Desea agregar otro Estado de aceptacion (A)? (S/N): ");

        if (resp == 'N' && cantidadAceptacion == 0) {
            /* CP-26: no permite terminar con A vacio */
            printf("  Error: Los Estados de aceptacion (A) deben contener al menos un estado.\n\n");
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
 * RF-07
 */
void registrarFuncionTransicion(AFD *afd) {
    char buf[MAX_BUF];

    limpiarPantalla();
    mostrarEncabezado();
    printf("\n  -- Registro de la Funcion de transicion (f) --\n");
    printf("  Se solicitara f(q, s) = q' para cada par (q, s) en el Conjunto de estados posibles (Q) x Alfabeto de simbolos de entrada (Sigma_E).\n\n");

    /* Iterar sobre Q x Sigma_E */
    Nodo *nodoActual = afd->Q;
    while (nodoActual != NULL) {
        Simbolo *simboloActual = afd->alfaEntrada;
        while (simboloActual != NULL) {
            Nodo *destino = NULL;

            do {
                printf("  f(%s, %c) = ", nodoActual->nombre, simboloActual->valor);
                leerLinea(buf, MAX_BUF);

                /* Validar: no vacio */
                if (buf[0] == '\0') {
                    printf("  Error: Entrada invalida. Intente de nuevo.\n\n");
                    continue;
                }

                /* Validar: q' en Q */
                destino = nodoExiste(afd->Q, buf);
                if (destino == NULL) {
                    printf("  Error: El estado '%s' no existe en el Conjunto de estados posibles (Q). Intente de nuevo.\n\n", buf);
                }

            } while (destino == NULL);

            /* RF-07: registrar f(q,s) = q' mediante Arista */
            agregarArista(nodoActual, simboloActual->valor, '\0', destino);
            printf("  Transicion f(%s, %c) = %s registrada.\n\n",
                   nodoActual->nombre, simboloActual->valor, destino->nombre);

            simboloActual = simboloActual->siguiente;
        }
        nodoActual = nodoActual->Apuntador;
    }

    printf("  Funcion de transicion (f) completamente definida.\n\n");
}

/*
 * registrarFuncionSalida -- RF-08
 * Proposito   : Solicitar g(q, s) = y para cada par (q, s) en Q x Sigma_E.
 *               Valida: y en Sigma_S, |y|=1, y no vacio.
 *               Solo para AFDT. Actualiza el campo ALPS de cada Arista existente.
 * Parametros  : afd -- puntero al AFD (debe ser TIPO_AFDT)
 * Retorna     : (void)
 * RF-08
 */
void registrarFuncionSalida(AFD *afd) {
    char buf[MAX_BUF];

    limpiarPantalla();
    mostrarEncabezado();
    printf("\n  -- Registro de la Funcion de salida (g) --\n");
    printf("  Se solicitara g(q, s) = y para cada par (q, s) en el Conjunto de estados posibles (Q) x Alfabeto de simbolos de entrada (Sigma_E).\n\n");

    Nodo *nodoActual = afd->Q;
    while (nodoActual != NULL) {
        Simbolo *simboloActual = afd->alfaEntrada;
        while (simboloActual != NULL) {
            char y = '\0';

            do {
                printf("  g(%s, %c) = ", nodoActual->nombre, simboloActual->valor);
                leerLinea(buf, MAX_BUF);

                /* Validar: exactamente 1 caracter */
                if (buf[0] == '\0' || buf[1] != '\0') {
                    printf("  Error: Entrada invalida. Intente de nuevo.\n\n");
                    continue;
                }

                /* Validar: y en Sigma_S */
                if (!simboloExiste(afd->alfaSalida, buf[0])) {
                    printf("  Error: El simbolo '%c' no pertenece al Alfabeto de simbolos de salida (Sigma_S). Intente de nuevo.\n\n",
                           buf[0]);
                    continue;
                }

                y = buf[0];

            } while (y == '\0');

            /* Actualizar el campo ALPS en la Arista correspondiente (ya creada en RF-07) */
            Arista *arista = buscarArista(nodoActual, simboloActual->valor);
            if (arista != NULL) {
                arista->ALPS = y;
            }

            printf("  Salida g(%s, %c) = %c registrada.\n\n",
                   nodoActual->nombre, simboloActual->valor, y);

            simboloActual = simboloActual->siguiente;
        }
        nodoActual = nodoActual->Apuntador;
    }

    printf("  Funcion de salida (g) completamente definida.\n\n");
}

/* -------------------------------------- */
/*  SECCION 6: Procesamiento de la palabra*/
/* -------------------------------------- */

/*
 * capturarPalabra -- RF-09
 * Proposito   : Solicitar y validar la palabra de entrada en Sigma_E+.
 *               Valida: palabra no vacia, todos los simbolos en Sigma_E.
 * Parametros  : afd -- puntero al AFD (para consultar Σ_E)
 * Retorna     : cadena dinamica con la palabra (el llamador debe liberar). RNF-01.
 * RF-09
 */
char *capturarPalabra(const AFD *afd) {
    char buf[MAX_BUF];
    const char *tipoPalabra = (afd->tipo == TIPO_AFDT)
                              ? "a traducir" : "a validar";

    printf("\n  -- Captura de la Palabra (a) --\n\n");

    /* Mostrar Sigma_E como referencia */
    printf("  Alfabeto de simbolos de entrada (Sigma_E) = { ");
    const Simbolo *s = afd->alfaEntrada;
    while (s != NULL) {
        printf("%c", s->valor);
        if (s->siguiente != NULL) printf(", ");
        s = s->siguiente;
    }
    printf(" }\n\n");

    while (1) {
        printf("  Ingrese la palabra %s: ", tipoPalabra);
        leerLinea(buf, MAX_BUF);

        /* Validar: no vacia */
        if (buf[0] == '\0') {
            printf("  Error: Entrada invalida. Ingrese al menos un simbolo.\n\n");
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
            printf("  Error: La Palabra (a) contiene simbolos fuera del Alfabeto de simbolos de entrada (Sigma_E). Intente de nuevo.\n\n");
            continue;
        }

        /* RNF-01: copiar en memoria dinamica */
        size_t len = strlen(buf) + 1;
        char *alpha = (char *)malloc(len);
        if (alpha == NULL) {
            fprintf(stderr, "Error: No se pudo asignar memoria.\n");
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
 *               - Muestra en pantalla la traza: (qn, si) -> q' (RF-10).
 *               - Para AFDT tambien muestra g(qn,si)=y y construye la traduccion (RF-11).
 *               - Al terminar la palabra, determina si qn esta en A.
 *               - Si la transicion es indefinida (NULL), detiene y reporta error.
 * Parametros  : afd      -- puntero al AFD
 *               alpha    -- palabra de entrada
 *               esValida -- (salida) 1 si qn en A, 0 si no
 * Retorna     : cadena de traduccion dinamica para AFDT (llamador libera), NULL para AFDV.
 * RF-10, RF-11
 */
char *procesarPalabra(const AFD *afd, const char *alpha, int *esValida) {
    int    n       = (int)strlen(alpha);
    Nodo  *qActual = afd->q0;    /* qn := q0 */
    char  *beta    = NULL;        /* beta := vacio (solo AFDT) */
    int    longitudBeta = 0;

    *esValida = 0;

    /* RNF-01: reservar beta dinamicamente para AFDT */
    if (afd->tipo == TIPO_AFDT) {
        /* En el peor caso beta tiene n chars + '\0' */
        beta = (char *)malloc((size_t)(n + 1));
        if (beta == NULL) {
            fprintf(stderr, "Error: No se pudo asignar memoria.\n");
            return NULL;
        }
        beta[0] = '\0';
    }

    if (afd->tipo == TIPO_AFDV) {
        printf("\n  -- Procesamiento de la palabra \"%s\" --\n\n", alpha);
        printf("  %-18s %-10s %-18s\n", "Estado actual (qn)", "Simbolo", "Estado destino (q')");
        printf("  %-18s %-10s %-18s\n", "------------------", "-------", "-------------------");
    } else {
        printf("\n  -- Traduccion de la palabra \"%s\" --\n\n", alpha);
        printf("  %-18s %-10s %-10s %-12s %-18s\n",
               "Estado actual (qn)", "Simbolo", "Salida (y)", "Traduccion parcial", "Estado destino (q')");
        printf("  %-18s %-10s %-10s %-12s %-18s\n",
               "------------------", "-------", "----------", "------------------", "-------------------");
    }

    /* Recorrer la palabra simbolo por simbolo */
    for (int i = 0; i < n; i++) {
        char si = alpha[i];

        /* RF-10: buscar transicion f(qActual, si) */
        Arista *arista = buscarArista(qActual, si);

        if (arista == NULL || arista->Flecha == NULL) {
            /* Transicion indefinida -> detener con error */
            printf("\n  Error: Transicion indefinida para f(%s, %c). Procesamiento detenido.\n\n",
                   qActual->nombre, si);
            *esValida = 0;
            return beta; /* Retorna lo que se haya construido de β (puede ser "") */
        }

        if (afd->tipo == TIPO_AFDV) {
            /* RF-10: imprimir traza */
            printf("  %-18s %-10c %-18s\n",
                   qActual->nombre, si, arista->Flecha->nombre);
        } else {
            /* RF-11: g(qn, si) = y; beta := beta + y */
            char y = arista->ALPS;
            beta[longitudBeta++] = y;
            beta[longitudBeta]   = '\0';

            printf("  %-18s %-10c %-10c %-12s %-18s\n",
                   qActual->nombre, si, y, beta, arista->Flecha->nombre);
        }

        /* qn := q' */
        qActual = arista->Flecha;
    }

    /* Determinar si la palabra es valida: qn en A */
    *esValida = qActual->aceptacion;

    printf("\n  Estado final alcanzado: %s\n", qActual->nombre);

    if (*esValida) {
        printf("  Resultado: Palabra VALIDA (qn en A).\n");
    } else {
        printf("  Resultado: Palabra INVALIDA (qn no pertenece a A).\n");
    }

    if (afd->tipo == TIPO_AFDT) {
        printf("  Traduccion obtenida = \"%s\"\n", beta);
    }

    printf("\n");

    return beta; /* NULL para AFDV; cadena de traduccion para AFDT */
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
 * RNF-02
 */
void liberarAFD(AFD *afd) {
    liberarNodos(&afd->Q);
    liberarSimbolos(&afd->alfaEntrada);
    liberarSimbolos(&afd->alfaSalida);
    afd->q0 = NULL;
}
