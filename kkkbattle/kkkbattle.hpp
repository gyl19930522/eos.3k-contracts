#pragma once
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/transaction.hpp>
#include <string>
#include <vector>
#include "eosio.token.hpp"
#include "kkktoken.hpp"

namespace eosio {
   using std::string;
   using std::vector;

   CONTRACT kkkbattle : public contract {
      public:
         using contract::contract;

         #define ONEWEEK 24 * 3600 * 7
         #define ONEHOUR 3600
         static constexpr symbol EOS_SYMBOL = symbol("EOS", 4);
         static constexpr symbol KKK_SYMBOL = symbol("KKK", 1);
         static constexpr name ACTIVE_PERMISSION{"active"_n};
         static constexpr name EOS_TOKEN_ACCOUNT{"eosio.token"_n};
         static constexpr name KKK_TOKEN_ACCOUNT{"kkkgametoken"_n};
         static constexpr name KKK_TOKEN_ISSUER{"eossanguokkk"_n};
         static constexpr name SPARE_MODE{"spare"_n};
         static constexpr name BASIC_MODE{"basic"_n};
         static constexpr name MIDDLE_MODE{"middle"_n};
         static constexpr name SENIOR_MODE{"senior"_n};
         static constexpr uint64_t INVALID_ID = (1ull << 63) - 1;
         static constexpr uint64_t INVALID_RANK = (1ull << 48) - 1;
         const asset EMPTY_EOS_ASSET = asset(0, EOS_SYMBOL);
         const asset EMPTY_KKK_ASSET = asset(0, KKK_SYMBOL);
         const asset BASIC_BET = asset(1000, EOS_SYMBOL);
         const asset MIDDLE_BET = asset(10000, EOS_SYMBOL);
         const asset SENIOR_BET = asset(100000, EOS_SYMBOL);

		   ACTION frzaccount(const name player);
 
		   ACTION rclaccount(const name player);

		   ACTION setgblpara(const asset& quantity_1, const asset& quantity_2, const asset& quantity_3,
                           const asset& quantity_4, const asset& quantity_5, const asset& quantity_6);

		   ACTION startnewgame(const name player);

		   ACTION startmatch(const name player);

		   ACTION cancelmatch(const name player);

         ACTION matchtimeout(const name player);

		   ACTION automatch(const name gamemode, const asset& minprofix, const asset& maxprofix);

         ACTION manualmatch(const name player_1, const name player_2, const name player_3, const name player_4,
                            const name player_5, const name player_6, const name player_7, const name player_8,
                            const name player_9, const name player_10, const name player_11, const name player_12,
                            const name gamemode, const uint64_t& game_id);

		   ACTION reveal(const uint64_t& rank, const name gamemode, const uint64_t& game_id,
					        const asset& award_1, const asset& award_2, const asset& award_3,
					        const asset& award_4, const asset& award_5, const asset& award_6,
					        const asset& award_7, const asset& award_8, const asset& award_9,
					        const asset& award_10, const asset& award_11, const asset& award_12);

         ACTION autoreward();

         ACTION manualreward(const name player_1, const name player_2, const name player_3, const name player_4, const name player_5,
                             const name player_6, const name player_7, const name player_8, const name player_9, const name player_10,
                             const name player_11, const name player_12, const name player_13, const name player_14, const name player_15,
                             const name player_16, const name player_17, const name player_18, const name player_19, const name player_20,
                             const name player_21, const name player_22, const name player_23, const name player_24, const name player_25);
                             
         ACTION withdraw(const name player, const asset& quantity);

         ACTION tablereset();

         void gamelogin(const name player, const name inviter, const name game);

         void transfer(name from, name to, asset quantity, string memo);

      private:
         TABLE globalpara {
            asset           lower_threshold_1;
            asset           upper_threshold_1;
            asset           lower_threshold_2;
            asset           upper_threshold_2;
            asset           lower_threshold_3;
            asset           lower_threshold_4;
            asset           total_profit;
            asset           weekly_profit_stamp;
            time_point_sec  reward_time_stamp;

            uint64_t  primary_key()const { return lower_threshold_1.symbol.code().raw(); }
            EOSLIB_SERIALIZE(globalpara, (lower_threshold_1)(upper_threshold_1)(lower_threshold_2)\
                                         (upper_threshold_2)(lower_threshold_3)(lower_threshold_4)\
                                         (total_profit)(weekly_profit_stamp)(reward_time_stamp));
         };
         typedef eosio::multi_index< "globalpara"_n, globalpara > globalparas;

         TABLE account {
            name            player;
            name            inviter;
            bool            account_status;
			   uint64_t		    game_number;
            asset           expend_eos_quantity;
            asset           eos_total_profit;
            asset           eos_balance;
            asset           kkk_balance;

            uint64_t  primary_key()const { return player.value; }
			   uint128_t  get_game_number()const { return (static_cast<uint128_t>(game_number) << 64 | static_cast<uint128_t>(player.value)); }
            EOSLIB_SERIALIZE(account, (player)(inviter)(account_status)(game_number)(expend_eos_quantity)(eos_total_profit)(eos_balance)(kkk_balance));
         };
         typedef eosio::multi_index< "account"_n, account,
								            indexed_by< "gamenumber"_n, const_mem_fun<account,
								            uint128_t, &account::get_game_number> >
								           > accounts;

         TABLE playertemp {
            name            player;
            uint64_t        game_id;
            name            gamemode;
            asset           euip_money;
 
            uint64_t  primary_key()const { return player.value; }
            EOSLIB_SERIALIZE(playertemp, (player)(game_id)(gamemode)(euip_money));
         };
         typedef eosio::multi_index< "playertemp"_n, playertemp > playertemps;

         TABLE gamematch {
            name            player;
            time_point_sec  ready_time;
            asset           eos_total_profit;

            uint64_t  primary_key()const { return player.value; }
            uint128_t  get_ready_time()const { return (static_cast<uint128_t>(ready_time.utc_seconds) << 64 | static_cast<uint128_t>(player.value)); }
            uint128_t  get_profit()const { return (static_cast<uint128_t>(eos_total_profit.amount) << 64 | static_cast<uint128_t>(player.value)); }
            EOSLIB_SERIALIZE(gamematch, (player)(ready_time)(eos_total_profit));
         };
         typedef eosio::multi_index< "gamematch"_n, gamematch,
                                    indexed_by< "readytime"_n, const_mem_fun<gamematch, 
                                    uint128_t, &gamematch::get_ready_time> >,
                                    indexed_by< "profit"_n, const_mem_fun<gamematch, 
                                    uint128_t, &gamematch::get_profit> > 
                                   > gamematchs;

         TABLE gameoffer {
            uint64_t        game_id;
            vector<name>    totalplayer;

            uint64_t  primary_key()const { return game_id; }
            EOSLIB_SERIALIZE(gameoffer, (game_id)(totalplayer));
         };
         typedef eosio::multi_index< "gameoffer"_n, gameoffer > gameoffers;

         asset rewarding(const name player, const asset& kkk_award, const uint8_t& rank);
   };
} /// namespace eosio