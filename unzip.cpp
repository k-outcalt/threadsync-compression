// Katherine Outcalt, Bryce Chinn, and Andrew Varela                                                                                                                                                                                    
// wunzip.cpp
// 2.13.2021

/** Wunzip decompresses a zip (.z) file that used run time encoding.
 * The program outputs the decompressed contents onto the console.
 */

#include <iostream>
#include <fstream>
using namespace std;

// decompress decompresses the file of "fname" onto the console
// pre: fname is a valid file name
// post: file opened and closed (read)
void decompress(char* fname); 

int main(int argc, char **argv)
{                                                                                                                     
  if (argc == 1)
  {
    cout << "wunzip: file1 [file2 ...]" << endl;
    exit(1);
  }

  for (int i = 1; i < argc; i++)
  {
    decompress(argv[i]); 
  }
  return 0;
}

void decompress(char* fname)
{
  cout << "In decompress function" << endl;
  FILE* fhandle = fopen(fname, "rb");
  if (fhandle == NULL)
  {
    cout << "wunzip: cannot open file" << endl;
    exit(1);
  } 

  char buffer;
  int frequency; 
  
  while (!feof(fhandle)) 
  {
    if (fread(&frequency, sizeof(int), 1, fhandle) != 0 && 
      fread(&buffer, sizeof(char), 1, fhandle) != 0) 
    {
      for (int i = 0; i < frequency; i++) 
      {
        cout << buffer; 
      }
    }
  }
  fclose(fhandle); 
}