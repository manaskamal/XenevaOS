#include <Uefi.h>

extern "C"
EFI_STATUS
EFIAPI
EfiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)L"XenevaOS RISC-V Bootloader - C++ Entry Point Reached!\r\n");
  
  // Hang to wait for user to see output
  while(1);
  
  return EFI_SUCCESS;
}
