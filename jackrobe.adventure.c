//
// Created by Robert on 4/20/2016.
//
#include <stdio.h>
#include <dirent.h>

#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <assert.h>
#define ARYSZ(x)  ( (sizeof(x) / sizeof((x)[0])) )

//*********************FUNCTIONS
FILE * open_file(char * dir, char * fileName, char * action);
int  get_room_type(char * file); //gets the type of room
int gen_files(); //generates the room files name only
int gen_connections(); // generates all random connections appends them to the file
int gen_room_type(); // gives each room a random type
char * find_start_room(); // locates which file to start at
void display_EOG(); // displays end-of-game message
void increase_path(char * pathname); // adds to the path
void cleanUp(); // removes two tmp files
void get_direction();
//*********************VARIABLES

const int  MINROOMX = 3; // minimum number of room connections
const int MAXCONNECTIONS = 6; // maximum number of connections per room
const int MAXROOMS = 7; // max number of rooms in play
char *rooms_str[]={ "Dumbeldor's Office", "Room_of_Requirement", "Great_Hall", "Potions", "Divination", "Hospital", "Herbology", "Owlry", "Gryffendor_Commons", "Slytherin_Commons" };
//char *roomsTypes_str[]={ "MID_ROOM", "END_ROOM", "START_ROOM"};
enum roomTypes{ MID_ROOM=-1, END_ROOM=1, START_ROOM=0 };
int inPlay[10]={0,1,2,3,4,5,6,7,8,9}; // array to track which rooms are in play
char ans[50];
char dirName[] = "./Jackrobe.Rooms.";
int GAMEOVER =0;
int pCount=0; // the path count

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
void get_direction(){

    memset(ans, 0, 50);
    printf("\nWHERE TO?> ");
    fgets(ans, 50, stdin);

    if(!strchr(ans, '\n'))
        while(fgetc(stdin)!='\n');//discard until newline

    ans[strcspn(ans, "\n")] = 0;
}


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

        if (strcmp(action, "a") ==0);

        error("Couldn't open file " );
        return NULL;
    }
    return fp;
}

//todo Ideally combine the file generation inside one function to minimize open/closes
//#################### GENERATE FILES FOR GAME
// Returns 0 on success
int gen_files(){

    int i;
    char buff[12];
    int nRooms = ARYSZ(rooms_str); // the number of rooms
    int * arryP = inPlay;
    FILE * fp;

    //get the pid into a char
    snprintf(buff, 12, "%d", getpid());

    //updating the dirName to have the PID, and making the directory
    // http://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
    struct stat st = {0};
    if (stat(strcat(dirName, buff), &st) == -1) {
        mkdir(dirName, 0700);
    }

    array_shuffle(arryP, nRooms); // reshuffle and use only the first 7 from here on out

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

    FILE *fp;

    //cycle over the files
    for( i= 0; i < MAXROOMS; i++) {

        int numCx = rand() % 4 + MINROOMX; // number of connections to be added to each room

        // no reason to add more connections than necessary
        if(cxCount[i] <= MINROOMX ) {

            while (cxCount[i] < numCx) {//make a random number of connections for the room

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

        fp = open_file(dirName, rooms_str[inPlay[i]], "a");// opening the file

        int count =1;
        for(j=0; j < MAXROOMS; j++) {
            if ((conxMatrix[i][j] == 1)) {
                fprintf(fp, "CONNECTION %i: %s\n", count, rooms_str[inPlay[j]]);
                count++;
            }
            //printf("%i ", conxMatrix[i][j]);
        }
        //printf("\n");
        fclose(fp);
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
    fclose(fp);

    fp = open_file(dirName, rooms_str[inPlay[room2]], "a"); // opening the file
        //error("can't open file for end room");

    fprintf(fp, "ROOM TYPE: END_ROOM\n");
    fclose(fp);

    // Add the default midroom to the remainder of rooms
    for(i=0; i < MAXROOMS; i++){
        if ((i != room1) && (i != room2)){
            fp = open_file(dirName, rooms_str[inPlay[i]], "a");
                //error("cant open files for room types mid_room"); // opening the file
            fprintf(fp, "ROOM TYPE: MID_ROOM\n");
            fclose(fp);
        }
    }
    return 0;
}
//#################### FINDS THE STARTING ROOM
// reads the file director and returns the name of the starting file.
char * find_start_room(){

    DIR *dirP =NULL;
    struct dirent * dirList = NULL; //pointer to a dirent struct

    // look at the directory where files are
    dirP = opendir(dirName);

    if(dirP == NULL)
        error("cannot read the directory, bruh");

    //loop though it, checking the room type. if found we have a start location
    while ((dirList = readdir(dirP)) != NULL){
        int rmtyp;
        rmtyp = get_room_type(dirList->d_name);
        if(rmtyp == 0){

            return dirList->d_name;
        }
    }
    return NULL;
}
//#################### READS A ROOM FILE
// @param  the filename to look up
// Returns 0 for good
// prints to stdout the location, and connections
// Sets GAMEOVER=1 if ROOM TYPE IS END_ROOM
int read_room(char *file){
    FILE * fp;
    FILE * tmp_x; // a file that will hold the list of possible connections

    fp = open_file(dirName, file, "r+");
    tmp_x = open_file(dirName, "rooms.tmp", "w"); // temp file for holding possible destinations

    char connections[250]={}; // holding possible connections
    char content[50]={};
    int tCount =0;  //total line count
    int count =0; // current line count

    //error checking
    if(fp == NULL)
        error("bad file in file reading");
    if(tmp_x == NULL)
        error("bad file in file reading");

    //check first if we have the endgame
    if (get_room_type(file) == 1){
        GAMEOVER=1;
        display_EOG();
        return 1;
    }


    //read the file to get a count of the lines
    while( fscanf(fp, "%*s %*s %49[^\n]",  content ) != EOF){
        tCount++;
    }
    rewind(fp); // reset the pointer in the file

    //reading the file (little shout out to Kristen Dhuse, for the help on words with spaces)
    while( fscanf(fp, "%*s %*s %49[^\n]\n",  content ) != EOF){
        //First line is the location
        if(count == 0){
           printf("CURRENT LOCATION:%s\n", content);

        //everything before last line is a connection
        }else if((count >= 1) && (count < tCount-1)){

            fprintf(tmp_x, "%s\n", content);

            strcat(connections, content);
            if (count < tCount-2){
                strcat(connections, ", ");
            }else{
                strcat(connections, ".");
            }
        }
        count++;
    }
    printf("POSSIBLE CONNECTIONS: %s", connections);

    fclose(fp);
    fclose(tmp_x);

    return 0;
}

//#################### GETS ROOM TYPE
// @param  the filename to look up
// Returns 2 for END, 1 for START, -1 FOR a MID_ROOM
int  get_room_type(char * file){

    FILE * fp;
    char roomTyp[9];
    fp = open_file(dirName, file, "r");
    fseek(fp, -8, SEEK_END);

    while( fscanf(fp, "%s",  roomTyp ) != EOF){

        if(strcmp(roomTyp, "RT_ROOM") == 0 ){
            return START_ROOM;
        }else if(strcmp(roomTyp, "ND_ROOM") == 0){
            return END_ROOM;
        }else{
            return MID_ROOM;
        }
    }
}

//#################### Checks destination against answer
// @param  the filename to look up
// Returns 0 for good 1 for bad
int  match_destinations(char * file, char * ans){

    FILE * fp;
    char dest[50];
    fp = open_file(dirName, file, "r");
    //read in each line if it matches return 0
    while( fscanf(fp, "%49[^\n]\n",  dest ) != EOF){

        if(strcmp(dest, ans) == 0 ) {
            increase_path(dest);
            return 0;
        }
    }
    printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
    return 1;
}

void increase_path(char * pathname){
    pCount++;
    FILE *fp;
    fp = open_file(dirName, "path.tmp", "a");

    fprintf(fp, "%s\n", pathname);

    fclose(fp);
}

void display_EOG(){
    FILE *fp;
    char path[50];
    fp = open_file(dirName, "path.tmp", "r");

    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n"
                   "YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n", pCount);

    //print out the path
//    fgets(path, 50, fp);
//    printf("%s\n", path);
    while( fscanf(fp, "%49[^\n]\n",  path ) != EOF){

        printf("%s\n", path);
    }

    fclose(fp);
    exit(0);
}

//void cleanUp(){
//
//    FILE *fp;
//    fp = open_file(dirName, "rooms.tmp", "w");
//    remove(fp)
//
//}

int main(int argc, char *argv[]) {

    srand(time(NULL));
    char *room; // pointer to a room name
    int readState =1; // readState is one keep on reading answers

    if(gen_files() < 0){
        error("check");
    };
    gen_connections();
    gen_room_type();
    room = find_start_room();

    while (!GAMEOVER){
        readState =1; // readState is one keep on reading answers
        read_room(room);

        while(readState == 1){

            get_direction();
            readState= match_destinations("rooms.tmp", ans);
        }

        room = ans; // change the room and back we go

    }
    //TODO clean TEMP FILES


exit(0);
}

