#include <pthread.h>
#include <iostream>
#include <string>
#include <bits/stdc++.h> // stringstream
#include <cctype> // isalpha | ispunct | isdigit

using namespace std;

string phrase;
bool bit[2] = {0, 0};

stringstream flag;
string word;

void *alpha(void *arg)
{
  while(flag)
  {
    if (isalpha(word[0]) || ispunct(word[0]))
    { // alphabets | symbols | punctuations
      cout << "alpha: " << word << endl;
      bit[1] = true; // enables the next word to be flagged
    }
    else
    {
      // block Alpha thread until Numeric thread encounters a word starting with an alphabet
      bit[0] = true;
      // after this thread wakes, it should not flag the next word as the other thread flags it
      bit[1] = false;
      if (!flag){
        break; // if numeric thread finishes flagging the phrase, then simply stop
      }
      while (bit[0]){
        continue; // if digit, then wait
      }
    }
    if (bit[1])
    { // only flag next word if last word was an alphabet
      flag >> word;
    }
  }

  bit[0] = true; // frees numeric thread
  pthread_exit(NULL);
  // return NULL;
}

void *numeric(void *arg)
{
  while(flag)
  {
    if (isdigit(word[0]))
    {
      cout << "numeric: " << word << endl;
      bit[1] = true; // enables the next word to be flagged
    }
    else
    {
      // block Numeric thread until Alpha thread encounters a word starting with a numeric
      bit[0] = false;
      // after this thread wakes, it should not flag the next word until the other thread flags it
      bit[1] = false;
      if (!flag){
        break; // if alpha thread finishes flagging the phrase, then simply stop
      }
      while (!bit[0]){
        continue; // if alphabet, then wait
      }
    }
    if (bit[1])
    { // only flag next word if last word was a numeric
      flag >> word;
    }
  }

  bit[0] = false; // frees alpha thread
  pthread_exit(NULL);
  // return NULL;
}

int main(int argc, char* argv[]) {
    // error checking for number of arguments
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s <message string>\n", argv[0]);
        exit(-1);
    }

     // flagging the phrase from terminal and storing in a global variable, phrase
    phrase = argv[1];
    // initialize parser to flag first word in phrase
    flag << phrase;
    flag >> word;

    // creating two concurrent threads (total 3 including parent thread)
    pthread_t Alpha, Numeric;
    pthread_create(&Alpha, NULL, alpha, NULL);
    pthread_create(&Numeric, NULL, numeric, NULL);

    pthread_join(Alpha, NULL);
    pthread_join(Numeric, NULL);

    return 0;
}
