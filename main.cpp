#include <Windows.h>

DWORD* player_base;
DWORD* game_base;
DWORD* gold;
DWORD ret_address = 0xCCAF90;

// Our codecave that program execution will jump to. The declspec naked attribute tells the compiler to not add any function
// headers around the assembled code
__declspec(naked) void codecave() 
{
	// Asm blocks allow you to write pure assembly
	// In this case, we use it to save all the registers
	__asm 
	{
		pushad
	}

	// Set the player's gold in the same method discussed in https://gamehacking.academy/lesson/16
	player_base = (DWORD*)0x017EED18;
	game_base = (DWORD*)(*player_base + 0xA90);
	gold = (DWORD*)(*game_base + 4);
	*gold = 9999;

	// Restore the registers and then recreate the original instructions that we overwrote
	// After those, jump back to the instruction after the one we overwrote
	_asm 
	{
		popad
		mov eax, dword ptr ds : [ecx]
		lea esi, dword ptr ds : [esi]
		jmp ret_address
	}
}

// When our DLL is attached, unprotect the memory at the code we wish to write at
// Then set the first opcode to E9, or jump
// Caculate the location using the formula: new_location - original_location+5
// Finally, since the original instructions totalled 6 bytes, NOP out the last remaining byte
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
	DWORD old_protect;
	unsigned char* hook_location = (unsigned char*)0x00CCAF8A;

	if (fdwReason == DLL_PROCESS_ATTACH) 
	{
		VirtualProtect((void*)hook_location, 6, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		*(DWORD*)(hook_location + 1) = (DWORD)&codecave - ((DWORD)hook_location + 5);
		*(hook_location + 5) = 0x90;
	}

	return true;
}