/****************************
* @copyright by Ge Yunlong
* @EOS Three Kingdoms Battle
****************************/
#include "kkkbattle.hpp"

namespace eosio {
	void kkkbattle::frzaccount(const name player)
	{
		require_auth(_self);
		accounts actntable(_self, _self.value);
		const auto& act = actntable.get(player.value, "player has not registered");
		eosio_assert(act.account_status, "player already has no permission yet");
		actntable.modify(act, _self, [&](auto& a) { a.account_status = false; });
	}

	void kkkbattle::rclaccount(const name player)
	{
		require_auth(_self);
		accounts actntable(_self, _self.value);
		const auto& act = actntable.get(player.value, "player has not registered");
		eosio_assert(!act.account_status, "player already has permission");
		actntable.modify(act, _self, [&](auto& a) { a.account_status = true; });
	}

	void kkkbattle::setgblpara(const asset& quantity_1, const asset& quantity_2, const asset& quantity_3,
							   const asset& quantity_4, const asset& quantity_5, const asset& quantity_6)
	{
		require_auth(_self);
		eosio_assert(quantity_1.is_valid() && quantity_2.is_valid() && quantity_3.is_valid() &&
		 			 quantity_4.is_valid() && quantity_5.is_valid() && quantity_6.is_valid(), "invalid quantity");
		eosio_assert(quantity_1.amount > 0 && quantity_2.amount > 0 && quantity_3.amount > 0 &&
					 quantity_4.amount > 0 && quantity_5.amount > 0 && quantity_6.amount > 0, "quantity must be positive");
		eosio_assert(quantity_1.symbol == EOS_SYMBOL && quantity_2.symbol == EOS_SYMBOL && 
					 quantity_3.symbol == EOS_SYMBOL && quantity_4.symbol == EOS_SYMBOL &&
					 quantity_5.symbol == EOS_SYMBOL && quantity_6.symbol == EOS_SYMBOL, "quantity symbol is not EOS");

		globalparas gbptable(_self, _self.value);
		auto gbp = gbptable.find(EOS_SYMBOL.code().raw());
		if (gbp == gbptable.end())
		{
			gbptable.emplace(_self, [&](auto& a) {
				a.lower_threshold_1 = quantity_1;
				a.upper_threshold_1 = quantity_2;
				a.lower_threshold_2 = quantity_3;
				a.upper_threshold_2 = quantity_4;
				a.lower_threshold_3 = quantity_5;
				a.lower_threshold_4 = quantity_6;
				a.total_profit = EMPTY_EOS_ASSET;
				a.weekly_profit_stamp = EMPTY_EOS_ASSET;
				a.reward_time_stamp = time_point_sec(0);
			});
		}
		else
		{
			gbptable.modify(gbp, _self, [&](auto& a) {
				a.lower_threshold_1 = quantity_1;
				a.upper_threshold_1 = quantity_2;
				a.lower_threshold_2 = quantity_3;
				a.upper_threshold_2 = quantity_4;
				a.lower_threshold_3 = quantity_5;
				a.lower_threshold_4 = quantity_6;
			});
		}
	}
	//synchronize from sanguo contracts
	void kkkbattle::gamelogin(const name player, const name inviter, const name game)
	{
		accounts actntable(_self, _self.value);
		auto act = actntable.find(player.value);
		eosio_assert(act == actntable.end(), "player has already registered");
		actntable.emplace(_self, [&](auto& a) {
			a.player = player;
			a.inviter = inviter;
			a.account_status = true;
			a.game_number = 0;
			a.expend_eos_quantity = EMPTY_EOS_ASSET;
			a.eos_total_profit = EMPTY_EOS_ASSET;
			a.eos_balance = EMPTY_EOS_ASSET;
			a.kkk_balance = EMPTY_KKK_ASSET;
		});
		if (inviter != player)
		{
			const auto& actinv = actntable.get(inviter.value, "inviter has not registered yet");
			eosio_assert(actinv.account_status, "inviter has no permission");
		}
	}

	void kkkbattle::startnewgame(const name player)
	{
		require_auth(player);
		accounts actntable(_self, _self.value);
		const auto& act = actntable.get(player.value, "player has not registered");
		globalparas gbptable(_self, _self.value);
		const auto& gbp = gbptable.get(EOS_SYMBOL.code().raw(), "global parameter is not set yet");
		if ((act.eos_total_profit >= act.expend_eos_quantity * 4 &&
			act.expend_eos_quantity >= gbp.lower_threshold_1 &&
			act.expend_eos_quantity < gbp.upper_threshold_1) ||
			(act.eos_total_profit >= act.expend_eos_quantity * 3 &&
			act.expend_eos_quantity >= gbp.lower_threshold_2 &&
			act.expend_eos_quantity < gbp.upper_threshold_2) ||
			(act.eos_total_profit >= act.expend_eos_quantity * 2 &&
			act.expend_eos_quantity >= gbp.lower_threshold_3 ||
			act.eos_total_profit >= gbp.lower_threshold_4))
			actntable.emplace(player, [&](auto& a) { a.account_status = false; });
		eosio_assert(act.account_status, "player has no permission");

		playertemps ptptable(_self, _self.value);
		auto ptp = ptptable.find(player.value);
		eosio_assert(ptp == ptptable.end(), "player does not cancel the last game");
		ptptable.emplace(player, [&](auto& b) {
			b.player = player;
			b.game_id = INVALID_ID;
			b.gamemode = SPARE_MODE;
		});
	}

	void kkkbattle::transfer(name from, name to, asset quantity, string memo)
	{
		if (from == _self || to != _self)
			return;
		if (from == "eosio.stake"_n || from == "eosio.ramfee"_n || from == "eosio.ram"_n)
			return;
		if (from == KKK_TOKEN_ACCOUNT || from == KKK_TOKEN_ISSUER)
			return;

		accounts actntable(_self, _self.value);
		auto act = actntable.find(from.value);
		if (act == actntable.end())
			return;
		if (!(act->account_status))
		{
			print("player has no permission to play the game");
			return;
		}
		playertemps ptptable(_self, _self.value);
		const auto& ptp = ptptable.get(from.value, "player does not start a new game");
		eosio_assert(ptp.game_id == INVALID_ID, "player can not transfer bet or buy equipment during a battle");

		if (quantity.symbol == EOS_SYMBOL)
		{
			if (memo == "bet")
			{
				eosio_assert(ptp.gamemode == SPARE_MODE, "player has already transfered bet");
				ptptable.modify(ptp, _self, [&](auto& a) {
					if (quantity == BASIC_BET)
						a.gamemode = BASIC_MODE;
					else if (quantity == MIDDLE_BET)
						a.gamemode = MIDDLE_MODE;
					else if (quantity == SENIOR_BET)
						a.gamemode = SENIOR_MODE;
					else
						eosio_assert(false, "bet quantity is invalid");
				});
			}
			else if (memo == "equipment")
				ptptable.modify(ptp, _self, [&](auto& a) { a.euip_money = quantity; });
		}
		else if (quantity.symbol == KKK_SYMBOL && memo == "equipment")
			ptptable.modify(ptp, _self, [&](auto& a) { a.euip_money = quantity; });
	}

	void kkkbattle::startmatch(const name player)
	{
		require_auth(player);
		accounts actntable(_self, _self.value);
		const auto& act = actntable.get(player.value, "player has not registered");
		eosio_assert(act.account_status, "player has no permission");

		playertemps ptptable(_self, _self.value);
		const auto& ptp = ptptable.get(player.value, "player does not start a new game");
		eosio_assert(ptp.gamemode == BASIC_MODE || ptp.gamemode == MIDDLE_MODE || ptp.gamemode == SENIOR_MODE, "player has no bet");

		gamematchs gmhtable(_self, ptp.gamemode.value);
		auto gmh = gmhtable.find(player.value);
		eosio_assert(gmh == gmhtable.end(), "player is already in ready");

		gmhtable.emplace(player, [&](auto& a) {
			a.player = player;
			a.ready_time = time_point_sec(now());
			a.eos_total_profit = act.eos_total_profit;
		});
	}

	void kkkbattle::cancelmatch(const name player)
	{
		require_auth(player);
		accounts actntable(_self, _self.value);
		const auto& act = actntable.get(player.value, "player has not registered");
		eosio_assert(act.account_status, "player has no permission");

		playertemps ptptable(_self, _self.value);
		const auto& ptp = ptptable.get(player.value, "player does not start a new game");
		eosio_assert(ptp.gamemode == BASIC_MODE || ptp.gamemode == MIDDLE_MODE || ptp.gamemode == SENIOR_MODE, "gamemode is invalid");
		
		gamematchs gmhtable(_self, ptp.gamemode.value);
		const auto& gmh = gmhtable.get(player.value, "player does not start matching yet");
		gmhtable.erase(gmh);
	}

	void kkkbattle::matchtimeout(const name player)
	{
		require_auth(player);
		accounts actntable(_self, _self.value);
		const auto& act = actntable.get(player.value, "player has not registered");
		eosio_assert(act.account_status, "player has no permission");

		playertemps ptptable(_self, _self.value);
		const auto& ptp = ptptable.get(player.value, "player does not start a new game");
		asset refund = EMPTY_EOS_ASSET;
		if (ptp.gamemode == BASIC_MODE)
			refund = BASIC_BET;
		else if (ptp.gamemode == MIDDLE_MODE)
			refund = MIDDLE_BET;
		else if (ptp.gamemode == SENIOR_MODE)
			refund = SENIOR_BET;
		else
			eosio_assert(false, "player has no bet");
		ptptable.erase(ptp);

		gamematchs gmhtable(_self, ptp.gamemode.value);
		const auto& gmh = gmhtable.get(player.value, "player does not start matching yet");
		eosio_assert(gmh.ready_time <= time_point_sec(now() - ONEHOUR), "matching is not timeout right now");
		gmhtable.erase(gmh);

		INLINE_ACTION_SENDER(eosio::token, transfer)(EOS_TOKEN_ACCOUNT, { {_self, ACTIVE_PERMISSION}, {player, ACTIVE_PERMISSION} }, 
		{ _self, player, refund, "refund EOS for game matching timeout" });
	}

	void kkkbattle::automatch(const name gamemode, const asset& minprofix, const asset& maxprofix)
	{
		require_auth(_self);
		eosio_assert(gamemode == BASIC_MODE || gamemode == MIDDLE_MODE || gamemode == SENIOR_MODE, "gamemode is invalid");
		eosio_assert(minprofix.is_valid() && maxprofix.is_valid(), "invalid minprofix and maxprofix");
		eosio_assert(minprofix.symbol == EOS_SYMBOL && maxprofix.symbol == EOS_SYMBOL, "quantity symbol is not EOS");
		eosio_assert(minprofix.amount >= 0 && maxprofix > minprofix, "profix amount is not correct");

		gamematchs gmhtable(_self, gamemode.value);
		auto idx1 = gmhtable.get_index< "profit"_n >();
		eosio_assert(idx1.lower_bound(static_cast<uint128_t>(minprofix.amount) << 64) != idx1.end(),
					 "profix of current players is less than minprofix");
		uint8_t counter1 = 0;
		for (auto itr1 = idx1.lower_bound(static_cast<uint128_t>(minprofix.amount) << 64);
			 itr1 != idx1.upper_bound(static_cast<uint128_t>(maxprofix.amount) << 64 ) && counter1 < 12; itr1++)
			 counter1++;
		eosio_assert(counter1 >= 12, "players meet profix section are not enough");
		auto idx2 = gmhtable.get_index<"readytime"_n>();
		auto itr2 = idx2.begin();

		uint64_t id = INVALID_ID;
		gameoffers gmotable(_self, gamemode.value);
		gmotable.emplace(_self, [&](auto& a) {
			for (uint8_t counter2 = 1; counter2 <= 12; )
			{
				if (itr2->eos_total_profit >= minprofix && itr2->eos_total_profit <= maxprofix)
				{
					a.totalplayer.push_back(itr2->player);
					if (counter2 == 12)
					{
						a.game_id = gmotable.available_primary_key();
						eosio_assert(a.game_id < INVALID_ID, "game id is invalid");
						id = a.game_id;
					}
					print("player_" + std::to_string(static_cast<uint32_t>(counter2)) + ": " + itr2->player.to_string() + " ");
					counter2++;
				}
				itr2++;
			}
		});

		const auto& gmo = gmotable.get(id);
		playertemps ptptable(_self, _self.value);
		for (uint8_t i = 0; i < 12; i++)
		{
			auto gmh = gmhtable.find(gmo.totalplayer[i].value);
			gmhtable.erase(gmh);
			auto ptp = ptptable.find(gmo.totalplayer[i].value);
			ptptable.modify(ptp, _self, [&](auto& b) { b.game_id = id; });
		}
	}

	void kkkbattle::manualmatch(const name player_1, const name player_2, const name player_3, const name player_4,
								const name player_5, const name player_6, const name player_7, const name player_8,
								const name player_9, const name player_10, const name player_11, const name player_12,
								const name gamemode, const uint64_t& game_id)
	{
		require_auth(_self);
		eosio_assert(gamemode == BASIC_MODE || gamemode == MIDDLE_MODE || gamemode == SENIOR_MODE, "gamemode is invalid");
		eosio_assert(game_id < INVALID_ID, "game id is invalid");

		name playertempset[12];
		playertempset[0] = player_1;
		playertempset[1] = player_2;
		playertempset[2] = player_3;
		playertempset[3] = player_4;
		playertempset[4] = player_5;
		playertempset[5] = player_6;
		playertempset[6] = player_7;
		playertempset[7] = player_8;
		playertempset[8] = player_9;
		playertempset[9] = player_10;
		playertempset[10] = player_11;
		playertempset[11] = player_12;

		gamematchs gmhtable(_self, gamemode.value);
		playertemps ptptable(_self, _self.value);
		for (uint8_t i = 0; i < 12; i++)
		{
			auto gmh = gmhtable.find(playertempset[i].value);
			eosio_assert(gmh != gmhtable.end(), (playertempset[i].to_string() + " is not in ready yet").c_str());
			gmhtable.erase(gmh);
			auto ptp = ptptable.find(playertempset[i].value);
			eosio_assert(ptp != ptptable.end(), (playertempset[i].to_string() + " does not start a new game").c_str());
			ptptable.modify(ptp, _self, [&](auto& b) { b.game_id = game_id; });
		}

		gameoffers gmotable(_self, gamemode.value);
		auto gmo = gmotable.find(game_id);
		eosio_assert(gmo == gmotable.end(), "game id is already exist");
		gmotable.emplace(_self, [&](auto& a) {
			a.game_id = game_id;
			for (uint8_t i = 0; i < 12; i++)
				a.totalplayer.push_back(playertempset[i]);
		});
	}

	void kkkbattle::reveal(const uint64_t& rank, const name gamemode, const uint64_t& game_id,
						   const asset& reward_1, const asset& reward_2, const asset& reward_3,
						   const asset& reward_4, const asset& reward_5, const asset& reward_6,
						   const asset& reward_7, const asset& reward_8, const asset& reward_9,
						   const asset& reward_10, const asset& reward_11, const asset& reward_12)
	{
		require_auth(_self);
		eosio_assert(game_id < INVALID_ID, "game id is invalid");
		eosio_assert(rank < INVALID_RANK, "rank input is invalid");

		const uint8_t ranking[12] = { static_cast<uint8_t>(rank & 0xF), static_cast<uint8_t>(rank >> 4 & 0xF),
									  static_cast<uint8_t>(rank >> 8 & 0xF), static_cast<uint8_t>(rank >> 12 & 0xF),
									  static_cast<uint8_t>(rank >> 16 & 0xF), static_cast<uint8_t>(rank >> 20 & 0xF),
									  static_cast<uint8_t>(rank >> 24 & 0xF), static_cast<uint8_t>(rank >> 28 & 0xF), 
									  static_cast<uint8_t>(rank >> 32 & 0xF), static_cast<uint8_t>(rank >> 36 & 0xF),
									  static_cast<uint8_t>(rank >> 40 & 0xF), static_cast<uint8_t>(rank >> 44 & 0xF) };
		uint8_t ranksum = 0;
		for (uint8_t i = 0; i < 12; i++)
			ranksum += ranking[i];
		eosio_assert(ranksum == 78, "rank input is not correct");

		asset gamebenefits = EMPTY_EOS_ASSET;
		globalparas gbptable(_self, _self.value);
		const auto& gbp = gbptable.get(EOS_SYMBOL.code().raw(), "global parameter is not set yet");
		gameoffers gmotable(_self, gamemode.value);
		const auto& gmo = gmotable.get(game_id, "game offer table does not match game id");
		gamebenefits += rewarding(gmo.totalplayer[0], reward_1, ranking[0]);
		gamebenefits += rewarding(gmo.totalplayer[1], reward_2, ranking[1]);
		gamebenefits += rewarding(gmo.totalplayer[2], reward_3, ranking[2]);
		gamebenefits += rewarding(gmo.totalplayer[3], reward_4, ranking[3]);
		gamebenefits += rewarding(gmo.totalplayer[4], reward_5, ranking[4]);
		gamebenefits += rewarding(gmo.totalplayer[5], reward_6, ranking[5]);
		gamebenefits += rewarding(gmo.totalplayer[6], reward_7, ranking[6]);
		gamebenefits += rewarding(gmo.totalplayer[7], reward_8, ranking[7]);
		gamebenefits += rewarding(gmo.totalplayer[8], reward_9, ranking[8]);
		gamebenefits += rewarding(gmo.totalplayer[9], reward_10, ranking[9]);
		gamebenefits += rewarding(gmo.totalplayer[10], reward_11, ranking[10]);
		gamebenefits += rewarding(gmo.totalplayer[11], reward_12, ranking[11]);
		gmotable.erase(gmo);

		gbptable.modify(gbp, _self, [&](auto& a) { a.total_profit += gamebenefits; });
		INLINE_ACTION_SENDER(eosio::token, transfer)(EOS_TOKEN_ACCOUNT, {_self, ACTIVE_PERMISSION},
		{_self, KKK_TOKEN_ISSUER, gamebenefits * 90 / 100, "gamebenefit"});
	}

	void kkkbattle::autoreward()
	{
		require_auth(_self);
		globalparas gbptable(_self, _self.value);
		const auto& gbp = gbptable.get(EOS_SYMBOL.code().raw(), "global parameter is not set yet");
		eosio_assert(time_point_sec(now() - ONEWEEK + 60) >= gbp.reward_time_stamp, "score reward time interval should exceed 24 hours");
		const auto weekly_profit = (gbp.total_profit - gbp.weekly_profit_stamp) * 10 / 100;
		eosio_assert(weekly_profit.amount > 0, "amount of weekly profit should be positive");
		gbptable.modify(gbp, _self, [&](auto& a) {
			a.weekly_profit_stamp = a.total_profit;
			a.reward_time_stamp = time_point_sec(now());
		});
		accounts actntable(_self, _self.value);
		eosio_assert(actntable.begin() != actntable.end(), "no player has registered yet");
		auto idx = actntable.get_index< "gamenumber"_n >();
		uint32_t counter = 1;
		for (auto itr = idx.rbegin(); itr != idx.rend() && counter <= 25; itr++)
		{
			if (itr->account_status)
			{
				auto acn = actntable.find(itr->player.value);
				actntable.modify(acn, _self, [&](auto& b) {
					if (counter == 1)
						b.eos_balance += weekly_profit * 25 / 100;
					else if (counter == 2)
						b.eos_balance += weekly_profit * 15 / 100;
					else if (counter == 3)
						b.eos_balance += weekly_profit * 10 / 100;
					else if (counter == 4)
						b.eos_balance += weekly_profit * 7 / 100;
					else if (counter == 5)
						b.eos_balance += weekly_profit * 5 / 100;
					else if (counter > 5 && counter <= 10)
						b.eos_balance += weekly_profit * 3 / 100;
					else if (counter > 10 && counter <= 15)
						b.eos_balance += weekly_profit * 2 / 100;
					else if (counter > 15 && counter <= 20)
						b.eos_balance += weekly_profit * 8 / 500;
					else if (counter > 20 && counter <= 25)
						b.eos_balance += weekly_profit * 1 / 100;
				});
				counter++;
			}
		}
	}

	void kkkbattle::manualreward(const name player_1, const name player_2, const name player_3, const name player_4, const name player_5,
								 const name player_6, const name player_7, const name player_8, const name player_9, const name player_10,
								 const name player_11, const name player_12, const name player_13, const name player_14, const name player_15,
								 const name player_16, const name player_17, const name player_18, const name player_19, const name player_20,
								 const name player_21, const name player_22, const name player_23, const name player_24, const name player_25)
	{
		require_auth(_self);
		globalparas gbptable(_self, _self.value);
		const auto& gbp = gbptable.get(EOS_SYMBOL.code().raw(), "global parameter is not set yet");
		eosio_assert(time_point_sec(now() - ONEWEEK + 60) >= gbp.reward_time_stamp, "score reward time interval should exceed 24 hours");
		const auto weekly_profit = (gbp.total_profit - gbp.weekly_profit_stamp) * 10 / 100;
		eosio_assert(weekly_profit.amount > 0, "amount of weekly profit should be positive");
		gbptable.modify(gbp, _self, [&](auto& a) {
			a.weekly_profit_stamp = a.total_profit;
			a.reward_time_stamp = time_point_sec(now());
		});

		name playertempset[25];
		playertempset[0] = player_1;
		playertempset[1] = player_2;
		playertempset[2] = player_3;
		playertempset[3] = player_4;
		playertempset[4] = player_5;
		playertempset[5] = player_6;
		playertempset[6] = player_7;
		playertempset[7] = player_8;
		playertempset[8] = player_9;
		playertempset[9] = player_10;
		playertempset[10] = player_11;
		playertempset[11] = player_12;
		playertempset[12] = player_13;
		playertempset[13] = player_14;
		playertempset[14] = player_15;
		playertempset[15] = player_16;
		playertempset[16] = player_17;
		playertempset[17] = player_18;
		playertempset[18] = player_19;
		playertempset[19] = player_20;
		playertempset[20] = player_21;
		playertempset[21] = player_22;
		playertempset[22] = player_23;
		playertempset[23] = player_24;
		playertempset[24] = player_25;		

		accounts actntable(_self, _self.value);
		for (uint8_t i = 0; i < 25; i++)
		{
			if (playertempset[i] == _self)
				continue;
			auto act = actntable.find(playertempset[i].value);
			eosio_assert(act != actntable.end(), (playertempset[i].to_string() + " has not registered yet").c_str());
			eosio_assert(act->account_status, (playertempset[i].to_string() + " has no permission").c_str());
			actntable.modify(act, _self, [&](auto& b) {
				if (i == 1)
					b.eos_balance += weekly_profit * 25 / 100;
				else if (i == 2)
					b.eos_balance += weekly_profit * 15 / 100;
				else if (i == 3)
					b.eos_balance += weekly_profit * 10 / 100;
				else if (i == 4)
					b.eos_balance += weekly_profit * 7 / 100;
				else if (i == 5)
					b.eos_balance += weekly_profit * 5 / 100;
				else if (i > 5 && i <= 10)
					b.eos_balance += weekly_profit * 3 / 100;
				else if (i > 10 && i <= 15)
					b.eos_balance += weekly_profit * 3 / 100;
				else if (i > 15 && i <= 20)
					b.eos_balance += weekly_profit * 8 / 500;
				else if (i > 20 && i <= 25)
					b.eos_balance += weekly_profit * 1 / 100;
			});
		}
	}

	void kkkbattle::withdraw(const name player, const asset& quantity)
	{
		require_auth(player);
		eosio_assert(quantity.is_valid(), "invalid quantity");
		eosio_assert(quantity.amount > 0, "redeem quantity must be positive");
		accounts acntable(_self, _self.value);
		const auto& acn = acntable.get(player.value, "player has not registered yet");
		eosio_assert(acn.account_status, "player has no permission");
		
		if (quantity.symbol == EOS_SYMBOL)
		{
			eosio_assert(quantity <= acn.eos_balance, "quantity is more than total profit balance");
			acntable.modify(acn, player, [&](auto& a) { a.eos_balance -= quantity; });
			INLINE_ACTION_SENDER(eosio::token, transfer)(EOS_TOKEN_ACCOUNT, { {_self, ACTIVE_PERMISSION}, {player, ACTIVE_PERMISSION} }, 
			{ _self, player, quantity, "EOS profit of KKK battle game" });
		}
		else if (quantity.symbol == KKK_SYMBOL)
		{
			eosio_assert(quantity <= acn.kkk_balance, "quantity is more than total profit balance");
			acntable.modify(acn, player, [&](auto& a) { a.kkk_balance -= quantity; });
			INLINE_ACTION_SENDER(eosio::kkktoken, transfer)(KKK_TOKEN_ACCOUNT, { {_self, ACTIVE_PERMISSION}, {player, ACTIVE_PERMISSION} }, 
			{ _self, player, quantity, "KKK profit of KKK battle game" });
		}
		else
			eosio_assert(false, "quantity symbol is invalid");
	}

	void kkkbattle::tablereset()
	{	
		require_auth(_self);		
		globalparas gbptable(_self, _self.value);
		for (auto gbp = gbptable.begin(); gbp != gbptable.end(); )
		{
			gbptable.erase(gbp);
			gbp = gbptable.begin(); 
		}

		accounts actntable(_self, _self.value);
		for (auto acn = actntable.begin(); acn != actntable.end(); )
		{
			actntable.erase(acn);
			acn = actntable.begin();
		}

		playertemps ptptable(_self, _self.value);
		for (auto ptp = ptptable.begin(); ptp != ptptable.end(); )
		{
			ptptable.erase(ptp);
			ptp = ptptable.begin();
		}

		const name mode[3] = { BASIC_MODE, MIDDLE_MODE, SENIOR_MODE };
		for (uint8_t i = 0; i < 3; i++)
		{
			gamematchs gmhtable(_self, mode[i].value);
			for (auto gmh = gmhtable.begin(); gmh != gmhtable.end(); )
			{
				gmhtable.erase(gmh);
				gmh = gmhtable.begin();
			}

			gameoffers gmotable(_self, mode[i].value);
			for (auto gmo = gmotable.begin(); gmo != gmotable.end(); )
			{
				gmotable.erase(gmo);
				gmo = gmotable.begin();
			}
		}
	}

	asset kkkbattle::rewarding(const name player, const asset& kkk_reward, const uint8_t& rank)
	{
		accounts actntable(_self, _self.value);
		const auto& act = actntable.get(player.value, (player.to_string() + " has not registered yet").c_str());
		eosio_assert(act.account_status, (player.to_string() + " has no permission").c_str());
		playertemps ptptable(_self, _self.value);
		const auto& ptp = ptptable.get(player.value, (player.to_string() + " does not start a new game").c_str());
		auto bet = EMPTY_EOS_ASSET;
		if (ptp.gamemode == BASIC_MODE)
			bet = BASIC_BET;
		else if (ptp.gamemode == MIDDLE_MODE)
			bet = MIDDLE_BET;
		else if (ptp.gamemode == SENIOR_MODE)
			bet = SENIOR_BET;
		else
			eosio_assert(false, (player.to_string() + " has no bet").c_str());

		auto benefits = EMPTY_EOS_ASSET;
		if (ptp.euip_money.symbol == EOS_SYMBOL)
			benefits = bet * 5 / 100 + ptp.euip_money;
		else
			benefits = bet * 5 / 100;

		auto eos_profit = EMPTY_EOS_ASSET;
		if (rank == 1)
			eos_profit += 12 * bet * 40 / 100;
		else if (rank == 2)
			eos_profit += 12 * bet * 25 / 100;
		else if (rank == 3)
			eos_profit += 12 * bet * 15 / 100;
		else if (rank <= 6 && rank >= 4)
			eos_profit += 12 * bet * 9 / 100;
		else if (rank <= 10 && rank >= 7)
			eos_profit += 12 * bet * 6 / 100;

		actntable.modify(act, _self, [&](auto& a) {
			a.game_number++;
			if (ptp.euip_money.symbol == EOS_SYMBOL)
				a.expend_eos_quantity = bet + ptp.euip_money;
			else
				a.expend_eos_quantity = bet;
			a.eos_total_profit += eos_profit;
			a.eos_balance += eos_profit;
			a.kkk_balance += kkk_reward;
		});

		if (act.inviter != player)
		{
			auto actinv = actntable.find(act.inviter.value);
			if (actinv != actntable.end() && actinv->account_status)
				actntable.modify(actinv, _self, [&](auto& c) { c.kkk_balance += kkk_reward * 10 / 100; });
		}
		//delete temp data
		ptptable.erase(ptp);
		return benefits;
	}
}

#define EOSIO_DISPATCH_EX( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
	  auto self = receiver; \
	  if ((code == "eosio.token"_n.value || code == "kkkgametoken"_n.value) && action == "transfer"_n.value) { \
		 eosio::execute_action(eosio::name(self), eosio::name(code), &eosio::kkkbattle::transfer); } \
	  else if (code == "eossanguokkk"_n.value && action == "gamelogin"_n.value) { \
	  	 eosio::execute_action(eosio::name(self), eosio::name(code), &eosio::kkkbattle::gamelogin); } \
      if (code == self) { \
	   switch( action ) { EOSIO_DISPATCH_HELPER( TYPE, MEMBERS ) } } \
   } \
} \
/// @}  dispatcher

EOSIO_DISPATCH_EX(eosio::kkkbattle, (frzaccount)(rclaccount)(setgblpara)(startnewgame)(startmatch)(cancelmatch)(matchtimeout)\
									(automatch)(manualmatch)(reveal)(autoreward)(manualreward)(withdraw)(tablereset))