    this doc descript the new changes in branch UI-bugfix-improvement

## UI
- add width for sample ID input for monitor screen
    - to prevent label `Date & Time:` goes to the first line
- add width for label `Default`
- set the ins widget to similar to the previous one, including:
    - remove boarder
    - add underline for label `INSTRUCTION`
    - add highlighting to key label
- set the default text in table more clear
    - by using text instead of using placeholder

## bugfix
- fix the bug that when input stop condition of P2 >1s and pause when motor run >1s, the motor will run non-stop