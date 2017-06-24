#!/bin/bash
python WikiExtractor.py --no-templates $1 -q -b 100M
FILES=text/AA//*
for f in $FILES
do
	sed -i -e 's/<[^>]*>//g' $f
	cat $f | tr '\n' '\r' | sed -e 's/<[^>]*>//g' | tr '\r' '\n' > tmp
	mv tmp $f
	sed -i '/__NOEDITSECTION__/d' $f
	echo "Processing $f file..."
done
