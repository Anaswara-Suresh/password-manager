#include "vaultstate.h"

namespace {
    // default: vault is locked until app unlocks it
    bool g_vaultUnlocked = false;
}

void VaultState::setUnlocked(bool unlocked)
{
    g_vaultUnlocked = unlocked;
}

bool VaultState::isUnlocked()
{
    return g_vaultUnlocked;
}
