/*
    Name: Adam Sheeres-Paulicpulle
    ID: 1036569
    Course: CIS 3090, F21
    Project: A3

    Purpose: Generate a maze. This can be done parallel or not. 

    How to run: 
        compile: make

        run serial version: ./maze -n x -s y
            Where x is size and y is seed (both optional)

        run parallel version: ./mazep 4 -n x -s y
            Where x is size and y is seed (both optional)

        Easy way to run: 
        make run (serial version)
        make runp (parallel version)
*/

#include "stack.h"
#include <string.h>
#include <time.h>
#include <omp.h>
#include <stdbool.h>


//some default definitions
int size = 11;
int thread_count = 4;
unsigned int seed = 0;

/* 
    Some macros used. 
    loop: I find for loops long to write out and often times i is used as the iterator
    index: used to index the cells. Macro so I don't have to type out the size thing each time
*/
#define loop(x) for (int i = 0 ; i < x ; i++)
#define index(x, y) ((size * x) + y)

//enum used to differentiate position when finding neighbours
enum position {up = 0, right = 1, down = 2, left = 3};

typedef struct maze {
    int nRows;
    int nCols;
    char *cells;
} maze;

typedef struct neighbours {
    struct coords coord[4];
} neighbours;

//Functions
struct coords *newCoords(int , int);
void deleteMaze(struct maze*);
void showMaze(struct maze*);
void encodeLocation(struct stack *, struct maze *, int );
bool isWall(struct maze *, struct coords *);
struct neighbours *getNeighbours(struct maze *, struct coords *);
void cleanMaze(struct maze*);
void doMaze(struct maze*);
struct maze *initMaze();

int main(int argc, char *argv[]) {
    struct maze *newMaze;

    //loop over list of imputs
    loop(argc) {

        if (strcmp(argv[i], "-n") == 0 && i < argc) {
            size = atoi(argv[i+1]); //get the size

        } else if (strcmp(argv[i], "-s") == 0 && i < argc) {
            seed = atoi(argv[i+1]); //get the seed, if provided
        }
    }

    //initialize the maze
    newMaze = initMaze(size, size);
    
    //start parallel if required
    #pragma omp parallel num_threads (thread_count) 
    doMaze(newMaze);


    //housekeeping
    showMaze(newMaze);
    deleteMaze(newMaze);
    return 0;
}

/*
    Function: newCoords
    -------------------------------- 
    Purpose: creates a new coordinate object from primitive data types.

    x: x value of the coordinate
    y: y value of the coordinate

    returns: the new coordinate object
    
*/
struct coords *newCoords(int x, int y) {
    struct coords *new = malloc(sizeof(struct coords));
    new->x = x;
    new->y = y;
    return new;
}

/*
    Function: deleteMaze
    -------------------------------- 
    Purpose: housekeeping - cleans up the maze and makes sure everything is freed. 

    toDelete: the maze to be deleted
    
*/
void deleteMaze(struct maze *toDelete) {
    free(toDelete->cells);
    free(toDelete);
}

/*
    Function: showMaze
    -------------------------------- 
    Purpose: Displays the maze on command line

    toShow: the maze that will be displayed
    
*/
void showMaze(struct maze *toShow) {
    loop(toShow->nRows) {
        for (int j = 0 ; j < toShow->nCols ; j++) {
            printf("%c ", toShow->cells[index(i, j)]);
        }
        printf("\n");
    }
}

/*
    Function: encode location
    -------------------------------- 
    Purpose: Allows encoding a locatino on the maze without taking from the stack. This is important mostly during the begining when
    setting up the maze as you do not want to take the starting locations off of the stack. 

    root: the stack being used
    maze: the maze being used
    encodeNum: the id that we are using to encode (ex for thread 0, encodedNum = 0)
    
*/
void encodeLocation(struct stack *root, struct maze *maze, int encodeNum) {
    struct coords *temp = peek(&root);
    maze->cells[index(temp->x, temp->y)] = encodeNum;
}

/*
    Function: isWall
    -------------------------------- 
    Purpose: Checks to see if a certain cell is a wall or not (walls are denoted using a '.').

    maz: the maze to be checked
    check: the coordinate the function needs to check

    returns: true or false, if or if not a wall
    
*/
bool isWall(struct maze *maz, struct coords *check) {
    if (maz->cells[index(check->x, check->y)] == '.') {
        return true;
    }
    return false;
}

/*
    Function: getNeighbours
    -------------------------------- 
    Purpose: returns a list of four neighbours to the *start* coords. If the cell is occupied or out of bounds, it sets that neighbour's
    x value to be -1, which indicates it is unavailable. 

    maze: This function needs to check if a cell is available or if it is occupied by another number. This enables that to happen.
    start: The starting coordinate that the neighbours will be checked around. 2 up, 2 right, 2 down, 2 left.

    Returns: A structure containing four coordinates; each coordinate is a neighbour
    
*/
struct neighbours *getNeighbours(struct maze *maze, struct coords *start) {
    struct neighbours *retNeighbours = malloc(sizeof(struct neighbours));
    enum position curPos = up;
    struct coords *toCheck = malloc(sizeof(struct coords));

    //up   ---------------------------------------------
    curPos = up;
    if (start->y - 2 >= 0) { //if it is a valid coordinate
        //add this coord to the list
        retNeighbours->coord[curPos].x = start->x;
        retNeighbours->coord[curPos].y = start->y - 2;
    } else {
        retNeighbours->coord[curPos].x = -1;
    }
    
    //right---------------------------------------------
    curPos = right;
    if (start->x + 2 < size) {
        //add this coord to the list

        retNeighbours->coord[curPos].x = start->x + 2;
        retNeighbours->coord[curPos].y = start->y;
    } else {
        retNeighbours->coord[curPos].x = -1;
    }

    //down ---------------------------------------------
    curPos = down;

    if (start->y + 2 < size) {
        //add this coord to the list
        
        retNeighbours->coord[curPos].x = start->x;
        retNeighbours->coord[curPos].y = start->y + 2;
    } else {
        retNeighbours->coord[curPos].x = -1;
    }

    //left ---------------------------------------------
    curPos = left;
    if (start->x - 2 >= 0) {
        //add this coord to the list
        
        retNeighbours->coord[curPos].x = start->x - 2;
        retNeighbours->coord[curPos].y = start->y;
    } else {
        retNeighbours->coord[curPos].x = -1;
    }
    free(toCheck);
    return retNeighbours;
}

/*
    function: cleanMaze
    -------------------------------- 
    Purpose: Trimms the edges of the maze, if the input size is even
    
    maze: the maze to be trimmed

*/

   void cleanMaze(struct maze *maze) {
       loop(maze->nRows) {
           for (int j = 0 ; j < maze->nCols ; j ++) {

               if (i == 0 || i == size-1) {
                    maze->cells[index(i, j)] = '.';
               } else if (j == 0 || j == size-1) {
                    maze->cells[index(i, j)] = '.';
               }

           }
       }

   }

/*
    Function: doMaze
    -------------------------------- 
    Purpose: Completes the maze and displays the output. Does this by checking the neighb ours from a node
    that is on the stack. It randomly goes around to each neighbour and sets the cell as active. Contains
    certain sections where when parallel will not run in parallel. This is to make sure a certain thread does 
    not start writing to a cell when it should not be. 

    maze : the maze that is initialized
    
*/

void doMaze(struct maze *maze) {

    srand(seed);
    int startX, startY = 1;
    struct coords *new = NULL;
    struct stack *root = NULL;
    struct coords *curLocation = NULL;
    struct neighbours *neighbour;
    int threadNum = 0;

    //the starting locations for threads
    struct coords startingLocations[4];
    startingLocations[0].x = 1;
    startingLocations[0].y = 1;

    startingLocations[1].x = 1;
    startingLocations[1].y = size-2;

    startingLocations[2].x = size-2;
    startingLocations[2].y = 1;

    startingLocations[3].x = size-2;
    startingLocations[3].y = size-2;

    int threadID = 48; //'0'
    enum position curPos = up;

    //get new random coords


    //if the desired size is even, make sure the starting coordinates line up
    if (size%2 == 0) {
        startingLocations[1].x = 1;
        startingLocations[1].y = size-3;

        startingLocations[2].x = size-3;
        startingLocations[2].y = 1;

        startingLocations[3].x = size-3;
        startingLocations[3].y = size-3;
    }


    //If threading, setup thread starting locations
    #ifdef _OPENMP
        threadNum = omp_get_thread_num();
        threadID += threadNum;
        startX = startingLocations[threadNum].x;
        startY = startingLocations[threadNum].y;
    #else
        startX = startingLocations[0].x;
        startY = startingLocations[0].y;
    #endif

    //push to stack and encode location on maze
    new = newCoords(startX, startY);
    push(&root, new);
    encodeLocation(root, maze, threadID);

    //wait for all threads to gather
    #pragma omp barrier

    //start loop -------------------------------------------------
    while(isEmpty(root) == false) {
        curLocation = pop(&root);

        if (curLocation == NULL) break;

    //critical as this needs to evaluate its neighbours, which can still be in the act of being written to
        #pragma omp critical 
        {
            neighbour = getNeighbours(maze, curLocation);
        }

        free(curLocation);
        

    //loop 4 for each neighbours
        loop(4) {
            if (neighbour->coord[curPos].x != -1) { //if the place is in bounds
                if (maze->cells[ index(neighbour->coord[curPos].x, neighbour->coord[curPos].y) ] == '.') {//if it is empty

                    //encode the neighbour (2 away)
                    maze->cells[ index(neighbour->coord[curPos].x, neighbour->coord[curPos].y) ] = threadID;


                    //encode the neighbour (1 away)
                    switch(curPos) {
                        case 0 : 
                            maze->cells[ index(neighbour->coord[curPos].x, neighbour->coord[curPos].y + 1) ] = threadID; //up
                        break;

                        case 1 : 
                            maze->cells[ index(neighbour->coord[curPos].x, neighbour->coord[curPos].y - size) ] = threadID; //right
                        break;
                            
                        case 2 : 
                            maze->cells[ index(neighbour->coord[curPos].x, neighbour->coord[curPos].y - 1) ] = threadID; //down
                        break;

                        case 3 : 
                            maze->cells[ index(neighbour->coord[curPos].x, neighbour->coord[curPos].y + size) ] = threadID; //left
                        break;
                    }

                    //create next thing for the stack
                    new->x = neighbour->coord[curPos].x;
                    new->y = neighbour->coord[curPos].y;
                    push(&root, new);
                }
                
            }
            //iterate over the current position
            if (curPos != left) {
                curPos++;
            } else {
                curPos = 0;
            }

            
        }
        
        free(neighbour);
        curPos = rand()%4; //reset current position
    }
    free(new);
    if (size%2 == 0) {
        cleanMaze(maze);
    }
}

/*
    Function: initMaze
    -------------------------------- 
    Purpose: Initializes the maze. Allocates space for the maze and the table of cells that will be manipulated

    x: the size of the columns
    y: the size of the rows
    
*/
struct maze *initMaze(int x, int y) {
    struct maze *newMaze = malloc(sizeof(struct maze));
    newMaze->nRows = x;
    newMaze->nCols = y;
    newMaze->cells = malloc(sizeof(char) * x * y);

    loop(x*y) {
        newMaze->cells[i] = '.';
    }
    return newMaze;
}