/* SISTEMAS OPERATIVOS I - MINISHELL (PRACTICA 2)
 * ---------------------------------------------
 * Grupo LaTripleta.
 * Autores:
 *  Blanca Atienzar Martinez
 *  Pau Toni Bibiloni Martinez
 *  Unai Pueyo Almodovar 
 */
 
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

// Definimos constantes
#define COMMAND_LINE_SIZE 1024  // Longitud de la línea de comando (IN ALL)
#define ARGS_SIZE           64  // Longitud de los argumentos (IN ALL)

// Caracter de prompt
#define PROMPT '$'

// Colores ANSI
#define RESET "\033[0m"
#define GRIS_T "\x1b[38;5;244m"
#define ROJO_T "\x1b[38;5;196m"
#define AZUL_T "\x1b[34m"
#define BLANCO_T "\x1b[97m"
#define CREMA_T "\x1b[38;2;255;235;155m"
#define NEGRITA "\x1b[1m"

// Dummy debugs
#define DEBUGN1 0 // 1 -> Información del parse_args()
#define DEBUGN2 1 // 1 -> Información del internal_cd && internal_export

// Declaración de funciones
char *read_line(char *line);
void imprimir_prompt();
int execute_line(char *line);
int parse_args(char **args, char *line);

int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs();
int internal_fg(char **args);
int internal_bg(char **args);

void procesar_token(char *dest, const char *src);

// Función principal
int main() {

    // Declaramos la línea donde se guardará el comando
    char line[COMMAND_LINE_SIZE];
    memset(line, 0, COMMAND_LINE_SIZE);

    while (1) {
        if (read_line(line)) {
            execute_line(line);
        }
    }
    return 0;
}

/*
 * Función: imprimir_prompt
 * -------------------------
 * Imprime el prompt en la terminal, mostrando información del usuario y directorio actual.
 */
void imprimir_prompt() {
    char *user = getenv("USER");
    char *pwd = getcwd(NULL, 0);

    printf(NEGRITA AZUL_T"%s" BLANCO_T":" CREMA_T"~%s" BLANCO_T"%c " RESET, user, pwd, PROMPT);
    fflush(stdout);
    
    free(pwd);

    // Esperar antes de vaciar el buffer
    sleep((unsigned int) 0.5);
}

/*
 * Función: read_line
 * -------------------
 * Lee una línea de comando desde la entrada estándar y la devuelve.
 */ 
char *read_line(char *line) {
    imprimir_prompt();

    char *inputread = fgets(line, COMMAND_LINE_SIZE, stdin); // Leer línea de comando
    if (inputread != NULL) { 
        // Eliminar el carácter de la nueva línea si está presente
        char *newline = strchr(line, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }

    } else if (feof(stdin)) { // Si se ha llegado al final de fichero (Ctrl+D)
        printf("\nBye Bye\n");
        exit(0);
    }

    return inputread; 
}

/*
 * Función: execute_line
 * ----------------------
 * Ejecuta una línea de comando, verificando si es un comando interno o externo.
 */
int execute_line(char *line) {
    char* args[COMMAND_LINE_SIZE];
    int nargs = parse_args(args, line);

    // If args available check if internal command
    if (nargs == 0) return 0;

    // Check if is an internal cmd and call the method with one more arg
    int res;
    if((res = check_internal(args)) != 0) return res;

    return 0;
}

/*
 * Función: parse_args
 * -------------------
 * Parsea los argumentos de la línea de comando y los almacena en un array.
 */
int parse_args(char **args, char *line) {
    int i = 0;

    args[i] = strtok(line, " \t\n\r"); // Primer token

    #if DEBUGN1
        fprintf(stderr, GRIS_T "[parse_args()-> token %i: %s]\n" RESET, i, args[i]);
    #endif

    while (args[i] != NULL && args[i][0] != '#') { // Mientras haya tokens y no sea un comentario
        i++;
        args[i] = strtok(NULL, " \t\n\r"); // Resto de tokens
        #if DEBUGN1 
            fprintf(stderr, GRIS_T "[parse_args()-> token %i: %s]\n" RESET, i, args[i]);
        #endif
    }

    if (args[i]) { // Si hay comentario, eliminarlo
        args[i] = NULL;
        #if DEBUGN1
            fprintf(stderr, GRIS_T "[parse_args()-> token %i corregido: %s]\n" RESET, i, args[i]);
        #endif 
    }

    return i; // Número de argumentos
}

/*
 * Función: check_internal
 * -----------------------
 * Verifica si el comando proporcionado es interno y ejecuta la función correspondiente.
 */
int check_internal(char **args) {
    if (!strcmp(args[0], "cd")) return internal_cd(args);
    if (!strcmp(args[0], "export")) return internal_export(args);
    if (!strcmp(args[0], "source")) return internal_source(args);
    if (!strcmp(args[0], "jobs")) return internal_jobs(args);
    if (!strcmp(args[0], "fg")) return internal_fg(args);
    if (!strcmp(args[0], "bg")) return internal_bg(args);
    if (!strcmp(args[0], "exit")) {
        printf("\rBye Bye\n");
        exit(0);
    }
    return 0;
}

/*
 * Función: internal_cd
 * ---------------------
 * Implementa el comando interno "cd", que cambia el directorio de trabajo.
 */
int internal_cd(char **args) {
    #if DEBUGN1 
        fprintf(stderr, GRIS_T "[internal_cd()-> Esta función cambiará el directorio]\n");
    #endif

    char cwd[PATH_MAX];
    strcpy(cwd, "");

    if (args[1] == NULL) {
        chdir(getenv("HOME"));
    } else {
        int i = 1;
        while (args[i] != NULL) {
            char aux[PATH_MAX];
            procesar_token(aux, args[i]);

            if (args[i + 1] != NULL) {
                strcat(aux, " ");
            }
            strcat(cwd, aux);
            
            i++;
        }

        if (chdir(cwd) == -1) {
            fprintf(stderr, ROJO_T"chdir: %s\n", strerror(errno));
            return -1;
        }
        
    }

    getcwd(cwd, PATH_MAX);

    #if DEBUGN2
        fprintf(stderr, GRIS_T "[internal_cd()-> PWD: %s]\n" RESET, cwd);
    #endif

    return 1;
}

/*
 * Función: internal_export
 * -------------------------
 * Implementa el comando interno "export", que asigna valores a variables de entorno.
 */
int internal_export(char **args) {
    #if DEBUGN1 
        printf("[internal_export()→ Esta función asignará valores a variables de entorno]\n");
    #endif

    char *nombre;
    char *valor;

    nombre = strtok(args[1], "=");
    if(nombre == NULL) {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
    } else {
        valor = strtok(NULL, "");
        if (valor == NULL) {
            #if DEBUGN2
                fprintf(stderr, GRIS_T "[internal_export()→ nombre: %s]\n" RESET, nombre);
                fprintf(stderr, GRIS_T "[internal_export()→ valor: %s]\n" RESET, valor);
            #endif
            fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        } else {
            #if DEBUGN2
                fprintf(stderr, GRIS_T "[internal_export()→ nombre: %s]\n" RESET, nombre);
                fprintf(stderr, GRIS_T "[internal_export()→ valor: %s]\n" RESET, valor);
                fprintf(stderr, GRIS_T "[internal_export()→ antiguo valor para %s: %s]\n" RESET, nombre, getenv(nombre));
            #endif

            setenv(nombre, valor, 1);

            #if DEBUGN2
            fprintf(stderr, GRIS_T "[internal_export()→ nuevo valor para %s: %s]\n" RESET, nombre, getenv(nombre));
            #endif
        }
    }
    
    return 1;
}

/*
 * Función: internal_source
 * ------------------------
 * Implementa el comando interno "source", que ejecuta un fichero de líneas de comandos.
 */
int internal_source(char **args) {
    #if DEBUGN1 
        printf("[internal_source()-> Esta función ejecutará un fichero de líneas de comandos]\n");
    #endif
    return 1;
}

/*
 * Función: internal_jobs
 * ----------------------
 * Implementa el comando interno "jobs", que muestra el PID de los procesos que no están en foreground.
 */
int internal_jobs() {
    #if DEBUGN1 
        printf("[internal_jobs()-> Esta función mostrará el PID de los procesos que no estén en foreground]\n");
    #endif
    return 1;
}

/*
 * Función: internal_fg
 * ---------------------
 * Implementa el comando interno "fg", que recupera un trabajo que esté ejecutándose 
 * en background o que esté detenido y le devuelve una ejecución en foreground.
 */
int internal_fg(char **args) {
    #if DEBUGN1 
        printf("[internal_fg()→ Esta función enviará un trabajo detenido al foreground reactivando su ejecución, o uno del background al foreground]\n");
    #endif
    return 1;
}

/*
 * Función: internal_bg
 * ---------------------
 * Implementa el comando interno "bg", que recupera un trabajo que esté detenido 
 * y le devuelve una ejecución en background.
 */
int internal_bg(char **args) {
    #if DEBUGN1 
        printf("[internal_bg()→ Esta función reactivará un proceso detenido para que siga ejecutándose pero en "
               "segundo plano]\n");
    #endif
    return 1;
}

/*
 * Función: procesar_token
 * -----------------------
 * Procesa un token teniendo en cuenta caracteres de escape y comillas.
 */
void procesar_token(char *dest, const char *src) {
    int i = 0; 
    int j = 0; 
    int entre_comillas = 0; 

    while (src[i]) {  
        if (src[i] == '\\' && !entre_comillas) { 
            // Si encontramos un carácter de escape y no estamos dentro de comillas, avanzamos y copiamos el siguiente carácter literal
            i++;
            dest[j++] = src[i];
        } else if (src[i] == '"' || src[i] == '\'') { 
            // Si encontramos comillas, alternamos el estado entre_comillas
            entre_comillas = !entre_comillas; 
        } else {
            dest[j++] = src[i]; 
        }

        i++;
    }

    dest[j] = '\0'; 
}