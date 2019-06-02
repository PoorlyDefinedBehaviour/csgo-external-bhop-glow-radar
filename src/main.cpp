#include "gamemanager/gamemanager.h"

int main()
{
  GameManager gameManager;
  gameManager.openGameProcess("Counter-Strike: Global Offensive");

  while (true)
  {
    gameManager.enableGlow()
        .enableBhop()
        .enableRadar()
        .sleep(1);
  }
}