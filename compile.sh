    g++ subscriber.cpp -o subscriber -I/opt/homebrew/include -L/opt/homebrew//lib -lzmq
    g++ publisher.cpp -o publisher -I/opt/homebrew/include -L/opt/homebrew//lib -lzmq
    g++ broker.cpp -o broker -I/opt/homebrew/include -L/opt/homebrew//lib -lzmq