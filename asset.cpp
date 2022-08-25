#include "randomness_generator.cpp"
#include <stdint.h>

ACTION dapp::registernfts(vector<uint64_t> asset_ids, name owner)
{
  check(has_auth(owner) || has_auth(get_self()), "No authority!");

  name ram_payer = has_auth(get_self()) ? get_self() : owner;

  auto player = user.find(owner.value);
  if (player == user.end())
  {
    regnewplayer(owner, owner);
  }
  player = user.find(owner.value);

  auto config = configs.begin();
  atomicassets::assets_t own_assets = atomicassets::get_assets(owner);

  for (auto asset_id : asset_ids)
  {
    auto asset_itr =
        own_assets.require_find(asset_id, "The player does not own this asset");

    atomicassets::schemas_t collection_schemas =
        atomicassets::get_schemas(asset_itr->collection_name);
    auto schema_itr = collection_schemas.find(asset_itr->schema_name.value);

    atomicassets::templates_t collection_templates =
        atomicassets::get_templates(asset_itr->collection_name);
    auto template_itr = collection_templates.find(asset_itr->template_id);

    vector<uint8_t> immutable_serialized_data =
        template_itr->immutable_serialized_data;

    atomicassets::ATTRIBUTE_MAP idata =
        atomicdata::deserialize(immutable_serialized_data, schema_itr->format);

    vector<uint8_t> mutable_serialized_data =
        asset_itr->mutable_serialized_data;

    atomicassets::ATTRIBUTE_MAP mdata =
        atomicdata::deserialize(mutable_serialized_data, schema_itr->format);

    if (asset_itr->schema_name == name("testdapp"))
    {
      auto charc = charcs.find(asset_id);

      check(charc == charcs.end(), "Already registered");
      {
        check(getnftcount(owner, "charc", "none") < player->nft_counts.maxCharc, "Max Characters have been registered");

        string race = get<string>(idata["Race"]);

        int chance_percent = cfinder(config->race_chance_values, race);
        float chance = config->race_chance_values[chance_percent].value;

        int chance_delay = cfinder(config->race_delay_values, race);
        uint32_t delay =
            (uint32_t)config->race_delay_values[chance_delay].value;

        charcs.emplace(ram_payer, [&](auto &v)
                       {
          v.asset_id = asset_id;
          v.owner = owner;
          v.race = race;
          v.delay_seconds = delay;
          v.last_search.utc_seconds = 0; });
      }
    }

    else if (asset_itr->schema_name == name("employee"))
    {

      auto employee = employees.find(asset_id);
      check(employee == employees.end(), "Already registered");
      if (players != user.end())
      {

        string type = get<string>(idata["Type"]);
        string name = get<string>(idata["name"]);
        uint64_t uses = 60;

        if (type == "userCollector")
        {
          uint64_t maxvalue = 0;
          uses = 60;
          for (auto var : player->nft_counts.maxStaking)
          {
            if (var.key == name)
            {
              maxvalue = var.value;
            }
          }
          check(getnftcount(owner, "stakingType", name) < maxvalue, "Max" + name + "s have been registered");
        }
        else
        {
          if (name != "Token Generation")
            uses = 180;
          else
            uses = 90;
        }
        uint64_t last_used = 0;

        time_point_sec last_time = time_point_sec(last_used);

        auto checkd = mdata.find("Staking left)");
        if (checkd != mdata.end())
          uses = get<uint64_t>(mdata["Staking left)"]);
        else
        {
          mdata["Staking left"] = uses;
          update_assert_on_atomic(owner, asset_id, mdata);
        }

        work.emplace(ram_payer, [&](auto &v)
                            {
          v.asset_id = asset_id;
          v.owner = owner;
          v.type = type;
          v.name = name;
          v.uses_left = uses; });
      }
    }
    else if (asset_itr->schema_name == name("object"))
    {
      auto item = items.find(asset_id);

      check(item == items.end(), "Already registered");

      string type = get<string>(idata["Type"]);
      string name = get<string>(idata["name"]);
      string rarity = get<string>(idata["Rarity"]);

      uint64_t uses = 60;
      uint64_t last_used = 0;

      time_point_sec last_time = time_point_sec(last_used);

      auto checkd = mdata.find("Resilience left)");
      if (checkd != mdata.end())
        uint64_t uses = get<uint64_t>(mdata["Resilience left)"]);
      else
      {
        mdata["Resilience left)"] = uses;
        update_assert_on_atomic(owner, asset_id, mdata);
      }
      pair_string_float32 temp;
      temp.value = rarity == "Rare" ? 5 : rarity == "Epic" ? 10
                                                                 : 15;
      uint32_t t_id = asset_itr->template_id;
      auto itemcombo = config->item_combo;
      string ty = "";
      for (auto var : itemcombo)
      {
        if (var.template_id == t_id)
        {
          ty = var.type;
        }
      }
      temp.key = ty;
      items.emplace(ram_payer, [&](auto &v)
                    {
          v.asset_id = asset_id;
          v.owner = owner;
          v.name = name;
          v.staking=0;
          v.equipped = false;
          v.uses_left = uses; });
    }
    else
    {
      check(false, "Asset ID is Invalid");
    }
  }
}
