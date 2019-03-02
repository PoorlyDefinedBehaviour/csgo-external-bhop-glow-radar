#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

const DWORD dwLocalPlayer = 0xCBD6A4;
const DWORD m_fFlags = 0x104;
const DWORD dwForceJump = 0x5170DF0;
const DWORD dwEntityList = 0x4CCDC3C;
const DWORD m_iTeamNum = 0xF4;
const DWORD m_bSpotted = 0x93D;
const DWORD m_iGlowIndex = 0xA3F8;
const DWORD dwGlowObjectManager = 0x520DA80;
const DWORD modelAmbientMin = 0x58DD1C;
const DWORD m_iCrosshairId = 0xB394;
const DWORD dwbSendPackets = 0xD210A;

HANDLE process;   // csgo process
DWORD clientBase; // base address of client_panorama.dll
DWORD engineBase; // base address of engine.dll

// Get base address of module
DWORD getModuleBaseAddress(DWORD pid, const char *name)
{
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
  MODULEENTRY32 mEntry;
  mEntry.dwSize = sizeof(MODULEENTRY32);
  do
  {
    if (!strcmp(mEntry.szModule, name))
    {
      CloseHandle(snapshot);
      return (DWORD)mEntry.modBaseAddr;
    }
  } while (Module32Next(snapshot, &mEntry));
}

// Read from specified memory address
template <typename T>
T readMem(DWORD address)
{
  T buffer;
  ReadProcessMemory(process, (LPVOID)address, &buffer, sizeof(buffer), 0);
  return buffer;
}

// Write to specified memory address
template <typename T>
void writeMem(DWORD address, T value)
{
  WriteProcessMemory(process, (LPVOID)address, &value, sizeof(value), 0);
}

void bhop()
{
  // Read flags
  int flags = readMem<int>(readMem<DWORD>(clientBase + dwLocalPlayer) + m_fFlags);
  // Check flags and key state
  if (flags & (1 << 0) && GetAsyncKeyState(VK_SPACE))
    writeMem<DWORD>(clientBase + dwForceJump, 6); // Jump
}

void glow()
{
  DWORD glowObj = readMem<DWORD>(clientBase + dwGlowObjectManager);
  DWORD myTeam = readMem<DWORD>(readMem<DWORD>(clientBase + dwLocalPlayer) + m_iTeamNum);

  for (int x = 0; x < 32; x++)
  {
    DWORD player = readMem<DWORD>(clientBase + dwEntityList + x * 0x10);
    if (player == 0)
      continue;

    // Check if player is dormant
    bool dormant = readMem<bool>(player + 0xED);
    if (dormant)
      continue;

    // Check team
    DWORD team = readMem<DWORD>(player + m_iTeamNum);
    if (team != 2 && team != 3)
      continue;

    // Glow player
    DWORD currentGlowIndex = readMem<DWORD>(player + m_iGlowIndex);

    if (team != myTeam)
    {
      // Enemy
      writeMem<float>(glowObj + currentGlowIndex * 0x38 + 0x4, 255);
      writeMem<float>(glowObj + currentGlowIndex * 0x38 + 0x8, 0);
      writeMem<float>(glowObj + currentGlowIndex * 0x38 + 0xC, 0);
      writeMem<float>(glowObj + currentGlowIndex * 0x38 + 0x10, 255);
      writeMem<bool>(glowObj + currentGlowIndex * 0x38 + 0x24, true);
      writeMem<bool>(glowObj + currentGlowIndex * 0x38 + 0x25, false);
    }
  }
}

void radar()
{
  for (int x = 0; x < 32; x++)
  {
    DWORD player = readMem<DWORD>(clientBase + dwEntityList + x * 0x10);
    if (player == 0)
      continue;
    // Check if player is dormant
    if (readMem<bool>(player + 0xED))
      continue;
    // Check team
    DWORD team = readMem<DWORD>(player + m_iTeamNum);
    if (team != 2 && team != 3)
      continue;
    // 1 = player has been spotted
    writeMem<bool>(player + m_bSpotted, 1);
  }
}

int main()
{
  SetConsoleTitle("nao sei gente fina");
  std::cout << "Esperando pelo csgo...\n";

  // Find csgo window
  HWND hwnd;
  do
  {
    hwnd = FindWindowA(0, "Counter-Strike: Global Offensive");
    Sleep(50);
  } while (!hwnd);

  // Get process id of csgo.exe
  DWORD pid;
  GetWindowThreadProcessId(hwnd, &pid);
  process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

  std::cout << "Processo do csgo encontrado com ID: " << pid << ".\n";

  // Get base address of client_panorama.dll
  do
  {
    clientBase = getModuleBaseAddress(pid, "client_panorama.dll");
    Sleep(50);
  } while (!clientBase);

  // Get base address of engine.dll
  do
  {
    engineBase = getModuleBaseAddress(pid, "engine.dll");
    Sleep(50);
  } while (!engineBase);

  std::cout << "BHOP: ON" << std::endl;
  std::cout << "GLOW: ON" << std::endl;
  std::cout << "RADAR: ON" << std::endl;

  while (true)
  {
    bhop();
    glow();
    radar();
    Sleep(1);
  }
}
