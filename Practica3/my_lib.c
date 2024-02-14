/*
 * Blanca Atienzar Martinez
 * Pau Toni Bibiloni Martinez
 * Unai Pueyo Almodovar
 */
#include "my_lib.h"

// --- STRING PROCESSING -----------------------------------------

/* Función: my_strlen
 * --------------------
 * calcula el nº de bytes de la cadena apuntada por str.
 * str: apunta a la cadena de la cual se ha de encontrar la longitud.
 * devuelve: la longitud de la cadena apuntada por str. No devuelve error.
 */
size_t my_strlen(const char *str) {
    size_t length = 0;
    while (str[length]) length++;
    return length;
}

/* Función: my_strcmp
 * --------------------
 * compara las cadenas apuntadas por str1 y str2.
 * str1: apunta a la 1ª cadena a comparar.
 * str2: apunta a la 2ª cadena a comparar.
 * Devuelve un entero:
 *  < 0 indica que str1 < str2.
 *  > 0 indica que str2 < str1.
 *  = 0 indica que str1 = str2.
 */
int my_strcmp(const char *str1, const char *str2) {
    while (*str1 != '\0' && *str2 != '\0' && *str1 == *str2) {
        /*Mientras el caracter sea el ultimo de la cadena o
          igual, pasamos al siguiente caracter*/
        str1++,str2++;
    }
    /*El numero entera es el resultado de la siguiente resta*/
    return *str1 - *str2;
}

/* Función: my_strcpy
 * --------------------
 * copia la cadena apuntada por src en la memoria apuntada por dest.
 * dest: es el puntero a la cadena destino donde se copiará el contenido.
 * src: puntero a la cadena a copiar.
 * Devuelve: el puntero dest. No devuelve error.
 */
char *my_strcpy(char *dest, const char *src) {
    if (dest == NULL || src == NULL) {
        /*Manejar el caso de punteros nulos si es necesario*/
        return NULL;
    }
    /*Copiar caracteres hasta encontrar el carácter nulo '\0'*/
    while ((*dest++ = *src++));
    return dest;
}

/* Función: my_strncpy
 * --------------------
 * copia n caracteres de la cadena apuntada por src en 
 * la memoria apuntada por dest.
 * dest: es el puntero a la cadena destino donde se copiará el contenido.
 * src: apunta a la cadena a copiar.
 * n: la cantidad de caracteres que se copiarán de la cadena apuntada por src.
 * Devuelve: el puntero dest. No devuelve error.
 */
char *my_strncpy(char *dest, const char *src, size_t n){
    int i=0;
    char *px = dest; /*Guardamos un puntero de la cadena de dest*/

    if(strlen(src) < n ){ /*Comprobamos si src es más corta que n */
        memset(dest, 0, *dest);
        /*Si lo es, llenamos la cadena dest con caracteres nulos (0)*/
    }

    while(i != n ){ /*Copiamos los primeros n caracteres de src a dest*/
        *(dest+i) = src[i];
        i++;
    }
    return px;/*Devolvemos el puntero al principio de la cadena de dest*/
}

/* Función: my_strcat
 * --------------------
 * añade la cadena apuntada por src a la cadena apuntada por dest.
 * dest: es el puntero a la cadena destino. Espacio reservado para *dest 
 * ha de ser lo suficientemente grande para contener la cadena resultante 
 * de la concatenación.
 * src: apunta a la cadena a concatenar.
 * Devuelve: el puntero dest. No devuelve error.
 */
char *my_strcat(char *dest, const char *src){
    int i=0;
    char *px = dest; /*Guardamos un puntero de la cadena de dest*/

    while(dest[i]) i++; /*Hallamos el tamaño de dest,
                          para llegar a su ultimo caracter*/

    for(int j=0; j < strlen(src); j++){
        *(dest+i) = src[j];
        /*Copiamos cada carácter de src al final de dst*/
        i++;
    }
    return px;/*Devolvemos el puntero al principio de la cadena de dest*/
}

/* Función: my_strchr
 * --------------------
 * escanea la cadena apuntada por str buscando la primera 
 * ocurrencia del carácter c.
 * str: apunta a la cadena a escanear.
 * c: es el carácter buscado.
 * Devuelve: el puntero a la primera ocurrencia del carácter c en la cadena
 * str o NULL si el carácter no se encuentra. No devuelve error.
 */
char *my_strchr(const char *str, int c) {
    while(*str != '0'){
        if(*str == c){
            return (char *) str;
        }
        str++;
    }
    return NULL; /*Si no se encuentra el carácter c */
}

// --- STACK -----------------------------------------

/* Función: my_stack_init
 * --------------------
 * Reserva espacio para una variable de tipo struct my_stack, que contendrá
 * el puntero top al nodo superior de la pila y el tamaño de los datos size. 
 * Inicializa top con NULL, y size con el tamaño de datos pasado por parámetro.
 * size: tamaño de los datos que se introducirane n la pila.
 * Devuelve: un puntero a la pila inicializada (struct my_stack).
 */
struct my_stack *my_stack_init (int size){
    //Reservar memoria y establecer 'defaults'
    struct my_stack* stack = malloc(sizeof(struct my_stack));
    stack->top = NULL;
    stack->size=size;
    return stack;
}

/* Función: my_stack_push
 * --------------------
 * Inserta un nuevo nodo en los elementos de la pila.
 * stack: puntero a la pila, que deberia haber sido inicializada.
 * data: puntero a los datos de ese nodo.
 * Devuelve: 0 si ha ido bien, -1 si hubo error.
 */
int my_stack_push (struct my_stack *stack, void *data) {
    if(!stack) return -1;

    //Crear el nuevo nodo y apuntar hacia top
    struct my_stack_node* node = malloc(sizeof(struct my_stack_node));
    node->next = stack->top;
    node->data = data;

    //Actualizar top
    stack->top = node;
    return 0;
}

/* Función: my_stack_pop
 * --------------------
 * Elimina el nodo superior de los elementos de la pila.
 * stack: puntero a la pila, que deberia haber sido inicializada.
 * Devuelve: el puntero a los datos del elemento eliminado. Si no 
 * existe(pila vacía), retorna NULL.
 */
void* my_stack_pop (struct my_stack* stack) {
    if(!stack || stack->size <= 0 || !stack->top) return NULL;

    //Obtener top y su dato
    struct my_stack_node* node = stack->top;
    void* data = node->data;

    //Actualizar top y liberar memoria
    stack->top = node->next;
    free(node);

    return data;
}

/* Función: my_stack_len
 * --------------------
 * Recorre la pila y retorna el número de nodos totales 
 * que hay en los elementos de la pila.
 * stack: puntero a la pila, que deberia haber sido inicializada.
 * Devuelve: el número de nodos que hay en los elementos de la pila.
 */
int my_stack_len(struct my_stack* stack) {
    if(!stack || !stack->top) return 0;

    //Iniciamos el contador a 1 (elemento actual)
    struct my_stack_node* node = stack->top;
    int count = 1;

    //Bucle hasta que no haya siguiente
    while (node->next) {
        count++;
        node = node->next;
    }

    return count;
}

/* Función: my_stack_purge
 * --------------------
 * Recorre la pila liberando la memoria que habíamos reservado
 * con malloc() en algún momento, se libera con la función free().
 * stack: puntero a la pila, que deberia haber sido inicializada.
 * Devuelve: el número de bytes liberados.
 */
int my_stack_purge(struct my_stack *stack) {
    //Comprobar que existe la pila
    if (!stack) return 0;

    int bytes_liberados = sizeof(struct my_stack);

    void* data;
    //Loop por todos los elementos eliminados
    while ((data = my_stack_pop(stack))) {
        free(data);
        bytes_liberados += sizeof(struct my_stack_node) + stack->size;
    }

    //Liberamos memoria del stack
    free(stack);
    return bytes_liberados;
}

/* Función: stackIterator
 * --------------------
 * recorre cada nodo desde la parte superior hasta llegar al último nodo y
 * por lo tanto escribe los datos desde el elemento inferior al superior.
 * fd: archivo donde debemos escribir/leer.
 * stack: puntero a la pila, que deberia haber sido inicializada.
 * node: nodo actual en el esta situado el top.
 * length: número de nodos que hay en los elementos de la pila.
 * Devuelve: el total de cambios realizados. Mientras
 * que cuando no quedan elementos, devolvemos 0
 */
int stackIterator(int fd, struct my_stack* stack, struct my_stack_node* node, int length) {
    int changes = 0;
    void* data;

    //Loop por cada elementos de la pila
    if(length--) {
        data = node->data;
        node = node->next;

        //Llamada recursiva para obtener el primer elemento
        changes += stackIterator(fd, stack, node, length);

        //Verificamos que la escritura fue exitosa
        int res = write(fd, data, stack->size);
        if(res == -1) {
            perror("Error: ");
            return 0;
        }

        changes++;
        return changes;
    }

    return 0;
}

/* Función: my_stack_write
 * --------------------
 * Almacena los datos de la pila en el fichero indicado por filename. Los 
 * datos han de quedar escritos en el fichero en el mismo orden en que 
 * fueron introducidos en la pila.
 * stack: puntero a la pila, que deberia haber sido inicializada.
 * filename: caracter que indica el fichero.
 * Devuelve: el número de elementos almacenados, -1 si hubo error.
 */
int my_stack_write (struct my_stack *stack, char *filename){
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    //Verificamos si el fichero se ha abierto correctamente
    if(fd == -1) {
        perror("Error: ");
        return -1;
    }


    //Verificamos si la escritura ha sido correctamente
    int res = write(fd, &stack->size, sizeof(int));
    if(res == -1 ) {
        perror("Error: ");
        return -1;
    }

    //Llamamos al escritor recurrente
    int count = stackIterator(fd, stack, stack->top, my_stack_len(stack));

    //Verificacion del cierre del fichero
    res = close(fd);
    if(res == -1 ) {
        perror("Error: ");
        return -1;
    }

    return count;
}

/* Función: my_stack_read
 * --------------------
 * Lee los datos de la pila almacenados en el fichero y 
 * reconstruye la pila en memoria principal.
 * filename: caracter que indica el fichero.
 * Devuelve: el puntero a la pila creada, y si hubo error, retorna NULL.
 */
struct my_stack *my_stack_read (char *filename){
    //Verificamos si el fichero se ha abierto correctamente
    int fd = open(filename, O_RDONLY);
    if(!fd) {
        perror("Error: ");
        return NULL;
    }

    int size = 0;
    read(fd, &size, sizeof(int));

    //Inicializar una nueva pila con el tamaño leído
    struct my_stack* stack = my_stack_init(size);

    //Leer datos desde el archivo y agregarlos a la pila
    void* buffer = malloc(size);
    while (read(fd, buffer, size) > 0) {
        my_stack_push(stack, buffer);
        buffer = malloc(size);
    }

    //Verificacion del cierre del fichero
    if(close(fd) == -1){
        perror("Error: ");
        return NULL;
    }

    return stack;
}