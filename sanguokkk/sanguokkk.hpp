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

   CONTRACT sanguokkk : public contract {
      public:
         using contract::contract;

		   #define ONEDAY 24 * 3600
         #define ONEWEEK 24 * 3600 * 7
         static constexpr symbol EOS_SYMBOL = symbol("EOS", 4);
         static constexpr symbol KKK_SYMBOL = symbol("KKK", 1);
         static constexpr name ACTIVE_PERMISSION{"active"_n};
         static constexpr name EOS_TOKEN_ACCOUNT{"eosio.token"_n};
         static constexpr name KKK_TOKEN_ACCOUNT{"kkkgametoken"_n};
         const asset EMPTY_EOS_ASSET = asset(0, EOS_SYMBOL);
         const asset EMPTY_KKK_ASSET = asset(0, KKK_SYMBOL);
         const time_point_sec INVALID_TIME = time_point_sec(0);

		   ACTION pushnewgame(const name gamecontract, const asset& login_rewards, const asset& invite_rewards, const string& gamememo);

		   ACTION setgblasset(const asset& quantity);

         ACTION frzaccount(const name player);
 
		   ACTION rclaccount(const name player);
         
         ACTION gamelogin(const name player, const name inviter, const name game);

		   ACTION redeem(const name player, const asset& quantity);

         ACTION deferredeem(const name player, const asset& quantity);

         ACTION dailyprofit();
         
		   ACTION profitshare();

         ACTION withdraw(const name player, const asset& quantity);

         ACTION tablereset();

         void transfer(name from, name to, asset quantity, string memo);

      private:
         struct record {
            time_point_sec          redeem_time;
            asset                   quantity;
            record( time_point_sec a, asset b ):redeem_time(a),quantity(b){}
            record(){}

            record& operator-=( const asset& x ) {
               quantity -= x;
               return *this;
            }
            EOSLIB_SERIALIZE(record, (redeem_time)(quantity));
         };

         TABLE gameasset {
            name                    game_contract;
            string                  game_memo;
            asset                   login_rewards;
            asset                   invite_rewards;
            asset                   game_profit;

            uint64_t  primary_key()const { return game_contract.value; }
            EOSLIB_SERIALIZE(gameasset, (game_contract)(game_memo)(login_rewards)(invite_rewards)(game_profit));
         };
         typedef eosio::multi_index< "gameasset"_n, gameasset > gameassets;

         TABLE globalasset {
            asset                   max_crowdfunding;
            asset                   crowdfunding;
            asset                   kkk_deposit;
            asset                   eos_profit;
            asset                   daily_profit_stamp;
            time_point_sec          daily_time_stamp;
            time_point_sec          weekly_time_stamp;

            uint64_t  primary_key()const { return max_crowdfunding.symbol.code().raw(); }
            EOSLIB_SERIALIZE(globalasset, (max_crowdfunding)(crowdfunding)(kkk_deposit)(eos_profit)\
                                          (daily_profit_stamp)(daily_time_stamp)(weekly_time_stamp));
         };
         typedef eosio::multi_index< "globalasset"_n, globalasset > globalassets;

         TABLE account {
            name                    player;
            bool                    account_status;
            bool                    redeem_status;
            asset                   deposit_balance;
            vector<record>          deposit_record;
            asset                   weekly_profit_balance;
            asset                   profit_balance;
 
            uint64_t  primary_key()const { return player.value; }
            EOSLIB_SERIALIZE(account, (player)(account_status)(redeem_status)(deposit_balance)(deposit_record)\
                                      (weekly_profit_balance)(profit_balance));
         };
         typedef eosio::multi_index< "account"_n, account > accounts;
   };
} /// namespace eosio