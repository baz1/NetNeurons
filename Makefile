QDOC=qdoc

all: doc

doc: MLP/src/* config.qdocconf
	$(QDOC) config.qdocconf

clean:
	rm -r doc
