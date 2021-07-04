#!/bin/bash

cd ../../build/
if [[ $1 == "" ]]; then
    make run-benchmarks >> ../test/benchmark/run.txt
#    ../../build/bin/biodynamo-benchmark >> run.txt
else
    for j in `seq 1 $1`;
    do
        make run-benchmarks >> ../test/benchmark/run.txt
#        ../../build/bin/biodynamo-benchmark >> run.txt
    done
fi

cd ../test/benchmark/
#Soma_Clusterung export=TRUE
printf "soma_clustering: export=true\n\n" >> runtime.txt
cat run.txt | awk '/SomaClustering1/ \
{print $2, $3, $4, $5, $6}' >> runtime.txt
printf "###################################\n\n" >> runtime.txt

#Soma_clustering export=FALSE
printf "soma_clustering: export=false\n\n" >> runtime.txt
cat run.txt | awk '/SomaClustering0/ \
{print $2, $3, $4, $5, $6}' >> runtime.txt
printf "###################################\n\n" >> runtime.txt

#Tumor_Concept export=FALSE
printf "tumor_concept: export=false\n\n" >> runtime.txt
cat run.txt | awk '/TumorConcept0/ \
{print $2, $3, $4, $5, $6}' >> runtime.txt
printf "###################################\n\n" >> runtime.txt

#Tumor_concept export=TRUE
printf "tumor_concept: export=true\n\n" >> runtime.txt
cat run.txt | awk '/TumorConcept1/ \
{print $2, $3, $4, $5, $6}' >> runtime.txt
printf "###################################\n\n" >> runtime.txt

rm run.txt

mkdir plots
./graph.py -a tumor_concept0
./graph.py -a tumor_concept1
./graph.py -a soma_clustering0
./graph.py -a soma_clustering1