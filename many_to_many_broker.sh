rm *.out

./broker tcp://*:5001 tcp://*:5002 &

for j in {1..10}; do
    for i in {1..10}; do
        ./subscriber tcp://127.0.0.1:5002 100000 4096 hello_${j} >> subsciber_${j}_${i}.out &
    done
    ./publisher tcp://127.0.0.1:5001 100000 4096 hello_${j} broker &
done


wait