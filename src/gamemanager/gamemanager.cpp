#include "gamemanager.h"
#include "../memory/memory.h"
#include "../offsets/offsets.h"
#include "../print/print.h"

GameManager::~GameManager()
{
  CloseHandle(gameProcess);
}

void GameManager::openGameProcess(const char *game)
{
  HWND hwnd;

  print("Waiting for game...");
  do
  {
    hwnd = FindWindowA(NULL, game);
    Sleep(50);
  } while (!hwnd);
  print("Found game window: ", game);

  DWORD pid;
  GetWindowThreadProcessId(hwnd, &pid);
  print("Found process id: ", pid);

  gameProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  print("Opened handle: ", gameProcess);

  do
  {
    clientBase = getModuleBaseAddress(pid, L"client_panorama.dll");
    Sleep(50);
  } while (!clientBase);
  print("Found client_panorama.dll");

  do
  {
    engineBase = getModuleBaseAddress(pid, L"engine.dll");
    Sleep(50);
  } while (!engineBase);
  print("Found engine.dll");
}

const GameManager &GameManager::enableBhop() const
{
  const int flags = readMem<int>(gameProcess, readMem<DWORD>(gameProcess, clientBase + offsets::dwLocalPlayer) + offsets::fFlags);

  if (flags & (1 << 0) && GetAsyncKeyState(VK_SPACE))
    writeMem<DWORD>(gameProcess, clientBase + offsets::dwForceJump, 6);

  return *this;
}

const GameManager &GameManager::enableGlow() const
{
  DWORD glowObj = readMem<DWORD>(gameProcess, clientBase + offsets::dwGlowObjectManager);
  DWORD myTeam = readMem<DWORD>(gameProcess, readMem<DWORD>(gameProcess, clientBase + offsets::dwLocalPlayer) + offsets::iTeamNum);

  for (int i = 0; i < 64; i++)
  {
    DWORD player = readMem<DWORD>(gameProcess, clientBase + offsets::dwEntityList + i * 0x10);
    if (player == 0)
      continue;

    bool dormant = readMem<bool>(gameProcess, player + 0xED);
    if (dormant)
      continue;

    DWORD team = readMem<DWORD>(gameProcess, player + offsets::iTeamNum);
    if (team != 2 && team != 3)
      continue;

    DWORD currentGlowIndex = readMem<DWORD>(gameProcess, player + offsets::iGlowIndex);

    if (team != myTeam)
    {
      writeMem<float>(gameProcess, glowObj + currentGlowIndex * 0x38 + 0x4, 255);
      writeMem<float>(gameProcess, glowObj + currentGlowIndex * 0x38 + 0x8, 0);
      writeMem<float>(gameProcess, glowObj + currentGlowIndex * 0x38 + 0xC, 0);
      writeMem<float>(gameProcess, glowObj + currentGlowIndex * 0x38 + 0x10, 255);
      writeMem<bool>(gameProcess, glowObj + currentGlowIndex * 0x38 + 0x24, true);
      writeMem<bool>(gameProcess, glowObj + currentGlowIndex * 0x38 + 0x25, false);
    }
  }

  return *this;
}

const GameManager &GameManager::enableRadar() const
{
  for (int i = 0; i < 64; i++)
  {
    DWORD player = readMem<DWORD>(gameProcess, clientBase + offsets::dwEntityList + i * 0x10);
    if (player == 0)
      continue;

    if (readMem<bool>(gameProcess, player + 0xED))
      continue;

    DWORD team = readMem<DWORD>(gameProcess, player + offsets::iTeamNum);
    if (team != 2 && team != 3)
      continue;

    writeMem<bool>(gameProcess, player + offsets::bSpotted, 1);
  }

  return *this;
}

const GameManager &GameManager::sleep(unsigned long ms) const
{
  Sleep(ms);
  return *this;
}