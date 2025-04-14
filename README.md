### Byte pair encoding algorithm

The algorithm compresses text by initially replacing the highest frequency pair of characters with a generated token. Through every iteration the token pair with the highest frequency gets replaced with another generated token. This goes on in a loop until the most frequent pair of tokens remain to occure onyl one time.

A look-up table of the replacements is required to rebuild the initial dataset.

#### Quick start

`git clone git@github.com:toxypiks/byte_pair_encoding.git` <br/>
`cd byte_pair_encoding`<br/>
`mkdir build`<br/>
`cd build`<br/>
`cmake ..`<br/>
`make`<br/>

#### Generating the dot graph

`./main > pair.dot`<br/>
`dot -Tpng pair.dot -o pair.png`<br/>

![Alt text](./dot_graph/pair.png)


#### How the code works

`Freq *freq` -> Hashmap for tokens (keys -> Pair {.l, .r}) and their occurence (value)

`uint32_t *tokens_in` -> text before next iteration of compression

`uint32_t *tokens_out` -> compressed text

`Pair *pairs = NULL;` -> look-up array whereas indexes 1-255 are the same as the .l value and the .r value equals to zero indicating that the index value corresponds to the ASCII value of the character. All newly generated tokens get appended representing the token with the highest occurcence within each loop iteration

1. Look-up table gets initialized with values from 1-255 for .l

2. Text to compress goes in tokens_in array

3. Tokens_in gets iterated pair-wise and pairs are put into freq hashmap if not already existent, if they are existent the value gets incremented representing the occurence of pair

4. Freq gets iterated to search for index of pair with max occurence, if that pair occures only one time the loop ends -> the algorithm is finished

5. The pair with the highest occurence gets appended to the pairs look-up array

6. Tokens_in gets iterated to find pair with max occurence and replace it with last index of pairs look-up array in tokens_out, if current pair has no max occurce its not getting replaced and just put into tokens_out

7. Tokens_in and tokens_out get swapped for next iteration
