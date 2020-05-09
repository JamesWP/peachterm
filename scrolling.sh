#Clean redraw
#Write pt length= 1: j
#SDL Child Data Event
#CSI (l) (25 ) [?] : '?25l'
#CSI (r) (1 23 ) [] : '1;23r'
#CSI (H) (23 1 ) [] : '23;1H'
#on_return
#on_newline


printf "\33[1;10r" # set up scrolling from 1st - 10th row
printf "\33[10;1H" # move to 10th row 1st col
printf "\33[2K" # erase line
printf "this is on 9th row\r\n"
printf "this is now on 10th row, other line ends up on 9th"
printf "\33[0J" # erase everything below here
printf "\33[r" # reset scrolling window to whole screen
printf "\33[11;1H" # move to 11th row 1st col



