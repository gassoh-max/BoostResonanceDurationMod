#pragma once

#include <DynamicOutput/Output.hpp>
#include <Mod/CppUserModBase.hpp>
#include <Unreal/UObjectGlobals.hpp>
#include <Unreal/UObject.hpp>
#include <Unreal/UFunction.hpp>
#include <Unreal/World.hpp>
#include "Hooks.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include <chrono>
#include <functional>

namespace VentruePassiveMod
{
    using namespace std;
    using namespace RC;
    using namespace RC::Unreal;
    using namespace chrono;

    constexpr bool DEBUG_MODE = false;

    class VentruePassive : public CppUserModBase {
    private:
        // Game state
        bool isGameReady = false;
        bool hooksRegistered = false;
        int IsActiveVentrueAbility = 0;
		static constexpr int WORLD_CHECK_DELAY_FRAMES = 5;
		int worldCheckDelayCounter = 0;
		UObject* cachedWorld = nullptr;

        // Cached UObjects
        //UObject* cachedPassiveAbility = nullptr;
        //UObject* cachedElixirVentrue = nullptr;
        //UObject* cachedPlayerState = nullptr;
        //UObject* cachedPlayerCharacter = nullptr;

        // Cached UFunctions
        UFunction* cachedK2_ActivateAbility = nullptr;
        UFunction* cachedK2_OnEndAbility = nullptr;
        //UFunction* cachedApplyPassiveEffect = nullptr;
        //UFunction* cachedChangeElixirCount = nullptr;
        //UFunction* cachedUpdateConsumable = nullptr;

        // Timing for delayed activation
        steady_clock::time_point lastAbilityEnd = steady_clock::now();
        bool shouldActivatePassive = false;
        int delayFrames = 0;
		int delayFramesSearchePassive = 0;
		static constexpr int ACTIVATION_DELAY_FRAMES = 3; // ~300ms at 10 updates per second

        // Static instance pointer for hook callback
        static VentruePassive* s_Instance;

        // Hook callback (static, called by UE4SS)
        static void OnProcessEventHook(UObject* Context, UFunction* Function, void* Parms);
        
    public:
        VentruePassive();
        ~VentruePassive() override;

        // Core search functions
        UObject* SearchClassByTexts(const vector<wstring>& searchTexts);

        // CRITICAL: NON restituisce puntatori - usa un callback per evitare di bloccare il GC
        bool WithPassiveAbilityInstance(function<void(UObject*)> callback);
        //UObject* GetElixirUClass();
        UObject* GetPlayerStateUClass();
        UObject* GetWrestlerTestPlayerUClass();

        // Game functions
        bool InitializeGameFunctions();
		void ClearCachedObjects();
        //void ConsumeElixir();
        void ActiveVentruePassive();
		void DeactiveVentruePassive();
        void HookK2_OnEndAbilityEvent(UFunction* Function);
		void HookK2_OnActivateAbilityEvent(UFunction* Function);

        // Hook management
        void SetupHooks();
		void UnregisterHooks();
        bool IsPlayerInGameWorld();

        // CppUserModBase override
        auto on_update() -> void override;

        // Utility
        void Log(const wstring& message);
    };
}
