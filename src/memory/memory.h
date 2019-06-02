#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <algorithm>
#include <string>

template <typename Elem1, typename Elem2>
bool strings_equal(Elem1 *str1, Elem2 *str2)
{
  return std::equal(str1, str1 + std::char_traits<Elem1>::length(str1),
                    str2, str2 + std::char_traits<Elem2>::length(str2));
}

template <typename Elem1, typename Elem2>
bool strings_equal(const std::basic_string<Elem1> &str1, const std::basic_string<Elem2> &str2)
{
  return std::equal(str1.data(), str1.data() + str1.length(),
                    str2.data(), str2.data() + str2.length());
}

template <typename Elem1, typename Elem2>
bool strings_equal(Elem1 *str1, const std::basic_string<Elem2> &str2)
{
  return std::equal(str1, str1 + std::char_traits<Elem1>::length(str1),
                    str2.data(), str2.data() + str2.length());
}

template <typename Elem1, typename Elem2>
bool strings_equal(const std::basic_string<Elem1> &str1, Elem2 *str2)
{
  return std::equal(str1.data(), str1.data() + str1.length(),
                    str2, str2 + std::char_traits<Elem2>::length(str2));
}

uintptr_t getModuleBaseAddress(DWORD procId, const wchar_t *modName)
{
  uintptr_t modBaseAddr = 0;
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);

  if (hSnap != INVALID_HANDLE_VALUE)
  {
    MODULEENTRY32 modEntry;
    modEntry.dwSize = sizeof(modEntry);
    if (Module32First(hSnap, &modEntry))
    {
      do
      {
        if (strings_equal(modEntry.szModule, modName))
        {
          modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
          break;
        }
      } while (Module32Next(hSnap, &modEntry));
    }
  }

  CloseHandle(hSnap);
  return modBaseAddr;
}

template <typename T>
T readMem(const HANDLE &process, uintptr_t address)
{
  T buffer;
  ReadProcessMemory(process, reinterpret_cast<BYTE *>(address), &buffer, sizeof(buffer), 0);
  return buffer;
}

template <typename T>
void writeMem(const HANDLE &process, uintptr_t address, T value)
{
  WriteProcessMemory(process, reinterpret_cast<BYTE *>(address), &value, sizeof(value), 0);
}
