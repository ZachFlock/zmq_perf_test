# ./publisher ipc:///tmp/pub_test.ipc 100000 4096 &
# ./subscriber ipc:///tmp/pub_test.ipc 100000 4096
# rm /tmp/pub_test.ipc


./publisher tcp://127.0.0.1:5001 100000 4096 hello no_broker &
./subscriber tcp://127.0.0.1:5001 100000 4096 hello
