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
        - data1.txt & data2.txt
        - easy hard coding
        - no REM data when start ESP, how can we get the name directly?
    - log the sample ID(x) log the log file filename(o) <- sample ID is useless, in fact, we only need the filename to open the log file
    - add a state for finish logging -> to notice that not to resume -> also for change the `testState` to true if resume
    - log the stop time and add it back to log file after resume
    - the get wifi at start should be kept
        - cuz we need to get the start time for logging again
        - ~~need to re-think when to disconnect WiFi~~
    - ~~need to think how to get the P2 running time~~
    - ~~need to think how to do if power cut off when doing logging~~
    - ~~need to think how to not to go homing~~
    - ~~need to think how to directly go to monitor screen in TFT~~
    - need to think how to get the accurate motor run time
    - need to think how to go back to that state(in motor task) after resume
- update idea for status:
    - record time for start time (motor run time)(for each cycle start)
        - also add some symbol before time, to notice that the last value is unused
    - append 'a' whenever LS touched -> count the status
    - append 'b' for P2 motor run time -> then cal run time <- appending use another file
    - reset each cycle
    - user input also need to store (store and read first) <- this one use the same file

## UPDATE: the above concept is not practical
- cannot work because of 
    - the limited write/erase cycle
    - log failure due to power failure
- but the following can still use
    - log file to state that should the test resume next time, and if yes, give the status
    - use 2 seperate file for logging these data (lets call it data1.txt, data2.txt)
    - data1.txt is used to read/write const data (same as above)
    - data2.txt is used to read/write the changing data (same concept, but need to change)
- new concept:
    1. use EEPROG or add a super-cap to avoid power failure
    2. only log data2.txt when cut off power
    3. to fasten the logging while cut off power by:
        - directly log the memory (not logging text)
        - use INA219 to check the bus voltage of the device (more spec, motor only)
        - ~~not use task to check power (in task, it needs time delay, which may postpone the time that do logging, also, redundent). Instead, write the value to a pin and use EXT INT to call it~~ update: as INT is not able to do logging, place it at task
- sth new changes:
    1. the motor run time use counting to measure the time because of 
        - with unknown reasons, `status.mototRunTime` cannot copy to `resumeStartTime`
        - in theory, millis() cannot run more than a month
        - thus, plz be aware when changing the timer timing interval
- think the following condition
    - ~~cut off power -> super-cap provide power for few sec -> re-connect to power again~~
    - pause -> cut off power 
    - cut off -> resume -> motor run time start counting before motor running
    - ~~cut off power -> super-cap provide power for few sec -> reach timeout~~