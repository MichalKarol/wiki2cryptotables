# wiki2cryptotables
C++ application and BASH script for generating list of occurrences of words and strings in text.

## Needed
[WikiExtractor](https://github.com/attardi/wikiextractor)  
[Wikimedia XML dumps](https://dumps.wikimedia.org)  
g++/Clang

## Getting ready
* Check accepted chars. Currently accepted is set for Polish language.
* Compile with -lpthread flag

## Usage
`./parse.sh wikimedia_xml_dump.xml.bz2`  
`./wiki2data inputFilePrefix numberOfFiles numberOfThreads numberOfCharacters`

## Results
chars_xx - parts of text xx characters long  
word_xx - words from text xx characters long

### Structure
word numberOfOccurences percent  
...  
sum of all occurences (some are not included in file due to cutoff in 0.0001%)

## Example
Exaple is for Polish Wikimedia dump  
news - wikinews  
wiki - wikipedia
