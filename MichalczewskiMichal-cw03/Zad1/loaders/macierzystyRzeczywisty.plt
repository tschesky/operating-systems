set grid
set term png
set output "../plots/macierzystyRzeczywisty.png"
set terminal png size 1024,768
set title "Proces macierzysty: czas rzeczywisty"
plot '../results/fork.txt' using 1:4 with line title "fork",\
'../results/vfork.txt' using 1:4 with line title "vfork",\
'../results/forkClone.txt' using 1:4 with line title "forkClone",\
'../results/vforkClone.txt' using 1:4 with line title "vforkClone"
replot
