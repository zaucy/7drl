#pragma once
#include <cstdint>
#include <string>

#include "gc.h"
#include "common.h"
#include "inventory.h"

namespace librl {

struct game_t;

enum subclass_t {
  ent_subclass_actor,
  ent_subclass_item,
  ent_subclass_equip,
};

struct entity_t : librl::gc_base_t {

  entity_t(const uint32_t type, const uint32_t subclass, game_t &game)
    : order(0)
    , pos(int2{-1, -1})
    , type(type)
    , subclass(subclass)
    , game(game)
  {
  }

  virtual void render() {
    // dummy
  }

  virtual bool turn() = 0;

  template <typename type_t>
  bool is_type() const {
    return type_t::TYPE == type;
  }

  template <typename type_t>
  bool is_subclass() const {
    return type_t::SUBCLASS == subclass;
  }

  uint64_t order;
  int2 pos;
  const uint32_t type;
  const uint32_t subclass;
  std::string name;

protected:
  game_t &game;
};

struct entity_actor_t : public entity_t {

  static const subclass_t SUBCLASS = ent_subclass_actor;

  entity_actor_t(const uint32_t type, game_t &game)
    : entity_t(type, SUBCLASS, game)
  {
  }

  virtual int32_t get_damage() const   { return inventory.get_damage(); }
  virtual int32_t get_defense() const  { return inventory.get_defense(); }
  virtual int32_t get_accuracy() const { return inventory.get_accuracy(); }
  virtual int32_t get_evasion() const  { return inventory.get_evasion(); }
  virtual int32_t get_crit() const     { return inventory.get_crit(); }

  virtual void attack(entity_actor_t *target);

  virtual void kill();

  int32_t hp;
  inventory_t inventory;
};

struct entity_item_t : public entity_t {

  static const subclass_t SUBCLASS = ent_subclass_item;

  entity_item_t(const uint32_t type, game_t &game, bool can_pickup)
    : entity_t(type, SUBCLASS, game)
    , can_pickup(can_pickup)
  {
  }

  bool turn() override {
    return true;
  }

  virtual void use_on(entity_t *e) = 0;

  virtual void picked_up(entity_t *by);

  bool can_pickup;
};

struct entity_equip_t : public entity_t {

  static const subclass_t SUBCLASS = ent_subclass_equip;

  entity_equip_t(const uint32_t type, game_t &game)
    : entity_t(type, SUBCLASS, game)
    , damage(0)
    , accuracy(0)
    , evasion(0)
    , defense(0)
    , crit(0)
  {
  }

  bool turn() override {
    return true;
  }

  virtual void picked_up(entity_t *by);

  int damage;
  int accuracy;
  int evasion;
  int defense;
  int crit;
};

} // librl
