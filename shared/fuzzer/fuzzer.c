#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

struct tar_t
{                              /* byte offset */
    char name[100];               /*   0 */
    char mode[8];                 /* 100 */
    char uid[8];                  /* 108 */
    char gid[8];                  /* 116 */
    char size[12];                /* 124 */
    char mtime[12];               /* 136 */
    char chksum[8];               /* 148 */
    char typeflag;                /* 156 */
    char linkname[100];           /* 157 */
    char magic[6];                /* 257 */
    char version[2];              /* 263 */
    char uname[32];               /* 265 */
    char gname[32];               /* 297 */
    char devmajor[8];             /* 329 */
    char devminor[8];             /* 337 */
    char prefix[155];             /* 345 */
    char padding[12];             /* 500 */
};

/**
 * Computes the checksum for a tar header and encode it on the header
 * @param entry: The tar header
 * @return the value of the checksum
 */
unsigned int calculate_checksum(struct tar_t* entry){
    // use spaces for the checksum bytes while calculating the checksum
    memset(entry->chksum, ' ', 8);

    // sum of entire metadata
    unsigned int check = 0;
    unsigned char* raw = (unsigned char*) entry;
    for(int i = 0; i < 512; i++){
        check += raw[i];
    }

    snprintf(entry->chksum, sizeof(entry->chksum), "%06o0", check);

    entry->chksum[6] = '\0'; // ?
    entry->chksum[7] = ' '; // ?
    return check;
}

/**
* Generates a number between in [32, 126], so printable ASCII char
*/
int getnbr(){ return rand() % (95) + 32; }

/**
* Testing various modification of broken tar files to test if it crashes the tar extractor.
* @param test_index : The test (archive modification) to try
*/
void fuzzFunction(int test_index){
	srand(time(NULL));

// Your fuzzer must work with archives: you cannot just try different values for different fields in the
// header. You must deal with headers with and without data, with archives containing multiple files, etc.

    // Why those values ? Create a valide archive then modifying on part of it ?
	char potential_name[100] = "";
    char potential_mode[8] = "";
    char potential_uid[8] = "";
    char potential_gid[8] = "";
    char potential_size[12] = "";
    char potential_mtime[12] = "";
    char potential_chksum[8] = "";
    char potential_typeflag;
    char potential_linkname[100] = "";
    char potential_magic[6] = "ustar";
    char potential_version[2] = "00";
    char potential_uname[32] = "";
    char potential_gname[32] = "";

    // TODO : make several case for each test to try
    switch(test_index) {

        // =========== TEST 1 - BROKEN NAME (ascii) ===========
        case 0:

        // Why would a random **ASCII** name break something ???? just test 1 fixed name different from the file
            for (int x = 0; x < 99; x++){
                int ranvalue = getnbr();
                potential_name[x] = (char) ranvalue;
            }
            potential_name[99] = '\0';
            FILE *archive = fopen("archive.tar", "wb");

            struct tar_t broken_header;
            memset(&broken_header, 0, sizeof(broken_header));
            memcpy(broken_header.name, potential_name, sizeof(broken_header.name));
            memcpy(broken_header.magic, potential_magic, sizeof(broken_header.magic));
            memcpy(broken_header.version, potential_version, sizeof(broken_header.version));

            calculate_checksum(&broken_header);

            fwrite(&broken_header, sizeof(broken_header), 1, archive);
            fclose(archive);

        //=========== TEST 2 - BROKEN NAME (non ascii) ===========
        case 1:
            println("todo");


        //....


        default:
            println("Wrong test index");
    }

}

/**
 * Launches another executable given as argument,
 * parses its output and check whether or not it matches "*** The program has crashed ***".
 * @param the path to the executable
 * @return -1 if the executable cannot be launched,
 *          0 if it is launched but does not print "*** The program has crashed ***",
 *          1 if it is launched and prints "*** The program has crashed ***".
 *
 * BONUS (for fun, no additional marks) without modifying this code,
 * compile it and use the executable to restart our computer.
 */
int main(int argc, char* argv[])
{   

    int crash_count = 0;
    int rv = 0;
    int test_index = 0;
    for (int a = 0; a < 10; a++){
        fuzzFunction(test_index);
        if (argc < 2)
            return -1;

        // command to execute : "executable" "archive.tar" ("./bug0 archive.tar")
        char cmd[51];
        strncpy(cmd, argv[1], 25);
        cmd[26] = '\0';
        strncat(cmd, " archive.tar", 25);

        char buf[33];
        FILE *fp;
        // execute the command
        if ((fp = popen(cmd, "r")) == NULL) {
            printf("Error opening pipe!\n");
            return -1;
        }

        if(fgets(buf, 33, fp) == NULL) {
            printf("No output\n");
            goto finally;
        }

        // +-1 (non zero = True) if strings different
        if(strncmp(buf, "*** The program has crashed ***\n", 33)) {
            printf("Not the crash message\n");
            goto finally; 
        } else {
            printf("Crash message\n");
            
            // TODO : Crash detected -> save the archive with a name starting "success_"
            crash_count++;
            // rename archive

            rv = 1;
            goto finally; // why jump to finally ?
        }
        finally:
        if(pclose(fp) == -1) {
            printf("Command not found\n");
            rv = -1;
        }
    }
    return rv;
}


