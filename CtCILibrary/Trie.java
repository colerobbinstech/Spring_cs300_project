package CtCILibrary;

import java.util.ArrayList;


/* Implements a trie. We store the input list of words in tries so
 * that we can efficiently find words with a given prefix. 
 */ 
public class Trie
{
    // The root of this trie.
    private TrieNode root;

    /* Takes a list of strings as an argument, and constructs a trie that stores these strings. */
    public Trie(ArrayList<String> list) {
        root = new TrieNode();
        for (String word : list) {
            root.addWord(word);
        }
    }  
    

    /* Takes a list of strings as an argument, and constructs a trie that stores these strings. */    
    public Trie(String[] list) {
        root = new TrieNode();
        for (String word : list) {
            root.addWord(word);
        }
    }    

    /* Checks whether this trie contains a string with the prefix passed
     * in as argument.
     */
    public boolean contains(String prefix, boolean exact) {
        TrieNode lastNode = root;
        int i = 0;
        for (i = 0; i < prefix.length(); i++) {
            lastNode = lastNode.getChild(prefix.charAt(i));
            if (lastNode == null) {
                return false;	 
            }
        }
        return !exact || lastNode.terminates();
    }
    
    public boolean contains(String prefix) {
    	return contains(prefix, false);
    }
    
    public TrieNode getRoot() {
    	return root;
    }

    public String getLongestWord(String prefix) {
        TrieNode head = root;
        //Set head to last char of prefix
        for(int i = 0; i < prefix.length(); i++) {
            head = head.getChild(prefix.charAt(i));
            if(head == null) return "----";
        }
        //traverseAndAppend appends the current char so we start the string with the last char removed
        String longestWord = traverseAndAppend(prefix.substring(0, prefix.length()-1), head);
        return longestWord;
    }

    public String traverseAndAppend(String body, TrieNode cur) {
        //No more children to traverse down
        if(cur.getChildren().size() == 0) {
            return (body + cur.getChar());
        }
        String longestWord = body;
        //Loop through hashmap for each child
        for(int i = (int)'a'; i < (int)'z'; i++) {
            if(cur.getChild((char)i) != null) {
                String temp = traverseAndAppend(body + cur.getChar(), cur.getChild((char)i));
                if(temp.length() >= longestWord.length()) {
                    longestWord = temp;
                }
            }
        }        
        return longestWord;
    }
}
