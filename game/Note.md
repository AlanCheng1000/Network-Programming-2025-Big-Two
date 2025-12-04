1.  The game logic is almost completed, but further minor adjustments are still necessary.
2.  Cpps related to DB are not completed, but a rough framework has been configured.
3.  Cpps related to DB: 01.cpp, database.cpp.
4.  Cpps for test: 01.cpp, test.cpp.
5.  Before running test.cpp ...\n
    a.  Must follow the specific compiling command.\n
    b.  The mixed agents are not implemented yet. Currently, in a game, all players are human, or all are bots, i.e., they are just automatically playing cards, no other algorithms implemented.\n
    c.  Must check Game.cpp, line 30, "players.emplace_back(std::make_unique<XXXXXPlayer>(i));". It determines which one of the agents is compiled.\n
6.  Before running 01.cpp ...\n
    a.  Must be sure to activate MySQL in VM.\n
    b.  Must be sure to use identical tables, or the query string in database.cpp should be edited.\n
    c.  Must follow the specific compiling command.\n
    d.  Must be sure to install packages and adjust the include path. Details are in the Line note.\n
    e.  Current known issues:\n
        (1)  When logging in, a repetitive successful login will still be considered valid, leading to a missing player in the following mock game.\n
        (2)  The match result can be successfully inserted into DB, but the score change fails to work.\n
7.  It is recommended to run exe files in the PC terminal, like PowerShell or Command Prompt. In the VS Code terminal, it may get stuck and ask for a relaunch.
