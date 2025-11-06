rm *.out

#./broker tcp://*:5001 tcp://*:5002 &

./broker ipc:///tmp/in ipc:///tmp/out & 

for j in {1..10}; do
    for i in {1..10}; do
        ./subscriber ipc:///tmp/out 1000 1024 hello_${j} >> subsciber_${j}_${i}.out &
#        ./subscriber tcp://127.0.0.1:5002 1000 128 hello_${j} >> subsciber_${j}_${i}.out &
    done
    ./publisher ipc:///tmp/in 1000 1024 hello_${j} broker &
#    ./publisher tcp://127.0.0.1:5001 1000 128 hello_${j} broker &
done


wait