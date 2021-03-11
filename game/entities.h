#pragma
#include <cstdio>
#include <deque>

// librl
#include "entity.h"
#include "gc.h"
#include "game.h"
#include "raycast.h"

// game
#include "enums.h"
#include "inventory.h"

namespace game {

struct ent_player_t : public librl::entity_actor_t {

  static const uint32_t TYPE = ent_type_player;

  ent_player_t(librl::game_t &game);

  int32_t get_accuracy() const override { return librl::entity_actor_t::get_accuracy() + 50; }
  int32_t get_damage() const   override { return librl::entity_actor_t::get_damage() + 10; }
  int32_t get_defense() const  override { return librl::entity_actor_t::get_defense() + 0;  }
  int32_t get_evasion() const  override { return librl::entity_actor_t::get_evasion() + 10; }
  int32_t get_crit() const     override { return librl::entity_actor_t::get_crit() + 2;  }

  void render() override {
    auto &con = game.console_get();
    con.attrib.get(pos.x, pos.y) = colour_player;
    con.chars.get(pos.x, pos.y) = '@';
  }

  void interact_with(entity_t *e);

  bool turn() override;

  void _enumerate(librl::gc_enum_t &func) override;

  uint32_t gold;
  librl::int2 user_dir;
};

struct ent_goblin_t : public librl::entity_actor_t {

  static const uint32_t TYPE = ent_type_goblin;

  ent_goblin_t(librl::game_t &game)
    : librl::entity_actor_t(TYPE, game)
    , seed(game.random())
  {
    name = "goblin";
    hp = 30;
  }

  int32_t get_accuracy() const override { return 40; }
  int32_t get_damage() const   override { return 10; }
  int32_t get_defense() const  override { return 0;  }
  int32_t get_evasion() const  override { return 0;  }
  int32_t get_crit() const     override { return 0;  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, game.walls_get())) {
      con.attrib.get(pos.x, pos.y) = colour_goblin;
      con.chars.get(pos.x, pos.y) = 'g';
    }
  }

  void interact_with(entity_t *e);

  bool turn() override;

  uint64_t seed;
};

struct ent_potion_t : public librl::entity_item_t {

  static const uint32_t TYPE = ent_type_potion;

  ent_potion_t(librl::game_t &game)
    : librl::entity_item_t(TYPE, game, /* can_pickup */ true)
    , recovery(20)
  {
    name = "potion";
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_potion;
      con.chars.get(pos.x, pos.y) = 'p';
    }
  }

  void use_on(entity_t *e) {
    if (e->is_type<ent_player_t>()) {
      game.message_post("%s used %s to recover %u hp", e->name.c_str(),
          name.c_str(), recovery);
      static_cast<ent_player_t*>(e)->hp += recovery;
    }
  }

  const uint32_t recovery;
};

struct ent_stairs_t : public librl::entity_item_t {

  // note: make sure to place where it wont get in the way of a corridor

  static const uint32_t TYPE = ent_type_stairs;

  ent_stairs_t(librl::game_t &game)
    : librl::entity_item_t(TYPE, game, /* can_pickup */ false)
  {
    name = "stairs";
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, game.walls_get())) {
      con.attrib.get(pos.x, pos.y) = colour_stairs;
      con.chars.get(pos.x, pos.y) = '=';
    }
  }

  void use_on(entity_t *e) {
    if (e->is_type<ent_player_t>()) {
      game.message_post("%s proceeds deeper into the dungeon", e->name.c_str());
      game.map_next();
    }
  }
};

struct ent_gold_t : public librl::entity_item_t {

  static const uint32_t TYPE = ent_type_gold;

  ent_gold_t(librl::game_t &game)
    : librl::entity_item_t(TYPE, game, /* can_pickup */ false)
    , seed(game.random())
  {
    name = "gold";
  }

  void render() override {
    auto &con = game.console_get();
    if (game.player) {
      if (librl::raycast(game.player->pos, pos, game.walls_get())) {
        con.attrib.get(pos.x, pos.y) = colour_gold;
        con.chars.get(pos.x, pos.y) = '$';
      }
    }
  }

  void use_on(entity_t *e) {
    if (e->is_type<ent_player_t>()) {
      int value = int(15 + librl::random(seed, 15));
      game.message_post("%s gained %d gold", e->name.c_str(), value);
      static_cast<ent_player_t*>(e)->gold += value;
      game.entity_remove(this);
    }
  }

  uint64_t seed;
};

struct ent_club_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_club;

  ent_club_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "club";
    damage = 4;
    evasion = -1;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

struct ent_mace_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_mace;

  ent_mace_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "mace";
    damage = 6;
    evasion = -1;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

struct ent_sword_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_sword;

  ent_sword_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "sword";
    damage = 8;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

struct ent_dagger_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_dagger;

  ent_dagger_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "dagger";
    damage = 6;
    evasion = 2;;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

struct ent_leather_armour_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_leather_armour;

  ent_leather_armour_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "leather armour";
    defense = 3;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

struct ent_metal_armour_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_leather_armour;

  ent_metal_armour_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "leather armour";
    defense = 6;
    evasion = -2;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

struct ent_cloak_t : public librl::entity_equip_t {

  static const uint32_t TYPE = ent_type_leather_armour;

  ent_cloak_t(librl::game_t &game)
    : librl::entity_equip_t(TYPE, game)
  {
    name = "cloak";
    defense = 1;
    evasion = 4;
  }

  void render() override {
    auto &con = game.console_get();
    if (!game.player) {
      return;
    }
    if (librl::raycast(game.player->pos, pos, 0x1, game.map_get())) {
      con.attrib.get(pos.x, pos.y) = colour_item;
      con.chars.get(pos.x, pos.y) = '?';
    }
  }
};

}  // game
