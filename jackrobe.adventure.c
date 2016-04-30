//
// Created by Robert on 4/20/2016.
//
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <time.h>
#include <sys/stat.h>

#define ARYSZ(x)  ( (sizeof(x) / sizeof((x)[0])) )
char NAME[10];
const int  MINROOMX = 3; // minimum number of room connections
char *rooms_str[]={ "Dumbeldor's Office", "Raven Claw Commons", "Hufflepuff Commons","Slytherin Commons","Gryffendor Commons", "Room of Requirement", "Great Hall", "Potions", "Divination", "Hospital" };
const char *rooms_indi[]={"ROOM NAME:", "ROOM TYPE:", "CONNECTION:"};
char dirName[] = "./Jackrobe.Rooms.";

//error function
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int get_user(){

    char ans[2];
    while(1){
        printf("Enter user name (10 chars max): ");

        fgets(NAME, 10, stdin);
        printf("Hi %s", NAME);


        if(!strchr(NAME, '\n'))
            while(fgetc(stdin)!='\n');//discard until newline

        printf("\nIs this correct? enter y or n: ");
        fgets(ans, 2, stdin);

        if(!strchr(ans, '\n'))
            while(fgetc(stdin)!='\n');//discard until newline

        if((strcmp(ans, "y") == 0 )|| (strcmp(ans, "Y") == 0)){
            return 0;
        }else{
            printf("Try Again - ");
        }
    }
}
//int add_to_file(char * txt, FILE * fp){
//
//    if(fopen(fp, "a") < 0)
//        error("Couldn't open file");
//
//    if(fprintf(fp, "\n%s", txt) < 0)
//        error("Couldn't write to file");
//
//    close(fp);
//    return 0;
//}




FILE * open_file(char * dir, char * fileName, char * action){
    char file[50];
    //dir name and name of file
    snprintf(file, 50, "%s/%s.txt", dir, fileName);

    //open the newfile for writing
    // with the ROOM: + Room Name
    FILE *fp;

    if((fp = fopen(file , action )) < 0)
        error("Couldn't create File");;

    return fp;
}

int gen_files(){

    int i;
    char buff[12];

    //get the pid into a char
    snprintf(buff, 12, "%d", getpid());

    //make the dir to hold files pid
    // http://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
    struct stat st = {0};
    if (stat(strcat(dirName, buff), &st) == -1) {
        mkdir(dirName, 0700);
    }

    int nRooms = ARYSZ(rooms_str); // the number of rooms in my array
    for( i= 0; i < nRooms; i++){

        char fLine[50];
        FILE * fp = open_file(dirName, rooms_str[i], "w");

        //make the first line
        //write the name of a room to it
        if(fprintf(fp, "ROOM NAME: %s\n", rooms_str[i]) < 0)
            error("Couldn't write to file");

        fclose(fp);
    }
    return(0);
}

int gen_connections(){
    int i, j;
    int nRooms = ARYSZ(rooms_str); // the number of rooms in my array
    int conxMatrix[nRooms][nRooms]; // a matrix of connected rooms
    memset(conxMatrix, 0, sizeof(conxMatrix));
    int cxCount[nRooms]; // number of connections in a room
    memset(cxCount, 0, sizeof(cxCount));

    FILE *curFile;
    FILE * ConnectFile;

    //cycle though the files
    for( i= 0; i < nRooms; i++) {
        // set the number of connection to be made
        srand(time(NULL));
        int numCx = rand() % 3 + MINROOMX; // number of connections to be added to each room

        if(cxCount[i] < 4) { // no reason to add more connections than necessary

            //make a random number of connections for the room
            //todo There is no reason for it to try an fill the whole matrix we only need half but...
            while (cxCount[i] < numCx) {

                srand(time(NULL) + i + j);
                int conx = rand() % nRooms;

                if (conx != i) { // no connecting same room
                    if ((conxMatrix[i][conx] == 0) && (conxMatrix[conx][i] == 0)) {
                        conxMatrix[i][conx] = 1;
                        conxMatrix[conx][i] = 1;
                        cxCount[i]++;
                        cxCount[conx]++;
                    }
                }
            }
        }
    }

    for(i=0; i < nRooms; i++){
        ConnectFile = open_file(dirName, rooms_str[i], "a+"); // opening the file

        int count =1;
        for(j=0; j < nRooms; j++) {
            if (conxMatrix[i][j] == 1) {
                fprintf(ConnectFile, "CONNECTION %i: %s\n", count, rooms_str[j]);
                count++;
            }
            printf("%i ", conxMatrix[i][j]);
        }
        printf("\n");
        close(ConnectFile);
    }

    return 0;
}


int main(int argc, char *argv[]) {

    //get_user();
    if(gen_files()){
        error("check");
    };
    gen_connections();

//    int nRooms = ARYSZ(rooms_str);
//    int i;
//    for(i=0; i < 100; i++){
//        srand(time(NULL) + i );
//        int numCx = rand() % 3 + MINROOMX;
//        srand(time(NULL) + i );
//        int conx = rand() % nRooms;
//        printf("NUM CX\tROOM#\n%i\t%i\n", numCx,conx );
//    }
}

