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
        for(int i = 0; i < prefix.length(); i++) {
            head = head.getChild(prefix.charAt(i));
            if(head == null) return "";
        }
        
        int length = 0, maxLength = 0;
        


        return prefix;
    }

    public String traverseAndAppend(String body, TrieNode cur) {
        
        for(TrieNode child : cur.getChildren().values()) {
            System.out.println(child);
        }        
        return body;
    }
}
