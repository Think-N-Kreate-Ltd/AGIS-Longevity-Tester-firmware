    this doc descript the new changes in branch UI-bugfix-improvement

## add new logging file for cut off -> resume
- reason:
    - the power must br totally cut off <- by Ting
    - thus, data in REM will disappear -> cannot get status from var
    - the only method is to store data in other place
    - while using DB or SD card is impractical
    - thus, the only way is to use FS, where LittleFS is better than SPIFFS
- should not use the log file because:
    - how to directly go to last line
    - especially we need to log to many days, the size of log file is too large
- prototype idea: 
    - if finished -> log status = finish ONLY
    - if not -> log filename + stop time + status(main file var L25-44, 58-63) + ?
    - new file with different name with log file (must)
    - the name had better be the same for all testing because:
        - easy hard coding
        - no REM data when start ESP, how can we get the name directly?
    - log the sample ID(x) log the log file filename(o) <- sample ID is useless, in fact, we only need the filename to open the log file
    - add a state for finish logging -> to notice that not to resume -> also for change the `testState` to true if resume
    - log the stop time and add it back to log file after resume
    - the get wifi at start should be kept
        - cuz we need to get the start time for logging again
        - need to re-think when to disconnect WiFi
    - need to think how to get the P2 running time
    - need to think how to do if power cut off when doing logging
    - need to think how to not to go homing
    - need to think how to directly go to monitor screen in TFT