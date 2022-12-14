
#include "engine.h"

#define width 20
#define height 20


int main(void) {

    // Set up the game world
    int playerPosY = 3;
    int playerPosX = 3;

    player user;
    user.health = 20;
    user.maxhealth = 20;
    user.level = 1;

    //Load Level
    FILE *fptr;
    fptr = fopen("2.lvl","r");
    printf("Loaded Level");
    if(fptr == NULL) {
      printf("Error! No level file in directory!");   
      exit(1);             
    }

    // Initialize the game world
    initLevel(fptr);
    world[playerPosY][playerPosX] = '@';
    
    initColor();

    drawMap(playerPosY, playerPosX);

    // Game loop
    while (1) {
        char ch;

        if (kbhit()) {
            world[playerPosY][playerPosX] = '.'; //restore last cell
            ch = getch();

            if(ch == 'w'){     
                if (playerPosY-1 >= 0 && playerPosY-1 < width && playerPosX >= 0 && playerPosX < height && collisionMap[playerPosY-1][playerPosX] != 1) {
                    playerPosY--;
                    world[playerPosY][playerPosX] = '@';
                    drawMap(playerPosY, playerPosX);
                    turn();
                }  
            }
                
            if (ch == 'a'){        
                if (playerPosY >= 0 && playerPosY < width && playerPosX-1 >= 0 && playerPosX-1 < height && collisionMap[playerPosY][playerPosX-1] != 1) {
                    playerPosX--;
                    world[playerPosY][playerPosX] = '@';
                    drawMap(playerPosY, playerPosX);
                }  
            }
                
            if(ch == 's'){
                if (playerPosY+1 >= 0 && playerPosY+1 < width && playerPosX >= 0 && playerPosX < height && collisionMap[playerPosY+1][playerPosX] != 1){
                    playerPosY++;
                    world[playerPosY][playerPosX] = '@';
                    drawMap(playerPosY, playerPosX);
                }  
            }
            
            if(ch == 'd'){          
                if (playerPosY >= 0 && playerPosY < width && playerPosX+1 >= 0 && playerPosX+1 < height && collisionMap[playerPosY][playerPosX+1] != 1){
                    playerPosX++;
                    world[playerPosY][playerPosX] = '@';
                    drawMap(playerPosY, playerPosX);
            }
                
            }               

        }


        // Check for end conditions and break out of the game loop if necessary
        if(user.health == 0)
            exit(1);
    }

    // Clean up and exit
    system("cleanup.bat");
    fclose(fptr);
    return 0;
}