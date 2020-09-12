/*
 * Copyright (c) 2020 LcfTrans authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#ifndef LCFTRANS_TYPES
#define LCFTRANS_TYPES

#include <lcf/context.h>
#include <lcf/rpg/fwd.h>
#include <lcf/rpg/eventcommand.h>

using Cmd = lcf::rpg::EventCommand::Code;
constexpr int lines_per_message = 4;

template <typename T>
using any_event_ctx = const lcf::Context<lcf::rpg::EventCommand, T>;

template <typename T>
using map_event_ctx = const lcf::Context<lcf::rpg::EventCommand,
lcf::Context<lcf::rpg::EventPage,
		lcf::Context<lcf::rpg::Event, T>>>;

template <typename T>
using common_event_ctx = const lcf::Context<lcf::rpg::EventCommand,
lcf::Context<lcf::rpg::CommonEvent, T>>;

template <typename T>
using battle_event_ctx = lcf::Context<lcf::rpg::EventCommand,
lcf::Context<lcf::rpg::TroopPage,
		lcf::Context<lcf::rpg::Troop, T>>>;

#endif

