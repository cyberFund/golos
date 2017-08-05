#pragma once

#include <steemit/chain/evaluators/evaluator.hpp>

#include <steemit/private_message/private_message_operations.hpp>
#include <steemit/private_message/private_message_plugin.hpp>

namespace steemit {
    namespace private_message {

        class private_message_evaluator final : public chain::evaluator_impl<database,private_message_evaluator,private_message_plugin_operation>{
        public:

            using operation_type = private_message_operation;

            template<typename DataBase>
            private_message_evaluator(DataBase& db,private_message_plugin* plugin) :chain::evaluator_impl<database,private_message_evaluator,private_message_plugin_operation>(db),_plugin(plugin){

            }

            void do_apply(const private_message_operation &) ;

            private_message_plugin* _plugin;
        };
        //DEFINE_PLUGIN_EVALUATOR(private_message_plugin, steemit::private_message::private_message_plugin_operation, private_message)

    }
}
