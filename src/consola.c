/*
 * =============================================================================
 * Archivo      : consola.c
 * Proyecto     : Gestor de Automatas Finitos Deterministas
 * Autor        : Victor Garza Maldonado
 * Fecha        : Julio 2026
 * =============================================================================
 * Descripcion:
 *   Implementacion de E/S de consola sin <stdio.h>.
 *   Windows : WriteConsoleA / ReadConsoleA / GetStdHandle  (Win32)
 *   POSIX   : write(2) / read(2)  (<unistd.h>)
 * =============================================================================
 */

#include "../include/consola.h"

/* --------------------------------------------------------------------------- */
/*  Handles de consola (solo Win32)                                            */
/* --------------------------------------------------------------------------- */

#ifdef _WIN32
/*
 * Devuelve el handle de stdout y garantiza CP1252 en la primera llamada,
 * independientemente de si configurarManejoSenales() fue invocado o no.
 */
static HANDLE hStdout(void) {
    static HANDLE h = INVALID_HANDLE_VALUE;
    if (h == INVALID_HANDLE_VALUE) {
        h = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleOutputCP(1252);
    }
    return h;
}

/* Devuelve el handle de stdin */
static HANDLE hStdin(void) {
    static HANDLE h = INVALID_HANDLE_VALUE;
    if (h == INVALID_HANDLE_VALUE) {
        h = GetStdHandle(STD_INPUT_HANDLE);
    }
    return h;
}

/* Devuelve el handle de stderr */
static HANDLE hStderr(void) {
    static HANDLE h = INVALID_HANDLE_VALUE;
    if (h == INVALID_HANDLE_VALUE) {
        h = GetStdHandle(STD_ERROR_HANDLE);
    }
    return h;
}

/*
 * Escribe bytes en un handle de Win32.
 * Si el handle apunta a una consola usa WriteConsoleA (respeta codepage).
 * Si es un pipe o archivo redirigido usa WriteFile (escribe bytes crudos).
 */
static void win32Write(HANDLE h, const char *s, DWORD len) {
    DWORD mode;
    if (GetConsoleMode(h, &mode)) {
        /* Handle es consola real */
        DWORD escritos;
        WriteConsoleA(h, s, len, &escritos, NULL);
    } else {
        /* Handle redirigido: escribir bytes directamente */
        DWORD escritos;
        WriteFile(h, s, len, &escritos, NULL);
    }
}
#endif /* _WIN32 */

/* --------------------------------------------------------------------------- */
/*  conEscribir                                                                 */
/* --------------------------------------------------------------------------- */
void conEscribir(const char *s) {
    if (s == NULL) return;
    DWORD len = (DWORD)strlen(s);
    if (len == 0) return;
#ifdef _WIN32
    win32Write(hStdout(), s, len);
#else
    write(STDOUT_FILENO, s, (size_t)len);
#endif
}

/* --------------------------------------------------------------------------- */
/*  conEscribirChar                                                             */
/* --------------------------------------------------------------------------- */
void conEscribirChar(char c) {
#ifdef _WIN32
    win32Write(hStdout(), &c, 1);
#else
    write(STDOUT_FILENO, &c, 1);
#endif
}

/* --------------------------------------------------------------------------- */
/*  conEscribirInt                                                              */
/* --------------------------------------------------------------------------- */
void conEscribirInt(int n) {
    /* Convertir entero a cadena decimal sin <stdio.h> */
    char buf[16];
    int i = 15;
    int negativo = 0;

    buf[i] = '\0';

    if (n < 0) {
        negativo = 1;
        /* Manejar INT_MIN sin desbordamiento */
        unsigned int u = (unsigned int)(-(n + 1)) + 1u;
        do {
            buf[--i] = (char)('0' + (u % 10));
            u /= 10;
        } while (u > 0);
    } else {
        unsigned int u = (unsigned int)n;
        do {
            buf[--i] = (char)('0' + (u % 10));
            u /= 10;
        } while (u > 0);
    }

    if (negativo) buf[--i] = '-';

    conEscribir(buf + i);
}

/* --------------------------------------------------------------------------- */
/*  conSnprintf                                                                 */
/*                                                                              */
/*  Formatos soportados: %s  %c  %d  %%                                        */
/*  Suficiente para todos los usos del proyecto.                               */
/* --------------------------------------------------------------------------- */
int conSnprintf(char *buf, int tamMax, const char *fmt, ...) {
    if (buf == NULL || tamMax <= 0) return -1;

    va_list args;
    va_start(args, fmt);

    int pos = 0; /* posicion de escritura en buf */

    for (int fi = 0; fmt[fi] != '\0'; fi++) {
        if (fmt[fi] != '%') {
            if (pos < tamMax - 1) buf[pos++] = fmt[fi];
            continue;
        }

        /* Especificador de formato */
        fi++;
        switch (fmt[fi]) {
            case 's': {
                const char *sv = va_arg(args, const char *);
                if (sv == NULL) sv = "(null)";
                for (int si = 0; sv[si] != '\0'; si++) {
                    if (pos < tamMax - 1) buf[pos++] = sv[si];
                }
                break;
            }
            case 'c': {
                char cv = (char)va_arg(args, int);
                if (pos < tamMax - 1) buf[pos++] = cv;
                break;
            }
            case 'd': {
                int dv = va_arg(args, int);
                /* Convertir a decimal */
                char tmp[16];
                int ti = 15;
                tmp[ti] = '\0';
                int neg = 0;
                if (dv < 0) {
                    neg = 1;
                    unsigned int u = (unsigned int)(-(dv + 1)) + 1u;
                    do { tmp[--ti] = (char)('0' + (u % 10)); u /= 10; } while (u > 0);
                } else {
                    unsigned int u = (unsigned int)dv;
                    do { tmp[--ti] = (char)('0' + (u % 10)); u /= 10; } while (u > 0);
                }
                if (neg && pos < tamMax - 1) buf[pos++] = '-';
                for (int di = ti; tmp[di] != '\0'; di++) {
                    if (pos < tamMax - 1) buf[pos++] = tmp[di];
                }
                break;
            }
            case '%': {
                if (pos < tamMax - 1) buf[pos++] = '%';
                break;
            }
            default:
                /* Especificador desconocido: escribir tal cual */
                if (pos < tamMax - 1) buf[pos++] = '%';
                if (pos < tamMax - 1) buf[pos++] = fmt[fi];
                break;
        }
    }

    buf[pos] = '\0';
    va_end(args);
    return pos;
}

/* --------------------------------------------------------------------------- */
/*  conLeerLinea                                                                */
/* --------------------------------------------------------------------------- */
void conLeerLinea(char *buf, int tamMax) {
    if (buf == NULL || tamMax <= 0) return;

#ifdef _WIN32
    /*
     * ReadConsoleA lee caracteres incluyendo '\r\n'.
     * Leer de a un caracter para poder descartar el exceso igual que fgets.
     */
    DWORD leidos;
    int pos = 0;
    char c;

    while (1) {
        if (!ReadConsoleA(hStdin(), &c, 1, &leidos, NULL) || leidos == 0) {
            /* EOF o error */
            break;
        }
        if (c == '\n') break;          /* fin de linea */
        if (c == '\r') continue;       /* ignorar CR de CRLF */
        if (pos < tamMax - 1) {
            buf[pos++] = c;
        }
        /* Si el buffer ya esta lleno seguimos leyendo hasta '\n' para descartar */
    }
    buf[pos] = '\0';
#else
    /* POSIX: read de a un byte */
    int pos = 0;
    char c;
    ssize_t r;

    while (1) {
        r = read(STDIN_FILENO, &c, 1);
        if (r <= 0) break;
        if (c == '\n') break;
        if (c == '\r') continue;
        if (pos < tamMax - 1) {
            buf[pos++] = c;
        }
    }
    buf[pos] = '\0';
#endif
}

/* --------------------------------------------------------------------------- */
/*  conFlush                                                                    */
/* --------------------------------------------------------------------------- */
void conFlush(void) {
    /*
     * WriteConsoleA escribe directamente al buffer de pantalla del SO;
     * no hay buffer intermedio de usuario que vaciar.
     * En POSIX write(2) tampoco tiene buffer de usuario.
     * Esta funcion existe para mantener la interfaz simetrica con el
     * antiguo fflush(stdout).
     */
    (void)0;
}

/* --------------------------------------------------------------------------- */
/*  conEscribirError                                                            */
/* --------------------------------------------------------------------------- */
void conEscribirError(const char *s) {
    if (s == NULL) return;
    DWORD len = (DWORD)strlen(s);
    if (len == 0) return;
#ifdef _WIN32
    win32Write(hStderr(), s, len);
#else
    write(STDERR_FILENO, s, (size_t)len);
#endif
}
