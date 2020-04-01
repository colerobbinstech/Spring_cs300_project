package edu.cs300;
import CtCILibrary.*;
import java.util.concurrent.*;
import java.io.*;
import java.util.ArrayList;

/***
 * PassageProcessor Handles the majority of the Java overhead
 * It starts by reading passages.txt and putting each path into an array
 * Two ArrayBlockingQueues are created, one for communicating the prefix
 * between this program and each Worker, the other for reading the output 
 * The Workers (one for each passage) are initialized on their own thread 
 * and they build their tries
 * This program then reads each prefix from JNI one by one and puts them in the 
 * ABQ for each Worker. It terminates if a prefix is invalid or empty
 * 
 */
@SuppressWarnings("unchecked") //For casting an array of ABQ to a Generic Type
public class PassageProcessor {
    public static void main(String args[]) throws IOException {

        //Read passages from passages.txt into array
        ArrayList<String> passages = new ArrayList<String>();
        BufferedReader reader = new BufferedReader(new FileReader("passages.txt"));
        try{
            String currentLine = reader.readLine();
            while(currentLine != null) {
                passages.add(currentLine);
                currentLine = reader.readLine();
            }
            reader.close();
        }catch(IOException e) {
            e.printStackTrace();
        }

        //Create array of workers, their inputs, and their outputs
        int passageCount = passages.size();
        ArrayBlockingQueue<SearchRequest>[] prefixInputArray = (ArrayBlockingQueue<SearchRequest>[]) new ArrayBlockingQueue[passageCount];
        ArrayBlockingQueue<SearchResult> resultsOutputArray=new ArrayBlockingQueue<SearchResult>(passageCount*10);
        ArrayList<Worker> workerArray = new ArrayList<Worker>();



        //Initialize prefixInputArray array and start new threads for each Worker with its own passage
        for (int i = 0; i < passageCount ; i++) {
            prefixInputArray[i]=new ArrayBlockingQueue<SearchRequest>(10);
            workerArray.add(new Worker(passages.get(i), i, prefixInputArray[i], resultsOutputArray));
            workerArray.get(i).start();
         }
        
        //Begin the prefix process
        while(true) { //Needed? Yes unlimited number of requests
            try {
                SearchRequest req = MessageJNI.readPrefixRequestMsg();

                System.out.print("**prefix(" + req.requestID + ") " + req.prefix + " received\n");

                //Check for invalid prefix and exit conditions
                if(req.prefix.length() < 3 || req.prefix.length() > 20)
                    break;
                if(req.prefix.equals("   ")) {
                    SearchRequest stop = new SearchRequest(-1, "");
                    for(int i = 0; i < passageCount; i++) {
                        prefixInputArray[i].put(stop);
                        workerArray.get(i).join();
                    }
                    break;
                }

                //Add prefix to prefixInputArray
                for(int i = 0; i < passageCount; i++) {
                    prefixInputArray[i].put(req);
                }

                //Get results back from prefixInputArray
                for(int i = 0; i < passageCount; i++) {
                    SearchResult result = resultsOutputArray.take();
                    if(result.found) {
                        MessageJNI.writeLongestWordResponseMsg(req.requestID, req.prefix, 
                                result.passageID, passages.get(result.passageID), result.longestWord,
                                 passageCount, 1);
                    }
                    else {
                        MessageJNI.writeLongestWordResponseMsg(req.requestID, req.prefix, 
                            result.passageID, passages.get(result.passageID), result.longestWord, 
                            passageCount, 0);
                    }
                }
            } catch(InterruptedException e) {}
        }

        System.out.println("Terminating...");
        System.exit(0);
    }
}

