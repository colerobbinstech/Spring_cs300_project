package edu.cs300;

/**
 * Class used for ease when dealing with Results ABQ
 * One parameter is used for when a longest word is found
 * The other provides a default value for longest word
 */
public class SearchResult {

  boolean found;
  int passageID;
  String longestWord;

  //Longest word found
  public SearchResult(boolean found, int passageID, String longestWord){
    this.found = found;
    this.passageID = passageID;
    this.longestWord = longestWord;
  }

  public SearchResult(int passageID){
    this.found = false;
    this.passageID = passageID;
    this.longestWord = "----";
  }

}