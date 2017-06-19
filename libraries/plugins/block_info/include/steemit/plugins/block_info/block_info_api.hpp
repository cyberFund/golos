
#pragma once

#include <fc/api.hpp>

#include <steemit/plugins/block_info/block_info.hpp>

namespace steemit {
    namespace application {
        struct api_context;
    }
}

namespace steemit {
    namespace plugin {
        namespace block_info {

            struct get_block_info_args {
                uint32_t start_block_num = 0;
                uint32_t count = 1000;
            };

            class block_info_api: public std::enable_shared_from_this<block_info_api> {
            public:
                block_info_api(const steemit::application::api_context &ctx);
                ~block_info_api();
                void on_api_startup();

                std::vector<block_info> get_block_info(get_block_info_args args);

                std::vector<block_with_info> get_blocks_with_info(get_block_info_args args);

            private:
                struct block_info_api_impl;
                std::unique_ptr<block_info_api_impl> my;
            };

        }
    }
}

FC_REFLECT(steemit::plugin::block_info::get_block_info_args,
        (start_block_num)
                (count)
)

FC_API(steemit::plugin::block_info::block_info_api,
        (get_block_info)
                (get_blocks_with_info)
)
