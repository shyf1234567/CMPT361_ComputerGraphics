/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>

using namespace std;
// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400;
int ysize = 720;

//newtile
int r_type; //random number for current tile type
int r_colour; //random number for current block colour
int state; //current tile state
unsigned linecounter = 0; //counting the number of full lines
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec4 tilecolour[4]; //colour of each block of the current tile
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)
bool removetable[10][20]; //record tiles to remove after each time's check

bool pause = false;
// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
//L, mirror L, S, mirror S, T, I
vec2 allRotationsShapes[6][4][4] =
        {
                {        // L
                        {vec2(0, 0),  vec2(-1, 0), vec2(1, 0),  vec2(-1, -1)},
                        {vec2(0, 0),  vec2(0, -1), vec2(0, 1), vec2(1, -1)},
                        {vec2(0, 0),  vec2(1, 0),  vec2(-1, 0), vec2(1, 1)},
                        {vec2(0, 0),  vec2(0, 1),  vec2(0, -1), vec2(-1, 1)}

                },
                {        //mirror L
                        {vec2(0, 0),  vec2(0, 1),  vec2(0, -1), vec2(-1, -1)},
                        {vec2(0, 0),  vec2(-1, 0), vec2(1, 0), vec2(1, -1)},
                        {vec2(0, 0),  vec2(0, -1), vec2(0, 1),  vec2(1, 1)},
                        {vec2(0, 0),  vec2(1, 0),  vec2(-1, 0), vec2(-1, 1)}
                },

                {        //S
                        {vec2(0, 0),  vec2(-1, 0), vec2(0, -1), vec2(1, -1)},
                        {vec2(0, 0),  vec2(0, -1), vec2(1, 0), vec2(1, 1)},
                        {vec2(0, 0),  vec2(-1, 0), vec2(0, -1), vec2(1, -1)},
                        {vec2(0, 0),  vec2(0, -1), vec2(1, 0),  vec2(1, 1)}
                },
                {        //mirror S
                        {vec2(0, 0),  vec2(1, 0),  vec2(0, -1), vec2(-1, -1)},
                        {vec2(0, 0),  vec2(0, 1),  vec2(1, 0), vec2(1, -1)},
                        {vec2(0, 0),  vec2(1, 0),  vec2(0, -1), vec2(-1, -1)},
                        {vec2(0, 0),  vec2(0, 1),  vec2(1, 0),  vec2(1, -1)}
                },

                {        //T
                        {vec2(0, 0),  vec2(-1, 0), vec2(1, 0),  vec2(0, -1)},
                        {vec2(0, 0),  vec2(0, -1), vec2(0, 1), vec2(1, 0)},
                        {vec2(0, 0),  vec2(1, 0),  vec2(-1, 0), vec2(0, 1)},
                        {vec2(0, 0),  vec2(0, 1),  vec2(0, -1), vec2(-1, 0)},


                },

                {        //I
                        {vec2(-2, 0), vec2(-1, 0), vec2(0, 0),  vec2(1, 0)},
                        {vec2(0, -2), vec2(0, -1), vec2(0, 0), vec2(0, 1)},
                        {vec2(-2, 0), vec2(-1, 0), vec2(0, 0),  vec2(1, 0)},
                        {vec2(0, -2), vec2(0, -1), vec2(0, 0),  vec2(0, 1)},
                }
        };

// colors
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0);
vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black = vec4(0.0, 0.0, 0.0, 1.0);
vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);
vec4 purple = vec4(1.0, 0.0, 1.0, 1.0);

//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20];
//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

//-------------------------------------------------------------------------------------------------------------------
//When the board is to be updated, update the VBO containing the board's vertext color data
void updateBoard() {
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]); // Bind the VBO containing current board vertex colours
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours); // Put the colour data in the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------
// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile() {
//	cout << "in update tile" << endl;
    // Bind the VBO containing current tile vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);

    // For each of the 4 'cells' of the tile,
    for (int i = 0; i < 4; i++) {
//		cout << "i:" << i << endl;
//		cout << "x:" << tile[i].x << endl;
//		cout << "y:" << tile[i].y << endl;
        // Calculate the grid coordinates of the cell
        GLfloat x = tilepos.x + tile[i].x;
        GLfloat y = tilepos.y + tile[i].y;

        // Create the 4 corners of the square - these vertices are using location in pixels
        // These vertices are later converted by the vertex shader
        vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
        vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
        vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
        vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

        // Two points are used by two triangles each
        vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4};

        // Put new data in the VBO
        glBufferSubData(GL_ARRAY_BUFFER, i * 6 * sizeof(vec4), 6 * sizeof(vec4), newpoints);
    }

    glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------
// Called at the start of play and every time a tile is placed
void newtile() {
    tilepos = vec2(5, 19); // Put the tile at the top of the board
    state = 0;

    // Update the geometry VBO of current tile
    r_type = rand() % 6;
    for (int i = 0; i < 4; i++)
        tile[i] = allRotationsShapes[r_type][state][i]; // Get the 4 pieces of the new tile
    updatetile();

    // Update the color VBO of current tile
    vec4 newcolours[24];
    for (int i = 0; i < 4; i++) {
        r_colour = rand() % 5;
        switch (r_colour) {
            case 0:
                tilecolour[i] = orange;
                break;
            case 1:
                tilecolour[i] = green;
                break;
            case 2:
                tilecolour[i] = yellow;
                break;
            case 3:
                tilecolour[i] = red;
                break;
            case 4:
                tilecolour[i] = purple;
                break;
        }
        for (int j = 0; j < 6; j++) {
            newcolours[6 * i + j] = tilecolour[i];
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------
//initialize the grid, coloured white
void initGrid() {
    // ***Generate geometry data
    vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
    vec4 gridcolours[64]; // One colour per vertex
    // Vertical lines
    for (int i = 0; i < 11; i++) {
        gridpoints[2 * i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
        gridpoints[2 * i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);

    }
    // Horizontal lines
    for (int i = 0; i < 21; i++) {
        gridpoints[22 + 2 * i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
        gridpoints[22 + 2 * i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
    }
    // Make all grid lines white
    for (int i = 0; i < 64; i++)
        gridcolours[i] = white;


    // *** set up buffer objects
    // Set up first VAO (representing grid lines)
    glBindVertexArray(vaoIDs[0]); // Bind the first VAO
    glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

    // Grid vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
    glBufferData(GL_ARRAY_BUFFER, 64 * sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vPosition); // Enable the attribute

    // Grid vertex colours
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
    glBufferData(GL_ARRAY_BUFFER, 64 * sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vColor); // Enable the attribute
}

//---------------------------------------------------------------------------------
//initialize the board, all black initially
void initBoard() {
    // *** Generate the geometric data
    vec4 boardpoints[1200];
    for (int i = 0; i < 1200; i++)
        boardcolours[i] = black; // Let the empty cells on the board be black
    // Each cell is a square (2 triangles with 6 vertices)
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
            vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
            vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
            vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);

            // Two points are reused
            boardpoints[6 * (10 * i + j)] = p1;
            boardpoints[6 * (10 * i + j) + 1] = p2;
            boardpoints[6 * (10 * i + j) + 2] = p3;
            boardpoints[6 * (10 * i + j) + 3] = p2;
            boardpoints[6 * (10 * i + j) + 4] = p3;
            boardpoints[6 * (10 * i + j) + 5] = p4;
        }
    }

    // Initially no cell is occupied
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            board[i][j] = false;


    // *** set up buffer objects
    glBindVertexArray(vaoIDs[1]);
    glGenBuffers(2, &vboIDs[2]);

    // Grid cell vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
    glBufferData(GL_ARRAY_BUFFER, 1200 * sizeof(vec4), boardpoints, GL_STATIC_DRAW);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vPosition);

    // Grid cell vertex colours
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
    glBufferData(GL_ARRAY_BUFFER, 1200 * sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vColor);
}

//-----------------------------------------------------------------------------
// initialize the current tile
void initCurrentTile() {
    //bind the VAO
    glBindVertexArray(vaoIDs[2]);
    glGenBuffers(2, &vboIDs[4]);

    // Current tile vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vPosition);

    // Current tile vertex colours
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vColor);
}

//--------------------------------------------------------------------------------
void init() {
    // Load shaders and use the shader program
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    // Get the location of the attributes (for glVertexAttribPointer() calls)
    vPosition = glGetAttribLocation(program, "vPosition");
    vColor = glGetAttribLocation(program, "vColor");

    // Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
    glGenVertexArrays(3, &vaoIDs[0]);

    // Initialize the grid, the board, and the current tile
    initGrid();
    initBoard();
    initCurrentTile();

    // The location of the uniform variables in the shader program
    locxsize = glGetUniformLocation(program, "xsize");
    locysize = glGetUniformLocation(program, "ysize");

    // Game initialization
    newtile(); // create new next tile

    // set to default
    glBindVertexArray(0);
    glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------
// Rotates the current tile, if there is room
//direction +1 means rotating counterclockwise, -1 on the contrary
void rotate(int direction) {
    int i;
    GLfloat x, y;
    //enumerate four blocks of the current tile
    for (i = 0; i < 4; i++) {
        // calculate the position after the rotation
        x = tilepos.x + allRotationsShapes[r_type][(state + direction) % 4][i].x;
        y = tilepos.y + allRotationsShapes[r_type][(state + direction) % 4][i].y;
        //check whether it crosses the boundary or collides with other blocks
        if (board[int(x)][int(y)] == true ||
            x < 0 ||
            x >= 10 ||
            y < 0)
            break;
    }
    // if passed the check
    if (i == 4) {
        //update the state and position of current tile
        state = (state + direction) % 4;
        for (int j = 0; j < 4; j++) {
            tile[j].x = allRotationsShapes[r_type][state][j].x;
            tile[j].y = allRotationsShapes[r_type][state][j].y;
        }
    }
    updatetile();
}

//-------------------------------------------------------------------------------------------------------------------
// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, label all tiles in this row as true in removetable
void checkfullrow(int row) {
    int i;
    for (i = 0; i < 10; i++) if (!board[i][row]) break;
    //this row is full
    if (i == 10) {
        linecounter++;
        for (int j = 0; j < 10; j++) removetable[j][row] = true;
    }

}

//-------------------------------------------------------------------------------------------------------------------
//vec4 class has no '==' operator, adding this function to avoid modifying that class
bool comparecolour(vec4 &a, vec4 &b) {
    if (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w) return true;
    else return false;
}

//-------------------------------------------------------------------------------------------------------------------
// check whether the same on one diagonal, vertical or horizontal line there are more than three consecutive blocks of
// the same colour, if so, label the coordinate as true in removetable
bool checkcolour(int x, int y) {
    //black colour won't be considered
    if (comparecolour(boardcolours[6 * (10 * y + x)], black)) return false;

    //four different directions
    int d_up = 0, d_down = 0; //vertical
    int d_left = 0, d_right = 0; //horizontal
    int d_dia_lu = 0, d_dia_rd = 0; //left up corner to right bottom corner
    int d_dia_ld = 0, d_dia_ru = 0; //left bottom corner to right up corner
    //calculate the number of consecutive blocks of the same colour on each direction
    // up and down directions
    while (y + d_up + 1 <= 19 &&
           comparecolour(boardcolours[6 * (10 * (y + d_up + 1) + x)], boardcolours[6 * (10 * y + x)]))
        d_up++;
    while (y - d_down - 1 >= 0 &&
           comparecolour(boardcolours[6 * (10 * (y - d_down - 1) + x)], boardcolours[6 * (10 * y + x)]))
        d_down++;

    //left and right directions
    while (x + d_right + 1 <= 9 &&
           comparecolour(boardcolours[6 * (10 * y + x + d_right + 1)], boardcolours[6 * (10 * y + x)]))
        d_right++;
    while (x - d_left - 1 >= 0 &&
           comparecolour(boardcolours[6 * (10 * y + x - d_left - 1)], boardcolours[6 * (10 * y + x)]))
        d_left++;

    //left up to right down diagonal
    while (x - d_dia_lu - 1 >= 0 && y + d_dia_lu + 1 <= 19 &&
           comparecolour(boardcolours[6 * (10 * (y + d_dia_lu + 1) + (x - d_dia_lu - 1))],
                         boardcolours[6 * (10 * y + x)]))
        d_dia_lu++;
    while (x + d_dia_rd + 1 <= 9 && y - d_dia_rd - 1 >= 0 &&
           comparecolour(boardcolours[6 * (10 * (y - d_dia_rd - 1) + (x + d_dia_rd + 1))],
                         boardcolours[6 * (10 * y + x)]))
        d_dia_rd++;

    //left down to right up diagonal
    while (x - d_dia_ld - 1 >= 0 && y - d_dia_ld - 1 >= 0 &&
           comparecolour(boardcolours[6 * (10 * (y - d_dia_ld - 1) + (x - d_dia_ld - 1))],
                         boardcolours[6 * (10 * y + x)]))
        d_dia_ld++;
    while (x + d_dia_ru + 1 <= 9 && y + d_dia_ru + 1 <= 19 &&
           comparecolour(boardcolours[6 * (10 * (y + d_dia_ru + 1) + (x + d_dia_ru + 1))],
                         boardcolours[6 * (10 * y + x)]))
        d_dia_ru++;

    //return false if none of the four directions has the number greater than or equal to 3, including the central one at (x,y)
    if (d_up + d_down < 2 && d_left + d_right < 2 && d_dia_lu + d_dia_rd < 2 && d_dia_ld + d_dia_ru < 2) return false;
    //else, label as true in remove table, and return true
    else {
        removetable[x][y] = true;
        if (d_up + d_down >= 2) {
            for (int i = 1; i <= d_up; i++) removetable[x][y + i] = true;
            for (int i = 1; i <= d_down; i++) removetable[x][y - i] = true;
        }
        if (d_left + d_right >= 2) {
            for (int i = 1; i <= d_left; i++) removetable[x - i][y] = true;
            for (int i = 1; i <= d_right; i++) removetable[x + i][y] = true;
        }
        if (d_dia_lu + d_dia_rd >= 2) {
            for (int i = 1; i <= d_dia_lu; i++) removetable[x - i][y + i] = true;
            for (int i = 1; i <= d_dia_rd; i++) removetable[x + i][y - i] = true;
        }
        if (d_dia_ru + d_dia_ld >= 2) {
            for (int i = 1; i <= d_dia_ru; i++) removetable[x + i][y + i] = true;
            for (int i = 1; i <= d_dia_ld; i++) removetable[x - i][y - i] = true;
        }
    }
    return true;
}

//-------------------------------------------------------------------------------------------------------------------
// Starts the game over - empties the board, creates new tiles, resets line counters
void restart() {
    //make linecounter zero
    linecounter = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            board[i][j] = false;   //change all board blocks to false
        }
        for (int j = 0; j < 1200; j++) {
            boardcolours[j] = black; //all board blocks are black now
        }
        updateBoard();
    }
    //restart game by making a new tile
    newtile();

}

//-------------------------------------------------------------------------------------------------------------------
//change all entries in the removetable to false
void clearremovetable() {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            removetable[i][j] = false;
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
//move the board block from (from_x, from_y) to (to_x, to_y)
void moveboard(int from_x, int from_y, int to_x, int to_y) {
    // the destination coordinate must be in the board
    if (to_x <= 9 && to_x >= 0 && to_y <= 19 && to_y >= 0) {
        // if the from coordinate is in the board, move it as it should be
        if (from_x <= 9 && from_x >= 0 && from_y <= 19 && from_y >= 0) {
            for (int k = 0; k < 6; k++) {
                //move the colour and occupying information
                boardcolours[6 * (10 * (to_y) + to_x) + k] = boardcolours[6 * (10 * (from_y) + from_x) + k];
                board[to_x][to_y] = board[from_x][from_y];
            }
        }
        //if the from coordinates is outside the board, take it as a black, empty one
        else {
            for (int k = 0; k < 6; k++) {
                boardcolours[6 * (10 * to_y + to_x) + k] = black;
                board[to_x][to_y] = false;
            }
        }
    }

}
//------------------------------------------------------------------------------------------------------------------
//move the blocks on the board as it's labeled in removetable
void executeremovetable() {
    //update each column
    for (int i = 0; i < 10; i++) {
        int count = 0;
        for (int j = 0; j < 20; j++) {
            // count number of 'holes' in the column
            if (removetable[i][j] == true) count++;
            // if there are holes under this board, move it downwards for that count
            else if(count>0) {
                moveboard(i, j, i, j - count);
            }
        }
        //set the blocks at the top of this column to empty
        for (int j = count; j > 0; j--) {
            for (int k = 0; k < 6; k++) {
                boardcolours[6 * (10 * (20 - j) + i) + k] = black;
                board[i][20 - j] = false;
            }
        }
    }
    clearremovetable();

    //check whether there are new combinations satisfying our colour scheme
    int flag = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            if (checkcolour(i, j) == true) flag++;
        }
    }
    //if so, recursively clear them
    if (flag != 0) executeremovetable();
}

//-------------------------------------------------------------------------------------------------------------------
// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settle() {
    // assign the colour of current tile to the board
    for (int i = 0; i < 4; i++) {
        board[int(tilepos.x + tile[i].x)][int(tilepos.y + tile[i].y)] = true;
        for (int j = 0; j < 6; j++) {
            boardcolours[6 * (10 * int(tilepos.y + tile[i].y) + int(tilepos.x + tile[i].x)) + j] = tilecolour[i];
        }
    }
    updateBoard();
    clearremovetable();
    //check full row
    for (int i = 0; i < 4; i++) {
        checkfullrow(tilepos.y + tile[i].y);
    }
    //check colour
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 10; j++) {
            checkcolour(j, tilepos.y + tile[i].y);
        }
    }
    executeremovetable();
    clearremovetable();
    updateBoard();
    //check whether the tiles are stacked up to the top of the board
    int i;
    for (i = 0; i < 10; i++) {
        if (board[i][19]) break;
    }
    // continue
    if (i == 10) newtile();
    //lose and restart
    else restart();
}

//-------------------------------------------------------------------------------------------------------------------
// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction) {
    GLfloat x, y;
    //check whether touching the border or colliding with other blocks
    int i;
    for (i = 0; i < 4; i++) {
        x = tile[i].x + tilepos.x + direction.x;
        y = tile[i].y + tilepos.y + direction.y;
        if (board[int(x)][int(y)] == true || y < 0 || x < 0 || x >= 10) break;
    }
    //if not, move it on the specified direction for one unit
    if (i == 4) {
        tilepos.x += direction.x;
        tilepos.y += direction.y;
        updatetile();
        return true;
    }
    //settle the block
    else if (direction.x == 0 && direction.y == -1) {
        settle();
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display() {
    if(pause) return;

    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(locxsize,
                xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
    glUniform1i(locysize, ysize);

    glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
    glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

    glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
    glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles)

    glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
    glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)


    glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h) {
    if(pause) return;
    xsize = w;
    ysize = h;
    glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y) {
    if(pause) return;
    switch (key) {
        case GLUT_KEY_LEFT:
            movetile(vec2(-1, 0));
            break;
        case GLUT_KEY_RIGHT:
            movetile(vec2(1, 0));
            break;
        case GLUT_KEY_DOWN:
            movetile(vec2(0, -1));
            break;
        case GLUT_KEY_UP:
            rotate(1);
            break;
    }
}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 033: // Both escape key and 'q' cause the game to exit
            exit(EXIT_SUCCESS);
            break;
        case 'q':
            exit(EXIT_SUCCESS);
            break;
        case 'r': // 'r' key restarts the game
            restart();
            break;
        case 'p':
            pause=!pause;
            break;
    }
    glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

void idle(void) {
    if(pause) return;
    glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

void timer(int value) {
    glutTimerFunc(300, timer, 10);
    if(pause) return;
    movetile(vec2(0, -1));
    updatetile();

}

//-------------------------------------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(xsize, ysize);
    glutInitWindowPosition(680, 200); // Center the game window (well, on a 1920x1080 display)
    glutCreateWindow("Fruit Tetris");
    glewInit();
    init();

    // Callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    glutTimerFunc(300, timer, 10);

    glutMainLoop(); // Start main loop
    return 0;
}
