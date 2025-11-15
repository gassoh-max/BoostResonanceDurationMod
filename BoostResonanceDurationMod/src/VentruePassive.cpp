#include "VentruePassive.hpp"

namespace VentruePassiveMod
{
	using namespace RC;
	using namespace RC::Unreal;

	// Initialize static instance pointer
	VentruePassive* VentruePassive::s_Instance = nullptr;

	VentruePassive::VentruePassive() {
		ModVersion = STR("1.0");
		ModName = STR("VentruePassiveMod");
		ModAuthors = STR("Alrikh");
		ModDescription = STR("Auto-activates Ventrue passive ability and manages Elixir");

		// Set static instance for hook callback
		s_Instance = this;

		Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] v1.0 - Initialized\n"));
	}

	VentruePassive::~VentruePassive() {
		Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Shutdowning..\n"));

		UnregisterHooks();
		ClearCachedObjects();

		// Clear static instance
		if (s_Instance == this)
		{
			s_Instance = nullptr;
		}

		Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Shutdown\n"));
	}

	void VentruePassive::Log(const std::wstring& message)
	{
		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] {}\n"), message);
		}
	}

	// Clears cached UObject/UFunction pointers and internal flags to avoid holding references
	void VentruePassive::ClearCachedObjects()
	{
		cachedWorld = nullptr;
		//hooksRegistered = false;
		//cachedPassiveAbility = nullptr;
		cachedK2_ActivateAbility = nullptr;
		cachedK2_OnEndAbility = nullptr;
		//cachedElixirVentrue = nullptr;
		//cachedPlayerState = nullptr;
		//cachedPlayerCharacter = nullptr;

		shouldActivatePassive = false;
		delayFrames = 0;
		delayFramesSearchePassive = 0;
		IsActiveVentrueAbility = 0;
	}

	bool VentruePassive::IsPlayerInGameWorld()
	{
		UObject* PlayerController = UObjectGlobals::FindFirstOf(STR("BP_WrestlerPlayerControllerInGame_C"));
		if (!PlayerController) return false;

		auto PawnPtr = PlayerController->GetValuePtrByPropertyNameInChain<UObject*>(STR("Pawn"));
		if (!PawnPtr || !*PawnPtr) return false;

		UObject* Pawn = *PawnPtr;
		std::wstring pawnName = Pawn->GetFullName();

		bool found = pawnName.find(STR("BP_WrestlerTestPlayerCharacter_C")) != std::wstring::npos;
		PlayerController = nullptr;
		Pawn = nullptr;

		return found;
	}

	UObject* VentruePassive::SearchClassByTexts(const std::vector<std::wstring>& searchTexts)
	{
		UObject* result = nullptr;

		UObjectGlobals::ForEachUObject([&](UObject* obj, ...) {
			if (!obj) return LoopAction::Continue;

			std::wstring fullName = obj->GetFullName();
			bool matchAll = true;

			for (const auto& text : searchTexts)
			{
				if (fullName.find(text) == std::wstring::npos)
				{
					matchAll = false;
					break;
				}
			}

			if (matchAll)
			{
				result = obj;
				if (DEBUG_MODE)
				{
					Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Found UObject: {}\n"), fullName);
				}
				return LoopAction::Break;
			}

			return LoopAction::Continue;
		});

		return result;
	}

	// Helper locale per verificare che un UObject sia ancora "vivo"
	static bool IsUObjectAlive(RC::Unreal::UObject* obj)
	{
		using namespace RC::Unreal;
		if (!obj) return false;

		// Raw validity (serial number / internal table)
		if (!UObject::IsReal(static_cast<const void*>(obj))) return false;

		// Oggetto marcato come unreachable / garbage
		if (obj->IsUnreachable()) return false;

		// Flag di distruzione iniziata / completata
		if (obj->HasAnyFlags(EObjectFlags::RF_BeginDestroyed) || obj->HasAnyFlags(EObjectFlags::RF_FinishDestroyed))
		{
			return false;
		}

		return true;
	}

	// CRITICAL: Questo metodo NON restituisce puntatori per evitare di bloccare il GC
	// Usa invece un callback che riceve l'oggetto e lo usa IMMEDIATAMENTE
	bool VentruePassive::WithPassiveAbilityInstance(std::function<void(UObject*)> callback)
	{
		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Searching for GA_Ventrue_Passive_bloodbased_C instance\n"));
		}

		std::vector<UObject*> instances;
		UObjectGlobals::FindAllOf(STR("GA_Ventrue_Passive_bloodbased_C"), instances);

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Found {} instances\n"), instances.size());
		}

		// Cerca l'istanza del player e chiamala IMMEDIATAMENTE senza tenere riferimenti
		for (auto* instance : instances)
		{
			if (!instance || !IsUObjectAlive(instance)) continue;

			std::wstring fullName = instance->GetFullName();

			if (fullName.find(STR("PersistentLevel.BP_WrestlerTestPlayerCharacter")) != std::wstring::npos)
			{
				if (DEBUG_MODE)
				{
					Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Found and using instance: {}\n"), fullName);
				}

				// Chiama il callback IMMEDIATAMENTE, poi scarta il puntatore
				callback(instance);
				return true;
			}
		}

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Instance not found\n"));
		}

		return false;
	}

	/*UObject* VentruePassive::GetElixirUClass()
	{
		if (cachedElixirVentrue)
		{
			return cachedElixirVentrue;
		}

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Scan for ElixirVentrue Class\n"));
		}

		std::vector<std::wstring> searchTexts = {
			STR("GA_ElixirVentrue_C"),
			STR("/Game/"),
			STR("BP_WrestlerTestPlayerCharacter_C")
		};

		cachedElixirVentrue = SearchClassByTexts(searchTexts);
		return cachedElixirVentrue;
	}*/

	/*UObject* VentruePassive::GetPlayerStateUClass()
	{
		if (cachedPlayerState)
		{
			return cachedPlayerState;
		}

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Scan for WrestlerPlayerStateInGame Class\n"));
		}

		std::vector<std::wstring> searchTexts = {
			STR("PersistentLevel"),
			STR("BP_WrestlerPlayerStateInGame_C"),
			STR("/Game/")
		};

		cachedPlayerState = SearchClassByTexts(searchTexts);
		return cachedPlayerState;
	}

	UObject* VentruePassive::GetWrestlerTestPlayerUClass()
	{
		if (cachedPlayerCharacter)
		{
			return cachedPlayerCharacter;
		}

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Scan for WrestlerTestPlayerChar Class\n"));
		}

		std::vector<std::wstring> searchTexts = {
			STR("BP_WrestlerTestPlayerCharacter_C "),
			STR("/Game/"),
			STR("PersistentLevel.BP_WrestlerTestPlayerCharacter_C")
		};

		cachedPlayerCharacter = SearchClassByTexts(searchTexts);
		return cachedPlayerCharacter;
	}*/

	bool VentruePassive::InitializeGameFunctions()
	{
		if (cachedK2_ActivateAbility && cachedK2_OnEndAbility) //&& cachedApplyPassiveEffect
		{
			return true;
		}

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Searching for game functions...\n"));
		}

		// K2_ActivateAbility
		const wchar_t* activatePath = L"/Game/WrestlerCommon/Abilities/Player/Passive/Ventrue/GA_Ventrue_Passive_bloodbased.GA_Ventrue_Passive_bloodbased_C:K2_ActivateAbility";
		if (auto func = UObjectGlobals::FindObject<UFunction>(nullptr, activatePath); func) {
			cachedK2_ActivateAbility = func;
			if (DEBUG_MODE) Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Found K2_ActivateAbility\n"));
		}

		// K2_OnEndAbility
		const wchar_t* endPath = L"/Game/WrestlerCommon/Abilities/Player/Passive/Ventrue/GA_Ventrue_Passive_bloodbased.GA_Ventrue_Passive_bloodbased_C:K2_OnEndAbility";
		if (auto func = UObjectGlobals::FindObject<UFunction>(nullptr, endPath); func) {
			cachedK2_OnEndAbility = func;
			if (DEBUG_MODE) Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Found K2_OnEndAbility\n"));
		}

		if (DEBUG_MODE)
		{
			auto f = s_Instance->cachedK2_OnEndAbility->GetFullName();
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] {} to check in HOOK\n"), f);
		}

		//// ChangeElixirCount
		//const wchar_t* elixirPath = L"/Script/Wrestler.WrestlerPlayerStateInGame:ChangeElixirCount";
		//if (auto func = UObjectGlobals::FindObject<UFunction>(nullptr, elixirPath); func) {
		//    cachedChangeElixirCount = func;
		//    if (DEBUG_MODE) Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Found ChangeElixirCount\n"));
		//}

		//// UpdateConsumable
		//const wchar_t* consumePath = L"/Script/Wrestler.WrestlerPlayerStateInGame:UpdateConsumable";
		//if (auto func = UObjectGlobals::FindObject<UFunction>(nullptr, consumePath); func) {
		//    cachedUpdateConsumable = func;
		//    if (DEBUG_MODE) Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Found UpdateConsumable\n"));
		//}

		return (cachedK2_ActivateAbility != nullptr && cachedK2_OnEndAbility != nullptr);
	}

	//void VentruePassive::ConsumeElixir()
	//{
	//    UObject* playerState = GetPlayerStateUClass();
	//    UObject* elixir = GetElixirUClass();
	//
	//    if (!playerState || !elixir || !cachedChangeElixirCount || !cachedUpdateConsumable)
	//    {
	//        if (DEBUG_MODE)
	//        {
	//            Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Cannot consume elixir - missing objects or functions\n"));
	//        }
	//        return;
	//    }
//
	//    // ChangeElixirCount(bool bAdd, int32 Count, struct FCanUseElixir& CanUseElixir)
	//    struct ChangeElixirParams {
	//        bool bAdd;
	//        int32_t Count;
	//        struct {
	//            bool CanUseElixir;
	//        } OutStruct;
	//    };
	//
	//    ChangeElixirParams params;
	//    params.bAdd = true;
	//    params.Count = 3;
	//    params.OutStruct.CanUseElixir = false;
	//
	//    playerState->ProcessEvent(cachedChangeElixirCount, &params);
	//
	//    if (DEBUG_MODE)
	//    {
	//        Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] CanUseElixir: {}\n"), params.OutStruct.CanUseElixir);
	//    }
	//
	//    // UpdateConsumable(int32 Index, bool bAdd)
	//    struct UpdateConsumableParams {
	//        int32_t Index;
	//        bool bAdd;
	//    };
	//
	//    UpdateConsumableParams updateParams;
	//    updateParams.Index = 3;
	//    updateParams.bAdd = true;
	//
	//    playerState->ProcessEvent(cachedUpdateConsumable, &updateParams);
	//
	//    // Set Duration property on Elixir
	//    auto durationPtr = elixir->GetValuePtrByPropertyNameInChain<float>(STR("Duration"));
	//    if (durationPtr)
	//    {
	//        *durationPtr = 100.0f;
	//    }
	//
	//    // Activate ability
	//    if (cachedK2_ActivateAbility)
	//    {
	//        struct ActivateParams {
	//            bool ReturnValue;
	//        };
	//
	//        ActivateParams activateParams;
	//        activateParams.ReturnValue = false;
	//
	//        elixir->ProcessEvent(cachedK2_ActivateAbility, &activateParams);
	//
	//        if (DEBUG_MODE)
	//        {
	//            Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Elixir activation result: {}\n"), activateParams.ReturnValue);
	//        }
	//    }
	//}

	void VentruePassive::ActiveVentruePassive()
	{
		if (!cachedK2_ActivateAbility)
		{
			if (DEBUG_MODE)
			{
				Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Cannot activate - function not found\n"));
			}
			return;
		}

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Activating passive ability...\n"));
		}

		//CALL LUA SCRIPT TO START VENTRUE PASSIVE ABILITY!
		// Execute the Lua script command via console
		UObject* GameViewportClient = UObjectGlobals::FindFirstOf(STR("GameViewportClient"));
		if (GameViewportClient)
		{
			// Execute console command to trigger Lua script
			FString command(STR("start_passive"));
			FOutputDevice outputDevice;

			bool x = GameViewportClient->ProcessConsoleExec(*command, outputDevice, nullptr);

			if (DEBUG_MODE)
			{
				Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Executed console command: start_passive, with result: {}\n"), x);
			}
		}
		else if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] GameViewportClient not found\n"));
		}

		//// Usa il callback pattern - l'oggetto viene trovato, usato e IMMEDIATAMENTE scartato
		//WithPassiveAbilityInstance([this](UObject* passive) {
		//	passive->ProcessEvent(passive->GetFunctionByName(L"ApplyPassiveEffect"), nullptr);
		//	passive->ProcessEvent(cachedK2_ActivateAbility, nullptr);
		//	
		//	// passive esce dallo scope QUI - nessun riferimento tenuto
		//});

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Activation complete\n"));
		}
	}

	void VentruePassive::DeactiveVentruePassive()
	{
		if (!cachedK2_OnEndAbility)
		{
			if (DEBUG_MODE)
			{
				Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Cannot deactivate - function not found\n"));
			}
			return;
		}

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Deactivating passive ability...\n"));
		}

		struct {
			bool bWasCancelled;
		} Params;
		Params.bWasCancelled = false;

		// Usa il callback pattern - l'oggetto viene trovato, usato e IMMEDIATAMENTE scartato
		WithPassiveAbilityInstance([this, &Params](UObject* passive) {

			passive->ProcessEvent(cachedK2_OnEndAbility, &Params);
			// passive esce dallo scope QUI - nessun riferimento tenuto
		});

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Deactivation complete\n"));
		}
	}

	void VentruePassive::HookK2_OnEndAbilityEvent(UFunction* Function) {
		// Check if this is the K2_OnEndAbility function we're interested in
		if (!s_Instance || !s_Instance->cachedK2_OnEndAbility)
		{
			return;
		}

		// Check if this is our target function
		if (Function != s_Instance->cachedK2_OnEndAbility)
		{
			return;
		}

		if (DEBUG_MODE)
		{
			auto f = Function->GetFullName();
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] HOOK FIRED: {}\n"), f);
		}

		// Increment run counter and schedule reactivation
		if (s_Instance->IsActiveVentrueAbility == 0) {
			s_Instance->IsActiveVentrueAbility = 1;
			s_Instance->shouldActivatePassive = true;
			s_Instance->delayFrames = 0;
			s_Instance->lastAbilityEnd = steady_clock::now();
		}
	}

	void VentruePassive::HookK2_OnActivateAbilityEvent(UFunction* Function) {
		// Check if this is the K2_OnEndAbility function we're interested in
		if (!s_Instance || !s_Instance->cachedK2_ActivateAbility)
		{
			return;
		}

		// Check if this is our target function
		if (Function != s_Instance->cachedK2_ActivateAbility)
		{
			return;
		}

		if (DEBUG_MODE)
		{
			auto f = Function->GetFullName();
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] HOOK FIRED: {}\n"), f);
		}

	}

	//void VentruePassive::OnProcessEventHook(UObject* Context, UFunction* Function, void* Parms)
	//{
	//	// CRITICAL: Evita di processare eventi dopo shutdown
	//	if (!VentruePassive::s_Instance) {
	//		return;
	//	}

	//	// CRITICAL: Non processare eventi se non siamo nel mondo di gioco
	//	if (!VentruePassive::s_Instance->isGameReady) {
	//		return;
	//	}

	//	// Se gli hook non sono più registrati, non processare nulla
	//	if (!VentruePassive::s_Instance->hooksRegistered) {
	//		return;
	//	}

	//	// CRITICAL: Valida Context e Function prima di qualsiasi operazione
	//	if (!s_Instance->cachedWorld) {
	//		return;
	//	}

	//	// Call specific hook handlers
	//	s_Instance->HookK2_OnEndAbilityEvent(Function);
	//	s_Instance->HookK2_OnActivateAbilityEvent(Function);
	//}

	void VentruePassive::SetupHooks()
	{
		if (hooksRegistered)
		{
			return;
		}

		// Verifica solo che le funzioni esistano - NON cercare l'istanza
		if (!cachedK2_OnEndAbility || !cachedK2_ActivateAbility)
		{
			if (DEBUG_MODE)
			{
				Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Cannot setup hooks - functions not found\n"));
			}
			return;
		}

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Setting up hooks\n"));
		}

		// Register hook on K2_OnEndAbility to detect when passive ends
		cachedK2_OnEndAbility->RegisterPostHook([](UnrealScriptFunctionCallableContext& Context, void* CustomData) {
			if (VentruePassive::s_Instance)
			{
				VentruePassive::s_Instance->HookK2_OnEndAbilityEvent(Context.TheStack.Node());
			}
		});

		// Register hook on K2_ActivateAbility to detect when passive starts
		cachedK2_ActivateAbility->RegisterPostHook([](UnrealScriptFunctionCallableContext& Context, void* CustomData) {
			if (VentruePassive::s_Instance)
			{
				VentruePassive::s_Instance->HookK2_OnActivateAbilityEvent(Context.TheStack.Node());
			}
		});

		//Hook::RegisterProcessEventPreCallback(&OnProcessEventHook);
		hooksRegistered = true;

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Hooks registered\n"));
		}
	}

	void VentruePassive::UnregisterHooks()
	{
		if (!hooksRegistered)
		{
			return;
		}

		// Se vuoi usare gli id specifici:
		// for (const auto& id : hooks_id) { cachedK2_OnEndAbility->UnregisterHook(id); ... }
		// Ma più semplice: rimuovi tutti gli hook dalle funzioni note.

		if (cachedK2_OnEndAbility)
		{
			cachedK2_OnEndAbility->UnregisterAllHooks();
		}

		if (cachedK2_ActivateAbility)
		{
			cachedK2_ActivateAbility->UnregisterAllHooks();
		}

		hooksRegistered = false;

		if (DEBUG_MODE)
		{
			Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Hooks unregistered\n"));
		}
	}

	auto VentruePassive::on_update() -> void
	{
		// Check if game world is ready
		if (!isGameReady)
		{
			worldCheckDelayCounter++;
			if (worldCheckDelayCounter >= WORLD_CHECK_DELAY_FRAMES)
			{
				worldCheckDelayCounter = 0;
				if (IsPlayerInGameWorld())
				{
					isGameReady = true;
					cachedWorld = UObjectGlobals::FindFirstOf(STR("World"));
					if (DEBUG_MODE)
					{
						Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Player entered game world, mod is now active\n"));
					}
				}
			}
			return;
		}
		else {
			if (!IsPlayerInGameWorld())
			{
				UnregisterHooks();
				isGameReady = false;
				ClearCachedObjects();
				if (DEBUG_MODE)
				{
					Output::send<LogLevel::Warning>(STR("[VentruePassiveMod] Player left game world, resetting state\n"));
				}
				return;
			}
		}

		if (InitializeGameFunctions())
		{
			SetupHooks();
		}

		// Handle delayed passive activation (triggered by hook)
		if (shouldActivatePassive)
		{
			delayFrames++;

			if (delayFrames >= ACTIVATION_DELAY_FRAMES)
			{
				shouldActivatePassive = false;
				delayFrames = 0;
				if (IsActiveVentrueAbility == 1) // Only activate if IsActiveVentrueAbility is 1 (as per Lua logic)
				{
					ActiveVentruePassive();
					//IsActiveVentrueAbility = 0; // Reset counter
				}
			}
		}

		if (IsActiveVentrueAbility > 0)
		{
			if (lastAbilityEnd + std::chrono::seconds(32) < std::chrono::steady_clock::now())
			{
				// Reset IsActiveVentrueAbility if too much time has passed since last ability end
				//DeactiveVentruePassive();
				IsActiveVentrueAbility = 0;
			}
		}
	}
}
