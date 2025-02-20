import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.*;

public class server {
    public static void main(String[] args) {
        int inputPort = Integer.valueOf(args[0]);
        ServerSocket server;
        try {
            server = new ServerSocket(inputPort);
        } catch(IOException ex) {
            System.out.println("Exception thrown when creating server socket: " + ex.getMessage());
            return;
        }

        boolean serverRunning = true;

        while(serverRunning) {
            try {
                Socket clientSocket = server.accept();

                Worker worker = new Worker(clientSocket);
                Thread thr = new Thread(worker);
                thr.start();
                System.out.println("Finished!");
            } catch(IOException ex) {
                System.out.println("Exception when running server: " + ex.getMessage());
            }
        }
    
    }

    
}
