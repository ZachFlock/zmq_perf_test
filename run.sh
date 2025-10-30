# ./publisher ipc:///tmp/pub_test.ipc 10000 4096 &
# ./subscriber ipc:///tmp/pub_test.ipc 10000 4096
# rm /tmp/pub_test.ipc

./publisher tcp://127.0.0.1:5001 10000 4096 &
./subscriber tcp://127.0.0.1:5001 10000 4096
