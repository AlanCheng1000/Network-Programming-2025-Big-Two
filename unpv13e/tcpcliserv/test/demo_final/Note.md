1.  The game logic is almost completed, but further minor adjustments are still necessary.
2.  Cpps related to DB are not completed, but a rough framework has been configured.
3.  Cpps related to DB: 01.cpp, database.cpp.
4.  Cpps for test: 01.cpp.
5.  The functionality of test.cpp is integrated in 01.cpp, so it is removed and no longer used.
6.  Before running 01.cpp ...
    a.  Must be sure to activate MySQL in VM.
    b.  Must be sure to use identical tables, or the query string in database.cpp should be edited.
    c.  Must follow the specific compiling command.
    d.  Must be sure to install packages and adjust the include path. Details are in the Line note.
    e.  Current known issues:
        (1)  When logging in, a repetitive successful login will still be considered valid, leading to a missing player in the following mock game.
        (2)  The match result can be successfully inserted into DB, but the score change fails to work.
7.  It is recommended to run exe files in the PC terminal, like PowerShell or Command Prompt. In the VS Code terminal, it may get stuck and ask for a relaunch.
