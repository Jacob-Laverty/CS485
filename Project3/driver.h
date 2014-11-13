#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//Lua includes
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define T1 "tasks/Assignment3_1.lua"
#define T2 "tasks/Assignment3_2.lua"

typedef enum {TASK1, TASK2, TASK3, TASK4, TASK5} LuaTask;
struct elem {
  LuaTask task;
  struct elem *next;
};

void addTask(LuaTask);
int main(int argc, char **argv);
