Introducing Golos (alpha)
-----------------

Seed Nodes
----------

    cyberfounder           45.55.217.111:12150                         
    litvintech             104.168.154.160:40696


How to Mine
-----------

The mining algorithm used by Golos requires the owner to have access to the private key
used by the account. This means it does not favor mining pools.

    ./golosd --miner=["accountname","${WIFPRIVATEKEY}"] --witness="accountname" --seed-node="52.38.66.234:2001"

Make sure that your accountname is unique and not already used by someone else or your proof of work
might not be accepted by the blockchain.

OS-specific build instructions
------------------------------

See [here]() for Linux and [here]() for OSX (Apple).

cmake Build Options
--------------------------

### CMAKE_BUILD_TYPE=[Release/Debug]

Specifies whether to build with or without optimization and without or with the symbol table for debugging. Unless you are specifically
debugging or running tests, it is recommended to build as release.

### LOW_MEMORY_NODE=[OFF/ON]

Builds golosd to be a consensus only low memory node. Data and fields not needed for consensus are not stored in the object database.
This option is recommended for witnesses and seed-nodes.

### ENABLE_CONTENT_PATCHING=[ON/OFF]

Allows content to be updated using a patch rather than a complete replacement.
If you do not need an API server or need to see the result of patching content then you can set this to OFF.

### CLEAR_VOTES=[ON/OFF]

Clears old votes from memory that are not longer required for consensus.

### BUILD_GOLOS_TESTNET=[OFF/ON]

Builds golosd for use in a private testnet. Also required for correctly building unit tests
