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
#include <assert.h>

# ifndef TYPE
# define TYPE      void*
# endif
typedef struct Room Room;

#define ARYSZ(x)  ( (sizeof(x) / sizeof((x)[0])) )

char NAME[10];
const int  MINROOMX = 3; // minimum number of room connections
const int MAXCONNECTIONS = 6; // maximum number of connections per room
const int MAXROOMS = 7; // max number of rooms in play
char *rooms_str[]={ "Dumbeldor's_Office", "Room_of_Requirement", "Great_Hall", "Potions", "Divination", "Hospital", "Herbology", "Owlry", "Gryffendor_Commons", "Slytherin_Commons" };
char *roomsTypes_str[]={ "MID_ROOM", "END_ROOM", "START_ROOM"};

int inPlay[10]={0,1,2,3,4,5,6,7,8,9}; // arry to track which rooms are in play

char dirName[] = "./Jackrobe.Rooms.";
int GAMEOVER =0;



//error function
void error(char *msg)
{
    perror(msg);
    exit(1);
}

//#################### Shuffles an Array
// @param  a pointer to an array
// @param number of elements in the array
void array_shuffle(int *a, int size){
    if (size > 1)
    {
        int i;
        for (i = 0; i < rand()% 100 ; i++) {
            int sp = rand() % 10; // start point for shuffle
            int sw = rand() % 10; // swap point for shuffle
            int t = a[sp]; // hold one
            a[sp] = a[sw];
            a[sw] = t;
        }
    }
}

//
//int get_user(){
//
//    char ans[2];
//    while(1){
//        printf("Enter user name (10 chars max): ");
//
//        fgets(NAME, 10, stdin);
//        printf("Hi %s", NAME);
//
//
//        if(!strchr(NAME, '\n'))
//            while(fgetc(stdin)!='\n');//discard until newline
//
//        printf("\nIs this correct? enter y or n: ");
//        fgets(ans, 2, stdin);
//
//        if(!strchr(ans, '\n'))
//            while(fgetc(stdin)!='\n');//discard until newline
//
//        if((strcmp(ans, "y") == 0 )|| (strcmp(ans, "Y") == 0)){
//            return 0;
//        }else{
//            printf("Try Again - ");
//        }
//    }
//}
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

//#################### OPENS FILES
// @param  the directory
// @param the file name
// @param the action <a, w, r, etc...>
// Returns 0 on success
FILE * open_file(char * dir, char * fileName, char * action){
    char file[50];
    //dir name and name of file
    snprintf(file, 50, "%s/%s", dir, fileName);

    //open the newfile for writing
    // with the ROOM: + Room Name
    FILE *fp;

    if((fp = fopen(file , action )) < 0) {
        error("Couldn't open file " );;
    }else{
        return fp;
    }
}

//todo Ideally combine the file generation inside one function to minimize open/closes
//#################### GENERATE FILES FOR GAME
// Returns 0 on success
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

    int nRooms = ARYSZ(rooms_str); // the number of rooms
    int * arryP = inPlay;
    array_shuffle(arryP, nRooms); // reshuffle and use only the first 7 from here on out
    FILE * fp;

    for( i= 0; i < MAXROOMS; i++){

        char fLine[50];

        fp = open_file(dirName, rooms_str[inPlay[i]], "w");

        //make the first line
        //write the name of a room to it
        if(fprintf(fp, "ROOM NAME: %s\n", rooms_str[inPlay[i]]) < 0)
            error("Couldn't write to file");

        fclose(fp);
    }
    return(0);
}
//#################### GENERATE ROOM'S CONNECTIONS
//creates a matrix then writes that matrix
// Returns 0 on success
int gen_connections(){

    int i, j;
    int conxMatrix[MAXROOMS][MAXROOMS]; // a matrix of connected rooms
    memset(conxMatrix, 0, sizeof(conxMatrix));

    int cxCount[MAXROOMS]; // number of connections in a room
    memset(cxCount, 0, sizeof(cxCount));

    FILE * ConnectFile;

    //cycle over the files
    for( i= 0; i < MAXROOMS; i++) {

        int numCx = rand() % 4 + MINROOMX; // number of connections to be added to each room

        // no reason to add more connections than necessary
        if(cxCount[i] <= MINROOMX ) {

            while (cxCount[i] < numCx) {//make a random number of connections for the room
//
                int conx = rand() % MAXROOMS;
                //dont connect if the connecting room as too many conection or the room is the same
                if (cxCount[conx] < MAXCONNECTIONS && conx != i){

                    if ((conxMatrix[i][conx] == 0) && (conxMatrix[conx][i] == 0) ) { //only change if it hasn't already been
                        conxMatrix[i][conx] = 1;
                        conxMatrix[conx][i] = 1;
                        //increase the count of connections for both
                        cxCount[i]++;
                        cxCount[conx]++;
                    }
                }
            }
        }
    }
    // Write info to the files
    for(i=0; i < MAXROOMS; i++){

        ConnectFile = open_file(dirName, rooms_str[inPlay[i]], "a");// opening the file
        fseek(ConnectFile, 0, SEEK_END);
        int count =1;
        for(j=0; j < MAXROOMS; j++) {
            if ((conxMatrix[i][j] == 1)) {
                fprintf(ConnectFile, "CONNECTION %i: %s\n", count, rooms_str[inPlay[j]]);
                count++;
            }
            //printf("%i ", conxMatrix[i][j]);
        }
        //printf("\n");
        close(ConnectFile);
    }
    return 0;
}

//#################### GENERATE ROOM TYPES FOR EXISTING ROOMS
// Returns 0 on success
int gen_room_type(){
    /*todo make this happen in the gen files, only it requires the ability to read a file if done that way */

    int i, room1, room2;
    FILE * fp;
    room1 = rand() % MAXROOMS;
    room2 = rand() % MAXROOMS;
    //make sure we get two differnt ones
    while(room1 == room2){
        room2 = rand() % MAXROOMS;
    }

    fp = open_file(dirName, rooms_str[inPlay[room1]], "a"); // opening the file
        //error("can't open file for start room");

    fprintf(fp, "ROOM TYPE: START_ROOM\n");
    close(fp);

    fp = open_file(dirName, rooms_str[inPlay[room2]], "a"); // opening the file
        //error("can't open file for end room");

    fprintf(fp, "ROOM TYPE: END_ROOM\n");
    close(fp);

    // Add the defualt midroom to the remainder
    for(i=0; i < MAXROOMS; i++){
        if ((i != room1) && (i != room2)){
            fp = open_file(dirName, rooms_str[inPlay[i]], "a");
                //error("cant open files for room types mid_room"); // opening the file
            fprintf(fp, "ROOM TYPE: MID_ROOM\n");
            close(fp);
        }
    }
    return 0;
}

int find_start_room(){


}


int read_file(){
    FILE * fp;
    fp = open_file("D://CS//cs344//Proj2", "test", "r+");

    char * action[50];
    char * connections[50]={};
    char * content[50];
    char * ROOMNAME;
    int tCount =0;
    int count =0;

    while( fscanf(fp, "%*s %*s %s",  content ) != EOF){
        tCount++;
    }
    rewind(fp);

    while( fscanf(fp, "%*s %*s %s\n",  content ) != EOF){

        if(count == 0){
           printf("CURRENT LOCATION:%s\n", content);


        }else if((count >= 1) && (count < tCount-1)){

            strcat(connections, content);
            strcat(connections, ", ");

        }else{

            if ( strcmp(content, "END_ROOM") ==  0){

                GAMEOVER=1;
                printf("Game over!");
            }

        }

        count++;
    }

    printf("POSSIBLE CONNECTIONS: %s", connections);

    close(fp);

}

int main(int argc, char *argv[]) {

    srand(time(NULL));
//    //get_user();
    if(gen_files()){
        error("check");
    };
    gen_connections();
    gen_room_type();

//    int nRooms = ARYSZ(rooms_str);
//    printf("arraySize %i", nRooms);
//    int i;
//    for(i=0; i < 100; i++){
//        srand(time(NULL) + i );
//        int numCx = rand() % 4 + MINROOMX;
//        srand(time(NULL) + i );
//        int conx = rand() % nRooms;
//        printf("NUM CX\tROOM#\n%i\t\t%i\n", numCx,conx );
//    }

//int array[10]={0,1,2,3,4,5,6,7,8,9};
//    int i;
//    int * p = inPlay;
//    array_shuffle(p, 10);
//    for(i=0; i < 10; i ++){
//        printf("%i ", inPlay[i]);
//    }

exit(0);
}

