#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() { 
   int i;
   unsigned char byte, dummy;
   FILE *fp;

   if ((fp = fopen("/dev/parlelport", "w")) == NULL) {
      printf("The device /dev/parlelport does not exist\n");
      exit(0);
   }

   /* Specifies a buffer for stream fp.  The function allows one to 
      specify the mode and size of the buffer (in bytes).  
        int setvbuf ( FILE * stream, char * buffer, int mode, size_t size );
      where _IONBF means no buffering (others: _IOLBF - line buffering
      _IOFBF - full buffering

      Used here only to remove a buffer from the I/O
   */
   setvbuf(fp, &dummy, _IONBF, 1);

   /* Initialize the byte to send to the port */
   byte = 97;

   for (i =0 ; i < 1000 ; i++) { 
      /* Writing to the parallel port */
      printf("Byte value is %d\n",byte);
      fwrite(&byte, 1, 1, fp);
      sleep(1);

      /* Increment the byte value */
      byte++;
      if (byte == 127) byte = 97;
   }

   fclose(fp);
}
