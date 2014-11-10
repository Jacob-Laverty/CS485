#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

typedef enum {TASK1, TASK2, TASK3, TASK4, TASK5} LuaTask;

struct elem {
 LuaTask task;
 struct elem *next;
};

struct elem *start = NULL;
struct elem *curr = NULL;
void addTask(LuaTask task) {
  if(start == NULL) {
    if((start = malloc(sizeof(struct elem))) != NULL) {
    start -> task = task;
    curr = start;
    }    
  } else {
    if((curr -> next = malloc(sizeof(struct elem))) != NULL) {
      curr -> next -> task = task; 
      curr -> next -> next = NULL;
      curr = curr -> next;
    }
  }
}


int main(int argc, char **argv) {
  printf("Robot driver starting up\n");
  LuaTask task; 
  lua_State *L;
  L = luaL_newstate();
  luaL_openlibs(L);

  int arg;
  opterr = 0;

  while((arg = getopt(argc, argv, "12345")) != -1) {
    switch(arg) {
      case '1':
        task = TASK1;
        break;
      case '2':
        task = TASK2;
        break;
      case '3':
        task = TASK3;
        break;
      case '4':
        task = TASK4;
        break;
      case '5':
        task = TASK5;
        break;
      default:
        printf("Default\n");
    }
    addTask(task);
  }

  curr = start;
  while (curr != NULL) {
    switch(curr -> task) {
      case TASK1:
        printf("running task1\n");
        break;
      case TASK2:
        printf("running task2\n");
        break;
      case TASK3:
        printf("running task3\n");
        break;
      case TASK4:
        printf("running task4\n");
        break;
      case TASK5:
        printf("running task5\n");
        break;
      default:
        printf("I dont know what Im doing\n");
    }
    curr = curr -> next;
  }
  return 0;
}




