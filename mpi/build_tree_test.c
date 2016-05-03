#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>


#define INIT 1 // Message giving size and height
#define DATA 2 // Message giving vector to sort
#define ANSW 3 // Message returning sorted vector



// void communicate ( int myHeight, int myRank )
// {  int parent = myRank & ~(1<<myHeight);

//    if ( myHeight > 0 )
//    {  int nxt     = myHeight - 1;
//       int rtChild = myRank | ( 1 << nxt );

//       printf ("%d sending data to %d\n", myRank, rtChild);
//       communicate ( nxt, myRank );
//       communicate ( nxt, rtChild );
//       printf ("%d getting data from %d\n", myRank, rtChild);
//    }
//    if ( parent != myRank )
//       printf ("%d transmitting to %d\n", myRank, parent);
// }

/**
 * Partitioned merge logic
 *
 * The working core:  each internal node recurses on this function
 * both for its left side and its right side, as nodes one closer to
 * the leaf level.  It then merges the results into the vector passed.
 *
 * Leaf level nodes just sort the vector.
 */
// void partitionedSort ( char *vector, int size, int myHeight, int mySelf )
// {  int parent,
//        rtChild;
//    int nxt;

//    parent = mySelf & ~(1 << myHeight);
//    nxt = myHeight - 1;
//    rtChild = mySelf | ( 1 << nxt );

//    if ( myHeight > 0 )
//    {
//       int   left_size  = size / 2,
//             right_size = size - left_size;
//       char *leftArray  = (char*) calloc (left_size, sizeof *leftArray),
//            *rightArray = (char*) calloc (right_size, sizeof *rightArray);
//       int   i, j, k;                   // Used in the merge logic

//       memcpy (leftArray, vector, left_size*sizeof *leftArray);
//       memcpy (rightArray, vector+left_size, right_size*sizeof *rightArray);

//       partitionedSort ( leftArray, left_size, nxt, mySelf );
//       partitionedSort ( rightArray, right_size, nxt, rtChild );

//       // Merge the two results back into vector
//       i = j = k = 0;
//       while ( i < left_size && j < right_size )
//          if ( leftArray[i] > rightArray[j])
//             vector[k++] = rightArray[j++];
//          else
//             vector[k++] = leftArray[i++];
//       while ( i < left_size )
//          vector[k++] = leftArray[i++];
//       while ( j < right_size )
//          vector[k++] = rightArray[j++];
//       free(leftArray);  free(rightArray);   // No memory leak!
//    }
//    else
//       qsort( vector, size, sizeof *vector, compare );
// }

int compare (const void * a, const void * b)
{
   return ( *(char*)a - *(char*)b );
}

void parallelMerge ( char *vector, long size, int myHeight )
{  int parent;
   int myRank, nProc;
   int rc, nxt, rtChild;

   rc = MPI_Comm_rank (MPI_COMM_WORLD, &myRank);
   rc = MPI_Comm_size (MPI_COMM_WORLD, &nProc);

   parent = myRank & ~(1 << myHeight);
   nxt = myHeight - 1;
   rtChild = myRank | ( 1 << nxt );

   if ( myHeight > 0 )
   {//Possibly a half-full node in the processing tree
      if ( rtChild >= nProc )     // No right child.  Move down one level
         parallelMerge ( vector, size, nxt );
      else
      {
         int   left_size  = size / 2,
               right_size = size - left_size;
         char *leftArray  = (char*) calloc (left_size,
                                            sizeof *leftArray),
              *rightArray = (char*) calloc (right_size,
                                            sizeof *rightArray);
         int   iVect[2];
         int   i, j, k;                // Used in the merge logic
         MPI_Status status;            // Return status from MPI

         memcpy (leftArray, vector,
                 left_size*sizeof *leftArray);
         memcpy (rightArray, vector+left_size,
                 right_size*sizeof *rightArray);
         iVect[0] = right_size;
         iVect[1] = nxt;
         rc = MPI_Send( iVect, 2, MPI_INT, rtChild, INIT,
              MPI_COMM_WORLD);
         rc = MPI_Send( rightArray, right_size, MPI_CHAR, rtChild,
              DATA, MPI_COMM_WORLD);

         parallelMerge ( leftArray, left_size, nxt );
         rc = MPI_Recv( rightArray, right_size, MPI_CHAR, rtChild,
              ANSW, MPI_COMM_WORLD, &status );

         // Merge the two results back into vector
         i = j = k = 0;
         while ( i < left_size && j < right_size )
            if ( leftArray[i] > rightArray[j])
               vector[k++] = rightArray[j++];
            else
               vector[k++] = leftArray[i++];
         while ( i < left_size )
            vector[k++] = leftArray[i++];
         while ( j < right_size )
            vector[k++] = rightArray[j++];
      }
   }
   else
   {
      qsort( vector, size, sizeof *vector, compare );
   }



   if ( parent != myRank )
      rc = MPI_Send( vector, size, MPI_CHAR, parent, ANSW,
           MPI_COMM_WORLD );
}



long getSize(char *file_name){
  FILE *fp;
  char *mode = "r";
  fp = fopen(file_name, mode);
  fseek(fp, 0L, SEEK_END);
  return ftell(fp);
}

void getData(char *vector, long size, char* file_name){

  size_t n = 0;
  FILE *fp;
  char *mode = "r";
  fp = fopen(file_name, mode);
  char c;
  while ((c = fgetc(fp)) != EOF)
  {
      vector[n++] = (char) c;
  }
  qsort(vector, size, sizeof(char), compare);


}

void writeFile(char *vector, long size, char* file_name){

  FILE *file = fopen(file_name, "w");
  int results = fputs(vector, file);
  fclose(file);
}

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Status stat;

    MPI_Init(NULL, NULL);
    double t1, t2; 
    t1 = MPI_Wtime();

    // Get the number of processes
    int nProc;
    MPI_Comm_size(MPI_COMM_WORLD, &nProc);

    // Get the rank of the process
    int myRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    long size;
    char * vector;
    char *solo;
    double start, middle, finish;
    int rc;


    // printf("processor: %d of total: %d \n", world_rank, world_size);
    // Print off a hello world message
	   if ( myRank == 0 )        // Host process
	   {
	      int rootHt = 0, nodeCount = 1;

	      while ( nodeCount < nProc )
	      {  nodeCount += nodeCount; rootHt++;  }

	      printf ("%d processes mandates root height of %d\n", nProc, rootHt);
        char *input_file_name = argv[1];
        char *output_file_name = argv[2];
	      long size = getSize(input_file_name);
        printf("%d\n", size);
        vector = malloc((size+1) * sizeof(char));

        getData(vector, size, input_file_name);

	   	  // Capture time to sequentially sort the idential array
	      solo = (char*) calloc (size, sizeof *solo );
	      memcpy (solo, vector, size * sizeof *solo);

	      start = MPI_Wtime();
	      parallelMerge (vector, size, rootHt);
	      middle = MPI_Wtime();

        

	   }
	   else                      // Node process
	   {
	      int   iVect[2],        // Message sent as an array
	            height,          // Pulled from iVect
	            parent;          // Computed from myRank and height
	      MPI_Status status;     // required by MPI_Recv

	      rc = MPI_Recv( iVect, 2, MPI_INT, MPI_ANY_SOURCE, INIT,
	           MPI_COMM_WORLD, &status );
	      size   = iVect[0];     // Isolate size
	      height = iVect[1];     // and height

        
	      vector = (char*) calloc (size, sizeof *vector);
	      rc = MPI_Recv( vector, size, MPI_CHAR, MPI_ANY_SOURCE, DATA,
	           MPI_COMM_WORLD, &status );

	      parallelMerge (vector, size, height );
        printf("Size = %u, %d\n", sizeof * vector, size);
	      MPI_Finalize();

	      return 0;
	   }
	// Only the rank-0 process executes here.

     printf("HELLO1\n");
    // START OF CODE
    // END OF CODE
     printf("Vector = %s Size = %d\n", vector, sizeof * vector);

     int i;
      for(i = 0; i< strlen(vector); i++){
        printf("%c character\n", vector[i]);
      }
    
    MPI_Finalize();
    
}