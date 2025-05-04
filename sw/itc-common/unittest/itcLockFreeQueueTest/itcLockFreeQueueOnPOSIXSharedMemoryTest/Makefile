BIN=./bin



all: $(BIN)/receiver $(BIN)/sender

$(BIN)/receiver: receiver.cc
	@mkdir -p $(BIN)
	g++ -I../../../inc -I../../../../itc-api/if -o $@ $^

$(BIN)/sender: sender.cc
	@mkdir -p $(BIN)
	g++ -I../../../inc -I../../../../itc-api/if -o $@ $^

receiver:
	$(BIN)/receiver

sender:
	$(BIN)/sender
	
clean:
	rm -rf $(BIN)/*
	