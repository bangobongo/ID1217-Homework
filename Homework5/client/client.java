import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.*;
import java.util.*;

class client {
    public static void main(String[] args) throws InterruptedException {
        int inputPort = Integer.valueOf(args[0]);
        int clientNumber = Integer.valueOf(args[1]);

        int timesToEat = 10;

        try {
            Socket client = new Socket("localhost", inputPort);
            System.out.println("Connected!");

            OutputStream out = client.getOutputStream();

            Random rand = new Random();

            for(int i = 0; i < timesToEat; i++) {
                out.write(clientNumber);
                Thread.sleep(rand.nextInt(1000, 3000));
            }
            out.write(100);
            client.close();
            
        }
        catch(IOException ex) {
            System.out.println("Exception when running client: " + ex.getMessage());
        }
        
    
    }
}






