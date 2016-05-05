#include <stdio.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <stdlib.h>
#include <sys/time.h>

double lastTime;

double getTime() {
  timeval now;
  gettimeofday(&now, NULL);
  return ((double) now.tv_sec) + ((double) now.tv_usec)/1000000.;
}
void start () {
  lastTime = getTime();
} 

double stop () {
  double d = (getTime()-lastTime);
  return d;
} 

int compare (const void * a, const void * b)
{
   return ( *(char*)a - *(char*)b );
}

int main(int argc, char * argv[])
{
	FILE * fp;
	FILE * writer;

	if(argc < 2)
	{
		std::cout << "Please provide file to sort on. ( ./externalMergeSort example.txt )\n";
		return 0;
	}

	fp = fopen(argv[1], "r+");

	if( fp == NULL ) 
   {
      perror ("Error opening file");
      return(-1);
   }

   fseek(fp, 0, SEEK_END);
   int len = ftell(fp); //length of file in bytes
   fseek(fp, 0, SEEK_SET);

   char * buffer = (char *)malloc(len);

   std::cout << len << " " << std::endl;

   int x;
   int iter = 0;
    
   fread(buffer, sizeof(char), len , fp);

   start();

   qsort(buffer, len, sizeof(char), compare);
   
   std::cout << "Time = " << stop() << std::endl;

   writer = fopen("temp.txt", "w");
   fwrite(buffer, sizeof(char), len, writer);

   free(buffer);
   fclose(fp);
   fclose(writer);


}
