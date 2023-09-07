    this doc descript the new changes in branch UI-bugfix-improvement

## UI
- add width for sample ID input for monitor screen
    - to prevent label `Date & Time:` goes to the first line
- add width for label `Default`
- set the ins widget to similar to the previous one, including:
    - remove the border
    - add underline for label `INSTRUCTION`
    - add highlighting to key label
- set the default text in textarea more clear by:
    - removing the border
    - using text instead of using placeholder
    - set the text in textarea align in middle
- remove the border of the grid
- remove the border of the element in the grid
- set the bg color in row 1

## bugfix
- fix the bug that when input stop condition of P2 >1s and pause when motor run >1s, the motor will run non-stop

## improvement
- the display update the monitor screen when motor start move