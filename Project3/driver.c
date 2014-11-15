#include "driver.h"

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
  char *file;
  while (curr != NULL) {
    switch(curr -> task) {
      case TASK1:
        printf("loading task1\n");
        file = T1;
        break;
      case TASK2:
        printf("loading task2\n");
        file = T2;
        break;
      case TASK3:
        printf("loading task3\n");
        break;
      case TASK4:
        printf("loading task4\n");
        break;
      case TASK5:
        printf("loading task5\n");
        break;
      default:
        printf("I dont know what Im doing\n");
    }

    if(luaL_loadfile(L, file)) {
      fprintf(stderr, "ERROR: luaL_loadFile failed for: %s", lua_tostring(L, -1));
    }
    printf("Executing...\n");
    if(lua_pcall(L, 0, 0, 0)) {
      fprintf(stderr, "ERROR: luaL_pcall failed for: %s", lua_tostring(L, -1));
    }
    printf("Done. Shutting down task.\n");
    lua_close(L);

    curr = curr -> next;
  }
  return 0;
}




