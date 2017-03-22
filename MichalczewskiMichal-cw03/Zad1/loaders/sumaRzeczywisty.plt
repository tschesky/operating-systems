set grid
set term png
set output "../plots/sumaRzeczywisty.png"
set terminal png size 1024,768
set title "Suma procesów: czas rzeczywisty"
plot '../results/fork.txt' using 1:12 with line title "fork",\
'../results/vfork.txt' using 1:12 with line title "vfork",\
'../results/forkClone.txt' using 1:12 with line title "forkClone",\
'../results/vforkClone.txt' using 1:12 with line title "vforkClone"
replot
