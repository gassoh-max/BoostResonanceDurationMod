#include "VentruePassive.hpp"

#define MOD_API __declspec(dllexport)
extern "C" {
    MOD_API RC::CppUserModBase* start_mod() {
        return new VentruePassiveMod::VentruePassive();
    }

    MOD_API void uninstall_mod(RC::CppUserModBase* mod) {
        delete mod;
    }
}
