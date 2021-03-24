// Katherine Outcalt, Andrew Varela, Bryce Chinn
// 2.14.2021
// pzip.cpp

/** The purpose of pzip.cpp is to use multithreading techniques to compress
 * files to a zip file. Pzip uses a thread pool that waits on a semaphore
 * to get a task, which is a file mapped to memory. Mutex locks and semaphore
 * libraries are imported to execute this. 
 */ 

#include <sys/sysinfo.h>
#include <iostream>
#include <sys/mman.h>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <vector>

using namespace std;

// global variables for threads
int taskCount = 0;
int taskTotal; 
pthread_mutex_t mapLock; 
sem_t resources; // unbounded buffer 
vector<sem_t> signal; // compression synchronization

// struct to store char buffer and size from mmap()
struct Maps {
  char* charArr;
  size_t len;
  int taskNum; 
};

// struct to store compressed data
struct Item {
  int count;
  char letter;
};

// pre: valid filename
// post: file at filename will be mapped. TaskCount incremented
// and taskQueue enqueued with Maps
FILE* submitFile(char* filename, int orderNum);

// pre: file material in contents and length > 0
// post: taskCount decremented, and taskQueue dequeued
void* getContents(void* args);

// In compress func, threads use semaphores to wait on previous thread.
// pre: Semaphore vector is initialized
// post: file will be unmapped and contents printed to stdout
void compress(Maps task); 

// struct to store mapped file addresss
vector<Maps> taskQueue;


int main(int argc, char **argv)
{
  if (argc == 1)
  {
    cout << "wzip: file1 [file2...]" << endl; 
    exit(1);
  }
  
  // create mutex lock
  if (pthread_mutex_init(&mapLock, NULL) != 0)
  {
    perror("Mutex init failed.\n");
    exit(1);
  }
  
  // create semaphores
  sem_init(&resources, 0, 0);

  sem_t tempSem; 
  for (int i = 0; i < argc - 1; i++)
  {
    sem_init(&tempSem, 0, 0);
    signal.push_back(tempSem);
  }

  // create threads
  taskTotal = argc - 1;
  pthread_t pool[get_nprocs()];
  
  for (int i = 0; i < argc - 1; i++)
  {
    if (pthread_create(&pool[i], NULL, &getContents, NULL) != 0)
    {
      perror("Thread creation failed.\n");
      exit(1);
    }
  }

  // create resources/file maps
  FILE* fileptrs[argc - 1]; 
  for (int i = 1; i < argc; i++)
  {
    fileptrs[i - 1] = submitFile(argv[i], i - 1);
  }

  // join threads
  for (int i = 0; i < argc - 1; i++)
  {
    if (pthread_join(pool[i], NULL) != 0)
    {
      perror("Thread join failed.\n");
      exit(1);
    }
  }
  
  // destroy resources
  for (int i = 0; i < argc - 1; i++)
  {
    fclose(fileptrs[i]);
  }

  pthread_mutex_destroy(&mapLock);
  sem_destroy(&resources);

  for (int i = 0; i < argc - 1; i++)
  {
    sem_destroy(&signal[i]);
  }
  return 0;
}

FILE* submitFile(char* filename, int orderNum)
{
  if (taskCount >= taskTotal)
  {
    cout << "Max tasks reached" << endl;
    exit(1);
  }
  FILE* fileptr; 
  int fdNum;
  size_t fileLength;
  char* fAddress;

  fileptr = fopen(filename, "r");
  // get file size and file descriptor 
  if (fileptr == NULL)
  { 
    cout << "File " << filename << " failed to open." << endl;
    exit(1);
  }

  fdNum = fileno(fileptr);
  
  fseek(fileptr, 0, SEEK_END);
  fileLength = ftell(fileptr);
  rewind(fileptr);

  // map file and get buffer array contents
  fAddress = static_cast<char*>(mmap(
     NULL, fileLength, PROT_READ, MAP_PRIVATE, fdNum, 0));

  Maps task;
  task.taskNum = orderNum;
  task.charArr = fAddress;
  task.len = fileLength;

  // wait until there are open spots in buffer
  pthread_mutex_lock(&mapLock);

  taskQueue.push_back(task);
  taskCount++;
  
  pthread_mutex_unlock(&mapLock);
  sem_post(&resources);

  return fileptr; 
}

void* getContents(void* args)
{
  Maps task; 
  // wait until there are resources in buffer
  sem_wait(&resources);
  pthread_mutex_lock(&mapLock);

  task = taskQueue[0];
  taskQueue.erase(taskQueue.begin());
  
  pthread_mutex_unlock(&mapLock);
  compress(task);
  return NULL;
}

void compress(Maps task)
{
  // store file contents into temporary buffer
  vector<Item> tempBuffer;  
  char prev = task.charArr[0];   
  int same = 1;

  for (size_t i = 1; i < task.len; i++)
  {
    if (prev == task.charArr[i])
      same++;
    else
    {
      Item item;
      item.count = same;
      item.letter = prev;
      tempBuffer.push_back(item);
      same = 1;
    }
    prev = task.charArr[i];
  }

  // if NOT first task, then wait on previous task
  if (task.taskNum != 0)
  {
    sem_wait(&signal[task.taskNum - 1]);
  }

  // print to stdout
  for (long unsigned int i = 0; i < tempBuffer.size(); i++)
  {
    fwrite(&(tempBuffer[i].count), sizeof(int), 1, stdout);
    cout << tempBuffer[i].letter;
  }
  
  sem_post(&signal[task.taskNum]);
  
  munmap(task.charArr, task.len);
}
