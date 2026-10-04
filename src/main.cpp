#include <windows.h>
#include "MinHook.h"

#ifdef _MSC_VER
#include <intrin.h>
#define RETURN_ADDRESS() _ReturnAddress()
#else
#define RETURN_ADDRESS() __builtin_return_address(0)
#endif

#define ADDR_PLAYER_PED                 0xB6F5F0
#define ADDR_SPRINT_JUST_DOWN           0x5407F0
#define ADDR_GET_BUTTON_SPRINT_RESULTS  0x60A820
#define ADDR_HANDLE_SPRINT_ENERGY       0x60A550

#define ADDR_CONTROL_SPRINT_RETADDR     0x60A692
#define ADDR_DUCK_ABORT_RETADDR         0x691F16
#define ADDR_CONTROL_ENERGY_RETADDR     0x60A7A5

#define PEDSTATE_OFFSET    0x530
#define PEDSTATE_DRIVING   50
#define SPRINT_GROUND      0

#define SPRINT_BOOST       0.25f
#define HOLD_DRAIN_RATE    1.0f
#define TAP_DRAIN_RATE     0.5f

typedef bool(__thiscall *SprintJustDown_t)(void *pad);
typedef float(__thiscall *GetButtonSprintResults_t)(void *playerPed, DWORD sprintType);
typedef bool(__thiscall *HandleSprintEnergy_t)(void *playerPed, int sprint, float rate);

static SprintJustDown_t         g_origSprintJustDown;
static GetButtonSprintResults_t g_origGetButtonSprintResults;
static HandleSprintEnergy_t     g_origHandleSprintEnergy;

static bool __fastcall HookSprintJustDown(void *pad, void *)
{
    DWORD_PTR retAddr = (DWORD_PTR)RETURN_ADDRESS();
    if (retAddr == ADDR_CONTROL_SPRINT_RETADDR || retAddr == ADDR_DUCK_ABORT_RETADDR)
    {
        BYTE *player = *(BYTE **)ADDR_PLAYER_PED;
        if (player == NULL || *(DWORD *)(player + PEDSTATE_OFFSET) != PEDSTATE_DRIVING)
        {
            return false;
        }
    }
    return g_origSprintJustDown(pad);
}

static float __fastcall HookGetButtonSprintResults(void *playerPed, void *, DWORD sprintType)
{
    float result = g_origGetButtonSprintResults(playerPed, sprintType);
    if (sprintType == SPRINT_GROUND)
    {
        result += SPRINT_BOOST;
    }
    return result;
}

static bool __fastcall HookHandleSprintEnergy(void *playerPed, void *, int sprint, float rate)
{
    if (sprint != 0
        && rate == HOLD_DRAIN_RATE
        && (DWORD_PTR)RETURN_ADDRESS() == ADDR_CONTROL_ENERGY_RETADDR)
    {
        rate = TAP_DRAIN_RATE;
    }
    return g_origHandleSprintEnergy(playerPed, sprint, rate);
}

BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        MH_Initialize();
        MH_CreateHook((LPVOID)ADDR_SPRINT_JUST_DOWN, (LPVOID)&HookSprintJustDown, (LPVOID *)&g_origSprintJustDown);
        MH_CreateHook((LPVOID)ADDR_GET_BUTTON_SPRINT_RESULTS, (LPVOID)&HookGetButtonSprintResults, (LPVOID *)&g_origGetButtonSprintResults);
        MH_CreateHook((LPVOID)ADDR_HANDLE_SPRINT_ENERGY, (LPVOID)&HookHandleSprintEnergy, (LPVOID *)&g_origHandleSprintEnergy);
        MH_EnableHook(MH_ALL_HOOKS);
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        MH_Uninitialize();
    }

    return TRUE;
}
