#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

using namespace eosio;


class [[eosio::contract("supertaskeos")]] supertaskeos : public eosio::contract {
  private:
    const name admin;
    const symbol token_symbol;
    name node;
    name seller;
    name buyer;
    bool initiated;
    bool dispatched;
    bool received;

    struct [[eosio::table]] balance
    {
      eosio::asset funds;
      uint64_t primary_key() const { return funds.symbol.raw(); }
    };

    using balance_table = eosio::multi_index<"balance"_n, balance>;


  public:
    using contract::contract;

    supertaskeos(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds),token_symbol("EOS", 4),admin("desmondlieos"){}

    [[eosio::on_notify("eosio.token::transfer")]]
    void deposit(name username, name to, eosio::asset quantity, std::string memo) {
      if (username == admin || to != admin)
      {
        return;
      }
      //check(node != username, "You have not appointed a proxy yet");
      check(quantity.symbol == token_symbol, "This is not an accepted form of payment.");

      balance_table balance(admin, node.value);
      auto stake_holder = balance.find(token_symbol.raw());

      balance.emplace(admin, [&](auto &row) {
        row.funds = quantity;
      });

    }

    [[eosio::action]]

    void initialise(name username, name Seller, name Buyer)
    {
      require_auth(username);
      check(username == admin, "You are not the admin.");
      initiated = true;
      node = Seller;
      seller = Seller;
      buyer = Buyer;
    }

    [[eosio::action]]

    void appoint(name username, name proxy)
    {
      require_auth(username);
      //check(username == node, "You do not have the required authority.");
      node = proxy;
    }

    [[eosio::action]]

    void dispatching(name username)
    {
      require_auth(username);
      //check(username == seller, "You are not the seller.");
      //check(node != seller, "You have not appoint a proxy yet.");
      dispatched = true;
    }

    [[eosio::action]]

    void confirmation(name username)
    {
      require_auth(username);
      //check(node == buyer, "Your item has not arrived yet.");
      node = admin;
      seller = admin;
      buyer = admin;
      initiated = false;
      dispatched = false;
      received = false;
    }

    [[eosio::action]]

    void withdrawal(name username)
    {

      require_auth(username);
      //check(received ==  true, "The item has not been delivered yet.");

      balance_table balance(admin, username.value);
      auto stake_holder = balance.find(token_symbol.raw());


      check(stake_holder != balance.end(), "You are not on the payment list.");

      action{
        permission_level{admin, "active"_n},
        "eosio.token"_n,
        "transfer"_n,
        std::make_tuple(admin, username, stake_holder->funds, std::string("Thank you for using our tracking service!"))
      }.send();

      balance.erase(stake_holder);
    }
};