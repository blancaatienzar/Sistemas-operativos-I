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
#define N_JOBS              64  // Número máximo de jobs(N3 N4 N5 N6)

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
#define DEBUGN2 0 // 1 -> Información del internal_cd && internal_export
#define DEBUGN3 0 // 1 -> Información del execute_line && internal_source
#define DEBUGN4 0 // 1 -> Información del reaper() && ctrlc()
#define DEBUGN5 0 // 1 -> Información del ctrlz()
#define DEBUGN6 1 // 1 -> Información del internal_fg && internal_bg

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

void reaper(int signum);
void ctrlc(int signum);
void ctrlz(int signum);

int is_background(char **args);
int jobs_list_add(pid_t pid, char estado, char *cmd);
int jobs_list_find(pid_t pid);
int jobs_list_remove(int pos);

int is_output_redirection(char **args);

struct info_job {
    pid_t pid;
    char estado; // ‘N’, ’E’, ‘D’, ‘F’ (‘N’: Ninguno, ‘E’: Ejecutándose y ‘D’: Detenido, ‘F’: Finalizado)
    char cmd[COMMAND_LINE_SIZE]; // Línea de comando asociada
};

static char mi_shell[COMMAND_LINE_SIZE];    // Nombre de la shell
static struct info_job jobs_list[N_JOBS];   // Lista de trabajos
static int n_job = 0;                       // Trabajos actuales

// Función principal
int main(int argc, char *argv[]) {
    // Init jobs_list[0]
    jobs_list->pid = 0;
    jobs_list->estado = 'N';
    memset(jobs_list->cmd, '\0', COMMAND_LINE_SIZE);

    strcpy(mi_shell, argv[0]); // Init shell name

    signal(SIGCHLD, reaper);
    signal(SIGINT, ctrlc);
    signal(SIGTSTP, ctrlz);

    // Declaramos la línea donde se guardará el comando
    char line[COMMAND_LINE_SIZE];
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
    char *args[ARGS_SIZE];
    
    char command_line[COMMAND_LINE_SIZE];
    memset(command_line, '\0', sizeof(command_line));
    strcpy(command_line, line);

    pid_t pid;

    // Parsear la línea de comandos
    if (parse_args(args, line) <= 0) return 0;

    // Comprobar si es un comando interno
    if (check_internal(args) != 0) return 0;

    // Creamos el hijo
    int background = is_background(args);
    pid = fork();

    if (pid == 0) { // Proceso hijo
        // Ignoramos las señales para el hijo
        signal(SIGCHLD, SIG_DFL);
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);

        // Comprobamos si hay que redireccionar el output
        is_output_redirection(args);

        // Ejecutar el comando
        if (execvp(args[0],args) < 0) {
            fprintf(stderr, ROJO_T "%s: no se encontró la orden \n" RESET, command_line);
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) { // Proceso padre
        #if DEBUGN3
            fprintf(stderr, GRIS_T "[execute_line()→ PID padre: %d (%s)] \n" RESET, getpid(), mi_shell);
            fprintf(stderr, GRIS_T "[execute_line()→ PID hijo: %d (%s)] \n" RESET, pid, command_line);
        #endif

        if (background == 1) { // Proceso en segundo plano (background)
            // Intentamos añadir el proceso
            if (jobs_list_add(pid, 'E', command_line) == -1) exit(EXIT_FAILURE);

            printf("[%d] %d\t%c\t%s \n",
                   n_job, jobs_list[n_job].pid, jobs_list[n_job].estado, jobs_list[n_job].cmd);

        } else { // Proceso en primer plano (foreground)
            //Actualizar los datos de jobs_list[0]
            jobs_list[0].pid = pid;
            jobs_list[0].estado = 'E';
            strcpy(jobs_list[0].cmd, command_line);

            while (jobs_list[0].pid > 0) pause();
        }

    } else {
        // Error al crear el hijo
        perror(ROJO_T "Error" RESET);
        exit(EXIT_FAILURE);
    }

    return 1;
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
 * Función: reaper
 * ----------------------
 * Maneja la señal SIGCHLD (señal enviada a un proceso cuando uno de 
 * sus procesos hijos termina), también denominado enterrador de hijos.
 */
void reaper(int signum) {
    // Volver a asociar señal SIGCHLD a reaper()
    signal(SIGCHLD, reaper);
  
    pid_t ended; 
    int estado; 

    while ((ended = waitpid(-1, &estado, WNOHANG)) > 0) { // Esperar a que termine un proceso hijo
        if (jobs_list[0].pid == ended) {  // Foreground
            #if DEBUGN4
                fprintf(stderr, GRIS_T "\n[reaper()→ recibida señal %i (SIGCHLD)]\n" RESET, SIGCHLD);
            #endif
            if(WIFEXITED(estado)) {
                #if DEBUGN4
                    fprintf(stderr,GRIS_T "[reaper()→ Proceso hijo %d en foreground (%s) finalizado con exit code %d]\n" RESET, ended, jobs_list[0].cmd, WEXITSTATUS(estado));
                #endif
            }else if (WIFSIGNALED(estado)){
                #if DEBUGN4
                    fprintf(stderr,GRIS_T "[reaper()→ Proceso hijo %d en foreground (%s) finalizado por señal %d]\n" RESET, ended, jobs_list[0].cmd, WTERMSIG(estado));
                #endif
            }
            jobs_list[0].pid = 0;
            jobs_list[0].estado = 'F';
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);

        }else { // Background
            // Buscamos el proceso en jobs_list
            int pos = jobs_list_find(ended);
            
            #if DEBUGN5
                fprintf(stderr, GRIS_T "\n[reaper()→ recibida señal %i (SIGCHLD)]\n" RESET, SIGCHLD);
            #endif
            if(WIFEXITED(estado)) {
                #if DEBUGN5
                    fprintf(stderr,GRIS_T "[reaper()→ Proceso hijo %d en background (%s) finalizado con exit code %d]\n" RESET, ended, jobs_list[pos].cmd, WEXITSTATUS(estado));
                #endif
            }else if (WIFSIGNALED(estado)){
                #if DEBUGN5
                    fprintf(stderr,GRIS_T "[reaper()→ Proceso hijo %d en background (%s) finalizado por señal %d]\n" RESET, ended, jobs_list[pos].cmd, WTERMSIG(estado));
                #endif
            }

            // Motramos el proceso terminado y lo eliminamos
            printf("\nTerminado PID %d (%s) en jobs_list[%i] con estado %i \n",
                   ended,jobs_list[pos].cmd,pos,estado);
            jobs_list_remove(pos);
        }
    }
}

/*
 * Función: ctrlc
 * ----------------------
 * Maneja la señal SIGINT (Ctrl + C) que aborta un proceso.
 */
void ctrlc(int signum) {
    // Volver a asociar señal SIGINT a ctrlc()
    signal(SIGINT, ctrlc);

    #if DEBUGN4
        fprintf(stderr, GRIS_T "\n[ctrlc()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n" RESET, getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
    #endif

    if (jobs_list[0].pid > 0) { //Si hay proceso en foreground
        if (strcmp(jobs_list[0].cmd, mi_shell) != 0) { //Si no es el minishell
            #if DEBUGN4
                fprintf(stderr, GRIS_T "[ctrlc()→ recibida señal %d (SIGINT)]\n" RESET, SIGINT);
            #endif

            if (kill(jobs_list[0].pid, SIGTERM) == 0) { //Enviamos señal SIGTERM
                #if DEBUGN4
                    fprintf(stderr, GRIS_T "[ctrlc()→ Señal %d (SIGTERM) enviada a %d (%s) por %d (%s)]\n" RESET, SIGTERM, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
                #endif
                
            } else { //Si no se ha podido enviar la señal
                perror("Error kill");
                exit(-1);
            }
        } else { //Si es el minishell
            #if DEBUGN4 
                fprintf(stderr, GRIS_T "\n[ctrlc()→ Señal %i (SIGTERM) no enviada por %d (%s) debido a que el proceso en foreground es el shell]\n" RESET, SIGTERM, getpid(), mi_shell);
            #endif
        }
        
    } else {
        #if DEBUGN4
            fprintf(stderr, GRIS_T "[ctrlc()→ recibida señal %d (SIGINT)]\n" RESET, SIGINT);
            fprintf(stderr, GRIS_T "[ctrlc()→ Señal %d (SIGTERM) no enviada por %d (%s) debido a que no hay proceso en foreground]\n" RESET, SIGTERM, getpid(), mi_shell);
        #endif
    } 
    printf("\n");
}

/*
 * Función: ctrlz
 * ----------------------
 * Maneja la señal SIGTSTP (Ctrl + Z) que suspende un proceso.
 */
void ctrlz(int signum) {
    // Volver a asociar señal SIGTSTP a ctrlz()
    signal(SIGTSTP, ctrlz);
    #if DEBUGN5
        fprintf(stderr, GRIS_T "[ctrlz()→ Soy el proceso con PID %d, el proceso en foreground es %d (%s)]\n" RESET, getpid(), jobs_list[0].pid, jobs_list[0].cmd);
        fprintf(stderr, GRIS_T "[ctrlz()→ recibida señal %i (SIGTSTP)]\n" RESET, SIGTSTP);
    #endif
    if(jobs_list[0].pid > 0){ //Si hay proceso en foreground
    
        if(strcmp(jobs_list[0].cmd, mi_shell) != 0){ //Si no es el minishell

            #if DEBUGN5
                fprintf(stderr, GRIS_T "[ctrlz()→ Señal %i (SIGSTOP) enviada a %d (%s) por %d (%s)]\n" RESET, SIGSTOP, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
            #endif

            // Enviamos SIGSTOP y actualizamos el estado del proceso
            kill(jobs_list[0].pid,SIGSTOP);
            jobs_list[0].estado = 'D';

            // Añadimos el proceso a jobs_list
            jobs_list_add(
                    jobs_list[0].pid,
                    jobs_list[0].estado,
                    jobs_list[0].cmd
            );

            int contador = jobs_list_find(jobs_list[0].pid);

            //Si no se ha podido añadir
            if (contador == -1) {
                fprintf(stderr, ROJO_T "Error al añadir el proceso\n" RESET);
                exit(-1);
            }

            //Actualizamos los datos de jobs_list[0] y lo mostramos
            jobs_list[0].pid = 0;
            printf("\r\n[%i] %d\t%c\t%s \n", contador,
                   jobs_list[contador].pid, jobs_list[contador].estado, jobs_list[contador].cmd);
        }else{
            #if DEBUGN5
                fprintf(stderr, GRIS_T "[ctrlz()→ Señal %i (SIGSTOP) no enviada por %d (%s) debido a que el proceso en foreground es el shell]\n" RESET, SIGSTOP, getpid(), mi_shell);
            #endif           
        }
    }else{
        #if DEBUGN5
            fprintf(stderr, GRIS_T "[ctrlz()→ Señal %i (SIGSTOP) no enviada por %d debido a que no hay proceso en foreground]\n" RESET, SIGSTOP, getpid());
        #endif
    }
}

/*
 * Función: is_background
 * ----------------------
 * Devuelve 1 si localiza el token '&' en args[] (comando en background)
 * y 0 en caso contrario (comando en foreground).
 */
int is_background(char **args) {
    int i = 0;

    while (args[i]) { 
        if (strcmp (args[i], "&") == 0) { // Si el comando contiene '&'
            args[i] = NULL; // Eliminar '&' del array de argumentos
            return 1; // Comando en background
        }
        i++; 
    }
    return 0;
}

/*
 * Función: jobs_list_add
 * ----------------------
 * Añade un nuevo trabajo al array en la posición indicada 
 * por la variable 'n_job'.
 */
int jobs_list_add(pid_t pid, char estado, char *cmd) {
    // Añadimos el trabajo si es posible e incrementamos n_job
    if(n_job < N_JOBS - 1) {
        n_job++;
		jobs_list[n_job].pid = pid;
		strcpy(jobs_list[n_job].cmd, cmd); 
		jobs_list[n_job].estado = estado; 
    
		return 0;
	}

    return -1;
}

/*
 * Función: jobs_list_find
 * -----------------------
 * Busca en el array de trabajos el PID que recibe como argumento 
 * y retorna la posición de este.
 */
int jobs_list_find(pid_t pid) {
    int i = 1; // Ignoramos i=0 -> foreground

    // Buscamos el proceso en jobs_list
    while(jobs_list[i].pid != pid && (i<n_job && i < N_JOBS)) i++;

    //Si lo hemos encontrado lo devolvemos sino -1
    return jobs_list[i].pid == pid ? i : -1;
}

/*
 * Función: jobs_list_remove
 * -------------------------
 * Elimina un trabajo del array. 
 */
int jobs_list_remove(int pos) {
    if (pos < n_job) { //Si no es el último proceso
        jobs_list[pos] = jobs_list[n_job]; //Movemos el último proceso a la posición del proceso a eliminar
    }
    n_job--; //Actualizamos el número de procesos en jobs_list

    return 0; //Si se ha eliminado correctamente
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
    
    char linea[COMMAND_LINE_SIZE];
    if (args[1] == NULL) {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: source <nombre_fichero>\n" RESET);
        return -1;
    }

    FILE* f = fopen(args[1], "r");
    if(f == NULL) {
        perror(ROJO_T "fopen" );
        return -1;
    }

    while(fgets(linea, COMMAND_LINE_SIZE, f) != NULL) {
        for (int i = 0; i < COMMAND_LINE_SIZE; i++) {
            if (linea[i] == '\n') linea[i] = '\0';
        }
        #if DEBUGN3
                fprintf(stderr, GRIS_T "[internal_source()-> LINE: %s]\n" RESET, linea);
        #endif
        fflush(f);
        execute_line(linea);
    }

    fclose(f);
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

    int i = 1;
    while (i <= n_job) {
        printf("[%d] %d\t %c\t%s\n", i, jobs_list[i].pid, jobs_list[i].estado, jobs_list[i].cmd);
        i++;
    }
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

    if (args[1] == NULL) {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: fg <job>\n" RESET);
        return -1;
    }

    int pos = (int) strtol(args[1], NULL, 10);
    if(pos > n_job || pos == 0) {
        fprintf(stderr, ROJO_T "fg %i: No existe ese trabajo\n" RESET, pos);
        return -1;
    }

    if(jobs_list[pos].estado == 'D') {
        kill(jobs_list[pos].pid,SIGCONT);
        jobs_list[pos].estado = 'E';

        #if DEBUGN6
        fprintf(stderr, GRIS_T "[internal_fg()→ Señal %i (SIGCONT) enviada a %d (%s)]\n" RESET, SIGCONT,
                jobs_list[pos].pid, jobs_list[pos].cmd);
        #endif
    }
    // Buscamos '&'
    int i = 0;
    while(jobs_list[pos].cmd[i] != 0) {
        if(jobs_list[pos].cmd[i] == '&') jobs_list[pos].cmd[i] = 0;
        i++;
    }

    jobs_list[0] = jobs_list[pos];
    jobs_list_remove(pos);
    printf("%s\n", jobs_list[0].cmd);

    // Esperamos a que termine el proceso
    while(jobs_list[0].pid > 0) pause();

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

    if (args[1] == NULL) {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: bg <job>\n" RESET);
        return -1;
    }

    int pos = (int) strtol(args[1], NULL, 10);
    if(pos > n_job || pos == 0) {
        fprintf(stderr, ROJO_T "bg %i: no existe ese trabajo\n" RESET, pos);
        return -1;
    }

    if(jobs_list[pos].estado == 'E') { // Si esta ejecutandose
        fprintf(stderr, ROJO_T "bg %i: el trabajo ya está en segundo plano\n" RESET, pos);
        return -1;
    }

    // Añadimos '&' al final y cambiamos el estado a 'E'
    char c[2] = " &";
    strncat(jobs_list[pos].cmd, c, 2);
    jobs_list[pos].estado = 'E';
    kill(jobs_list[pos].pid,SIGCONT);

    #if DEBUGN6
    fprintf(stderr, GRIS_T "[internal_bg()→ Señal %i (SIGCONT) enviada a %d (%s)]\n" RESET, SIGCONT,
            jobs_list[pos].pid, jobs_list[pos].cmd);
    #endif

    printf("[%i] %d\t%c\t%s \n", pos, jobs_list[pos].pid, jobs_list[pos].estado, jobs_list[pos].cmd);

    return 1;
}

/*
 * Función: procesar_token
 * -----------------------
 * Permite el redireccionamiento de la salida de un comando externo a un fichero externo
 */
int is_output_redirection(char **args) { 
    int fd; 
    int i = 0;


    while (args[i] != NULL) {
        // Si encontramos '>'
        if (*args[i] == '>' && args[i+1] != NULL) {
            // Abrimos el fichero
            fd = open(args[i+1], O_WRONLY|O_CREAT|O_TRUNC);

            // Si hemos encontrado el fichero lo redireccionamos
            if(fd >= 0) {
                args[i] = 0;
                args[i+1] = NULL;
                dup2(fd, 1);

                close(fd);
                return 1;
            }

            perror(ROJO_T "Error al abrir archivo" RESET); //Caso fichero incorrecto
            return 0;
        }
        i++; 
    }

    return 0;
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