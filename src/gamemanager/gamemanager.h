#pragma once

#include <Windows.h>
#include <cstdint>

class GameManager
{
private:
  HANDLE gameProcess;
  uintptr_t clientBase;
  uintptr_t engineBase;

public:
  GameManager() = default;
  ~GameManager();
  void openGameProcess(const char *game);
  const GameManager &enableBhop() const;
  const GameManager &enableGlow() const;
  const GameManager &enableRadar() const ;
  const GameManager &sleep(unsigned long ms) const;
};