package Homework5;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.*;
import java.util.*;

class client {
    public static void main(String[] args) {
        try {
            int inputPort = Integer.valueOf(args[0]);
            Socket client = new Socket("localhost", inputPort);

            System.out.println("Connected!");
        
            OutputStream out = client.getOutputStream();
            out.write(65);
        
            InputStream in = client.getInputStream();
            int b = in.read();
        
            System.out.println("Received: " + b);
        }
        catch(IOException ex) {
            System.out.println("Fuck off: " + ex.getMessage());
        }
        
    
    }
}






