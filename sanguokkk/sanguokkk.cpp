/*********************************************
* @copyright by Ge Yunlong
* @EOS Three Kingdoms series platform contract
**********************************************/
#include "sanguokkk.hpp"

namespace eosio {
	//add a new game contract
	void sanguokkk::pushnewgame(const name gamecontract, const asset& login_rewards, const asset& invite_rewards, const string& gamememo)
	{
		require_auth(_self);
		eosio_assert(is_account(gamecontract), "gamecontract is not exist");
		eosio_assert(gamememo.size() <= 256, "gamememo has more than 256 bytes");
		eosio_assert(login_rewards.is_valid() && invite_rewards.is_valid(), "invalid register rewards");
		eosio_assert(login_rewards.amount > 0 && invite_rewards.amount > 0, "register rewards must be positive");
		eosio_assert(login_rewards.symbol == KKK_SYMBOL && invite_rewards.symbol == KKK_SYMBOL, "register rewards symbol must be KKK");

		gameassets gmatable(_self, _self.value);
		auto gma = gmatable.find(gamecontract.value);
		if (gma == gmatable.end())
		{
			gmatable.emplace(_self, [&](auto& a) {
				a.game_contract = gamecontract;
				a.game_memo = gamememo;
				a.login_rewards = login_rewards;
				a.invite_rewards = invite_rewards;
				a.game_profit = EMPTY_EOS_ASSET;
			});
		}
		else
		{
			gmatable.modify(gma, _self, [&](auto& a) {
				a.game_memo = gamememo; 
				a.login_rewards = login_rewards;
				a.invite_rewards = invite_rewards;
			});
		}
	}
	//set max crowdfunding amount
	void sanguokkk::setgblasset(const asset& quantity)
	{
		require_auth(_self);
		eosio_assert(quantity.is_valid(), "invalid quantity");
		eosio_assert(quantity.amount > 0, "quantity must be positive");
		eosio_assert(quantity.symbol == EOS_SYMBOL, "quantity symbol must be EOS");

		globalassets gbatable(_self, _self.value);
		auto gba = gbatable.find(quantity.symbol.code().raw());
		if (gba == gbatable.end())
			gbatable.emplace(_self, [&](auto& a) {
				a.max_crowdfunding = quantity;
				a.crowdfunding = EMPTY_EOS_ASSET;
				a.kkk_deposit = EMPTY_KKK_ASSET;
				a.eos_profit = EMPTY_EOS_ASSET;
				a.daily_profit_stamp = EMPTY_EOS_ASSET;
				a.daily_time_stamp = time_point_sec(0);
				a.weekly_time_stamp = time_point_sec(0);
			});
		else
			gbatable.modify(gba, _self, [&](auto& a) { a.max_crowdfunding = quantity; });
	}
	//freeze player account
	void sanguokkk::frzaccount(const name player)
	{
		require_auth(_self);
		accounts acntable(_self, _self.value);
		const auto& acn = acntable.get(player.value, "player has not registered");
		eosio_assert(acn.account_status, "player already has no permission yet");
		acntable.modify(acn, _self, [&](auto& a) { a.account_status = false; });
	}
	//recall player account
	void sanguokkk::rclaccount(const name player)
	{
		require_auth(_self);
		accounts acntable(_self, _self.value);
		const auto& acn = acntable.get(player.value, "player has not registered");
		eosio_assert(!(acn.account_status), "player already has permission");
		acntable.modify(acn, _self, [&](auto& a) { a.account_status = true; });
	}
	//player register for game
	void sanguokkk::gamelogin(const name player, const name inviter, const name game)
	{
		require_auth(player);
		eosio_assert(is_account(inviter), "inviter is not exist");
		eosio_assert(is_account(game), "game contract account is not exist");
		gameassets gmatable(_self, _self.value);
		const auto& gma = gmatable.get(game.value, "game is not pushed out yet");
		//update player's information
		accounts acntable(_self, _self.value);
		auto acn = acntable.find(player.value);
		if (acn == acntable.end())
		{
			acntable.emplace(player, [&](auto& a) { 
				a.player = player;
				a.account_status = true;
				a.redeem_status = false;
				a.deposit_balance = EMPTY_KKK_ASSET;
				a.weekly_profit_balance = EMPTY_EOS_ASSET;
				a.profit_balance = EMPTY_EOS_ASSET;
			});
		}
		else
			eosio_assert(acn->account_status, "player has no permission");
		//send login and inviting rewards
		INLINE_ACTION_SENDER(eosio::kkktoken, transfer)(KKK_TOKEN_ACCOUNT, { {_self, ACTIVE_PERMISSION}, {player, ACTIVE_PERMISSION} }, 
		{ _self, player, gma.login_rewards, "rewards for registering three kingdoms series game: " + gma.game_memo });

		if (player != inviter)
		{
			const auto& acninv = acntable.get(inviter.value, "inviter has not registered yet");
			eosio_assert(acninv.account_status, "inviter has no permission");
			INLINE_ACTION_SENDER(eosio::kkktoken, transfer)(KKK_TOKEN_ACCOUNT, { {_self, ACTIVE_PERMISSION}, {player, ACTIVE_PERMISSION} }, 
			{ _self, inviter, gma.invite_rewards, "rewards for inviting " + player.to_string() + " for registering game: " + gma.game_memo });
		}
		require_recipient(game);
	}
	//synchronize from token contract
	void sanguokkk::transfer(name from, name to, asset quantity, string memo)
	{
		if (from == _self || to != _self)
			return;
		if (from == "eosio.stake"_n || from == "eosio.ramfee"_n || from == "eosio.ram"_n)
			return;
		if (from == KKK_TOKEN_ACCOUNT)
			return;
		//deposit KKK
		if (quantity.symbol == KKK_SYMBOL && memo == "deposit")
		{
			eosio_assert(quantity.amount % 1000 == 0 && quantity.amount >= 1000, "deposit KKK amount nust be hundredfold");

			accounts acntable(_self, _self.value);
			const auto& acn = acntable.get(from.value, "player has not registerd yet");
			if (!(acn.account_status))
			{
				print("player has no permission for deposit");
				return;
			}
			acntable.modify(acn, _self, [&](auto& a) {
				a.deposit_balance += quantity;
				a.deposit_record.emplace_back(time_point_sec(now()), quantity);
			});

			globalassets gbatable(_self, _self.value);
			const auto& gba = gbatable.get(EOS_SYMBOL.code().raw(), "global asset has not been set yet");
			gbatable.modify(gba, _self, [&](auto& b) { b.kkk_deposit += quantity; });
		}
		//team rewards recording
		else if (quantity.symbol == EOS_SYMBOL && memo == "gamebenefit")
		{
			gameassets gmatable(_self, _self.value);
			auto gma = gmatable.find(from.value);
			if (gma != gmatable.end())
				gmatable.modify(gma, _self, [&](auto& a) { a.game_profit += quantity; });

			globalassets gbatable(_self, _self.value);
			auto gba = gbatable.find(EOS_SYMBOL.code().raw());
			if (gba != gbatable.end())
				gbatable.modify(gba, _self, [&](auto& b) { b.eos_profit += quantity; });
		}
		//crowdfunding
		else if (quantity.symbol == EOS_SYMBOL && memo == "crowdfunding")
		{
			eosio_assert(quantity.amount >= 1000, "crowdfunding quantity is at least 0.1 EOS");
			accounts acntable(_self, _self.value);
			auto acn = acntable.find(from.value);
			if (acn != acntable.end()) 
			{
				if (!(acn->account_status))
				{
					print("player has no permission for crowdfunding");
					return; 
				}
			}
			globalassets gbatable(_self, _self.value);
			const auto& gba = gbatable.get(EOS_SYMBOL.code().raw(), "global asset has not been set yet");
			const auto kkkbalance = token::get_balance(KKK_TOKEN_ACCOUNT, _self, KKK_SYMBOL.code());
			const auto kkk_quantity = asset(quantity.amount, KKK_SYMBOL);
			eosio_assert(kkkbalance >= kkk_quantity, "KKK balance of issuer is not enough");
			eosio_assert(gba.crowdfunding + quantity <= gba.max_crowdfunding, "crowdfunding total quantity reaches upper limit");

			gbatable.modify(gba, _self, [&](auto& a) { a.crowdfunding += quantity; });
			INLINE_ACTION_SENDER(eosio::kkktoken, transfer)(KKK_TOKEN_ACCOUNT, { _self, ACTIVE_PERMISSION },
			{ _self, from, asset(quantity.amount, KKK_SYMBOL), "KKK token for crowdfunding" });
		}
	}
	//redeem for deposit KKK
	void sanguokkk::redeem(const name player, const asset& quantity)
	{
		require_auth(player);
		eosio_assert(quantity.is_valid(), "invalid quantity");
		eosio_assert(quantity.symbol == KKK_SYMBOL, "quantity symbol is not KKK");
		eosio_assert(quantity.amount % 1000 == 0 && quantity.amount >= 1000, "redeem quantity must be hundredfold");

		globalassets gbatable(_self, _self.value);
		const auto& gba = gbatable.get(EOS_SYMBOL.code().raw(), "global asset has not been set yet");
		gbatable.modify(gba, player, [&](auto& a) { a.kkk_deposit -= quantity; });

		accounts acntable(_self, _self.value);
		const auto& acn = acntable.get(player.value, "player has not registered yet");
		eosio_assert(acn.account_status, "player has no permission");
		eosio_assert(quantity <= acn.deposit_balance, "quantity is more than deposit balance");
		eosio_assert(!(acn.deposit_record.empty()), "player has no deposit record");
		auto balance = quantity;
		acntable.modify(acn, player, [&](auto& b) {
			b.deposit_balance -= quantity;
			while (!(b.deposit_record.empty()) && b.deposit_record.back().quantity <= balance)
			{
				b.redeem_status = true;
				balance -= b.deposit_record.back().quantity;
				b.deposit_record.pop_back();
			}
			if (balance.amount > 0)
				b.deposit_record.back() -= balance;
		});
		//delay 24 hours to transfer deposit KKK
		std::vector<permission_level> auths;
		auths.emplace_back(_self, ACTIVE_PERMISSION);
		auths.emplace_back(player, ACTIVE_PERMISSION);
		transaction red;
		red.actions.emplace_back(auths, _self, "deferredeem"_n, std::make_tuple(player, quantity));
		red.delay_sec = ONEDAY;
		red.send(static_cast<uint128_t>(time_point_sec(now()).utc_seconds) << 64 | static_cast<uint128_t>(player.value), player);
	}
	//deferred action for modifying globalasset and account table during redeem
	void sanguokkk::deferredeem(const name player, const asset& quantity)
	{
		require_auth(_self);
		auto payer = has_auth(player) ? player : _self;

		accounts acntable(_self, _self.value);
		const auto& acn = acntable.get(player.value);
		if (acn.redeem_status)
		{
			acntable.modify(acn, _self, [&](auto& a) { a.redeem_status = false; });
			if (has_auth(player))
			{
				INLINE_ACTION_SENDER(eosio::kkktoken, transfer)(KKK_TOKEN_ACCOUNT, { {_self, ACTIVE_PERMISSION}, {player, ACTIVE_PERMISSION} }, 
				{ _self, player, quantity, "redeem KKK token" });
			}
			else
			{
				INLINE_ACTION_SENDER(eosio::kkktoken, transfer)(KKK_TOKEN_ACCOUNT, { _self, ACTIVE_PERMISSION },
				{ _self, player, quantity, "redeem KKK token" });
			}
		}
		else
		{
			acntable.modify(acn, _self, [&](auto& a) {
				a.account_status = false;
				a.redeem_status = false;
			});
		}
	}
	//caculate daily profit of each player owned KKK
	void sanguokkk::dailyprofit()
	{
		require_auth(_self);
		globalassets gbatable(_self, _self.value);
		const auto& gba = gbatable.get(EOS_SYMBOL.code().raw(), "global asset has not been set yet");
		const auto judge_time = time_point_sec(now() - ONEDAY + 60);
		eosio_assert(judge_time >= gba.daily_time_stamp, "profit caculate time interval should exceed 24 hours");
		const auto daily_profit = gba.eos_profit - gba.daily_profit_stamp;
		eosio_assert(daily_profit.amount > 0, "amount of daily profit should greater than 0");
		gbatable.modify(gba, _self, [&](auto& a) {
			a.daily_profit_stamp = a.eos_profit;
			a.daily_time_stamp = time_point_sec(now());
		});

		accounts acntable(_self, _self.value);
		eosio_assert(acntable.begin() != acntable.end(), "no player has registered yet");
		vector<asset> deposit_quantity;
		vector<name> deposit_player;
		for (auto acn = acntable.begin(); acn != acntable.end(); acn++)
		{
			auto each_player_deposit = EMPTY_KKK_ASSET;
			if (acn->account_status)
			{
				for (uint32_t i = 0; i < acn->deposit_record.size(); i++)
				{
					if (acn->deposit_record[i].redeem_time <= judge_time)
						each_player_deposit += acn->deposit_record[i].quantity;
				}
			}
			if (each_player_deposit.amount > 0)
			{
				deposit_quantity.emplace_back(each_player_deposit.amount, KKK_SYMBOL);
				deposit_player.emplace_back(acn->player.value);
			}
		}
		eosio_assert(!(deposit_player.empty()) || !(deposit_quantity.empty()), "no player has daily profit yet");

		auto total_deposit = EMPTY_KKK_ASSET;
		for (uint32_t j = 0; j < deposit_quantity.size(); j++)
			total_deposit += deposit_quantity[j];
		const int64_t rate_2 = (total_deposit / 1000).amount;
		for (auto idx = acntable.begin(); idx != acntable.end(); idx++)
		{
			if (idx->account_status && idx->player == deposit_player.front())
			{
				int64_t rate_1 = (deposit_quantity.front() / 1000).amount;
				acntable.modify(idx, _self, [&](auto& a) { a.weekly_profit_balance += (daily_profit * 25 / 100) * rate_1 / rate_2; });
				deposit_player.erase(deposit_player.begin());
				deposit_quantity.erase(deposit_quantity.begin());
			}
		}
	}
	//share EOS profit for those who have deposit KKK, call this function each week once
	void sanguokkk::profitshare()
	{
		require_auth(_self);
		globalassets gbatable(_self, _self.value);
		const auto& gba = gbatable.get(EOS_SYMBOL.code().raw(), "global asset has not been set yet");
		eosio_assert(time_point_sec(now() - ONEWEEK + 60) >= gba.weekly_time_stamp, "profit share time interval should exceed 1 week");
		gbatable.modify(gba, _self, [&](auto& a) { a.weekly_time_stamp = time_point_sec(now()); });
		
		accounts acntable(_self, _self.value);
		eosio_assert(acntable.begin() != acntable.end(), "no player has registered yet");
		for (auto acn = acntable.begin(); acn != acntable.end(); acn++)
		{
			if (acn->account_status && acn->weekly_profit_balance.amount > 0)
			{
				acntable.modify(acn, _self, [&](auto& a) {
					a.profit_balance += a.weekly_profit_balance;
					a.weekly_profit_balance = EMPTY_EOS_ASSET;
				});
			}
		}
	}

	void sanguokkk::withdraw(const name player, const asset& quantity)
	{
		require_auth(player);
		eosio_assert(quantity.is_valid(), "invalid quantity");
		eosio_assert(quantity.symbol == EOS_SYMBOL, "quantity symbol is not EOS");
		eosio_assert(quantity.amount > 0, "redeem quantity must be positive");

		accounts acntable(_self, _self.value);
		const auto& acn = acntable.get(player.value, "player has not registered yet");
		eosio_assert(acn.account_status, "player has no permission");
		eosio_assert(quantity <= acn.profit_balance, "quantity is more than total profit balance");
		acntable.modify(acn, player, [&](auto& a) { a.profit_balance -= quantity; });
		
		INLINE_ACTION_SENDER(eosio::token, transfer)(EOS_TOKEN_ACCOUNT, { {_self, ACTIVE_PERMISSION}, {player, ACTIVE_PERMISSION} }, 
		{ _self, player, quantity, "EOS profit share for deposit KKK" });
	}

	void sanguokkk::tablereset()
	{
		require_auth(_self);
		gameassets gmatable(_self, _self.value);
		for (auto gma = gmatable.begin(); gma != gmatable.end(); )
		{
			gmatable.erase(gma);
			gma = gmatable.begin();
		}

		globalassets gbatable(_self, _self.value);
		for (auto gba = gbatable.begin(); gba != gbatable.end(); )
		{
			gbatable.erase(gba);
			gba = gbatable.begin(); 
		}

		accounts acntable(_self, _self.value);
		for (auto acn = acntable.begin(); acn != acntable.end(); )
		{
			acntable.erase(acn);
			acn = acntable.begin();
		}
	}
}

#define EOSIO_DISPATCH_EX( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
	  auto self = receiver; \
	  if ((code == "eosio.token"_n.value || code == "kkkgametoken"_n.value) && action == "transfer"_n.value) { \
		 eosio::execute_action(eosio::name(self), eosio::name(code), &eosio::sanguokkk::transfer); } \
      if (code == self) { \
	   switch( action ) { EOSIO_DISPATCH_HELPER( TYPE, MEMBERS ) } } \
   } \
} \
/// @}  dispatcher

EOSIO_DISPATCH_EX(eosio::sanguokkk, (pushnewgame)(setgblasset)(frzaccount)(rclaccount)(gamelogin)(redeem)\
									(deferredeem)(dailyprofit)(profitshare)(withdraw)(tablereset))