local UEHelpers = require("UEHelpers")

-- CONFIGURAZIONE ---------------------------------------------------------
local DEBUG_MODE = true

local DURATION_PASSIVE = 30000 -- durata in ms dell'effetto della pozione

-- /CONFIGURAZIONE --------------------------------------------------------

-- stato persistente in memoria runtime
run = run or { step = 0 } -- stato globale del run

-- helper robusti
local function safe_pcall(fn, ...)
  local ok, a, b, c = pcall(fn, ...)
  return ok, a, b, c
end

local function safe_tostring(x)
  local ok, s = safe_pcall(function() return tostring(x) end)
  if ok and s then return s end
  return "<unprintable>"
end

local function log(...)
  local parts = {}
  for i = 1, select("#", ...) do parts[#parts+1] = safe_tostring(select(i, ...)) end
  local line = "[BoostResonanceDurationMod - LUA] " .. table.concat(parts, " ") .. "\n"
  if DEBUG_MODE then pcall(function() print(line) end) end
end

function FindClassByStrings(...)
  local texts = {}
  for i = 1, select("#", ...) do texts[#texts+1] = safe_tostring(select(i, ...)) end
  log("Search: ", table.concat(texts, " "))
  local results = {}
  ForEachUObject(function(Object, ChunkIndex, ObjectIndex)
    if not Object then return end
    local ok, fullname = pcall(function() return Object:GetFullName() end)
    if ok and fullname and type(fullname) == "string" then
      local match = true
      for _, pat in ipairs(texts) do
        if type(pat) ~= "string" or not fullname:find(pat, 1, true) then
          match = false
          break
        end
      end
      if match then
        results[#results + 1] = Object
        log("Found UObject:", safe_tostring(Object:GetFullName()))
      end
    end
  end)
  return results
end

--GA_Ventrue_Passive_bloodbased_C /Game/WrestlerHubOne/Maps/LV_Hub/LV_WP_Hub_Master.LV_WP_Hub_Master:PersistentLevel.BP_WrestlerTestPlayerCharacter_C_2147481123.GA_Ventrue_Passive_bloodbased_C_2147480056
local cache_ActivatePassive = nil
function GetPassiveAbilityClass()
	if cache_ActivatePassive == nil then
      log("Scan for ActivateShieldHud Class")
      local raws_search = {}
      raws_search = FindClassByStrings("GA_Ventrue_Passive_bloodbased_C ", "PersistentLevel.BP_WrestlerTestPlayerCharacter")
      cache_ActivatePassive = raws_search[1]
    end
    return cache_ActivatePassive
end

function activeVentruePassive()
  log("test activeVentruePassive run=", run.step)
  if run.step == 1 then
    GetPassiveAbilityClass():K2_ActivateAbility()
    ExecuteWithDelay(DURATION_PASSIVE, function() 
      ExecuteWithDelay(500, function() run.step = 0; log("Reset run to 0") end)
      if GetPassiveAbilityClass():IsValid() then
        GetPassiveAbilityClass():K2_OnEndAbility(true)
      end      
    end)
  end

  return true
end

function cmd_ActiveVentruePassive()
  run.step = 1
  activeVentruePassive() 
  return true
end

-- register console commands end)
RegisterConsoleCommandHandler("start_passive", function()  
  --return consume_elixir()
  return cmd_ActiveVentruePassive()
end)

-- Watcher for game start to cache Ventrue Passive AbilitySystemComponent

  --local registroHook = {}
  --HOOK START GAME
  RegisterHook("/Script/Engine.PlayerController:ClientRestart", function()
  --Work partial, only when player open map in game before, RegisterULocalPlayerExecPostHook(function(Player, World, Cmd, Ar)
    local World = UEHelpers.GetWorld()
    --log("HOOK FIRED: LoadMap | Context=", safe_tostring(World))
    log("World: ", World:GetFullName())
    if World:GetFullName():find("Minimal.Minimal") then
      log("Waiting for game World..")
      run.step = 0
    else
      --World is valid
      log("World is valid, setup hook for Ventrue Passive")
      GetPassiveAbilityClass()
      local path = "/Game/WrestlerCommon/Abilities/Player/Passive/Ventrue/GA_Ventrue_Passive_bloodbased.GA_Ventrue_Passive_bloodbased_C:"
      
      --if registroHook[path] == nil then
        --log("HOOK SETUP: /Script/Engine.PlayerController:ClientRestart -> K2_OnEndAbility, registro:", safe_tostring(registroHook[path]))
        NotifyOnNewObject("/Game/WrestlerCommon/Abilities/Player/Passive/Ventrue/GA_Ventrue_Passive_bloodbased.GA_Ventrue_Passive_bloodbased_C", function (Component)
          log("New Object detected: ", Component:GetFullName())
          if Component:GetFullName():find("GA_Ventrue_Passive_bloodbased_C") then
            cache_ActivatePassive = Component
          end      
        end)
    end

  end)
print("[BoostResonanceDurationMod - LUA] Mod loaded.")
-- fine file