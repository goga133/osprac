﻿#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

int incr(int semid, struct sembuf* buf) {
    buf->sem_flg = 0;
    buf->sem_num = 0;
    buf->sem_op = 1;

    return semop(semid, buf, 1);
}

int decr(int semid, struct sembuf* buf) {
    buf->sem_flg = 0;
    buf->sem_op = -1;
    buf->sem_num = 0;

    return semop(semid, buf, 1);
}


int main()
{
  struct sembuf mybuf;
  int semid;
  int     *array;
  int     shmid;
  int     new = 1;
  char    pathname[] = "07-3a.c";
  key_t   key;
  long    i;

  if ((key = ftok(pathname,0)) < 0) {
    printf("Can\'t generate key\n");
    exit(-1);
  }

  if ((shmid = shmget(key, 3*sizeof(int), 0666|IPC_CREAT|IPC_EXCL)) < 0) {
    if (errno != EEXIST) {
      printf("Can\'t create shared memory\n");
      exit(-1);
    } else {
      if ((shmid = shmget(key, 3*sizeof(int), 0)) < 0) {
        printf("Can\'t find shared memory\n");
        exit(-1);
      }
      new = 0;
    }
  }

  if ((array = (int *)shmat(shmid, NULL, 0)) == (int *)(-1)) {
    printf("Can't attach shared memory\n");
    exit(-1);
  }

  if ((semid = semget(key, 1, 0666)) < 0) {
      printf("Семафор не найден, повтор¤ю попытку с флагом IPC CREAT...\n");
      if ((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0) {
          printf("Создать невозможно, аварийно завершаюсь.\n");
          exit(-1);
      }
      printf("Успешно создан семафор!\n");
      incr(semid, &mybuf);
  }

  if (new) {
    array[0] =  1;
    array[1] =  0;
    array[2] =  1;
  } else {
    decr(semid, &mybuf);
    array[0] += 1;
    for(i=0; i<2000000000L; i++);
    array[2] += 1;
    incr(semid, &mybuf);
  }

  printf
    ("Program 1 was spawn %d times, program 2 - %d times, total - %d times\n",
    array[0], array[1], array[2]);

  if (shmdt(array) < 0) {
    printf("Can't detach shared memory\n");
    exit(-1);
  }

  return 0;
}
