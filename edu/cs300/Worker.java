package edu.cs300;
import CtCILibrary.*;
import java.util.concurrent.*;
import java.util.ArrayList;
import java.util.Scanner;
import java.io.File;
import java.io.FileNotFoundException;

/**
 * Worker class runs on its own thread and deals with one passage
 * It is constructed with a path to a file and starts by constructing
 * a trie of the words in that file that contain no punctuation
 * When the run section of the program is started the Worker starts
 * by taking a prefix from the Request ABQ and checks the trie for the 
 * longest word.
 * Based on if one is found, a SearchResult object is created and passed
 * in the Result ABQ to be read by the PassageProcessor
 */
class Worker extends Thread{

  Trie textTrieTree;
  ArrayBlockingQueue<SearchRequest> prefixRequestArray;
  ArrayBlockingQueue<SearchResult> resultsOutputArray;
  int id;
  String passageName;

  public Worker(String passageName,int id,ArrayBlockingQueue<SearchRequest> prefixABQ, ArrayBlockingQueue<SearchResult> results){

    //Read passage and strip words into ArrayList
    ArrayList<String> words = new ArrayList<>();
    File passage = new File(passageName);
    Scanner reader;
    try{
      reader = new Scanner(passage);
      //Sets delimiter to be any character not A-z or ' or -, the last two taken out fully
      reader.useDelimiter("[^A-Za-z'\\-]");
      while(reader.hasNext()) {
        String token = reader.next();
        token = token.toLowerCase();
        if(!(token.contains("-") || token.contains("\'"))){
          words.add(token);
        }
      }
      reader.close();
    } catch (FileNotFoundException e) {}


    //Assign values
    this.textTrieTree=new Trie(words);
    this.prefixRequestArray=prefixABQ;
    this.resultsOutputArray=results;
    this.id=id;
    this.passageName=passageName;
  }

  public void run() {
    System.out.println("Worker-"+this.id+" ("+this.passageName+") thread started ...");
    while (true){
      try {
        SearchRequest req = this.prefixRequestArray.take();
        String prefix = req.prefix;
        String longestWord = this.textTrieTree.getLongestWord(prefix);
        
        if (longestWord.equals("")){
          System.out.println("Worker-"+this.id+" "+req.requestID+":"+ prefix+" ==> not found ");
          resultsOutputArray.put(new SearchResult(this.id));
        } else{
          System.out.println("Worker-"+this.id+" "+req.requestID+":"+ prefix+" ==> " + longestWord);
          resultsOutputArray.put(new SearchResult(true, this.id, longestWord));
        }
      } catch(InterruptedException e){
        System.out.println(e.getMessage());
      }
    }
  }

}
