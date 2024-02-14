//test2a.c: test para probar el gestor de pilas sin escribir ni leer de disco
#include <stdio.h>
#include <string.h>
#include "my_lib.h"


struct my_data {
   int val;
   char name[8];
};
 
int main() {
   struct my_stack *s;
   struct my_data *data1, *data2;

   char *names[]={"Xisco", "Cati"};
   //char names[2][8];
   //strcpy(names[0],"Xisco");
   //strcpy(names[1],"Cati");

    //inicializamos la pila
   s = my_stack_init(sizeof(struct my_data));
   printf("Longitud inicial de la pila: %d\n", my_stack_len(s));
   //introducimos 2 nodos en la pila
   printf ("\nIntroducimos los nodos en la pila:\n");
   for (int i = 0; i < 2; i++) {
       data1 = malloc(sizeof(struct my_data));
       if (data1 == NULL) {
            fprintf(stderr, "Out of memory\n");
            return -1;
       }
       data1->val = i;
       strcpy(data1->name, names[i]);
       my_stack_push(s,data1);
       printf("Valor: %d\t", data1->val);
       printf("Nombre: %s\n", data1->name);
   }
   printf("Longitud de la pila tras meter los elementos: %d\n", my_stack_len(s));
   printf ("\nExtraemos todos los nodos de la pila\n");
   while (s->top){//extraemos los nodos con pop
       data2=my_stack_pop(s);
       printf("Valor: %d\t", data2->val);
       printf("Nombre: %s\n", data2->name);
       free(data2);
   }
   printf("Longitud de la pila tras extraer todos elementos: %d\n", my_stack_len(s));
   printf ("\nIntroducimos de nuevo los nodos en la pila:\n");
   for (int i = 0; i < 2; i++) {
       data1 = malloc(sizeof(struct my_data));
       if (data1 == NULL) {
            fprintf(stderr, "Out of memory\n");
            return -1;
       }
       data1->val = i;
       strcpy(data1->name, names[i]);
       my_stack_push(s,data1);
       printf("Valor: %d\t", data1->val);
       printf("Nombre: %s\n", data1->name);
   }
   
   printf("\nsizeof(struct my_stack): %lu\n", sizeof(struct my_stack));
   printf("sizeof(struct my_stack_node): %lu\n", sizeof(struct my_stack_node));
   printf("sizeof(struct my_data): %lu\n", sizeof(struct my_data));
   
   //liberamos todo el espacio ocupado por la pila
   printf ("\nLiberamos %lu bytes del struct my_stack, %lu bytes de los nodos y %lu de los datos. Total: %d\n", sizeof (struct my_stack), sizeof (struct my_stack_node)*2, sizeof(struct my_data)*2, my_stack_purge(s));
}
