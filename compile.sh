    g++ -O2 subscriber.cpp -o subscriber -I/opt/homebrew/include -L/opt/homebrew//lib -lzmq
    g++ -O2 publisher.cpp -o publisher -I/opt/homebrew/include -L/opt/homebrew//lib -lzmq
    g++ -O2 broker.cpp -o broker -I/opt/homebrew/include -L/opt/homebrew//lib -lzmq