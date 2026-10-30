/*
 * Copyright 2026 David Xanatos, xanasoft.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _SBIE_SERVICE_RUNTIME_RULE_HELPERS_H
#define _SBIE_SERVICE_RUNTIME_RULE_HELPERS_H

#include "common/program_control_runtime.h"

BOOLEAN SbieSvc_LoadRuntimeRulesetForSetting(
    const WCHAR* boxname,
    const WCHAR* setting,
    int useRuleExtensions,
    BreakoutAdjustRuleFn adjustRule,
    void* adjustContext,
    SBIE_RT_RULESET* ruleset);

#endif
