import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.*;
import java.util.*;

class client {
    public static void main(String[] args) throws InterruptedException {
        int inputPort = Integer.valueOf(args[0]);
        int clientNumber = Integer.valueOf(args[1]);
        try {
            Socket client = new Socket("localhost", inputPort);
            System.out.println("Connected!");
            OutputStream out = client.getOutputStream();
            for(int i = 0; i < 10; i++) {
                out.write(clientNumber);
                Thread.sleep(1000);
            }
            out.write(100);
            client.close();
            
        }
        catch(IOException ex) {
            System.out.println("Exception when running client: " + ex.getMessage());
        }
        
    
    }
}






