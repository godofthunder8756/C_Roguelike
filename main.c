
#include "engine.h"

#define width 20
#define height 20


int main(void) {

    // Set up the game world
    int playerX = 3;
    int playerY = 3;

    system("cls");

    //Load Level
    FILE *fptr;
    fptr = fopen("1.lvl","r");
    printf("Loaded Level");
    if(fptr == NULL) {
      printf("Error! No level file in directory!");   
      exit(1);             
    }

    // Initialize the game world
    char c;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            c = fgetc(fptr);
            if(c == 'w')
                world[x][y] = 'w';
            if(c == '0')
                world[x][y] = '.';
        }
    }
    world[playerX][playerY] = '@';
    //Sets up Color
    system("setup.bat");

    drawMap(playerX, playerY);

    // Game loop
    while (1) {
        char ch;

        if (kbhit()) {
            world[playerX][playerY] = '.';
            ch = getch();

            if(ch == 'w'){     
                if (playerX-1 >= 0 && playerX-1 < width && playerY >= 0 && playerY < height && world[playerX-1][playerY] != 'w') {
                    playerX--;
                    world[playerX][playerY] = '@';
                    drawMap(playerX, playerY);
                    turn();
                }  
            }
                
            if (ch == 'a'){        
                if (playerX >= 0 && playerX < width && playerY-1 >= 0 && playerY-1 < height && world[playerX][playerY-1] != 'w') {
                    playerY--;
                    world[playerX][playerY] = '@';
                    drawMap(playerX, playerY);
                }  
            }
                
            if(ch == 's'){
                if (playerX+1 >= 0 && playerX+1 < width && playerY >= 0 && playerY < height && world[playerX+1][playerY] != 'w'){
                    playerX++;
                    world[playerX][playerY] = '@';
                    drawMap(playerX, playerY);
                }  
            }
            
            if(ch == 'd'){          
                if (playerX >= 0 && playerX < width && playerY+1 >= 0 && playerY+1 < height && world[playerX][playerY+1] != 'w'){
                    playerY++;
                    world[playerX][playerY] = '@';
                    drawMap(playerX, playerY);
            }
                
            }               

        }


        // Check for end conditions and break out of the game loop if necessary
    }

    // Clean up and exit
    system("cleanup.bat");
    return 0;
}