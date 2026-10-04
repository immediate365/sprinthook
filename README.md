# SprintHook (by DarkP1xel) — reconstructed source

Source reconstruction of `sprinthook.asi` (original binary kept in repo root), an auto-sprint plugin for GTA: San Andreas 1.0 US.

## What it does

The rebuild replicates the original mod's gameplay behavior 1:1 and adds one improvement: sprint stamina drains at the vanilla *tap*-sprint rate instead of the more expensive hold-sprint rate.

Hooks (all addresses are gta_sa.exe 1.0 US, verified by disassembly):

1. `CPad::SprintJustDown` (`0x5407F0`) — returns `false` unless the local player ped (`*(CPed**)0xB6F5F0`, `m_nPedState` at `+0x530`) is `PEDSTATE_DRIVING` (50), exactly like the original. Applied via return-address check to the game's only two callers: `CPlayerPed::ControlButtonSprint` (ret `0x60A692`; disables tap pumping of `m_fMoveSpeed`, so the engine treats you as a hold-sprinter and never force-transitions into the full sprint state) and the crouch-task abort check near `CTaskSimpleDuck` (ret `0x691F16`; sprint no longer cancels crouch). Both behaviors are required for the one-switch litefoot slide: the crouch survives the sprint press, and the sprint state machine doesn't take over the slide pose. The `PEDSTATE_DRIVING` exemption keeps BMX pedaling vanilla.
2. `CPlayerPed::GetButtonSprintResults` (`0x60A820`) — adds a flat +0.25 to `SPRINT_GROUND` results, exactly like the original. Since taps are blocked, on-foot results are 1.0 → 1.25 while moving: constant near-tap sprint speed. Ground calls reach this function only from the anim-speed site in `CPlayerPed::SetRealMoveAnim` (`0x60B430`), so this cannot affect stamina.
3. `CPlayerPed::HandleSprintEnergy` (`0x60A550`) — new in this rebuild. When called from `ControlButtonSprint`'s drain site (ret `0x60A7A5`) with `sprint == true` and the hold-path rate of 1.0 (`PLAYER_SPRINT_SET[].field_14`), the rate is replaced with 0.5 — the rate vanilla charges tap-sprinters (`field_18`, ground row of the table at `0x8D2460`). Speed stays 1.25x, stamina lasts twice as long, matching a legitimate tapping player. The same halving applies to sprint-swimming (vanilla tap-swim rate is 0.3, so swimming is charged slightly more than a perfect tap-swimmer); the regeneration path and all other callers are untouched.

The original's `GetModuleHandleA("SprintHookByDarkP1xel.ASI")` filename self-check has been removed; the rebuild loads under any name.

## Build

Requires 32-bit target (the game is x86).

MSVC (VS2022):

```
cmake -S . -B build -A Win32 && cmake --build build --config Release
```

Output: `build/Release/sprinthook.asi`. Drop it next to `gta_sa.exe` (with an ASI loader).

MinGW (msys2 mingw32 shell): `cmake -S . -B build -G Ninja && cmake --build build`

## Analysis notes (how the original was decompiled)

- The original is UPX 4.20-packed with all `UPX` section/magic markers renamed to `XXX` (`XXX0/XXX1/XXX2`, `XXX!` at 0x3E0) to defeat `upx -d`. Patching the four markers back restores a normal UPX file that unpacks cleanly.
- Unpacked: PE32 DLL, VS2013 (linker 12.0), static CRT, statically linked MinHook (identifiable from the `SuspendThread`/`GetThreadContext`/`Toolhelp32`/`FlushInstructionCache` import cluster and the spinlock/HeapCreate pattern in DllMain).
- Detours in the binary are LTCG-optimized but byte-for-byte equivalent in behavior to the C above; the 0.25 was stored as a double constant and added via x87 `fadd`, then truncated to float — identical to `+ 0.25f`.
- Game addresses identified via gta-reversed / plugin-sdk: `0x5407F0` = `CPad::SprintJustDown`, `0x60A820` = `CPlayerPed::GetButtonSprintResults(eSprintType)`, `CPed+0x530` = `m_nPedState`.

MinHook (`libs/minhook`, v1.3.4) is BSD-2-licensed — see `libs/minhook/LICENSE.txt`.
