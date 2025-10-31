rm *.out

for j in {1..10}; do
    for i in {1..10}; do
        ./subscriber tcp://127.0.0.1:500${j} 100000 4096 hello_${j} >"subscriber_${j}_${i}.out" &
        #./subscriber tcp://127.0.0.1:500${j} 1000 4096 hello_${j} &
    done
    ./publisher tcp://127.0.0.1:500${j} 100000 4096 hello_${j} no_broker &
done


wait