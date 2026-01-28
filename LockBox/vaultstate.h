#pragma once

class VaultState
{
public:
    // Call this from your app when the user unlocks/locks the vault
    static void setUnlocked(bool unlocked);

    // Call this from anywhere (native host, GUI) to know current state
    static bool isUnlocked();
};
